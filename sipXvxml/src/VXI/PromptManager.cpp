/****************License************************************************
 *
 * Copyright 2000-2001.  SpeechWorks International, Inc.    
 *
 * Use of this software is subject to notices and obligations set forth
 * in the SpeechWorks Public License - Software Version 1.1 which is
 * included with this software.
 *
 * SpeechWorks is a registered trademark, and SpeechWorks Here,
 * DialogModules and the SpeechWorks logo are trademarks of SpeechWorks
 * International, Inc. in the United States and other countries.
 * 
 ***********************************************************************/

#include "PromptManager.hpp"
#include "SimpleLogger.hpp"            // for SimpleLogger
#include "VXIinet.h"                   // for VXIinetInterface
#include "VXIprompt.h"                 // for VXIpromptInterface
#include "VXML.h"                      // for node names
#include "DocumentModel.hpp"           // for VXMLNode, VXMLElement
#include "CommonExceptions.hpp"        // for InterpreterEvent, OutOfMemory
#include "PropertyList.hpp"            // for PropertyList
#include <sstream>

VXIchar SPACE = ' ';

//#############################################################################

bool static ConvertValueToString(const VXIValue * value, vxistring & source)
{
  if (value == NULL) return false;

  switch (VXIValueGetType(value)) {
  case VALUE_INTEGER:
  {
    std::basic_stringstream<VXIchar> attrStream;
    attrStream << VXIIntegerValue(reinterpret_cast<const VXIInteger *>(value));
    source = attrStream.str();
    return true;
  }
  case VALUE_FLOAT:
  {
    std::basic_stringstream<VXIchar> attrStream;
    attrStream << VXIFloatValue(reinterpret_cast<const VXIFloat *>(value));
    source = attrStream.str();
    return true;
  }
  case VALUE_STRING:
    source = VXIStringCStr(reinterpret_cast<const VXIString *>(value));
    return true;
  case VALUE_VECTOR:
  case VALUE_MAP:
  case VALUE_PTR:
  case VALUE_CONTENT:
  default:
    return false;
  }
}


inline void AddSSMLAttribute(const VXMLElement & elem, VXMLAttributeType attr,
                             const VXIchar * const name, vxistring & sofar)
{
  vxistring attrString;
  if (!elem.GetAttribute(attr, attrString)) return;

  sofar += SPACE;
  sofar += name;
  sofar += L"=\"";

  // We need to escape three characters: (",<,&) -> (&quot;, &lt;, &amp;)
  vxistring::size_type pos = 0;
  while ((pos = attrString.find_first_of(L"\"<&", pos)) != vxistring::npos) {
    switch (attrString[pos]) {
    case '\"':   attrString.replace(pos, 1, L"&quot;");    ++pos;  break;
    case '<':    attrString.replace(pos, 1, L"&lt;");      ++pos;  break;
    case '&':    attrString.replace(pos, 1, L"&amp;");     ++pos;  break;
    }
  };

  sofar += attrString;
  sofar += L"\"";
}


inline void AppendTextSegment(vxistring & sofar,
                              const vxistring & addition,
                              bool isSSML)
{
  if (addition.empty()) return;

  vxistring::size_type pos = sofar.length();

  if (!sofar.empty()) sofar += SPACE;
  sofar += addition;

  if (isSSML) {
    // We need to escape two characters: (<,&) -> (&lt;, &amp;)
    while ((pos = sofar.find_first_of(L"<&", pos)) != vxistring::npos) {
      switch (sofar[pos]) {
      case '<':    sofar.replace(pos, 1, L"&lt;");      ++pos;  break;
      case '&':    sofar.replace(pos, 1, L"&amp;");     ++pos;  break;
      }
    }
  }
}


inline void ConvertToXML(vxistring & sofar, bool & isSSML)
{
  if (isSSML) return;

  vxistring temp;
  AppendTextSegment(temp, sofar, true);
  sofar = temp;
  isSSML = true;
}


//#############################################################################

////////
// About prompts:
//
// Prompts may be defined in two places within VXML documents.
// 1) form items - menu, field, initial, record, transfer, subdialog, object
// 2) executable content - block, catch, filled
//
// The <enumerate> element only is valid with parent menu or fields having one
// or more option elements.  Otherwise an error.semantic results.
//
// From the VXML working drafts, barge-in may be controlled on a prompt by
// prompt basis.  There are several unresolved issues with this request.
// Instead this implements a different tactic.


bool PromptManager::ConnectResources(SimpleLogger * l, VXIpromptInterface * p)
{
  if (l == NULL || p == NULL) return false;
  log = l;
  prompt = p;
  enabledSegmentInQueue = false;
  timeout = -1;
  return true;
}


void PromptManager::PlayFiller(const PropertyList &propertylist, 
                               const vxistring & src)
{
  log->LogDiagnostic(2, L"PromptManager::PlayFiller()");

  if (src.empty()) return;

  VXIMap *properties = VXIMapCreate();

  const VXIchar * base = propertylist.GetProperty(PropertyList::BaseURI);
  if (base != NULL)
	  VXIMapSetProperty( properties, INET_URL_BASE, ( VXIValue * ) VXIStringCreate( base ) );

  prompt->PlayFiller(prompt, NULL, src.c_str(), NULL, properties, 0);

  VXIMapDestroy( &properties );
}


void PromptManager::Play()
{
  log->LogDiagnostic(2, L"PromptManager::Play()");

  VXIpromptResult temp;
  prompt->Wait(prompt, &temp);  // Wait for previous prompts to end
  prompt->Play(prompt);         // Start new ones
  enabledSegmentInQueue = false;
  timeout = -1;
}


void PromptManager::Stop()
{
  log->LogDiagnostic(2, L"PromptManager::Stop()");

  prompt->Stop(prompt);
  enabledSegmentInQueue = false;
  timeout = -1;
}


void PromptManager::PlayAll()
{
  log->LogDiagnostic(2, L"PromptManager::PlayAll()");

  VXIpromptResult temp;
  prompt->Wait(prompt, &temp);  // Wait for previous prompts to end
  prompt->Play(prompt);         // Start new ones
  prompt->Wait(prompt, &temp);  // Wait for them to end too!
  enabledSegmentInQueue = false;
  timeout = -1;
}


int PromptManager::GetMillisecondTimeout() const
{
  return timeout;
}


void PromptManager::Queue(const VXMLNode & node, const VXMLElement & ref,
                          const PropertyList & propertyList,
                          PromptTranslator & translator)
{
  log->LogDiagnostic(2, L"PromptManager::Queue()");

  // (1) Find <field> or <menu> associated with this prompt, if any.  This is
  // used by the <enumerate> element.
  VXMLElement item = ref;
  while (item != 0) {
    VXMLElementType type = item.GetName();
    if (type == NODE_FIELD || type == NODE_MENU) break;
    item = item.GetParent();
  }
   
  // (2) Get properties.
  VXIMapHolder properties;
  if (properties.GetValue() == NULL) throw VXIException::OutOfMemory();

  // (2.1) Flatten the property list.
  propertyList.GetProperties(properties);

  // (2.2) Add the URI information.
  propertyList.GetFetchobjBase(properties);

  // (3) Handle <prompt> elements
  vxistring language;
  vxistring sofar;
  bool isSSML = false;

  PromptManager::BargeIn bargein = PromptManager::ENABLED;
  const VXIchar * j = propertyList.GetProperty(PROP_BARGEIN);
  if (j != NULL && vxistring(L"false") == j)
    bargein = PromptManager::DISABLED;

  if (node.GetType() == VXMLNode::Type_VXMLElement &&
      reinterpret_cast<const VXMLElement &>(node).GetName() == NODE_PROMPT)
  {
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(node);

    // (3.1) Update timeout if specified.
    vxistring timeoutString;
    if (elem.GetAttribute(ATTRIBUTE_TIMEOUT, timeoutString) == true)
      PropertyList::ConvertTimeToMilliseconds(*log, timeoutString, timeout);

    // (3.2) Get bargein setting.
    vxistring bargeinAttr;
    if (elem.GetAttribute(ATTRIBUTE_BARGEIN, bargeinAttr) == true) {
      bargein = PromptManager::ENABLED;
      if (bargeinAttr == L"false") bargein = PromptManager::DISABLED;
    }

    // (3.3) Update language.
    const VXIchar * j = propertyList.GetProperty(PropertyList::Language);
    if (j != NULL) 
    {
      language = j;
      AddParamValue(properties, PROMPT_LANGUAGE, j);
    }
    else
    {
      // Search upwards starting with element for a language vxml tag
      VXMLElement check = item ;
      while (check != 0)
      {    	
        if (check.GetAttribute(ATTRIBUTE_XMLLANG, language) == true)
        {
          AddParamValue(properties, PROMPT_LANGUAGE, language);
          break ;
        }
    	check = check.GetParent() ;
      }
    }

    // (3.4) Recursively handle contents of the prompt.
    for (VXMLNodeIterator it(elem); it; ++it)
      ProcessSegments(*it, item, propertyList, translator,
                      bargein, properties, sofar, isSSML);
  }

  // (4) Otherwise, this is simple content or an audio / tts element.
  else
  {      
    const VXIchar * j = propertyList.GetProperty(PropertyList::Language);
    if (j != NULL) 
    {
      language = j;
      AddParamValue(properties, PROMPT_LANGUAGE, j);
	}
	else
	{
	  // Search upwards starting with element for a language vxml tag
	  VXMLElement check = item ;
	  while (check != 0)
	  {    	
	    if (check.GetAttribute(ATTRIBUTE_XMLLANG, language) == true)
	    {
          AddParamValue(properties, PROMPT_LANGUAGE, language);
          break ;
        }
        check = check.GetParent() ;
      }
    }	  
	  
    ProcessSegments(node, item, propertyList, translator,
                    bargein, properties, sofar, isSSML);
  }

  // (5) Add a new segment.
  if (!sofar.empty()) {
    if (!isSSML) AddSegment(SEGMENT_TEXT, sofar, properties, bargein);
    else {
      // Retrieve language from properties, if not specified on <prompt>        
      if (language.empty()) {
        const VXIchar * j = propertyList.GetProperty(PropertyList::Language);
        if (j != NULL) {
          language = j;
          AddParamValue(properties, PROMPT_LANGUAGE, j);
        }
      }
    
      vxistring prompt(L"<?xml version='1.0'?>");
      if (!language.empty()) {
        prompt += L"<speak xml:lang='";
        prompt += language;
        prompt += L"'>";
      } else 
        prompt += L"<speak>";

      prompt += sofar;
      prompt += L"</speak>";

      AddSegment(SEGMENT_SSML, prompt, properties, bargein);
    }
  }
}


void PromptManager::Queue(const vxistring & uri)
{
  if (uri.empty()) return;

  VXIMapHolder tmpVXIMapHolder;
  AddSegment(PromptManager::SEGMENT_AUDIO, uri, tmpVXIMapHolder,
             PromptManager::UNSPECIFIED);
}


bool PromptManager::AddSegment(PromptManager::SegmentType type,
                               const vxistring & data,
                               const VXIMapHolder & properties,
                               PromptManager::BargeIn bargein,
                               bool throwExceptions)
{
  const VXIMap * propMap = properties.GetValue();

  VXIpromptResult result = VXIprompt_RESULT_SUCCESS;

  // Handle the easy case.
  switch (type) {
  case PromptManager::SEGMENT_AUDIO:
    result = prompt->Queue(prompt, NULL, data.c_str(), NULL, propMap, bargein);
    break;
  case PromptManager::SEGMENT_SSML:
    result = prompt->Queue(prompt, VXI_MIME_SSML, NULL, data.c_str(), propMap, bargein);
    break;
  case PromptManager::SEGMENT_TEXT:
    result = prompt->Queue(prompt, VXI_MIME_UNICODE_TEXT, NULL, data.c_str(),
                           propMap, bargein);
    break;
  }

  if (throwExceptions) {
    switch (result) {
    case VXIprompt_RESULT_SUCCESS:
      break;
    case VXIprompt_RESULT_TTS_BAD_LANGUAGE:
      throw VXIException::InterpreterEvent(EV_UNSUPPORT_LANGUAGE);
    case VXIprompt_RESULT_FETCH_TIMEOUT:
    case VXIprompt_RESULT_FETCH_ERROR:
    case VXIprompt_RESULT_NON_FATAL_ERROR:
      throw VXIException::InterpreterEvent(EV_ERROR_BADFETCH);
    case VXIprompt_RESULT_HW_BAD_TYPE:
      throw VXIException::InterpreterEvent(EV_UNSUPPORT_FORMAT);
    case VXIprompt_RESULT_BAD_SAYAS_CLASS:
    case VXIprompt_RESULT_TTS_ACCESS_ERROR:
    case VXIprompt_RESULT_TTS_BAD_DOCUMENT:
    case VXIprompt_RESULT_TTS_SYNTAX_ERROR:
    case VXIprompt_RESULT_TTS_ERROR:
    default:
      throw VXIException::InterpreterEvent(EV_ERROR_SEMANTIC);
    }
  }

  switch (bargein) {
  case PromptManager::DISABLED:
    if (!enabledSegmentInQueue) prompt->Play(prompt);
    break;
  case PromptManager::ENABLED:
    enabledSegmentInQueue = true;
    break;
  case PromptManager::UNSPECIFIED:
    break;
  }

  return result == VXIprompt_RESULT_SUCCESS;
}


void PromptManager::ProcessSegments(const VXMLNode & node,
                                    const VXMLElement & item,
                                    const PropertyList & propertyList,
                                    PromptTranslator & translator,
                                    PromptManager::BargeIn bargein,
                                    VXIMapHolder & props,
                                    vxistring & sofar,
                                    bool & isSSML)
{
  // (1) Handle bare content.
  switch (node.GetType()) {
  case VXMLNode::Type_VXMLContent:
  {
    const VXMLContent & content = reinterpret_cast<const VXMLContent &>(node);
    AppendTextSegment(sofar, content.GetValue(), isSSML);
    return;
  }
  case VXMLNode::Type_VXMLElement:
    break;
  default: // This should never happen.
    log->LogError(999, SimpleLogger::MESSAGE,
                  L"unexpected type in PromptManager::ProcessSegments");
    throw VXIException::Fatal();
  }

  // (2) Retrieve fetch properties (if we haven't already)
  const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(node);
  vxistring attr;

  switch (elem.GetName()) {

  // (3) audio
  case NODE_AUDIO:
  {
    // (3.1) Determine the source of the audio.
    vxistring source;

    // (3.1.1) src has priority over expr.
    elem.GetAttribute(ATTRIBUTE_SRC, source);
    if (source.empty()) {
      elem.GetAttribute(ATTRIBUTE_EXPR, source);

      VXIValue * value = translator.EvaluateExpression(source);
      if (value != NULL) {
        // (3.1.2) Handle audio content from <record> elements.
        if (VXIValueGetType(value) == VALUE_CONTENT) {
          AddContent(elem, item, propertyList, translator, bargein, props,
                     sofar, value, isSSML);
          return;
        }

        // (3.1.3) Otherwise, try to convert the type into a string.
        bool conversionFailed = !ConvertValueToString(value, source);
        VXIValueDestroy(&value);

        if (conversionFailed) {
          log->LogDiagnostic(0, L"PromptManager::ProcessSegments - "
                             L"audio src evaluation failed");
          throw VXIException::InterpreterEvent(EV_ERROR_SEMANTIC,
                                               L"audio src evaluation failed");
        }
      }
    }

    // (3.1.4) Empty audio elements should be ignored.
    if (source.empty()) return;

    // (3.2) Get fetch properties.

    // (3.2.1) Create new audio reference data map.
    VXIMapHolder audioData;
    if (audioData.GetValue() == NULL) throw VXIException::OutOfMemory();

    // (3.2.2) Populate the inner map.
    AddParamValue(audioData, PROMPT_AUDIO_REF_SRC, source);
//    propertyList.GetFetchobjCacheAttrs(elem, PropertyList::Grammar, audioData);
    propertyList.GetFetchobjCacheAttrs(elem, PropertyList::Audio, audioData);

    // (3.2.3) Generate a hidden name and place the inner map inside the
    // properties.
    vxistring hiddenName;
    VXMLDocumentModel::CreateHiddenVariable(hiddenName);

    VXIMapHolder allRefs(NULL);

    // Get existing map (if it exists) or create a new one.
    const VXIValue * temp = VXIMapGetProperty(props.GetValue(),
                                              PROMPT_AUDIO_REFS);
    if (temp != NULL && VXIValueGetType(temp) == VALUE_MAP)
      allRefs.Acquire(VXIMapClone(reinterpret_cast<const VXIMap *>(temp)));
    else
      allRefs.Acquire(VXIMapCreate());
    if (allRefs.GetValue() == NULL) throw VXIException::OutOfMemory();

    // Place references for this value into the 'all refs' map.
    VXIMapSetProperty(allRefs.GetValue(), hiddenName.c_str(),
                      reinterpret_cast<VXIValue *>(audioData.Release()));

    // And replace the 'all refs' map in the properties.
    VXIMapSetProperty(props.GetValue(), PROMPT_AUDIO_REFS,
                      reinterpret_cast<VXIValue *>(allRefs.Release()));

    // (3.2.4) Create fake 'src' for this audio element.
    hiddenName = vxistring(PROMPT_AUDIO_REFS_SCHEME) + hiddenName;

    // (3.3) For SSML, simply add a new audio element.
    if (isSSML) {
      sofar += L"<audio src=\"";

      // We need to escape three characters: (",<,&) -> (&quot;, &lt;, &amp;)
      vxistring::size_type pos = 0;
      while ((pos = hiddenName.find_first_of(L"\"<&", pos)) != vxistring::npos)
      {
        switch (hiddenName[pos]) {
        case '\"':   hiddenName.replace(pos, 1, L"&quot;");    ++pos;  break;
        case '<':    hiddenName.replace(pos, 1, L"&lt;");      ++pos;  break;
        case '&':    hiddenName.replace(pos, 1, L"&amp;");     ++pos;  break;
        }
      };

      sofar += hiddenName;
      sofar += L"\"";

      // Deal with alternate text.
      if (elem.hasChildren()) {
        sofar += L">";
        for (VXMLNodeIterator it(elem); it; ++it)
          ProcessSegments(*it, item, propertyList, translator,
                          bargein, props, sofar, isSSML);
        sofar += L"</audio>";
      }
      else
        sofar += L"/>";
      return;
    }

    // (3.4) Handle simple data case.

    // (3.4.1) First add any existing text.
    if (!sofar.empty()) {
      AddSegment(SEGMENT_TEXT, sofar, props, bargein);
      sofar.erase();
    }

    // (3.4.2) Then add the audio segment.

    if (!AddSegment(SEGMENT_AUDIO, hiddenName, props, bargein,
                    !elem.hasChildren()))
    {
      // Unable to queue audio segment.  Resort to backup.
      for (VXMLNodeIterator it(elem); it; ++it)
        ProcessSegments(*it, item, propertyList, translator,
                        bargein, props, sofar, isSSML);
    }

    return;
  }

  // (4) Handle SSML tags.

  case NODE_BREAK:
    ConvertToXML(sofar, isSSML);
    sofar += L"<break";
    AddSSMLAttribute(elem, ATTRIBUTE_SIZE, L"size", sofar);
    AddSSMLAttribute(elem, ATTRIBUTE_TIME, L"time", sofar);
    sofar += L"/>";
    return;

  case NODE_EMPHASIS:
    ConvertToXML(sofar, isSSML);
    sofar += L"<emphasis";
    AddSSMLAttribute(elem, ATTRIBUTE_LEVEL, L"level", sofar);
    if (elem.hasChildren()) {
      sofar += L">";
      for (VXMLNodeIterator it(elem); it; ++it)
        ProcessSegments(*it, item, propertyList, translator,
                        bargein, props, sofar, isSSML);
      sofar += L"</emphasis>";
    }
    else
      sofar += L"/>";
    return;

  case NODE_MARK:
    ConvertToXML(sofar, isSSML);
    sofar += L"<mark";
    AddSSMLAttribute(elem, ATTRIBUTE_NAME, L"name", sofar);
    sofar += L"/>";
    return;

  case NODE_PARAGRAPH:
    ConvertToXML(sofar, isSSML);
    sofar += L"<p";
    AddSSMLAttribute(elem, ATTRIBUTE_XMLLANG, L"xml:lang", sofar);
    if (elem.hasChildren()) {
      sofar += L">";
      for (VXMLNodeIterator it(elem); it; ++it)
        ProcessSegments(*it, item, propertyList, translator,
                        bargein, props, sofar, isSSML);
      sofar += L"</p>";
    }
    else
      sofar += L"/>";
    return;

  case NODE_PHONEME:
    ConvertToXML(sofar, isSSML);
    sofar += L"<phoneme";
    AddSSMLAttribute(elem, ATTRIBUTE_PH, L"ph", sofar);
    AddSSMLAttribute(elem, ATTRIBUTE_ALPHABET, L"alphabet", sofar);
    if (elem.hasChildren()) {
      sofar += L">";
      for (VXMLNodeIterator it(elem); it; ++it)
        ProcessSegments(*it, item, propertyList, translator,
                        bargein, props, sofar, isSSML);
      sofar += L"</phoneme>";
    }
    else
      sofar += L"/>";
    return;

  case NODE_PROSODY:
    ConvertToXML(sofar, isSSML);
    sofar += L"<prosody";
    AddSSMLAttribute(elem, ATTRIBUTE_PITCH, L"pitch", sofar);
    AddSSMLAttribute(elem, ATTRIBUTE_CONTOUR, L"contour", sofar);
    AddSSMLAttribute(elem, ATTRIBUTE_RANGE, L"range", sofar);
    AddSSMLAttribute(elem, ATTRIBUTE_RATE, L"rate", sofar);
    AddSSMLAttribute(elem, ATTRIBUTE_DURATION, L"duration", sofar);
    AddSSMLAttribute(elem, ATTRIBUTE_VOLUME, L"volume", sofar);
    if (elem.hasChildren()) {
      sofar += L">";
      for (VXMLNodeIterator it(elem); it; ++it)
        ProcessSegments(*it, item, propertyList, translator,
                        bargein, props, sofar, isSSML);
      sofar += L"</prosody>";
    }
    else
      sofar += L"/>";
    return;

  case NODE_SAYAS:
    ConvertToXML(sofar, isSSML);
    sofar += L"<say-as";
    AddSSMLAttribute(elem, ATTRIBUTE_TYPE, L"type", sofar);
    AddSSMLAttribute(elem, ATTRIBUTE_SUB, L"sub", sofar);
    if (elem.hasChildren()) {
      sofar += L">";
      for (VXMLNodeIterator it(elem); it; ++it)
        ProcessSegments(*it, item, propertyList, translator,
                        bargein, props, sofar, isSSML);
      sofar += L"</say-as>";
    }
    else
      sofar += L"/>";
    return;

  case NODE_SENTENCE:
    ConvertToXML(sofar, isSSML);
    sofar += L"<s";
    AddSSMLAttribute(elem, ATTRIBUTE_XMLLANG, L"xml:lang", sofar);
    if (elem.hasChildren()) {
      sofar += L">";
      for (VXMLNodeIterator it(elem); it; ++it)
        ProcessSegments(*it, item, propertyList, translator,
                        bargein, props, sofar, isSSML);
      sofar += L"</s>";
    }
    else
      sofar += L"/>";
    return;

  case NODE_VOICE:
    ConvertToXML(sofar, isSSML);
    sofar += L"<voice";
    AddSSMLAttribute(elem, ATTRIBUTE_GENDER, L"gender", sofar);
    AddSSMLAttribute(elem, ATTRIBUTE_AGE, L"age", sofar);
    AddSSMLAttribute(elem, ATTRIBUTE_CATEGORY, L"category", sofar);
    AddSSMLAttribute(elem, ATTRIBUTE_VARIANT, L"variant", sofar);
    AddSSMLAttribute(elem, ATTRIBUTE_NAME, L"name", sofar);
    if (elem.hasChildren()) {
      sofar += L">";
      for (VXMLNodeIterator it(elem); it; ++it)
        ProcessSegments(*it, item, propertyList, translator,
                        bargein, props, sofar, isSSML);
      sofar += L"</voice>";
    }
    else
      sofar += L"/>";
    return;

  // (5) <value>
  case NODE_VALUE:
  {
    // (5.1) Evaluate the expression.  Can we handle this type?
    vxistring expr;
    if (elem.GetAttribute(ATTRIBUTE_EXPR, expr) == false || expr.empty())
      return;

    VXIValue * value = translator.EvaluateExpression(expr);
    if (value == NULL) return;

    VXIvalueType valueType = VXIValueGetType(value);
    switch (valueType) {
    case VALUE_INTEGER:
    case VALUE_FLOAT:
    case VALUE_STRING:
    case VALUE_CONTENT:
      break;
    case VALUE_VECTOR:
    case VALUE_MAP:
    case VALUE_PTR:
    default:
      VXIValueDestroy(&value);
      log->LogDiagnostic(0, L"PromptManager::ProcessSegments - "
                         L"value expr was not a simple type");
      throw VXIException::InterpreterEvent(EV_ERROR_SEMANTIC,
                                           L"value expr gave invalid type");
    }

    // (5.2) Handle VXIContent.
    if (valueType == VALUE_CONTENT) {
      AddContent(elem, item, propertyList, translator, bargein, props,
                 sofar, value, isSSML);
      return;
    }

    // (5.3) OTHERWISE this should be played as TTS. Convert value to string.
    ConvertValueToString(value, expr);
    VXIValueDestroy(&value);

    vxistring type;
    elem.GetAttribute(ATTRIBUTE_TYPE, type);

    // (5.3.1) Is this simple text?
    vxistring::size_type pos = expr.find('<');
    if (type.empty() && 
        (pos == vxistring::npos || expr.find('>', pos) == vxistring::npos))
    {
      AppendTextSegment(sofar, expr, isSSML);
      return;
    }

    // (5.3.2) Otherwise....
    ConvertToXML(sofar, isSSML);

    if (!type.empty()) {
      sofar += L"<say-as type=\"";
      sofar += type;
      sofar += L"\">";
    }

    sofar += expr;

    if (!type.empty()) {
      sofar += L"</say-as>";
    }

    return;
  }

  // (3.4) <enumerate>
  case NODE_ENUMERATE:
  {
    // (3.4.1) Is enumerate valid in this location?
    if (item == 0)
      throw VXIException::InterpreterEvent(EV_ERROR_SEMANTIC, L"invalid use "
                                           L"of enumerate element");

    for (VXMLNodeIterator it(item); it; ++it) {
      // (3.4.2) Ignore anything which isn't an <option> or <choice> grammar.
      if ((*it).GetType() != VXMLNode::Type_VXMLElement) continue;
      const VXMLNode & tmp = *it;
      const VXMLElement & itemElem = reinterpret_cast<const VXMLElement&>(tmp);
      switch (itemElem.GetName()) {
      case NODE_CHOICE:
      case NODE_OPTION:
        break;
      default:
        continue;
      }

      // (3.4.3) Convert item into text.
      vxistring promptText;
      for (VXMLNodeIterator choiceNode(itemElem); choiceNode; ++choiceNode) {
        if ((*choiceNode).GetType() == VXMLNode::Type_VXMLContent) {
          if (!promptText.empty()) promptText += SPACE;

          const VXMLNode & tmp = *choiceNode;
          promptText += reinterpret_cast<const VXMLContent &>(tmp).GetValue();
        }
      }

      // (3.4.4) Handle the simple case where enumerate does not specify a
      // format string.
      if (!elem.hasChildren()) {
        ConvertToXML(sofar, isSSML);
        AppendTextSegment(sofar, promptText, isSSML);
        sofar += L"<break size='small'/>";
      }
      else {
        // (3.4.5) Get the associated dtmf value.
        vxistring dtmfText;
        itemElem.GetAttribute(ATTRIBUTE_DTMF, dtmfText);

        translator.SetVariable(L"_prompt", promptText);
        translator.SetVariable(L"_dtmf",   dtmfText);

        for (VXMLNodeIterator subNode(elem); subNode; ++subNode)
          ProcessSegments(*subNode, item, propertyList, translator,
                          bargein, props, sofar, isSSML);
      }
    }
    break;
  }
  default:
    log->LogError(999, SimpleLogger::MESSAGE,
                  L"logic error PromptManager::ProcessSegments");
    throw VXIException::Fatal();
  }
}


void PromptManager::AddContent(const VXMLElement & elem,
                               const VXMLElement & item,
                               const PropertyList & propertyList,
                               PromptTranslator & translator,
                               BargeIn bargein,
                               VXIMapHolder & props,
                               vxistring & sofar,
                               VXIValue * value,
                               bool & isSSML)
{
  // (1) Create new audio reference data map.
  VXIMapHolder valueData;
  if (valueData.GetValue() == NULL) throw VXIException::OutOfMemory();

  // (2) Populate the inner map.
  VXIMapSetProperty(valueData.GetValue(), PROMPT_AUDIO_REF_DATA, value);
  propertyList.GetFetchobjCacheAttrs(elem, PropertyList::Grammar, valueData);

  // (3) Generate a hidden name and place the inner map inside the properties.
  vxistring hiddenName;
  VXMLDocumentModel::CreateHiddenVariable(hiddenName);

  VXIMapHolder allRefs(NULL);

  // Get existing map (if it exists) or create a new one.
  const VXIValue * temp = VXIMapGetProperty(props.GetValue(),
                                            PROMPT_AUDIO_REFS);
  if (temp != NULL && VXIValueGetType(temp) == VALUE_MAP)
    allRefs.Acquire(VXIMapClone(reinterpret_cast<const VXIMap *>(temp)));
  else
    allRefs.Acquire(VXIMapCreate());
  if (allRefs.GetValue() == NULL) throw VXIException::OutOfMemory();

  // Place references for this value into the 'all refs' map.
  VXIMapSetProperty(allRefs.GetValue(), hiddenName.c_str(),
                    reinterpret_cast<VXIValue *>(valueData.Release()));

  // And replace the 'all refs' map in the properties.
  VXIMapSetProperty(props.GetValue(), PROMPT_AUDIO_REFS,
                    reinterpret_cast<VXIValue *>(allRefs.Release()));

  // (5.2.4) Create fake 'src' for this audio element.
  hiddenName = vxistring(PROMPT_AUDIO_REFS_SCHEME) + hiddenName;

  // (5.2.5) Handle the SSML case
  if (isSSML) {
    sofar += L"<audio src=\"";
    sofar += hiddenName;
    sofar += L"\"";
    if (elem.hasChildren()) {
      sofar += L">";
      for (VXMLNodeIterator it(elem); it; ++it)
        ProcessSegments(*it, item, propertyList, translator,
                        bargein, props, sofar, isSSML);
      sofar += L"</audio>";
    }
    else
      sofar += L"/>";
    return;
  }

  // (5.2.6) Otherwise, we're dealing simple text & audio.
  
  if (!sofar.empty()) {
    AddSegment(SEGMENT_TEXT, sofar, props, bargein);
    sofar.erase();
  }

  AddSegment(SEGMENT_AUDIO, hiddenName, props, bargein);

  return;
}

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

#include "GrammarManager.hpp"
#include "VXIrec.h"
#include "SimpleLogger.hpp"
#include "CommonExceptions.hpp"
#include "PropertyList.hpp"
#include "VXML.h"
#include "DocumentModel.hpp"

//#############################################################################
// Grammar description class
//#############################################################################

const VXIchar * const GrammarManager::DTMFTerm      = L"@dtmfterm";
const VXIchar * const GrammarManager::FinalSilence  = L"@finalsilence";
const VXIchar * const GrammarManager::MaxTime       = L"@maxtime";
const VXIchar * const GrammarManager::RecordingType = L"@rectype";

enum GrammarScope {
  GRS_NONE,
  GRS_FIELD,
  GRS_DIALOG,
  GRS_DOC
};


class GrammarInfo {
public:
  GrammarInfo(VXIrecGrammar *, const vxistring &, const VXMLElement &);
  ~GrammarInfo();

  VXIrecGrammar * GetRecGrammar()          { return recgrammar; }

  void SetEnabled(bool b)                  { enabled = b; }

  bool IsEnabled() const                   { return enabled; }
  bool IsScope(GrammarScope s) const       { return s == scope; }

  bool IsField(const vxistring & f) const  { return f == field; }
  bool IsDialog(const vxistring & d) const { return d == dialog; }
  bool IsDoc(const vxistring & d) const    { return d == docID; }
  void GetElement(VXMLElement & e) const   { e = element; }

private:
  void _Initialize(const VXMLElement & elem);
  // This sets the field & dialog names.

  const VXMLElement element;
  VXIrecGrammar *   recgrammar;   // grammar handle returned from rec interface
  GrammarScope      scope;
  vxistring         field;
  vxistring         dialog;
  vxistring         docID;
  bool              enabled;
};


GrammarInfo::GrammarInfo(VXIrecGrammar * g,
                         const vxistring & id,
                         const VXMLElement & elem)
  : element(elem), recgrammar(g), scope(GRS_NONE), docID(id), enabled(false)
{
  // (1) Determine the grammar scope.
  
  // (1.1) Obtain default from position in the tree.
  VXMLElement par = elem;
  while (par != 0) {
    VXMLElementType name = par.GetName();
    if (name == NODE_INITIAL || name == NODE_FIELD ||
        name == NODE_RECORD  || name == NODE_TRANSFER)
      { scope = GRS_FIELD;  break; }
    else if (name == NODE_FORM || name == NODE_MENU)
      { scope = GRS_DIALOG; break; }
    else if (name == NODE_VXML)
      { scope = GRS_DOC;    break; }
    par = par.GetParent();
  }

  // (1.2) if scope explicitly specified in grammar or parent, override!
  vxistring str;
  elem.GetAttribute(ATTRIBUTE_SCOPE, str);
  if (str.empty() && par != 0) {
    par.GetAttribute(ATTRIBUTE_SCOPE, str);
  }
  if (!str.empty()) {
    if (str == L"dialog")        scope = GRS_DIALOG;
    else if (str == L"document") scope = GRS_DOC;
  }

  // (2) Do remaining initialization.
  _Initialize(elem);
}


GrammarInfo::~GrammarInfo()
{
  // NOTE: 'recgrammar' must be freed externally.
}


void GrammarInfo::_Initialize(const VXMLElement & elem)
{
  if (elem == 0) return;

  VXMLElementType name = elem.GetName();
  if (name == NODE_FIELD || name == NODE_INITIAL || name == NODE_TRANSFER)
    elem.GetAttribute(ATTRIBUTE__ITEMNAME, field);
  else if (name == NODE_FORM || name == NODE_MENU) {
    elem.GetAttribute(ATTRIBUTE__ITEMNAME, dialog);
    return;
  }

  _Initialize(elem.GetParent());
}


//#############################################################################
// Recognition Answer class
//#############################################################################

RecognitionAnswer::RecognitionAnswer()
  : numAnswers(0), waveform(NULL), result(NULL)
{
  keys.reserve(10);
  values.reserve(10);
  scores.reserve(10);
  utts.reserve(10);
}

RecognitionAnswer::~RecognitionAnswer()
{
  if (result != NULL)
    result->Destroy(&result);
}

void RecognitionAnswer::Bind(VXIrecRecognitionResult * r)
{
  result = r;
}


void RecognitionAnswer::Clear()
{
  keys.clear();
  values.clear();
  scores.clear();
  utts.clear();
  waveform = NULL;
  result = NULL;
  numAnswers = 0;
}


RecognitionAnswer & RecognitionAnswer::operator=(RecognitionAnswer & x)
{
  if (&x != this) {
    numAnswers = x.numAnswers;
    keys = x.keys;
    values = x.values;
    scores = x.scores;
    utts = x.utts;
    waveform = x.waveform;
    result = x.result;

    x.Clear();
  }

  return *this;
}


RecognitionAnswer::RecognitionAnswer(RecognitionAnswer & x)
{
  numAnswers = x.numAnswers;
  keys = x.keys;
  values = x.values;
  scores = x.scores;
  utts = x.utts;
  waveform = x.waveform;
  result = x.result;

  x.Clear();
}

//#############################################################################
// Grammar description class
//#############################################################################

GrammarManager::GrammarManager(VXIrecInterface * r, const SimpleLogger & l)
  : log(l), vxirec(r)
{
}


GrammarManager::~GrammarManager()
{
  ReleaseGrammars();
}


// This function is used to recursively walk through the tree, loading and
// speech or dtmf grammars which are found.
//
void GrammarManager::LoadGrammars(const VXMLElement& doc,
                                  vxistring & documentID,
                                  PropertyList & properties)
{
  if (doc == 0) return;

  VXIMapHolder recProps(NULL);  // Initialize an empty holder.

  // (1) Retrieve the ID for this document.  This is important for grammar
  // activation.

  if (doc.GetName() == NODE_VXML)
    doc.GetAttribute(ATTRIBUTE__ITEMNAME, documentID);

  // (2) Look for grammars in current nodes.

  for (VXMLNodeIterator it(doc); it; ++it) {
    VXMLNode child = *it;

    if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
    const VXMLElement & element = reinterpret_cast<const VXMLElement &>(child);

    VXMLElementType elementName = element.GetName();
    VXIrecGrammar * vg = NULL;

    if (recProps.GetValue() == NULL)
      recProps.Acquire(GetRecProperties(properties));

    // (3) Handle <grammar> & <dtmf>

    if (elementName == NODE_GRAMMAR || elementName == NODE_DTMF) {
      vxistring src;
      element.GetAttribute(ATTRIBUTE_SRC, src);

      // (3.1) Override the language setting (if specified as an attribute)
      vxistring lang;
      if (element.GetAttribute(ATTRIBUTE_XMLLANG, lang) == true)
        AddParamValue(recProps, REC_LANGUAGE, lang);

      // (3.2) Does the grammar come from an external URI?
      if (!src.empty()) {
        if (log.IsLogging(2)) {
          log.StartDiagnostic(2) << L"GrammarManager::LoadGrammars - <grammar "
            L"src=\"" << src << L"\">";
          log.EndDiagnostic();
        }

        VXIMapHolder fetchobj;
        if (fetchobj.GetValue() == NULL) throw VXIException::OutOfMemory();

        vxistring fragment;
        properties.GetFetchobjCacheAttrs(element, PropertyList::Grammar,
                                         fetchobj);
        properties.GetFetchobjURIs(element, fetchobj, src, fragment);

        if (!fragment.empty())
          log.LogError(215);

        vxistring mimeType;
        element.GetAttribute(ATTRIBUTE_TYPE, mimeType);

        VXIrecResult err = vxirec->LoadGrammarURI(vxirec, recProps.GetValue(),
                                                  mimeType.c_str(),
                                                  src.c_str(),
                                                  fetchobj.GetValue(), &vg);
        if (err != VXIrec_RESULT_SUCCESS)
          throw VXIException::InterpreterEvent(EV_ERROR_BAD_GRAMMAR);

        if (vg) AddGrammar(vg, documentID, element);
      }
      // (3.3) Otherwise this is an inlined grammar.
      else {
        log.LogDiagnostic(2, L"GrammarManager::LoadGrammars - <grammar>");

        vxistring text;
        GetEnclosedText(log, element, text);
        
        vxistring mimeType;
        element.GetAttribute(ATTRIBUTE_TYPE, mimeType);

        if (mimeType.empty() && elementName == NODE_DTMF)
          vg = GrammarManager::CreateGrammarFromString(vxirec, log, text,
                                                       REC_MIME_GENERIC_DTMF,
                                                       recProps);
        else
          vg = GrammarManager::CreateGrammarFromString(vxirec, log, text,
                                                       mimeType.c_str(),
                                                       recProps);
        
        if (vg == NULL) // "Error loading in-line grammar %s",text
          throw VXIException::InterpreterEvent(EV_ERROR_BAD_INLINE);

        AddGrammar(vg, documentID, element);
      }
    }

    // (4) Handle <choice>

    else if (elementName == NODE_CHOICE) {
      log.LogDiagnostic(2, L"GrammarManager::LoadGrammars - <choice>");

      // (4.1) If there is a <grammar> tag, it overrides any implicit grammar.

      // (4.1.1) Check for <grammar> element.
      bool foundGrammar = false;

      for (VXMLNodeIterator it(element); it; ++it) {
        VXMLNode child = *it;

        if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
        const VXMLElement & temp = reinterpret_cast<const VXMLElement&>(child);
        if (temp.GetName() != NODE_GRAMMAR) continue;
        foundGrammar = true;
        break;
      }
      // (4.1.2) If found, apply recursion.
      if (foundGrammar) {
        // <choice> nodes can't contain properties.  Don't need to call Push.
        LoadGrammars(element, documentID, properties);
      }

      // (4.1.3) Otherwise, construct a grammar from the prompt text.
      else {
        vxistring text;
        GetEnclosedText(log, element, text);

        // The text may be empty, e.g. for a dtmf only grammar.
        if (!text.empty()) {
          VXIrecGrammar * vg = NULL;

          vg = GrammarManager::CreateGrammarFromString(vxirec, log, text,
                                                       REC_MIME_CHOICE,
                                                       recProps);
          if (vg == NULL) // "Error loading in-line grammar %s",text
            throw VXIException::InterpreterEvent(EV_ERROR_BAD_CHOICE);

          AddGrammar(vg, documentID, element);
        }
      }

      // (4.2) Create associated DTMF grammar.
      //
      // Either an explict dtmf choice is given or implicit numbers are
      // generated for the first nine.  When both are specified, the explicit
      // choice wins.  This will quite possibly create unintentional (and
      // undetected) duplicates....

      vxistring dtmf;
      element.GetAttribute(ATTRIBUTE_DTMF, dtmf);

      if (!dtmf.empty()) {
        VXIrecGrammar * vg = NULL;

        vg = GrammarManager::CreateGrammarFromString(vxirec, log, dtmf,
                                                     REC_MIME_CHOICE_DTMF,
                                                     recProps);
        if (vg == NULL)
          throw VXIException::InterpreterEvent(EV_ERROR_BAD_CHOICE);

        AddGrammar(vg, documentID, element);
      }
    }

    // (5) Handle <field>.

    else if (elementName == NODE_FIELD) { 
      log.LogDiagnostic(2, L"GrammarManager::LoadGrammars - <field>");

      vxistring gram;
      VXIrecGrammar * vg;

      // Build option grammar (if one exists).
      GrammarManager::BuildOptionGrammar(log, element, false, gram);
      if (!gram.empty()) {
        vg = GrammarManager::CreateGrammarFromString(vxirec, log, gram,
                                                     REC_MIME_OPTION,
                                                     recProps);
        if (vg == NULL)
          throw VXIException::InterpreterEvent(EV_ERROR_BAD_OPTION);
        AddGrammar(vg, documentID, element);
      }

      // Build option dtmf grammar (if one exists).
      gram = L"";
      GrammarManager::BuildOptionGrammar(log, element, true, gram);
      if (gram.length() != 0) {
        vg = GrammarManager::CreateGrammarFromString(vxirec, log, gram,
                                                     REC_MIME_OPTION_DTMF,
                                                     recProps);
        if (vg == NULL)
          throw VXIException::InterpreterEvent(EV_ERROR_BAD_OPTION);
        AddGrammar(vg, documentID, element);
      }

      // Add the built-in grammars (if they exist).
      vg = NULL;
      vxistring type;
      element.GetAttribute(ATTRIBUTE_TYPE, type);
      if (!type.empty()) {
        vxistring newuri(L"builtin:grammar/");
        newuri += type;
        if (vxirec->LoadGrammarURI(vxirec, recProps.GetValue(), NULL,
                                   newuri.c_str(), NULL, &vg)
            != VXIrec_RESULT_SUCCESS)
        {
          throw VXIException::InterpreterEvent(EV_ERROR_BAD_GRAMMAR);
        }
        if (vg) AddGrammar(vg, documentID, element);

        newuri = L"builtin:dtmf/";
        newuri += type;
        if (vxirec->LoadGrammarURI(vxirec, recProps.GetValue(), NULL,
                                   newuri.c_str(), NULL, &vg)
            != VXIrec_RESULT_SUCCESS)
        {
          throw VXIException::InterpreterEvent(EV_ERROR_BAD_GRAMMAR);
        }
        if (vg) AddGrammar(vg, documentID, element);
      }

      // Recursively add grammars (this handles <grammar>)
      bool doPop = properties.PushProperties(element);
      LoadGrammars(element, documentID, properties);
      if (doPop) properties.PopProperties();
    }

    // (6) Handle <link>.

    else if (elementName == NODE_LINK) {
      log.LogDiagnostic(2, L"GrammarManager::LoadGrammars - <link>");

      // (6.1) Get properties at this level.
      bool doPop = properties.PushProperties(element);

      // (6.2) Create DTMF grammar is specified.
      vxistring dtmf;
      element.GetAttribute(ATTRIBUTE_DTMF, dtmf);
      if (!dtmf.empty()) {
        VXIrecGrammar * vg = NULL;
        vg = GrammarManager::CreateGrammarFromString(vxirec, log, dtmf,
                                                     REC_MIME_CHOICE_DTMF,
                                                     recProps);
        if (vg == NULL)
          throw VXIException::InterpreterEvent(EV_ERROR_BAD_CHOICE);

        AddGrammar(vg, documentID, element);
      }

      // (6.3) Create child grammars.
      LoadGrammars(element, documentID, properties);
      if (doPop) properties.PopProperties();
    }

    // (7) Handle <record> and <transfer>.

    else if (elementName == NODE_RECORD || elementName == NODE_TRANSFER) { 
      // The DTD allows <grammar> elements, but these don't make sense.  We
      // will therefore ignore them.
    }

    // (8) Handle <transfer>
    else if ( elementName == NODE_TRANSFER ) {
       bool doPop = properties.PushProperties(element);
       LoadGrammars(element, documentID, properties);
       if (doPop) properties.PopProperties();
    }
 
    // (9) Otherwise, nothing was found at this level.  Use recursion to check
    //     the next level down.

    else {
      bool doPop = properties.PushProperties(element);
      LoadGrammars(element, documentID, properties);
      if (doPop) properties.PopProperties();
    }
  }
}


void GrammarManager::BuildOptionGrammar(const SimpleLogger & log,
                                        const VXMLElement & doc, bool isDTMF,
                                        vxistring & gram)
{
  log.LogDiagnostic(2, L"GrammarManager::BuildOptionGrammar()");

  gram = L""; // Clear grammar string
  
  bool firstTime = true;

  for (VXMLNodeIterator it(doc); it; ++it) {
    VXMLNode child = *it;

    if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
    VXMLElement & domElem = reinterpret_cast<VXMLElement &>(child);
    if (domElem.GetName() != NODE_OPTION) continue;
      
    vxistring text;
    vxistring value;
    domElem.GetAttribute(ATTRIBUTE_VALUE, value);

    if (isDTMF) {
      domElem.GetAttribute(ATTRIBUTE_DTMF, text);
      if (value.empty())
        GrammarManager::GetEnclosedText(log, domElem, value);
    }
    else {
      GrammarManager::GetEnclosedText(log, domElem, text);
    }

    if (text.empty()) continue;

    if (!firstTime)
      gram += L" | ";

    gram += text;

    if (!value.empty()) {
      gram += L" {";
      gram += value;
      gram += L"}";
    }
    firstTime = false;
  }

  log.LogDiagnostic(2, L"GrammarManager::BuildOptionGrammar - end");
}


bool GrammarManager::GetEnclosedText(const SimpleLogger & log,
                                     const VXMLElement & doc, vxistring & str)
{
  log.LogDiagnostic(2, L"GrammarManager::GetEnclosedText()");

  // Clear the result first...
  str = L"";
  
  for (VXMLNodeIterator it(doc); it; ++it) {
    VXMLNode child = *it;

    if (child.GetType() == VXMLNode::Type_VXMLContent) {
      VXMLContent & content = reinterpret_cast<VXMLContent &>(child);

      vxistring temp = content.GetValue();
      while (temp.length() > 0) {
        // Strip off any empty characters from beginning
        VXIchar c = temp[0];
        if (c == ' ' || c == '\r' || c == '\n' || c == '\t')
          temp.erase(0, 1);
        else {
          str += temp;
          break;
        }
      }
    }
  }

  if (!str.empty()) {
    // Strip off any trailing whitespace
    while (str.length() > 0) {
      unsigned int len = str.length();
      VXIchar c = str[len - 1];
      if (c == ' ' || c == '\r' || c == '\n' || c == '\t')
        str.erase(len - 1, 1);
      else break;
    }
  }

  log.LogDiagnostic(2, L"GrammarManager::GetEnclosedText - end");
  return !str.empty();
}


VXIrecGrammar*
GrammarManager::CreateGrammarFromString(VXIrecInterface * vxirec,
                                        const SimpleLogger & log,
                                        const vxistring & source,
                                        const VXIchar * type,
                                        const VXIMapHolder & recProps)
{
  if (log.IsLogging(2)) {
    log.StartDiagnostic(2) << L"GrammarManager::CreateGrammarFromString(" <<
      type << L") " << source;
    log.EndDiagnostic();
  }

  VXIrecGrammar * grammar;
  VXIrecResult err = vxirec->LoadGrammarString(vxirec, recProps.GetValue(),
                                               type, source.c_str(), &grammar);

  log.LogDiagnostic(2, L"GrammarManager::CreateGrammarFromString - success");

  if (err != VXIrec_RESULT_SUCCESS) return NULL;
  return grammar;
}


void GrammarManager::AddGrammar(VXIrecGrammar * gr,
                                const vxistring & documentID,
                                const VXMLElement & elem)
{
  GrammarInfo* gp = new GrammarInfo(gr, documentID, elem);
  if (gp == NULL) throw VXIException::OutOfMemory();

  if (log.IsLogging(2)) {
    log.StartDiagnostic(2) << L"GrammarManager::AddGrammar(" << gp << L")";
    log.EndDiagnostic();
  }

  grammars.push_back(gp);
}


void GrammarManager::DisableAllGrammars()
{
  for (GRAMMARS::iterator i = grammars.begin(); i != grammars.end(); ++i) {
    if ((*i)->IsEnabled()) {
      vxirec->DeactivateGrammar(vxirec, (*i)->GetRecGrammar());
      (*i)->SetEnabled(false);
    }
  }
}


void GrammarManager::ReleaseGrammars()
{
  for (GRAMMARS::iterator i = grammars.begin(); i != grammars.end(); ++i) {
    VXIrecGrammar * grammar = (*i)->GetRecGrammar();
    vxirec->FreeGrammar(vxirec, &grammar);
    delete *i;
    *i = NULL;
  }

  grammars.clear();
}


bool GrammarManager::EnableGrammars(const vxistring & documentID,
                                    const vxistring & dialogName,
                                    const vxistring & fieldName,
                                    const VXIMapHolder & properties,
                                    bool isModal)
{
  bool enabled = false;

  for (GRAMMARS::iterator i = grammars.begin(); i != grammars.end(); ++i)
  {
    bool docsMatch    = (*i)->IsDoc(documentID);
    bool dialogsMatch = docsMatch    && (*i)->IsDialog(dialogName);
    bool fieldsMatch  = dialogsMatch && (*i)->IsField(fieldName);

    if ((!fieldName.empty() && fieldsMatch) ||
        // Enable those field grammars matching our (field, form) pair
        (!isModal && (*i)->IsScope(GRS_DIALOG) && dialogsMatch) ||
        // Enable form grammars & dialog scope fields, if not modal
        (!isModal && (*i)->IsScope(GRS_DOC))
        // Enable document level grammars, if not modal
       )
    {
      if (log.IsLogging(2)) {
        log.StartDiagnostic(2) << L"GrammarManager::EnableGrammar(" << (*i)
                               << L")";
        log.EndDiagnostic();
      }

      VXIrecResult err = vxirec->ActivateGrammar(vxirec, properties.GetValue(),
                                                 (*i)->GetRecGrammar());
      if (err != VXIrec_RESULT_SUCCESS)
        throw VXIException::InterpreterEvent(EV_ERROR_BAD_GRAMMAR);

      (*i)->SetEnabled(true);
      enabled = true;
    }
  }

  return enabled;
}


// This function is responsible for calling the VXIrec level Recognize function
// and then mapping the grammar back to the corresponding VXML node.
//
int GrammarManager::Recognize(const VXIMapHolder & properties,
                              RecognitionAnswer & recAnswer,
                              VXMLElement & recNode)
{
  recNode = VXMLElement();
  VXIrecRecognitionResult * answer = NULL;

  // (1) Do VXIrec level Recognitize and process return value.

  // (1.1) Recognize.
  VXIrecResult err = vxirec->Recognize(vxirec, properties.GetValue(),
                                       &answer);
  if (err == VXIrec_RESULT_OUT_OF_MEMORY) {
    log.LogDiagnostic(0,L"GrammarManager::InternalRecognize - Out of memory.");
    return GrammarManager::OutOfMemory;
  }

  if (err == VXIrec_RESULT_DISCONNECT) {
    // Here is the case that the call is being disconnected before the 
    // recognition even starts
    return GrammarManager::Disconnect;
  }

  // (1.2) Anything other than success indicates that recognition failed
  // (badly).  The normal error conditions are returned through the recogntion
  // result structure.
  if (err != VXIrec_RESULT_SUCCESS) {
    log.StartDiagnostic(0) << L"GrammarManager::InternalRecognize - "
      L"VXIrecInterface::Recognize returned " << int (err);
    log.EndDiagnostic();
    log.LogError(420, SimpleLogger::MESSAGE,
                 L"function did not return the expected VXIrecSUCCESS result");
    return GrammarManager::InternalError;
  }

  // (1.3) The answer structure must be defined.
  if (answer == NULL) {
    // Here is the case that the call is being disconnected before the recording even starts
    return GrammarManager::Disconnect;
  }

  // (1.4) Attach the answer structure returned by VXIrec and the waveform to
  // the recAnswer class.
  recAnswer.Bind(answer);
  recAnswer.waveform = answer->waveform;

  // (2) Process all non-Successful results.

  // (2.1) First the status code.

  switch (answer->status) {
  case REC_STATUS_SUCCESS:    // Recognition returned a hypothesis
  case REC_STATUS_DTMFSUCCESS:    // Recognition returned a hypothesis
    break;
  case REC_STATUS_FAILURE:    // Speech detected, no likely hypothesis
    return GrammarManager::Failure;
  case REC_STATUS_TIMEOUT:    // No speech was detected
    return GrammarManager::Timeout;
  case REC_STATUS_DISCONNECT: // Caller has disconnected; no hypothesis
    return GrammarManager::Disconnect;
  case REC_STATUS_ERROR:      // An error aborted recognition
    return GrammarManager::Error;
  default:
    log.StartDiagnostic(0) << L"GrammarManager::InternalRecognize - "
      L"VXIrecInterface::Recognize returned status " << int(answer->status);
    log.EndDiagnostic();
    log.LogError(420, SimpleLogger::MESSAGE,
                 L"function returned an invalid VXIrecStatus code");
    return GrammarManager::InternalError;
  }

  // (2.2) Recognition success. Verify that the input mode was set correctly.

  switch (answer->mode) {
  case REC_INPUT_MODE_DTMF:
  case REC_INPUT_MODE_SPEECH:
    break;
  default:
    log.StartDiagnostic(0) << L"GrammarManager::InternalRecognize - "
      L"VXIrecInterface::Recognize returned mode " << int(answer->mode);
    log.EndDiagnostic();
    log.LogError(420, SimpleLogger::MESSAGE,
                 L"function returned an invalid VXIrecInputMode value");
    return GrammarManager::InternalError;
  }

  // (3) Walk through results array and sanity check the results.

  // (3.1) How many answers are there?

  if (answer->results == NULL) {
    log.LogError(420, SimpleLogger::MESSAGE,
                 L"function returned a null results vector");
    return GrammarManager::InternalError;
  }

  unsigned int NUM_ANSWERS = VXIVectorLength(answer->results);
  if (NUM_ANSWERS == 0) {
    log.LogError(420, SimpleLogger::MESSAGE,
                 L"function returned an empty results vector");
    return GrammarManager::InternalError;
  }

  const VXIMap * bestAnswer = NULL;

  for (unsigned int i = 0; i < NUM_ANSWERS; ++i) {
    // (3.2) Find each element in the result vector.
    
    const VXIValue * temp = VXIVectorGetElement(answer->results, i);
    if (temp == NULL) {
      log.LogError(400, SimpleLogger::MESSAGE,
                   L"VXIVectorGetElement failed to return answer.");
      return GrammarManager::InternalError;
    }
    
    if (VXIValueGetType(temp) != VALUE_MAP) {
      log.LogError(420, SimpleLogger::MESSAGE,
                   L"function returned an invalid results vector");
      return GrammarManager::InternalError;
    }

    const VXIMap * nthAnswer = reinterpret_cast<const VXIMap*>(temp);
    if (i == 0) bestAnswer = nthAnswer;

    // (3.3) Validate types of member keys.

    temp = VXIMapGetProperty(nthAnswer, REC_KEYS);
    if (temp == NULL || VXIValueGetType(temp) != VALUE_VECTOR) {
      log.LogError(420, SimpleLogger::MESSAGE,
                   L"REC_KEYS must be defined with type vector");
      return GrammarManager::InternalError;
    }
    const VXIVector * keys = reinterpret_cast<const VXIVector *>(temp);

    temp = VXIMapGetProperty(nthAnswer, REC_VALUES);
    if (temp == NULL || VXIValueGetType(temp) != VALUE_VECTOR) {
      log.LogError(420, SimpleLogger::MESSAGE,
                   L"REC_VALUES must be defined with type vector");
      return GrammarManager::InternalError;
    }
    const VXIVector * values = reinterpret_cast<const VXIVector *>(temp);

    temp = VXIMapGetProperty(nthAnswer, REC_CONF);
    if (temp == NULL || VXIValueGetType(temp) != VALUE_VECTOR) {
      log.LogError(420, SimpleLogger::MESSAGE,
                   L"REC_CONF must be defined with type vector");
      return GrammarManager::InternalError;
    }
    const VXIVector * scores = reinterpret_cast<const VXIVector *>(temp);

    temp = VXIMapGetProperty(nthAnswer, REC_RAW);
    if (temp == NULL || VXIValueGetType(temp) != VALUE_VECTOR) {
      log.LogError(420, SimpleLogger::MESSAGE,
                   L"REC_RAW must be defined with type vector");
      return GrammarManager::InternalError;
    }
    const VXIVector * utts = reinterpret_cast<const VXIVector *>(temp);

    // (3.4) Validate vector lengths.

    const VXIunsigned length = VXIVectorLength(keys);
    if (length < 1) {
      log.LogError(420, SimpleLogger::MESSAGE,
                   L"result vector must have length of at least one");
      throw VXIException::Fatal();
    }
    if (length != VXIVectorLength(values) ||
        length != VXIVectorLength(scores) ||
        length != VXIVectorLength(utts))
    {
      log.LogError(420, SimpleLogger::MESSAGE,
                   L"result vector length must all match");
      return GrammarManager::InternalError;
    }

    temp = VXIVectorGetElement(keys, 0);
    if (temp == NULL || VXIValueGetType(temp) != VALUE_STRING) {
      log.LogError(420, SimpleLogger::MESSAGE,
                   L"first REC_KEYS must be defined with type string");
      return GrammarManager::InternalError;
    }

    // (3.5) Copy into results class.

    recAnswer.keys.push_back(keys);
    recAnswer.values.push_back(values);
    recAnswer.scores.push_back(scores);
    recAnswer.utts.push_back(utts);
  }

  recAnswer.numAnswers = NUM_ANSWERS;

  // (4) Find the VXML element associated with the matched grammar.

  // (4.1) Find the grammar corresponding to the best answer.

  const VXIValue * temp = VXIMapGetProperty(bestAnswer, REC_GRAMMAR);
  if (temp == NULL || VXIValueGetType(temp) != VALUE_PTR) {
    log.LogError(420, SimpleLogger::MESSAGE,
                 L"unable to obtain grammar key from 'results' #0");
    return GrammarManager::InternalError;
  }

  const VXIPtr * tempptr = reinterpret_cast<const VXIPtr *>(temp);
  const VXIrecGrammar * bestGrammar =
    reinterpret_cast<const VXIrecGrammar*>(VXIPtrValue(tempptr));

  // (4.2) Map the VXIrecGrammar pointer back to the source VXMLElement.

  for (GRAMMARS::iterator j = grammars.begin(); j != grammars.end(); ++j) 
  {
    if ((answer->mode == REC_INPUT_MODE_DTMF && REC_STATUS_DTMFSUCCESS != answer->status) 
      && (*j)->GetRecGrammar() != bestGrammar) continue;

    if (!(*j)->IsEnabled()) 
    {
      if (j == grammars.end()) {
        log.LogError(420, SimpleLogger::MESSAGE,
                     L"function returned an inactive grammar");
        return GrammarManager::InternalError;
      }
      continue;
    }

    (*j)->GetElement(recNode);

    if (answer->mode == REC_INPUT_MODE_DTMF)
      return GrammarManager::SuccessDTMF;

    return GrammarManager::Success;
  }

  log.LogError(420, SimpleLogger::MESSAGE,
               L"function returned a non-existent grammar");
  return GrammarManager::InternalError;
}


int GrammarManager::Record(const VXIMapHolder & properties,
                           VXIrecRecordResult * & resultStruct)
{
  // (1) Do record.

  VXIrecResult err = vxirec->Record(vxirec, properties.GetValue(),
                                       &resultStruct);
  if (err == VXIrec_RESULT_OUT_OF_MEMORY) {
    log.LogDiagnostic(0, L"GrammarManager::Record - Out of memory.");
    return GrammarManager::OutOfMemory;
  }

  if (err == VXIrec_RESULT_BAD_MIME_TYPE) {
    log.LogDiagnostic(0, L"GrammarManager::Record - Unsupported mime type.");
    return GrammarManager::BadMimeType;
  }

  if (err == VXIrec_RESULT_DISCONNECT) {
    // Here is the case that the call is being disconnected before the 
    // recording even starts
    return GrammarManager::Disconnect;
  }

  if (err != VXIrec_RESULT_SUCCESS) {
    log.StartDiagnostic(0) << L"GrammarManager::Record - "
      L"VXIrecInterface::Record returned " << int (err);
    log.EndDiagnostic();
    log.LogError(421, SimpleLogger::MESSAGE,
                 L"function did not return the expected VXIrecSUCCESS result");
    return GrammarManager::InternalError;
  }

  if (resultStruct == NULL) {
    // Here is the case that the call is being disconnected before the recording even starts
    return GrammarManager::Disconnect;
  }

  // (2) Process all non-Successful results.

  switch (resultStruct->status) {
  case REC_STATUS_SUCCESS:    // Record produced a recording
    break;
  case REC_STATUS_TIMEOUT:    // No signal was detected
    return GrammarManager::Timeout;
  case REC_STATUS_ERROR:      // An error aborted record
    return GrammarManager::Error;
  case REC_STATUS_DISCONNECT: // Caller has disconnected without a recording
    return GrammarManager::Disconnect;
  case REC_STATUS_FAILURE:    // This should never happen
    return GrammarManager::InternalError;
  default:
    log.StartDiagnostic(0) << L"GrammarManager::InternalRecognize - "
      L"VXIrecInterface::Recognize returned status "
      << int (resultStruct->status);
    log.EndDiagnostic();
    log.LogError(420, SimpleLogger::MESSAGE,
                 L"function returned an invalid VXIrecStatus code");
    return GrammarManager::InternalError;
  }

  // (3) Verify that in the success case, the information is valid.

  if (resultStruct->waveform == NULL) {
    log.LogError(421, SimpleLogger::MESSAGE,
                 L"function did not produce a recording");
    return GrammarManager::InternalError;
  }

  // Technically recordings could exceed an hour.  These users may just
  // change the source.
  if (resultStruct->duration == 0 || resultStruct->duration > 3600000) {
    log.LogError(421, SimpleLogger::MESSAGE,
                 L"function returned invalid recording duration");
    return GrammarManager::InternalError;
  }

  return GrammarManager::Success;
}


VXIMap * GrammarManager::GetRecProperties(const PropertyList & props,
                                          int timeout) const
{
  VXIMapHolder m;
  if (m.GetValue() == NULL) throw VXIException::OutOfMemory();

  // (1) Retrieve flattened property list.
  props.GetProperties(m);

  // (2) Convert & manipulate the properties.
  // TBD #pragma message ("GrammarManager::GetRecProperties - handle REC_RESULT_SAVE_WAVEFORM ???")
  // TBD #pragma message ("GrammarManager::GetRecProperties - handle REC_RESULT_NBEST_SIZE ???")

  const VXIchar * j;
  VXIint time;
  VXIflt32 fraction;

  // (2.1) Language
  j = props.GetProperty(PropertyList::Language);
  if (j != NULL)
    AddParamValue(m, REC_LANGUAGE, j);

  // (2.2) Completion timeout
  j = props.GetProperty(PROP_COMPLETETIME);
  if (j != NULL && props.ConvertTimeToMilliseconds(log, j, time))
    AddParamValue(m, REC_TIMEOUT_COMPLETE, time);

  // (2.3) Incompletion timeout
  j = props.GetProperty(PROP_INCOMPLETETIME);
  if (j != NULL && props.ConvertTimeToMilliseconds(log, j, time))
    AddParamValue(m, REC_TIMEOUT_INCOMPLETE, time);

  // (2.4) Inter-Digit timeout
  j = props.GetProperty(PROP_INTERDIGITTIME);
  if (j != NULL && props.ConvertTimeToMilliseconds(log, j, time))
    AddParamValue(m, REC_DTMF_TIMEOUT_INTERDIGIT, time);

  // (2.5) Inter-Digit timeout
  j = props.GetProperty(PROP_TERMTIME);
  if (j != NULL && props.ConvertTimeToMilliseconds(log, j, time))
    AddParamValue(m, REC_DTMF_TIMEOUT_TERMINATOR, time);

  // (2.6) Confidence level
  j = props.GetProperty(PROP_CONFIDENCE);
  if (j != NULL && props.ConvertValueToFraction(log, j, fraction))
    AddParamValue(m, REC_CONFIDENCE_LEVEL, fraction);

  // (2.7) Barge-in sensitivity level
  j = props.GetProperty(PROP_SENSITIVITY);
  if (j != NULL && props.ConvertValueToFraction(log, j, fraction))
    AddParamValue(m, REC_SENSITIVITY, fraction);

  // (2.8) Performance tradeoff - speed vs. accuracy
  j = props.GetProperty(PROP_SPEEDVSACC);
  if (j != NULL && props.ConvertValueToFraction(log, j, fraction))
    AddParamValue(m, REC_SPEED_VS_ACCURACY, fraction);
  
  // (2.9) DTMF terminator character
  j = props.GetProperty(PROP_TERMCHAR);
  if ((j != NULL) && (j[0] != L'\0')) {
    if (wcslen(j) == 1)
      AddParamValue(m, REC_DTMF_TERMINATOR_CHAR, j);
    else {
      log.StartDiagnostic(0) << L"GrammarManager::GetRecProperties - "
        L"Unable to set " << REC_DTMF_TERMINATOR_CHAR << L" from value \"" <<
        j << L"\".  Defaulting to '#'.";
      log.EndDiagnostic();
      // Should we use the default, or rather not set anything ?
      AddParamValue(m, REC_DTMF_TERMINATOR_CHAR, L"#");
    }
  }
  
  // (2.10) Input modes
  int mode = REC_INPUT_MODE_DTMF_SPEECH;
  j = props.GetProperty(PROP_INPUTMODES);
  if (j != NULL) { 
    vxistring value(j);
    if (value == L"voice dtmf" || value == L"dtmf voice")
      mode = REC_INPUT_MODE_DTMF_SPEECH;
    else if (value == L"voice")
      mode = REC_INPUT_MODE_SPEECH;
    else if (value == L"dtmf")
      mode = REC_INPUT_MODE_DTMF;
    else {
      log.StartDiagnostic(0) << L"GrammarManager::GetRecProperties - "
        L"Unable to set " << REC_INPUT_MODES << L" from value \"" << value <<
        L"\".";
      log.EndDiagnostic();
    }
  }
  AddParamValue(m, REC_INPUT_MODES, mode);

  // (2.11) Timeout settings
  if (timeout == -1) {
    j = props.GetProperty(PROP_TIMEOUT);
    if (j != NULL)
      PropertyList::ConvertTimeToMilliseconds(log, j, timeout);
  }

  if (timeout != -1) {
    AddParamValue(m, REC_DTMF_TIMEOUT, timeout);
    AddParamValue(m, REC_TIMEOUT, timeout);
  }


  // (3) Done

  return m.Release();
}


VXIMap * GrammarManager::GetRecordProperties(const PropertyList & props,
                                             int timeout) const
{
  VXIMapHolder m;
  if (m.GetValue() == NULL) throw VXIException::OutOfMemory();

  // (1) Retrieve flattened property list.
  props.GetProperties(m);

  // (2) Convert & manipulate the properties.
  const VXIchar * j;
  VXIint time;

  // (2.1) Completion timeout
  j = props.GetProperty(GrammarManager::MaxTime);
  if (j != NULL && props.ConvertTimeToMilliseconds(log, j, time))
    AddParamValue(m, REC_MAX_RECORDING_TIME, time);

  // (2.2) Final silence
  j = props.GetProperty(GrammarManager::FinalSilence);
  if (j != NULL && props.ConvertTimeToMilliseconds(log, j, time))
    AddParamValue(m, REC_TIMEOUT_COMPLETE, time);

  // (2.3) Type
  j = props.GetProperty(GrammarManager::RecordingType);
  if (j != NULL)
    AddParamValue(m, REC_RECORD_MIME_TYPE, j);

  // (2.4) DTMF terminates record?
  j = props.GetProperty(GrammarManager::DTMFTerm);
  if (j != NULL) {
    int dtmfterm;
    if (vxistring(j) == L"false")
      dtmfterm = 0;
    else
      dtmfterm = 1;
    
    AddParamValue(m, REC_TERMINATED_ON_DTMF, dtmfterm);
  }

  // (2.5) Timeout settings
  if (timeout == -1) {
    j = props.GetProperty(PROP_TIMEOUT);
    if (j != NULL)
      PropertyList::ConvertTimeToMilliseconds(log, j, timeout);
  }

  if (timeout != -1) {
    AddParamValue(m, REC_DTMF_TIMEOUT, timeout);
    AddParamValue(m, REC_TIMEOUT, timeout);
  }

  // (3) Done

  return m.Release();
}

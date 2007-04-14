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

#include "DocumentConverter.hpp"
#include "DocumentRep.hpp"
#include <sax/AttributeList.hpp>
#include <sax/SAXParseException.hpp>
#include <sax/Locator.hpp>
#include <vector>
#include <algorithm>
#include <sstream>                     // by ProcessNodeFinal
#include "XMLChConverter.hpp"

//#############################################################################

enum {
  PRIV_ELEM_RangeStart    = 0x200,   // This should always be the first entry
  PRIV_ELEM_DIV,                     // replaced in 2.0 by <s> & <p>
  PRIV_ELEM_EMP,                     // replaced in 2.0 by <emphasis>
  PRIV_ELEM_PROS,                    // replaced in 2.0 by <prosody>
  PRIV_ELEM_SAYAS                    // replaced in 2.0 by <say-as>
};

enum {
  PRIV_ATTRIB_RangeStart  = 0x200,   // This should always be the first entry
  PRIV_ATTRIB_CACHING,               // Replaced in 2.0 by maxage, maxstale
  PRIV_ATTRIB_CLASS,                 // <value>, <sayas> in 2.0 by type
  PRIV_ATTRIB_LANG,                  // <vxml>  replaced in 2.0 by xml:lang
  PRIV_ATTRIB_MSECS,                 // <break> replaced in 2.0 by msec
  PRIV_ATTRIB_PHON,                  // <sayas> replaced in 2.0 ph of <phoneme>
  PRIV_ATTRIB_RECSRC,                // <value> dropped in 2.0
  PRIV_ATTRIB_VERSION,
  PRIV_ATTRIB_VOL
};


struct VXMLElementInfo {
public:
  const VXIchar * key;
  int value;

  VXMLElementInfo(const VXIchar * k, int v)
    : key(k), value(v)  { }

  VXMLElementInfo(const VXMLElementInfo & k)
    : key(k.key), value(k.value)   { }

  VXMLElementInfo& operator=(const VXMLElementInfo & k)
    { key = k.key; value = k.value; return *this; }
};


bool operator< (const VXMLElementInfo & x, const VXMLElementInfo & y)
{ if (x.key == NULL || y.key == NULL) return false;
  return wcscmp(x.key, y.key) < 0; }


struct VXMLAttribute {
public:
  const VXIchar * key;
  int value;

  VXMLAttribute(const VXIchar * k, int v)
    : key(k), value(v)  { }

  VXMLAttribute(const VXMLAttribute & k)
    : key(k.key), value(k.value)       { }

  VXMLAttribute& operator=(const VXMLAttribute & k)
    { key = k.key; value = k.value; return *this; }
};


bool operator< (const VXMLAttribute & x, const VXMLAttribute & y)
{ if (x.key == NULL || y.key == NULL) return false;
  return wcscmp(x.key, y.key) < 0; }


typedef std::vector<VXMLAttribute>   TABLE_ATTRS;
typedef std::vector<VXMLElementInfo> TABLE_ELEMS;

TABLE_ATTRS  attrs;
TABLE_ELEMS  elems;

//#############################################################################
 
static void InitializeTables()
{
  // (1) VXML elements

  // (1.1) Added in VXML 1.0
  elems.push_back(VXMLElementInfo(L"assign"      , NODE_ASSIGN));
  elems.push_back(VXMLElementInfo(L"audio"       , NODE_AUDIO));
  elems.push_back(VXMLElementInfo(L"block"       , NODE_BLOCK));
  elems.push_back(VXMLElementInfo(L"break"       , NODE_BREAK));
  elems.push_back(VXMLElementInfo(L"cancel"      , NODE_CANCEL));
  elems.push_back(VXMLElementInfo(L"catch"       , NODE_CATCH));
  elems.push_back(VXMLElementInfo(L"choice"      , NODE_CHOICE));
  elems.push_back(VXMLElementInfo(L"clear"       , NODE_CLEAR));
  elems.push_back(VXMLElementInfo(L"disconnect"  , NODE_DISCONNECT));
  elems.push_back(VXMLElementInfo(L"dtmf"        , NODE_DTMF));
  elems.push_back(VXMLElementInfo(L"else"        , NODE_ELSE));
  elems.push_back(VXMLElementInfo(L"elseif"      , NODE_ELSEIF));
  elems.push_back(VXMLElementInfo(L"emphasis"    , NODE_EMPHASIS));
  elems.push_back(VXMLElementInfo(L"enumerate"   , NODE_ENUMERATE));
  elems.push_back(VXMLElementInfo(L"error"       , NODE_ERROR));
  elems.push_back(VXMLElementInfo(L"exit"        , NODE_EXIT));
  elems.push_back(VXMLElementInfo(L"field"       , NODE_FIELD));
  elems.push_back(VXMLElementInfo(L"filled"      , NODE_FILLED));
  elems.push_back(VXMLElementInfo(L"form"        , NODE_FORM));
  elems.push_back(VXMLElementInfo(L"goto"        , NODE_GOTO));
  elems.push_back(VXMLElementInfo(L"grammar"     , NODE_GRAMMAR));
  elems.push_back(VXMLElementInfo(L"help"        , NODE_HELP));
  elems.push_back(VXMLElementInfo(L"if"          , NODE_IF));
  elems.push_back(VXMLElementInfo(L"initial"     , NODE_INITIAL));
  elems.push_back(VXMLElementInfo(L"link"        , NODE_LINK));
  elems.push_back(VXMLElementInfo(L"log"         , NODE_LOG));
  elems.push_back(VXMLElementInfo(L"mark"        , NODE_MARK));
  elems.push_back(VXMLElementInfo(L"menu"        , NODE_MENU));
  elems.push_back(VXMLElementInfo(L"meta"        , NODE_META));
  elems.push_back(VXMLElementInfo(L"noinput"     , NODE_NOINPUT));
  elems.push_back(VXMLElementInfo(L"nomatch"     , NODE_NOMATCH));
  elems.push_back(VXMLElementInfo(L"object"      , NODE_OBJECT));
  elems.push_back(VXMLElementInfo(L"option"      , NODE_OPTION));
  elems.push_back(VXMLElementInfo(L"p"           , NODE_PARAGRAPH));
  elems.push_back(VXMLElementInfo(L"paragraph"   , NODE_PARAGRAPH));
  elems.push_back(VXMLElementInfo(L"param"       , NODE_PARAM));
  elems.push_back(VXMLElementInfo(L"prompt"      , NODE_PROMPT));
  elems.push_back(VXMLElementInfo(L"property"    , NODE_PROPERTY));
  elems.push_back(VXMLElementInfo(L"prosody"     , NODE_PROSODY));
  elems.push_back(VXMLElementInfo(L"record"      , NODE_RECORD));
  elems.push_back(VXMLElementInfo(L"return"      , NODE_RETURN));
  elems.push_back(VXMLElementInfo(L"reprompt"    , NODE_REPROMPT));
  elems.push_back(VXMLElementInfo(L"s"           , NODE_SENTENCE));
  elems.push_back(VXMLElementInfo(L"say-as"      , NODE_SAYAS));
  elems.push_back(VXMLElementInfo(L"script"      , NODE_SCRIPT));
  elems.push_back(VXMLElementInfo(L"sentence"    , NODE_SENTENCE));
  elems.push_back(VXMLElementInfo(L"subdialog"   , NODE_SUBDIALOG));
  elems.push_back(VXMLElementInfo(L"submit"      , NODE_SUBMIT));
  elems.push_back(VXMLElementInfo(L"throw"       , NODE_THROW));
  elems.push_back(VXMLElementInfo(L"transfer"    , NODE_TRANSFER));
  elems.push_back(VXMLElementInfo(L"value"       , NODE_VALUE));
  elems.push_back(VXMLElementInfo(L"var"         , NODE_VAR));
  elems.push_back(VXMLElementInfo(L"voice"       , NODE_VOICE));
  elems.push_back(VXMLElementInfo(L"vxml"        , NODE_VXML));

  // (1.2) from Defaults document
  elems.push_back(VXMLElementInfo(L"defaults"    , DEFAULTS_ROOT));
  elems.push_back(VXMLElementInfo(L"language"    , DEFAULTS_LANGUAGE));

  // (1.3) Internals elements (these are converted to others)
  elems.push_back(VXMLElementInfo(L"div"         , PRIV_ELEM_DIV));
  elems.push_back(VXMLElementInfo(L"emp"         , PRIV_ELEM_EMP));
  elems.push_back(VXMLElementInfo(L"pros"        , PRIV_ELEM_PROS));
  elems.push_back(VXMLElementInfo(L"sayas"       , PRIV_ELEM_SAYAS));

  // (2) VXML element attributes

  // (2.1) added in VXML 1.0
  attrs.push_back(VXMLAttribute(L"_itemname"     , ATTRIBUTE__ITEMNAME));
  attrs.push_back(VXMLAttribute(L"accept"        , ATTRIBUTE_ACCEPT));
  attrs.push_back(VXMLAttribute(L"age"           , ATTRIBUTE_AGE));
  attrs.push_back(VXMLAttribute(L"alphabet"      , ATTRIBUTE_ALPHABET));
  attrs.push_back(VXMLAttribute(L"application"   , ATTRIBUTE_APPLICATION));
  attrs.push_back(VXMLAttribute(L"archive"       , ATTRIBUTE_ARCHIVE));
  attrs.push_back(VXMLAttribute(L"bargein"       , ATTRIBUTE_BARGEIN));
  attrs.push_back(VXMLAttribute(L"bargeintype"   , ATTRIBUTE_BARGEINTYPE));
  attrs.push_back(VXMLAttribute(L"base"          , ATTRIBUTE_BASE));
  attrs.push_back(VXMLAttribute(L"beep"          , ATTRIBUTE_BEEP));
  attrs.push_back(VXMLAttribute(L"bridge"        , ATTRIBUTE_BRIDGE));
  attrs.push_back(VXMLAttribute(L"category"      , ATTRIBUTE_CATEGORY));
  attrs.push_back(VXMLAttribute(L"charset"       , ATTRIBUTE_CHARSET));
  attrs.push_back(VXMLAttribute(L"classid"       , ATTRIBUTE_CLASSID));
  attrs.push_back(VXMLAttribute(L"codebase"      , ATTRIBUTE_CODEBASE));
  attrs.push_back(VXMLAttribute(L"codetype"      , ATTRIBUTE_CODETYPE));
  attrs.push_back(VXMLAttribute(L"cond"          , ATTRIBUTE_COND));
  attrs.push_back(VXMLAttribute(L"connecttimeout", ATTRIBUTE_CONNECTTIME));
  attrs.push_back(VXMLAttribute(L"contour"       , ATTRIBUTE_CONTOUR));
  attrs.push_back(VXMLAttribute(L"count"         , ATTRIBUTE_COUNT));
  attrs.push_back(VXMLAttribute(L"data"          , ATTRIBUTE_DATA));
  attrs.push_back(VXMLAttribute(L"dest"          , ATTRIBUTE_DEST));
  attrs.push_back(VXMLAttribute(L"destexpr"      , ATTRIBUTE_DESTEXPR));
  attrs.push_back(VXMLAttribute(L"dtmf"          , ATTRIBUTE_DTMF));
  attrs.push_back(VXMLAttribute(L"dtmfterm"      , ATTRIBUTE_DTMFTERM));
  attrs.push_back(VXMLAttribute(L"duration"      , ATTRIBUTE_DURATION));
  attrs.push_back(VXMLAttribute(L"enctype"       , ATTRIBUTE_ENCTYPE));
  attrs.push_back(VXMLAttribute(L"event"         , ATTRIBUTE_EVENT));
  attrs.push_back(VXMLAttribute(L"expr"          , ATTRIBUTE_EXPR));
  attrs.push_back(VXMLAttribute(L"expritem"      , ATTRIBUTE_EXPRITEM));
  attrs.push_back(VXMLAttribute(L"fetchaudio"    , ATTRIBUTE_FETCHAUDIO));
  attrs.push_back(VXMLAttribute(L"fetchhint"     , ATTRIBUTE_FETCHHINT));
  attrs.push_back(VXMLAttribute(L"fetchtimeout"  , ATTRIBUTE_FETCHTIMEOUT));
  attrs.push_back(VXMLAttribute(L"finalsilence"  , ATTRIBUTE_FINALSILENCE));
  attrs.push_back(VXMLAttribute(L"gender"        , ATTRIBUTE_GENDER));
  attrs.push_back(VXMLAttribute(L"id"            , ATTRIBUTE_ID));
  attrs.push_back(VXMLAttribute(L"label"         , ATTRIBUTE_LABEL));
  attrs.push_back(VXMLAttribute(L"level"         , ATTRIBUTE_LEVEL));
  attrs.push_back(VXMLAttribute(L"maxage"        , ATTRIBUTE_MAXAGE));
  attrs.push_back(VXMLAttribute(L"maxstale"      , ATTRIBUTE_MAXSTALE));
  attrs.push_back(VXMLAttribute(L"maxtime"       , ATTRIBUTE_MAXTIME));
  attrs.push_back(VXMLAttribute(L"method"        , ATTRIBUTE_METHOD));
  attrs.push_back(VXMLAttribute(L"modal"         , ATTRIBUTE_MODAL));
  attrs.push_back(VXMLAttribute(L"mode"          , ATTRIBUTE_MODE));
  attrs.push_back(VXMLAttribute(L"name"          , ATTRIBUTE_NAME));
  attrs.push_back(VXMLAttribute(L"namelist"      , ATTRIBUTE_NAMELIST));
  attrs.push_back(VXMLAttribute(L"next"          , ATTRIBUTE_NEXT));
  attrs.push_back(VXMLAttribute(L"nextitem"      , ATTRIBUTE_NEXTITEM));
  attrs.push_back(VXMLAttribute(L"ph"            , ATTRIBUTE_PH));
  attrs.push_back(VXMLAttribute(L"pitch"         , ATTRIBUTE_PITCH));
  attrs.push_back(VXMLAttribute(L"range"         , ATTRIBUTE_RANGE));
  attrs.push_back(VXMLAttribute(L"rate"          , ATTRIBUTE_RATE));
  attrs.push_back(VXMLAttribute(L"scope"         , ATTRIBUTE_SCOPE));
  attrs.push_back(VXMLAttribute(L"size"          , ATTRIBUTE_SIZE));
  attrs.push_back(VXMLAttribute(L"slot"          , ATTRIBUTE_SLOT));
  attrs.push_back(VXMLAttribute(L"src"           , ATTRIBUTE_SRC));
  attrs.push_back(VXMLAttribute(L"srcexpr"       , ATTRIBUTE_SRCEXPR));
  attrs.push_back(VXMLAttribute(L"sub"           , ATTRIBUTE_SUB));
  attrs.push_back(VXMLAttribute(L"time"          , ATTRIBUTE_TIME));
  attrs.push_back(VXMLAttribute(L"timeout"       , ATTRIBUTE_TIMEOUT));
  attrs.push_back(VXMLAttribute(L"transferaudio"     , ATTRIBUTE_TRANSFERAUDIO));
  attrs.push_back(VXMLAttribute(L"transferaudioexpr" , ATTRIBUTE_TRANSFERAUDIOEXPR));
  attrs.push_back(VXMLAttribute(L"type"          , ATTRIBUTE_TYPE));
  attrs.push_back(VXMLAttribute(L"value"         , ATTRIBUTE_VALUE));
  attrs.push_back(VXMLAttribute(L"valuetype"     , ATTRIBUTE_VALUETYPE));
  attrs.push_back(VXMLAttribute(L"variant"       , ATTRIBUTE_VARIANT));
  attrs.push_back(VXMLAttribute(L"volume"        , ATTRIBUTE_VOLUME));
  attrs.push_back(VXMLAttribute(L"xml:lang"      , ATTRIBUTE_XMLLANG));

  // (2.2) added in VXML 2.0
  attrs.push_back(VXMLAttribute(L"eventexpr"     , ATTRIBUTE_EVENTEXPR));
  attrs.push_back(VXMLAttribute(L"message"       , ATTRIBUTE_MESSAGE));
  attrs.push_back(VXMLAttribute(L"messageexpr"   , ATTRIBUTE_MESSAGEEXPR));

  // (2.3) Internals attributes (these are converted to others)
  attrs.push_back(VXMLAttribute(L"caching"       , PRIV_ATTRIB_CACHING));
  attrs.push_back(VXMLAttribute(L"class"         , PRIV_ATTRIB_CLASS));
  attrs.push_back(VXMLAttribute(L"lang"          , PRIV_ATTRIB_LANG));
  attrs.push_back(VXMLAttribute(L"msecs"         , PRIV_ATTRIB_MSECS));
  attrs.push_back(VXMLAttribute(L"phon"          , PRIV_ATTRIB_PHON));
  attrs.push_back(VXMLAttribute(L"recsrc"        , PRIV_ATTRIB_RECSRC));
  attrs.push_back(VXMLAttribute(L"version"       , PRIV_ATTRIB_VERSION));
  attrs.push_back(VXMLAttribute(L"vol"           , PRIV_ATTRIB_VOL));


  // (3) Final stuff.
  std::sort(attrs.begin(), attrs.end());
  std::sort(elems.begin(), elems.end());
}


static void DeinitializeTables()
{
  attrs.clear();
  elems.clear();
}


static bool ConvertElement(const VXIchar * name, int & result)
{
  if (name == NULL) return false;

  TABLE_ELEMS::iterator i = std::lower_bound(elems.begin(), elems.end(),
                                             VXMLElementInfo(name, NODE_IF));

  if (i != elems.end() && wcscmp(name, (*i).key) != 0)
  {
     return false;
  }

  result = (*i).value;
  return true;
}


static bool ConvertAttribute(const VXIchar * name, int & result)
{
  if (name == NULL) return false;

  TABLE_ATTRS::iterator i = std::lower_bound(attrs.begin(), attrs.end(),
                                             VXMLAttribute(name,ATTRIBUTE_ID));

  if (i != attrs.end() && wcscmp(name, (*i).key) != 0)
  {
     return false;
  }

  result = (*i).value;

  return true;
}


//#############################################################################

class DummyLocator : public Locator {
public:
  DummyLocator()  { }
  virtual ~DummyLocator() { }
  virtual const XMLCh* getPublicId() const    { return NULL; }
  virtual const XMLCh* getSystemId() const    { return NULL; }
  virtual XMLSSize_t getLineNumber() const    { return -1; }
  virtual XMLSSize_t getColumnNumber() const  { return -1; }
};

static DummyLocator DUMMYLOCATOR;

//#############################################################################

bool DocumentConverter::Initialize()
{
  InitializeTables();
  return true;
}


void DocumentConverter::Deinitialize()
{
  DeinitializeTables();
}


DocumentConverter::DocumentConverter()
  : locator(&DUMMYLOCATOR), doc(NULL), strict(false)
{
}


DocumentConverter::~DocumentConverter()
{
}


void DocumentConverter::ParseException(const VXIchar * message) const
{
  if (message == NULL) throw SAXParseException(NULL, *locator);

  VXIcharToXMLCh text(message);
  throw SAXParseException(text.c_str(), *locator);
}


void DocumentConverter::startDocument()
{
  version = -1;
  ignoreDepth = 0;
  choiceNumber = 0;

  if (doc != NULL) VXMLDocumentRep::Release(doc);
  doc = new VXMLDocumentRep();
  if (doc == NULL)
    ParseException(L"unable to allocate memory for element");
}


void DocumentConverter::endDocument()
{
} 


void DocumentConverter::resetDocument()
{
  if (doc != NULL) VXMLDocumentRep::Release(doc);
  doc = NULL;
}


void DocumentConverter::setDocumentLocator(const Locator* const loc)
{
  if (loc == NULL)
    locator = &DUMMYLOCATOR;
  else
    locator = loc;
}


void DocumentConverter::startElement(const XMLCh* const name,
                                     AttributeList & attrs)
{
  XMLChToVXIchar elementName(name);

  // (1) Convert name string to enum.
  int elemType;
  if (!ConvertElement(elementName.c_str(), elemType)) {
    vxistring temp(L"unrecognized element - ");
    temp += elementName.c_str();
    ParseException(temp.c_str());
  }

  // (2) Check for ignored nodes and do version number processing.

  // (2.1) Catch illegal nodes and do version number processing.
  switch (elemType) {
  case DEFAULTS_ROOT:
    version = 2.0f;
    break;
  case NODE_VXML:
  {
    for (unsigned int index = 0; index < attrs.getLength(); ++index) {
      if (!Compare(attrs.getName(index), L"version")) continue;
      const XMLCh * attributeValue = attrs.getValue(index);
      if (Compare(attributeValue, L"1.0")) version = 1.0f;
      else if (Compare(attributeValue, L"2.0")) version = 2.0f;
      else ParseException(L"illegal version");
      break;
    }
    break;
  }
  case PRIV_ELEM_DIV:
  case PRIV_ELEM_EMP:
  case PRIV_ELEM_PROS:
  case PRIV_ELEM_SAYAS:
    if (version != 1.0f) {
      vxistring temp(L"support for element '");
      temp += elementName.c_str();
      temp += L"' was dropped after VXML 1.0";
      ParseException(temp.c_str());
    }
    break;
  default:
    break;
  }

  // (2.2) Should we just ignore this node?
  if (IsIgnorable(elemType)) return;

  // (2.3) Convert nodes as necessary.
  switch (elemType) {
  case PRIV_ELEM_DIV: // Convert <div> into either <p> or <s>.
    if (attrs.getLength() == 1 && Compare(attrs.getName(0), L"paragraph"))
      elemType = NODE_PARAGRAPH;
    else
      elemType = NODE_SENTENCE;
    break;
  case PRIV_ELEM_EMP:
    elemType = NODE_EMPHASIS;
    break;
  case PRIV_ELEM_SAYAS:
  {
    elemType = NODE_SAYAS;

    for (unsigned int index = 0; index < attrs.getLength(); ++index)
      if (Compare(attrs.getName(index), L"phon")) {
        elemType = NODE_PHONEME;
        break;
    }
    break;
  }
  default:
    break;
  }

  // (3) Create new element.

  if (elemType > PRIV_ELEM_RangeStart) {
    vxistring temp(L"internal error for element - ");
    temp += elementName.c_str();
    ParseException(temp.c_str());
  }

  try {
    doc->StartElement(VXMLElementType(elemType));
  }
  catch (const VXMLDocumentModel::OutOfMemory &) {
    ParseException(L"unable to allocate memory for element");
  }
  catch (const VXMLDocumentModel::InternalError &) {
    ParseException(L"corrupted document tree; unable to add element");
  }

  // (4) Add attributes to element.
  for (unsigned int index = 0; index < attrs.getLength(); ++index) {
    int attrType;

    // (4.1) Convert string to integer.

    XMLChToVXIchar attributeName(attrs.getName(index));

    if (!ConvertAttribute(attributeName.c_str(), attrType)) {
      vxistring temp(L"unrecognized attribute - ");
      temp += attributeName.c_str();
      ParseException(temp.c_str());
    }

    // (4.2) Handle a few global settings.
    switch (attrType) {
    case PRIV_ATTRIB_CACHING:
      if (version != 1.0f)
        ParseException(L"the caching attribute was replaced by maxage and "
                       L"maxstale after VXML 1.0");

      if (Compare(attrs.getValue(index), L"safe")) {
        vxistring attr;
        if (!doc->GetAttribute(ATTRIBUTE_MAXAGE, attr))
          doc->AddAttribute(ATTRIBUTE_MAXAGE, L"0");
      }
      continue;

    default:
      break;
    }

    // (4.3) Handle internal values.
    XMLChToVXIchar attributeValue(attrs.getValue(index));
    ProcessNodeAttribute(VXMLElementType(elemType),
                         attrType, attributeValue.c_str());
  }

  // (5) Verify the node.
  ProcessNodeFinal(VXMLElementType(elemType));
}


void DocumentConverter::endElement(const XMLCh* const name)
{
  XMLChToVXIchar elementName(name);

  int elemType;
  if (!ConvertElement(elementName.c_str(), elemType)) {
    vxistring temp(L"unrecognized element - ");
    temp += elementName.c_str();
    ParseException(temp.c_str());
  }

  if (IsIgnorable(elemType)) return;

  try {
    doc->EndElement();
  }
  catch (const VXMLDocumentModel::InternalError &) {
    ParseException(L"corrupted document tree; unable to complete element");
  }
}


void DocumentConverter::characters(const XMLCh* const chars,
                                   const unsigned int length)
{
  // Should we just ignore these characters?
  if (ignoreDepth > 0) return;

  // Ignore pure whitespace
  unsigned int l;
  for (l = 0; l < length; ++l) {
    XMLCh c = chars[l];
    if (c == ' ' || c == '\n' || c == '\r' || c == '\t') continue;
    break;
  }
  if (l == length) return;

  try {
    // This conversion should be safe.  CDATA may only contain valid XML
    // characters.  These are defined in the XML 1.0 specification as the set
    // 0x9, 0xA, 0xD, 0x20-0xD7FF, 0xE000-0xFFFD, 0x10000-0x10FFFF.
    XMLChToVXIchar data(chars);
    doc->AddContent(data.c_str(), wcslen(data.c_str()));
  }
  catch (const VXMLDocumentModel::OutOfMemory &) {
    ParseException(L"unable to allocate memory for content");
  }
  catch (const VXMLDocumentModel::InternalError &) {
    ParseException(L"corrupted document tree; unable to add content");
  }
}


void DocumentConverter::ignorableWhitespace(const XMLCh* const chars,
                                            const unsigned int l)
{ }


void DocumentConverter::processingInstruction(const XMLCh* const target,
                                              const XMLCh* const data)
{ }


VXMLDocumentRep * DocumentConverter::GetDocument()
{
  VXMLDocumentRep * temp = doc;
  doc = NULL;
  return temp;
}

//#############################################################################

bool DocumentConverter::IsIgnorable(int elemType)
{
  switch (elemType) {
  case NODE_MARK:        // 2.0 spec directs that this element be ignored
  case NODE_META:        // Just ignore meta data
  case PRIV_ELEM_PROS:   // <pros> does not convert to <prosody> cleanly
    return true;
  default:
    break;
  }

  return false;
}


void DocumentConverter::ProcessNodeAttribute(VXMLElementType elemType,
                                             int attrType,
                                             const VXIchar* const value)
{
  // This shouldn't ever happen, if it does, we'll just ignore it.
  if (value == NULL) return;

  switch (elemType) {
  case NODE_BREAK:
    // msecs was replaced by time in 2.0
    if (attrType == PRIV_ATTRIB_MSECS) {
      if (version != 1.0f)
        ParseException(L"the msec attribute on break was dropped after 1.0");
      vxistring temp(value);
      temp += L"ms";
      doc->AddAttribute(ATTRIBUTE_TIME, temp);
      return;
    }
    break;
  case NODE_BLOCK:
  case NODE_FIELD:
  case NODE_INITIAL:
  case NODE_OBJECT:
  case NODE_RECORD:
  case NODE_TRANSFER:
    // We associate a hidden interal name with this element.
    if (attrType == ATTRIBUTE_NAME) attrType = ATTRIBUTE__ITEMNAME;
    break;
  case NODE_CLEAR:
    if (attrType == ATTRIBUTE_NAMELIST && !value[0])
      ParseException(L"the namelist attribute on clear cannot be empty");
    break;
  case NODE_FORM:
  case NODE_MENU:
    // We associate a hidden interal name with this element.
    if (attrType == ATTRIBUTE_ID) attrType = ATTRIBUTE__ITEMNAME;
    break;
  case NODE_FILLED:
    if (doc->GetParentType() != NODE_FORM)
      ParseException(L"attributes valid on filled only at form level");
    if (attrType == ATTRIBUTE_NAMELIST && !value[0])
      ParseException(L"the namelist attribute on filled cannot be empty");
    break;
  case NODE_GRAMMAR:
  case NODE_DTMF:
    if (doc->GetParentType() != NODE_FORM && attrType == ATTRIBUTE_SCOPE)
      ParseException(L"the scope attribute is valid only on grammars at form "
                     L"level");
    break;
  case NODE_PARAGRAPH:
  case NODE_SENTENCE:
    // This is an artifact of converting <div> from 1.0.
    if (attrType == ATTRIBUTE_TYPE) return;
  case NODE_PHONEME:
    if (attrType == PRIV_ATTRIB_PHON) {
      doc->AddAttribute(ATTRIBUTE_ALPHABET, L"ipa");
      attrType = ATTRIBUTE_PH;
    }

    // This is an artifact of the conversion of <sayas> from 1.0.
    if (attrType != ATTRIBUTE_PH && attrType != ATTRIBUTE_ALPHABET) return;
    break;
  case NODE_SAYAS:
    // phon is processed elsewhere and may be ingored
    if (attrType == PRIV_ATTRIB_PHON) return;  // This shouldn't happen.
    // class was replaced by type in 2.0; <sayas> from 1.0 was replaced by
    // <say-as> in 2.0.
    if (attrType == PRIV_ATTRIB_CLASS) {
      vxistring temp(value);

      if (version == 1.0f) { // Convert 1.0 values.
        if (temp == L"phone")
          temp = L"telephone";
        else if (temp == L"literal")
          temp = L"acronym";
        else if (temp == L"digits")
          temp = L"number:digits";
      }

      doc->AddAttribute(ATTRIBUTE_TYPE, temp);
      return;
    }
    break;
  case NODE_SUBDIALOG:
    // We associate a hidden interal name with this element.
    if (attrType == ATTRIBUTE_NAME) attrType = ATTRIBUTE__ITEMNAME;
    // modal was dropped in 2.0.
    if (attrType == ATTRIBUTE_MODAL && version != 1.0f)
      ParseException(L"the modal attribute on subdialog was dropped after "
                     L"1.0");
    break;
  case NODE_VALUE:
    if (attrType == ATTRIBUTE_EXPR) break;

    if (doc->GetParentType() == NODE_LOG)
      ParseException(L"only the expr attribute is valid on value elements "
                     L"within a log element");

    if (doc->GetParentType() == NODE_SAYAS)
      ParseException(L"only the expr attribute is valid on value elements "
                     L"within a say-as element");

    if (version != 1.0f)
      ParseException(L"after 1.0, only the expr attribute is allowed on value "
                     L"elements");

    // Ignore recsrc & mode.
    if (attrType == PRIV_ATTRIB_RECSRC || attrType == ATTRIBUTE_MODE) return;

    // class was replaced by type in 2.0
    if (attrType == PRIV_ATTRIB_CLASS) {
      vxistring temp(value);
      if (temp == L"phone")
        temp = L"telephone";
      else if (temp == L"literal")
        temp = L"acronym";
      else if (temp == L"digits")
        temp = L"number:digits";
      doc->AddAttribute(ATTRIBUTE_TYPE, temp);
      return;
    }
    break;
  case NODE_VXML:
    // version is processed elsewhere and may be ignored.
    if (attrType == PRIV_ATTRIB_VERSION) return;
    // lang was replaced by xml:lang in 2.0
    if (attrType == PRIV_ATTRIB_LANG) {
      if (version != 1.0f)
        ParseException(L"the lang attribute on vxml was dropped after 1.0");
      attrType = ATTRIBUTE_XMLLANG;
    }
    break;
  default:
    break;
  }

  // (4.3) Add the attribute to the element.
  
  if (attrType > PRIV_ATTRIB_RangeStart)
    ParseException(L"internal error during attribute processing");

  doc->AddAttribute(VXMLAttributeType(attrType), value);
}


void DocumentConverter::ProcessNodeFinal(VXMLElementType elemType)
{
  // Convert attributes.
  vxistring attr;

  switch (elemType) {
  case NODE_BLOCK:
  case NODE_FIELD:
  case NODE_FORM:
  case NODE_INITIAL:
  case NODE_MENU:
  case NODE_OBJECT:
  case NODE_RECORD:
  case NODE_SUBDIALOG:
  case NODE_TRANSFER:
    // Name the 'unnamed' elements as neccessary.
    if (!doc->GetAttribute(ATTRIBUTE__ITEMNAME, attr)) {
      vxistring variable;
      VXMLDocumentModel::CreateHiddenVariable(variable);
      doc->AddAttribute(ATTRIBUTE__ITEMNAME, variable);
    }
    break;
  case NODE_FILLED:
    if (!doc->GetAttribute(ATTRIBUTE_MODE, attr))
      doc->AddAttribute(ATTRIBUTE_MODE, L"all");
    break;
  case NODE_VXML:
  case DEFAULTS_ROOT:
    VXMLDocumentModel::CreateHiddenVariable(attr);
    doc->AddAttribute(ATTRIBUTE__ITEMNAME, attr);
    break;
  default:
    break;
  }

  // Generate DTMF sequences for <choice> elements in <menu> if necessary.
  if (elemType == NODE_MENU) {
    if (doc->GetAttribute(ATTRIBUTE_DTMF, attr) && attr == L"true")
      choiceNumber = 1;
    else
      choiceNumber = 0;
  }

  if (elemType == NODE_CHOICE && choiceNumber > 0 && choiceNumber < 10 &&
      !doc->GetAttribute(ATTRIBUTE_DTMF, attr))
  {
    std::basic_stringstream<VXIchar> countString;
    countString << choiceNumber;
    doc->AddAttribute(ATTRIBUTE_DTMF, countString.str());
    ++choiceNumber;
  }
}

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

#include "PropertyList.hpp"
#include "CommonExceptions.hpp"       // for VXIException class
#include "VXIinet.h"
#include "VXML.h"
#include "SimpleLogger.hpp"           // for SimpleLogger
#include "DocumentParser.hpp"         // for string constants
#include "DocumentModel.hpp"
#include <sstream>

// ------*---------*---------*---------*---------*---------*---------*---------

// The use of spaces here creates identifiers which cannot be specified
// within the property or meta tags.
const VXIchar * const PropertyList::BaseURI     = L" base";
const VXIchar * const PropertyList::Language    = L" lang";
const VXIchar * const PropertyList::AbsoluteURI = L" absoluteURI";
const VXIchar * const PropertyList::AcceptedLang= L"accepted-language";


static vxistring toString(const VXIString * s)
{
  if (s == NULL) return L"";
  const VXIchar * temp = VXIStringCStr(s);
  if (temp == NULL) return L"";
  return temp;
}

static vxistring toString(const VXIchar * s)
{
  if (s == NULL) return L"";
  return s;
}

// ------*---------*---------*---------*---------*---------*---------*---------

PropertyList::PropertyList(const SimpleLogger & l) : log(l)
{
  properties.reserve(LAST_PROP);
  while (properties.size() < LAST_PROP)
    properties.push_back(STRINGMAP());
}


PropertyList::PropertyList(const PropertyList & p) : log(p.log)
{
  properties = p.properties;
}


PropertyList& PropertyList::operator=(const PropertyList & x)
{
  if (this != &x) {
    properties = x.properties;
  }

  return *this;
}


const VXIchar* PropertyList::GetProperty(const VXIchar * key,
                                         PropertyLevel L) const
{
  int pos = L;
  do {
    if (pos == LAST_PROP) --pos;
    STRINGMAP::const_iterator i = properties[pos].find(key);
    if (i != properties[pos].end()) return (*i).second.c_str();
  } while (--pos >= 0);

  return NULL;
}


bool PropertyList::SetProperty(const VXIchar * key, const VXIchar * value,
                               PropertyLevel level)
{
  if (level == LAST_PROP || key == NULL || value == NULL) return false;

  STRINGMAP & propmap = properties[level];
  propmap[key] = value;

  return true;
}


void PropertyList::SetProperties(const VXMLElement & doc, PropertyLevel level,
                                 const VXIMapHolder & docProps)
{
  // (0) Check the arguments

  if (doc == 0) return;

  if (properties.size() != LAST_PROP) {
    log.LogError(999, SimpleLogger::MESSAGE,
                 L"internal property list corrupted");
    throw VXIException::Fatal();
  }

  // (1) Clear all properties at this level and above.

  for (unsigned int i = level; i < LAST_PROP; ++i)
    properties[i].clear();
  STRINGMAP & propmap = properties[level];

  // (2) Add document properties.

  if (docProps.GetValue() != NULL) {
    const VXIValue * val;
    const VXIString * str;

    // (2.1) Handle document base URI.
    val = VXIMapGetProperty(docProps.GetValue(), PropertyList::BaseURI);
    str = reinterpret_cast<const VXIString *>(val);
    if (VXIStringLength(str) != 0)
      propmap.insert(propmap.end(),
                     STRINGMAP::value_type(PropertyList::BaseURI,
                                           toString(str)));

    // (2.2) Handle document language.  If the document property is not
    // specified, the value of the xml:lang property will be used.

    val = VXIMapGetProperty(docProps.GetValue(), PropertyList::Language);
    str = reinterpret_cast<const VXIString *>(val);
    if (VXIStringLength(str) != 0)
      propmap.insert(propmap.end(),
                     STRINGMAP::value_type(PropertyList::Language,
                                           toString(str)));
    else {
      const VXIchar * lang = GetProperty(L"xml:lang");
      if (lang != NULL)
        propmap.insert(propmap.end(),
                       STRINGMAP::value_type(PropertyList::Language,
                                             toString(lang)));
    }
  }

  // (3) Walk through this level.  Find all <property> nodes and set the
  // corresponding values.
  for (VXMLNodeIterator it(doc); it; ++it) {
    VXMLNode child = *it;

    if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);
    VXMLElementType nodeName = elem.GetName();
    if (nodeName == NODE_PROPERTY) {
      vxistring name;
      vxistring value;
      elem.GetAttribute(ATTRIBUTE_NAME, name);
      elem.GetAttribute(ATTRIBUTE_VALUE, value);

      // This added the name / value pair, converting to vxistrings.
      propmap.insert(propmap.end(), STRINGMAP::value_type(name, value));
    }
  }
}


//****************************************************************
//* Fetchobj building routines
//****************************************************************

bool PropertyList::ConvertTimeToMilliseconds(const SimpleLogger & log,
                                             const vxistring & time,
                                             VXIint & result)
{
  result = 0;

  if (time.empty()) {
    log.LogDiagnostic(0, L"PropertyList::ConvertTimeToMilliseconds - empty");
    return false;
  }

  int len = time.length();
  for (int i = 0; i < len; ++i) {
    char c;
    switch (time[i]) {
    case L'+':  continue;
    case L'0':  c = 0;  break;
    case L'1':  c = 1;  break;
    case L'2':  c = 2;  break;
    case L'3':  c = 3;  break;
    case L'4':  c = 4;  break;
    case L'5':  c = 5;  break;
    case L'6':  c = 6;  break;
    case L'7':  c = 7;  break;
    case L'8':  c = 8;  break;
    case L'9':  c = 9;  break;
    default:
      const vxistring units = time.substr(i, time.length() - i);
      if (units == L"ms")
	return true;
      if (units == L"s") {
        result *= 1000;
        return true;
      }

      // Otherwise the unit type is not recognized.

      log.StartDiagnostic(0) << L"PropertyList::ConvertTimeToMilliseconds - "
        L"Invalid units in value \"" << time << L"\".";
      log.EndDiagnostic();

      return false;
    }

    result = 10*result + c;
  }

  // No units were specified.  Milliseconds are assumed.
  return true;
}


bool PropertyList::ConvertValueToFraction(const SimpleLogger & log,
                                          const vxistring & value,
                                          VXIflt32& result)
{
#if defined(__GNUC__) && (__GNUC__ <= 2 || (__GNUC__ == 3 && __GNUC_MINOR__ == 0))
  // The G++ implementation of basic_stringstream is faulty.
  VXIchar * temp;
  result = VXIflt32(wcstod(value.c_str(), &temp));
#else
  std::basic_stringstream<VXIchar> attrStream(value);
  attrStream >> result;
  if (attrStream.bad()) result = -1;
#endif

  if (result < 0.0 || result > 1.0) {
    log.StartDiagnostic(0) << L"PropertyList::ConvertValueToFraction - "
      L"Unable to convert value \"" << value << L"\".";
    log.EndDiagnostic();
    return false;
  }

  return true;
}


void PropertyList::GetFetchobjCacheAttrs(const VXMLElement & elem,
                                         PropertyList::CacheAttrType type,
                                         VXIMapHolder & fetchobj) const
{
  vxistring attr;
  const VXIchar * propName = NULL;

  // (1) Attribute: maxage

  // (1.1) Get the attribute - first locally, then from the defaults.
  elem.GetAttribute(ATTRIBUTE_MAXAGE, attr);
  if (attr.empty()) {
    switch (type) {
    case PropertyList::Audio:      propName = L"audiomaxage";     break;
    case PropertyList::Document:   propName = L"documentmaxage";  break;
    case PropertyList::Grammar:    propName = L"grammarmaxage";   break;
    case PropertyList::Object:     propName = L"objectmaxage";    break;
    case PropertyList::Script:     propName = L"scriptmaxage";    break;
    }

    attr = toString(GetProperty(propName));
  }

  // (1.2) Process the value.
  if (!attr.empty()) {
    VXIint value = -1;

#if defined(__GNUC__) && (__GNUC__ <= 2 || (__GNUC__ == 3 && __GNUC_MINOR__ == 0))
  // The G++ implementation of basic_stringstream is faulty.
    VXIchar * temp;
    value = VXIint(wcstol(attr.c_str(), &temp, 10));
#else
    std::basic_stringstream<VXIchar> attrStream(attr);
    attrStream >> value;
#endif

    if (value < 0) {
      log.StartDiagnostic(0) << L"PropertyList::GetFetchobjCacheAttrs - "
        L"Invalid value for " << propName << L" of " << attr << L".";
      log.EndDiagnostic();
      throw VXIException::InterpreterEvent(EV_ERROR_SEMANTIC,
                                           L"invalid maxage value");
    }

    // (1.3) Add value to fetchobj
    VXIInteger * val = VXIIntegerCreate(value);
    if (val == NULL) throw VXIException::OutOfMemory();
    // UNUSED VARIABLE VXIvalueResult r =
                  VXIMapSetProperty(fetchobj.GetValue(),
                                         INET_CACHE_CONTROL_MAX_AGE,
                                         reinterpret_cast<VXIValue*>(val));
  }

  // (2) Attribute: maxstale

  // (2.1) Get the attribute - first locally, then from the defaults.
  attr.erase();
  elem.GetAttribute(ATTRIBUTE_MAXSTALE, attr);
  if (attr.empty()) {
    switch (type) {
    case PropertyList::Audio:      propName = L"audiomaxstale";     break;
    case PropertyList::Document:   propName = L"documentmaxstale";  break;
    case PropertyList::Grammar:    propName = L"grammarmaxstale";   break;
    case PropertyList::Object:     propName = L"objectmaxstale";    break;
    case PropertyList::Script:     propName = L"scriptmaxstale";    break;
    }

    attr = toString(GetProperty(propName));
  }

  // (2.2) Process the value.
  if (!attr.empty()) {
    VXIint value = -1;

#if defined(__GNUC__) && (__GNUC__ <= 2 || (__GNUC__ == 3 && __GNUC_MINOR__ == 0))
  // The G++ implementation of basic_stringstream is faulty.
    VXIchar * temp;
    value = VXIint(wcstol(attr.c_str(), &temp, 10));
#else
    std::basic_stringstream<VXIchar> attrStream(attr);
    attrStream >> value;
#endif

    if (value < 0) {
      log.StartDiagnostic(0) << L"PropertyList::GetFetchobjCacheAttrs - "
        L"Invalid value for " << propName << L" of " << attr << L".";
      log.EndDiagnostic();
      throw VXIException::InterpreterEvent(EV_ERROR_SEMANTIC,
                                           L"invalid maxstale value");
    }

    // (2.3) Add value to fetchobj
    VXIInteger * val = VXIIntegerCreate(value);
    if (val == NULL) throw VXIException::OutOfMemory();
    // UNUSED VARIABLE VXIvalueResult r =
	               VXIMapSetProperty(fetchobj.GetValue(),
                                         INET_CACHE_CONTROL_MAX_AGE,
                                         reinterpret_cast<VXIValue*>(val));
  }

  // (3) Attribute: fetchtimeout

  // (3.1) Get the attribute - first locally, then from the defaults.
  attr.erase();
  elem.GetAttribute(ATTRIBUTE_FETCHTIMEOUT, attr);
  if (attr.empty())
    attr = toString(GetProperty(L"fetchtimeout"));

  // (3.2) Process the value.
  if (!attr.empty()) {
    VXIint value;
    if (!ConvertTimeToMilliseconds(log, attr, value)) {
      log.StartDiagnostic(0) << L"PropertyList::GetFetchobjTimeout - "
        L"Invalid value for fetchtimeout of " << attr << L".";
      log.EndDiagnostic();
      throw VXIException::InterpreterEvent(EV_ERROR_SEMANTIC,
                                           L"invalid fetchtimeout value");
    }

    // (3.3) Add value to fetchobj
    VXIInteger * str = VXIIntegerCreate(value);
    if (str == NULL) throw VXIException::OutOfMemory();
    VXIvalueResult r = VXIMapSetProperty(fetchobj.GetValue(),INET_TIMEOUT_OPEN,
                                         reinterpret_cast<VXIValue*>(str));
    if (r == VXIvalue_RESULT_SUCCESS) {
      str = VXIIntegerCreate(value);
      r = VXIMapSetProperty(fetchobj.GetValue(), INET_TIMEOUT_IO,
                            reinterpret_cast<VXIValue*>(str));
    }
  }

  // TBD #pragma message ("PropertyList::GetFetchobjCacheAttrs - ignoring fetchhint")
}


bool PropertyList::GetFetchobjSubmitAttributes(const VXMLElement & elem,
                                               VXIMapHolder & submitData,
                                               VXIMapHolder & fetchobj) const
{
  // (1) Set submit method.

  // (1.1) Get the attribute - first locally, then from the defaults.
  vxistring attr;
  elem.GetAttribute(ATTRIBUTE_METHOD, attr);

  // (1.2) Process the value.
  const VXIchar * value = NULL;
  if (attr == L"get" || attr.empty())
    value = INET_SUBMIT_METHOD_GET;
  else if (attr == L"post")
    value = INET_SUBMIT_METHOD_POST;
  else {
    log.StartDiagnostic(0) << L"PropertyList::GetFetchobjSubmitMethod - "
      L"Bad value (" << ATTRIBUTE_METHOD << L" = \"" << attr << L"\"), "
      L"defaulting to " << INET_SUBMIT_METHOD_DEFAULT << L".";
    log.EndDiagnostic();
    value = INET_SUBMIT_METHOD_DEFAULT;
  }

  // (1.3) Add value to fetchobj
  VXIString * str = VXIStringCreate(value);
  if (str == NULL) throw VXIException::OutOfMemory();
  VXIvalueResult r = VXIMapSetProperty(fetchobj.GetValue(), INET_SUBMIT_METHOD,
                                       reinterpret_cast<VXIValue*>(str));
  if (r != VXIvalue_RESULT_SUCCESS) return false;

  // (2) Get encoding for the submit.

  // (2.1) Get the attribute.
  if (!elem.GetAttribute(ATTRIBUTE_ENCTYPE, attr))
    attr = L"application/x-www-form-urlencoded";

  // (2.2) Add value to fetchobj
  str = VXIStringCreate(attr.c_str());
  if (str == NULL) throw VXIException::OutOfMemory();
  r = VXIMapSetProperty(fetchobj.GetValue(), INET_SUBMIT_MIME_TYPE,
                        reinterpret_cast<VXIValue*>(str));
  if (r != VXIvalue_RESULT_SUCCESS) return false;

  // (3) Set the submit data.
  r = VXIMapSetProperty(fetchobj.GetValue(), INET_URL_QUERY_ARGS,
                        reinterpret_cast<VXIValue*>(submitData.Release()));

  return r == VXIvalue_RESULT_SUCCESS;
}


bool PropertyList::GetFetchobjBase(VXIMapHolder & fetchobj) const
{
  const VXIchar * base = GetProperty(PropertyList::BaseURI);
  if (base != NULL)
    AddParamValue(fetchobj, INET_URL_BASE, base);

  return true;
}


bool PropertyList::GetFetchobjURIs(const VXMLElement & elem,
                                   VXIMapHolder & fetchobj,
                                   vxistring & url,
                                   vxistring & fragment) const
{
  fragment = L"";

  // The URI may be decomposed into three elements - the base, a relative
  // address, and a fragment corresponding to a dialog within that module.

  // (1) First the base.
  GetFetchobjBase(fetchobj);

  // (2) Then determine if there is a fragment at all.
  if (url.empty()) return false;

  vxistring::size_type pos = url.find('#');
  if (pos == vxistring::npos) return true;

  // (3) There was a fragment.
  if (pos + 1 < url.length())
    fragment = url.substr(pos + 1, url.length() - pos - 1);

  url.erase(pos);

  return true;
}

//****************************************************************
//* Recognition & Grammar property related.
//****************************************************************

bool PropertyList::PushProperties(const VXMLElement & doc)
{
  // (1) Find the first empty level.
  unsigned int i;
  for (i = 0; i < LAST_PROP; ++i)
    if (properties[i].empty()) break;
  if (i == LAST_PROP) {
    log.LogError(999, SimpleLogger::MESSAGE, L"property list size exceeded");
    return false;
  }
  STRINGMAP & propmap = properties[i];

  // (2) Add properties at that point.

  bool foundSomething = false;

  for (VXMLNodeIterator it(doc); it; ++it) {
    VXMLNode child = *it;

    if (child.GetType() != VXMLNode::Type_VXMLElement) continue;
    const VXMLElement & elem = reinterpret_cast<const VXMLElement &>(child);
    VXMLElementType nodeName = elem.GetName();
    if (nodeName == NODE_PROPERTY) {
      vxistring name;
      vxistring value;
      elem.GetAttribute(ATTRIBUTE_NAME, name);
      elem.GetAttribute(ATTRIBUTE_VALUE, value);

      // This added the name / value pair, converting to vxistrings.
      propmap.insert(propmap.end(), STRINGMAP::value_type(name, value));

      foundSomething = true;
    }
  }

  return foundSomething;
}


void PropertyList::PopProperties()
{
  unsigned int i;
  for (i = 0; i < LAST_PROP; ++i)
    if (properties[i].empty()) break;
  if (i == 0) return;

  properties[i-1].clear();
}


// This starts at the lowest level, inserting entries into a newly created map.
// Values at higher levels will overwrite keys with identical names.
//
void PropertyList::GetProperties(VXIMapHolder & m) const
{
  // (1) Collapse the values down into a single map.
  STRINGMAP collapsed;
  PROPERTIES::const_iterator i;
  for (i = properties.begin(); i != properties.end(); ++i) {
    for (STRINGMAP::const_iterator j = (*i).begin(); j != (*i).end(); ++j) {
      collapsed[(*j).first] = (*j).second;
    }
  }

  // (2) Create a new map & copy the values over.
  STRINGMAP::const_iterator j;
  for (j = collapsed.begin(); j != collapsed.end(); ++j) {
    VXIString * value = VXIStringCreate((*j).second.c_str());
    if (value == NULL) throw VXIException::OutOfMemory();

    // Set this key.
    VXIMapSetProperty(m.GetValue(), (*j).first.c_str(),
                      reinterpret_cast<VXIValue *>(value));
  }
}

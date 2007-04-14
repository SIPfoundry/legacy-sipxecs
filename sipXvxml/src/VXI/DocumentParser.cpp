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
 ***********************************************************************
 *
 * Routines to handle fetching of documents and resources.
 *
 ***********************************************************************/

#include "DocumentParser.hpp"
#include "CommonExceptions.hpp"
#include "VXIinet.h"
#include <util/PlatformUtils.hpp>
#include <util/TransService.hpp>
#include <parsers/SAXParser.hpp>
#include <framework/MemBufInputSource.hpp>
#include "SimpleLogger.hpp"
#include "DTD.hpp"
#include "DefaultsDTD.hpp"
#include "Defaults.hpp"
#include "VXML.h"                    // for attribute names
#include "VXItrd.h"                  // for ThreadYield
#include <sax/ErrorHandler.hpp>      // by DOMTreeErrorReporter
#include <sax/SAXParseException.hpp> // by DOMTreeErrorReporter
#include <sax/EntityResolver.hpp>    // by DTDResolver
#include "DocumentConverter.hpp"     // for DocumentConverter
#include "DocumentModel.hpp"
#include "PropertyList.hpp"
#include "XMLChConverter.hpp"

//#############################################################################
// Utilities - these are specific to Xerces
//#############################################################################

class DOMTreeErrorReporter : public ErrorHandler {
public:
  DOMTreeErrorReporter()  { }
  ~DOMTreeErrorReporter() { }
  
  void warning(const SAXParseException& toCatch)     { /* Ignore */ }
  void fatalError(const SAXParseException& toCatch)  { error(toCatch); }
  void resetErrors() { }

  void error(const SAXParseException & toCatch)
  { throw SAXParseException(toCatch); }

private:
  DOMTreeErrorReporter(const DOMTreeErrorReporter &);
  void operator=(const DOMTreeErrorReporter &);
};


class DTDResolver : public EntityResolver {
public:
  virtual ~DTDResolver() { }
  DTDResolver() { }

  virtual InputSource * resolveEntity(const XMLCh * const publicId,
                                      const XMLCh * const systemId)
  {
    // The size - 1 is used to drop the final \0.
    if (Compare(publicId, L"SB_Defaults")) {
      VXIcharToXMLCh name(L"VXML Defaults DTD (for SB 1.0)");
      return new MemBufInputSource(VXML_DEFAULTS_DTD,
                                   VXML_DEFAULTS_DTD_SIZE - 1,
                                   name.c_str(), false);
    }

    VXIcharToXMLCh name(L"VXML DTD (for SB 1.0)");
    return new MemBufInputSource(VXML_DTD, VXML_DTD_SIZE - 1,
                                 name.c_str(), false);
  }
};

//#############################################################################
// Document Parser
//#############################################################################

bool DocumentParser::Initialize()
{
  try {
    XMLPlatformUtils::Initialize();
    if (!VXMLDocumentModel::Initialize()) return false;
    DocumentConverter::Initialize();
  }
  catch (const XMLException &) {
    return false;
  }

  return true;
}


void DocumentParser::Deinitialize()
{
  try {
    DocumentConverter::Deinitialize();
    VXMLDocumentModel::Deinitialize();
    XMLPlatformUtils::Terminate();
  }
  catch (const XMLException &) {
    // do nothing
  }
}


DocumentParser::DocumentParser()
  : lastParse(DocumentParser::NOTHING), parser(NULL), converter(NULL)
{
  converter = new DocumentConverter();
  if (converter == NULL) throw VXIException::OutOfMemory();

  parser = new SAXParser;
  if (parser == NULL) {
    delete converter;
    converter = NULL;
    throw VXIException::OutOfMemory();
  }

  DTDResolver * dtd = new DTDResolver();
  if (dtd == NULL) {
    delete converter;
    converter = NULL;
    delete parser;
    parser = NULL;
    throw VXIException::OutOfMemory();
  }
  parser->setEntityResolver(dtd);

  parser->setDoNamespaces(false);
  // parser->setValidationScheme(SAXParser::Val_Always);
  parser->setValidationScheme(SAXParser::Val_Auto);

  ErrorHandler *errReporter = new DOMTreeErrorReporter();
  parser->setErrorHandler(errReporter);

  parser->setDocumentHandler(converter);
}


DocumentParser::~DocumentParser()
{
  if (parser != NULL) {
    const ErrorHandler * reporter = parser->getErrorHandler();
    delete reporter;
    reporter = NULL;

    const EntityResolver * resolver = parser->getEntityResolver();
    delete resolver;
    resolver = NULL;

    delete parser;
    parser = NULL;
    delete converter;
    converter = NULL;
  }
}

//****************************************************************************
// FetchBuffer
//****************************************************************************

// 1: Invalid parameter
// 2: Unable to open URL
// 3: Unable to read from URL
int DocumentParser::FetchBuffer(const VXIchar * url,
                                const VXIMapHolder & properties,
                                VXIinetInterface * inet,
                                SimpleLogger & log,
                                const VXIbyte * & result,
                                VXIulong & read,
                                vxistring & docURL)
{
  if (log.IsLogging(2)) {
    log.StartDiagnostic(2) << L"DocumentParser::FetchBuffer(" << url << L")";
    log.EndDiagnostic();
  }

  if (inet == NULL || url == NULL || wcslen(url) == 0) return 1;

  // (1) Open URL
  VXIinetStream * stream;

  VXIMapHolder streamInfo;
  if (streamInfo.GetValue() == NULL) return -1;

  if (inet->Open(inet, L"vxi", url, INET_MODE_READ, 0, properties.GetValue(),
                 streamInfo.GetValue(), &stream) != 0)
  {
    if (log.IsLogging(0)) {
      log.StartDiagnostic(0) << L"DocumentParser::FetchBuffer - could not "
        L"open URL: " << url;
      log.EndDiagnostic();
    }
    return 2;
  }
  
  // (2) Determine document size & read into local memory buffer.
  const VXIValue * tempURL = NULL;
  tempURL = VXIMapGetProperty(streamInfo.GetValue(), INET_INFO_ABSOLUTE_NAME);
  if (tempURL == NULL || VXIValueGetType(tempURL) != VALUE_STRING) {
    inet->Close(inet, &stream);
    if (log.IsLogging(0)) {
      log.StartDiagnostic(0) << L"DocumentParser::FetchBuffer - could not "
        L"retrieve absolute path of document at URL: " << url;
      log.EndDiagnostic();
    }
    return 2;
  }
  docURL = VXIStringCStr(reinterpret_cast<const VXIString *>(tempURL));

  const VXIValue * tempSize = NULL;
  tempSize = VXIMapGetProperty(streamInfo.GetValue(), INET_INFO_SIZE_BYTES);
  if (tempSize == NULL || VXIValueGetType(tempSize) != VALUE_INTEGER) {
    inet->Close(inet, &stream);
    if (log.IsLogging(0)) {
      log.StartDiagnostic(0) << L"DocumentParser::FetchBuffer - could not "
        L"retrieve size of document at URL: " << url;
      log.EndDiagnostic();
    }
    return 2;
  }

  VXIint32 bufSize
    = VXIIntegerValue(reinterpret_cast<const VXIInteger *>(tempSize));

  if (bufSize < 2047)
    bufSize = 2047;

  ++bufSize;
  VXIbyte * buffer = new VXIbyte[bufSize];

  bool reachedEnd = false; 
  read = 0;

  while (!reachedEnd) {
    VXIulong bytesRead = 0;
    switch (inet->Read(inet, buffer+read, bufSize-read, &bytesRead, stream)) {
    case VXIinet_RESULT_SUCCESS:
      read += bytesRead;
      break;
    case VXIinet_RESULT_END_OF_STREAM:
      read += bytesRead;
      reachedEnd = true;  // exit while
      break;
    case VXIinet_RESULT_WOULD_BLOCK:
      VXItrdThreadYield();
      break;
    default:
      inet->Close(inet, &stream);
      delete[] buffer;
      buffer = NULL;

      log.LogDiagnostic(0, L"DocumentParser::FetchBuffer - "
                        L"could not read from URL.");
      return 3;
    }

    if (read == static_cast<VXIulong>(bufSize)) {
      // The number of bytes read exceeds the number expected.  Double the
      // size and keep reading.
      VXIbyte * temp = new VXIbyte[2*bufSize];
      memcpy(static_cast<void *>(temp), static_cast<void *>(buffer),
             bufSize * sizeof(VXIbyte));
      delete[] buffer;
      buffer = temp;
      bufSize *= 2;
    }
  }

  inet->Close(inet, &stream);
  result = buffer;

  log.LogDiagnostic(2, L"DocumentParser::FetchBuffer - success");

  return 0;
}


void DocumentParser::ReleaseBuffer(const VXIbyte * & buffer)
{
  if (buffer != PLATFORM_DEFAULTS_DOC)
    delete[] const_cast<VXIbyte *>(buffer);

  buffer = NULL;
}


// -2: Internal error
int DocumentParser::FetchDocument(const VXIchar * url,
                                  const VXIMapHolder & properties,
                                  VXIinetInterface * inet,
                                  SimpleLogger & log,
                                  VXMLDocument & document,
                                  VXIMapHolder & docProperties,
                                  bool isDefaults)
{
  int result;

  if (log.IsLogging(2)) {
    log.StartDiagnostic(2) << L"DocumentParser::FetchDocument(" << url << L")";
    log.EndDiagnostic();
  }

  // (1) Load the VXML DTD for validation.  This will override an externally
  // specified DTD if the user provides a link.

  try {
    if (isDefaults && lastParse != DocumentParser::DEFAULTS) {
      MemBufInputSource tmpMemBufInputSource(DUMMY_VXML_DEFAULTS_DOC,
                                          DUMMY_VXML_DEFAULTS_DOC_SIZE,
                                          "vxml 1.0 defaults");
      parser->parse(tmpMemBufInputSource);
      lastParse = DocumentParser::DEFAULTS;
    }
    else if (!isDefaults && lastParse != DocumentParser::DOCUMENT) {
      MemBufInputSource tmpMemBufInputSource(DUMMY_VXML_DOC,
                                          DUMMY_VXML_DOC_SIZE,
                                          "vxml 1.0 dtd");
      parser->parse(tmpMemBufInputSource);
      lastParse = DocumentParser::DOCUMENT;
    }
  }
  catch (const XMLException & exception) {
    XMLChToVXIchar message(exception.getMessage());
    log.StartDiagnostic(0) << L"DocumentParser::FetchDocument - XML parsing "
      L"error from DOM: " << message;
    log.EndDiagnostic();
    log.LogError(999, SimpleLogger::MESSAGE, L"unable to load VXML DTD");
    return 4;
  }
  catch (const SAXParseException & exception) {
    XMLChToVXIchar systemId(exception.getSystemId());
    XMLChToVXIchar message(exception.getMessage());
    log.StartDiagnostic(0) << L"DocumentParser::FetchDocument - Parse error "
                           << L"in file \"" 
			   << systemId 
                           << L"\", line " << exception.getLineNumber()
                           << L", column " << exception.getColumnNumber()
                           << L" - " << message;
    log.EndDiagnostic();
    log.LogError(999, SimpleLogger::MESSAGE, L"unable to load VXML DTD");
    return 4;
  }

  // (2) Load the url into memory.

  const VXIbyte * buffer;
  VXIulong bufSize;
  vxistring docURL;

  if (isDefaults && wcslen(url) == 0) {
    buffer  = PLATFORM_DEFAULTS_DOC;
    bufSize = PLATFORM_DEFAULTS_DOC_SIZE;
    docURL = L"builtin defaults";
    result = 0;
  }
  else {
    result = DocumentParser::FetchBuffer(url, properties, inet, log,
                                         buffer, bufSize, docURL);
  }

  if (result != 0) {
    if (log.IsLogging(0)) {
      log.StartDiagnostic(0) << L"DocumentParser::FetchDocument - exiting "
        L"with error result " << result;
      log.EndDiagnostic();
    }
    return result; // may return { -1, 1, 2, 3 }
  }

  // (3) Create memory buffer for Xerces.
  VXIcharToXMLCh membufURL(url);
  MemBufInputSource *inputSource = new MemBufInputSource(buffer, bufSize,
                                                         membufURL.c_str());
  if (inputSource == NULL) {
    DocumentParser::ReleaseBuffer(buffer);
    return -1;
  }

  // (4) Convert URL into DOM document.
  try {
    parser->parse(*inputSource);
  }
  catch (const XMLException & exception) {
    delete inputSource;
    inputSource = NULL;
    DocumentParser::ReleaseBuffer(buffer);
    if (log.IsLogging(0)) {
      XMLChToVXIchar message(exception.getMessage());
      log.StartDiagnostic(0) << L"DocumentParser::FetchDocument - XML parsing "
        L"error from DOM: " << message;
      log.EndDiagnostic();
    }
    return 4;
  }
  catch (const SAXParseException & exception) {
    delete inputSource;
    inputSource = NULL;
    DocumentParser::ReleaseBuffer(buffer);
    if (log.IsLogging(0)) {
      XMLChToVXIchar systemId(exception.getSystemId());
      XMLChToVXIchar message(exception.getMessage());
      log.StartDiagnostic(0) << L"DocumentParser::FetchDocument - Parse error "
                             << L"in file \"" 
			     << systemId
                             << L"\", line " << exception.getLineNumber()
                             << L", column " << exception.getColumnNumber()
                             << L" - " 
			     << message;
      log.EndDiagnostic();
    }
    return 4;
  }

  delete inputSource;
  inputSource = NULL;
  DocumentParser::ReleaseBuffer(buffer);

  // (5) Parse was successful, process document.  We want only the top level
  // <vxml> node.

  const VXMLDocument doc(converter->GetDocument());
  const VXMLElement root = doc.GetRoot();
  VXMLElementType nodeName = root.GetName();

  // If we're looking for the defaults, we can exit early.
  if (isDefaults && nodeName == DEFAULTS_ROOT) {
    log.LogDiagnostic(2, L"DocumentParser::FetchDocument - success");
    document = doc;
    return 0;
  }
  else if (nodeName != NODE_VXML) {
    document = VXMLDocument();
    if (log.IsLogging(0)) {
      log.StartDiagnostic(0) << L"DocumentParser::FetchDocument - unable to "
        L"find " << NODE_VXML << L" in document.";
      log.EndDiagnostic();
    }
    return 4;
  }

  vxistring temp;

  // If the properties map is NULL, don't bother with the remaining settings
  if (docProperties.GetValue() != NULL) {
    // Retrieve the properties and put them into the map.

    VXIString * str = NULL;
    temp = docURL;
    str = VXIStringCreate(temp.c_str());
    if (str == NULL) throw VXIException::OutOfMemory();
    VXIMapSetProperty(docProperties.GetValue(), PropertyList::AbsoluteURI,
                      reinterpret_cast<VXIValue *>(str));

    if (!root.GetAttribute(ATTRIBUTE_BASE, temp)) 
      temp = docURL;
    str = VXIStringCreate(temp.c_str());
    if (str == NULL) throw VXIException::OutOfMemory();
    VXIMapSetProperty(docProperties.GetValue(), PropertyList::BaseURI,
                      reinterpret_cast<VXIValue *>(str));

    if (root.GetAttribute(ATTRIBUTE_XMLLANG, temp)) {
      str = VXIStringCreate(temp.c_str());
      if (str == NULL) throw VXIException::OutOfMemory();
      VXIMapSetProperty(docProperties.GetValue(), PropertyList::Language,
                        reinterpret_cast<VXIValue *>(str));
    }
  }

  log.LogDiagnostic(2, L"DocumentParser::FetchDocument - success");

  document = doc;
  return 0;
}


int DocumentParser::FetchContent(const VXIchar * uri,
                                 const VXIMapHolder & properties,
                                 VXIinetInterface * inet,
                                 SimpleLogger & log,
                                 const vxistring & encoding,
                                 vxistring & content)
{

  const VXIbyte * buffer;
  VXIulong bufSize;
  vxistring docURL;

  // (1) Retrieve the URI.
  switch (DocumentParser::FetchBuffer(uri, properties, inet, log,
                                      buffer, bufSize, docURL)) {
  case -1: // Out of memory?
    return -1;
  case  0: // Success
    break;
  case  2: // Unable to open URI
    return 2;
  case  3: // Unable to read from URI
    return 3;
  case  1: // Invalid parameter
  default:
    return 1;
  }

  // (2) Create a transcoder for the requested type.

  VXIcharToXMLCh encName(encoding.c_str());
  XMLTransService::Codes failReason;
  XMLTranscoder* transcoder = 
      XMLPlatformUtils::fgTransService->makeNewTranscoderFor(encName.c_str(),
                                                             failReason,
                                                             8*1064);
  if (transcoder == NULL) return 4;

  // (3) Allocate memory for the conversion.

  XMLCh * convertedString = new XMLCh[bufSize+1];
  unsigned char* charSizes = new unsigned char[bufSize];

  if (convertedString == NULL || charSizes == NULL) {
    delete[] convertedString;
    convertedString = NULL;
    delete[] charSizes;
    charSizes = NULL;
    return -1;
  }

  // (4) Transcode the values into our string.

  unsigned int bytesEaten;
  unsigned int charsDone = transcoder->transcodeFrom(buffer, bufSize,
                                                     convertedString, bufSize,
                                                     bytesEaten, charSizes);

  // (5) Finally convert from XMLCh to VXIchar.
  convertedString[charsDone] = '\0';  // Add terminator. 
  XMLChToVXIchar result(convertedString);

  // (6) Done.  Release memory.

  content = result.c_str();
  delete[] convertedString;
  convertedString  = NULL;
  delete[] charSizes;
  charSizes  = NULL;
  ReleaseBuffer(buffer);

  return 0;
}

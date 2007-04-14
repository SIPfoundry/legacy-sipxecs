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

#include "VXIvalue.h"
#include <string>
#include "parsers/SAXParser.hpp"

XERCES_CPP_NAMESPACE_USE

XERCES_CPP_NAMESPACE_BEGIN
class SAXParser ;
XERCES_CPP_NAMESPACE_END
class DocumentConverter;
class VXMLDocument;
class SimpleLogger;
extern "C" struct VXIinetInterface;

typedef std::basic_string<VXIchar> vxistring;


class DocumentParser {
public:
  static bool Initialize();
  // One time initialization of DocumentParser interface.
  //
  // Returns: True - initialization succeeded.
  //          False - initialization failed.

  static void Deinitialize();
  // One time cleanup of DocumentParser interface.

  DocumentParser();   // May throw VXIException::OutOfMemory
  ~DocumentParser();

public:
  int FetchDocument(const VXIchar * uri, const VXIMapHolder & properties,
                    VXIinetInterface * inet, SimpleLogger & log,
                    VXMLDocument & document, VXIMapHolder & docProperties,
                    bool isDefaults = false);
  // This retrieves a given URI and converts its contents into a VXMLElement
  // tree.  The returned 'document' will contain only the contents of the
  // <vxml> section.  Thus, only documents containing <vxml> are supported.
  //
  // Returns: -1 Out of memory?
  //           0 Success
  //           1 Invalid parameter
  //           2 Unable to open URI
  //           3 Unable to read from URI
  //           4 Unable to parse contents of URI

  int FetchContent(const VXIchar * uri, const VXIMapHolder & properties,
                   VXIinetInterface * inet, SimpleLogger & log,
                   const vxistring & encoding, vxistring & content);
  // Returns: -1 Out of memory?
  //           0 Success
  //           1 Invalid parameter, such as unsupported encoding
  //           2 Unable to open URI
  //           3 Unable to read from URI

private:
  static int FetchBuffer(const VXIchar * uri, const VXIMapHolder & properties,
                         VXIinetInterface * inet, SimpleLogger & log,
                         const VXIbyte * & buffer, VXIulong & bufferSize,
                         vxistring & docURI);
  static void ReleaseBuffer(const VXIbyte * & buffer);
  // FetchBuffer will create a memory buffer containing the contents of the
  // URI.  ReleaseBuffer must be called by the consumer to cleanup this memory.
  //
  // Returns: -1 Out of memory?
  //           0 Success
  //           1 Invalid parameter
  //           2 Unable to open URI
  //           3 Unable to read from URI
  //           4 Unsupported encoding

private:
  enum {
    NOTHING,
    DEFAULTS,
    DOCUMENT
  } lastParse;

  SAXParser * parser;
  DocumentConverter * converter;
};

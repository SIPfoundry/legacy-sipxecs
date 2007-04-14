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

#include <sax/DocumentHandler.hpp>
#include "VXML.h"

XERCES_CPP_NAMESPACE_USE

XERCES_CPP_NAMESPACE_BEGIN
class Locator;
XERCES_CPP_NAMESPACE_END

class VXMLDocumentRep;

class DocumentConverter : public DocumentHandler {
public:
  static bool Initialize();
  // One time initialization.
  //
  // Returns: True - initialization succeeded.
  //          False - initialization failed.

  static void Deinitialize();
  // One time cleanup of DocumentParser interface.

public:
  DocumentConverter();
  virtual ~DocumentConverter();

  virtual void startDocument();
  virtual void endDocument();
  virtual void resetDocument();
  virtual void setDocumentLocator(const Locator* const locator);

  virtual void startElement(const XMLCh* const name, AttributeList & attrs);
  virtual void endElement(const XMLCh* const name);

  virtual void characters(const XMLCh* const chars,
                          const unsigned int length);
  virtual void ignorableWhitespace(const XMLCh* const chars,
                                   const unsigned int length);

  virtual void processingInstruction(const XMLCh* const target,
                                     const XMLCh* const data);

  VXMLDocumentRep * GetDocument();

private:
  void ParseException(const VXIchar * message) const;

  void ProcessNodeAttribute(VXMLElementType elemType, int attrType,
                            const VXIchar* const value);

  void ProcessNodeFinal(VXMLElementType elemType);

  bool IsIgnorable(int elemType);

  const Locator * locator;
  VXMLDocumentRep * doc;
  int ignoreDepth;
  float version;
  bool strict;
  int choiceNumber;
};

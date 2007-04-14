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

#include "DocumentModel.hpp"

class VXMLDocumentRep {
public:
  const VXMLNodeRef * GetRoot() const;

  void StartElement(VXMLElementType n);
  void EndElement();

  void AddContent(const VXIchar * c, unsigned int len);
  void AddAttribute(VXMLAttributeType name, const vxistring & attr);

  bool GetAttribute(VXMLAttributeType key, vxistring & attr) const;

  VXMLDocumentRep();

  static void AddRef(VXMLDocumentRep * t);
  static void Release(VXMLDocumentRep * & t);

  VXMLElementType GetParentType() const;

private:
  void AddChild(VXMLNodeRef *);

private:
  ~VXMLDocumentRep();                                    // only used by Release
  VXMLDocumentRep(const VXMLDocumentRep &);              // not implemented
  VXMLDocumentRep & operator=(const VXMLDocumentRep &);  // not implemented

  const VXMLNodeRef * root;
  VXMLNodeRef * pos;
  VXMLNode::VXMLNodeType posType;
  int count;
};

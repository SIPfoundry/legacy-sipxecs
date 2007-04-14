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
#include "VXML.h"
#include <string>

typedef std::basic_string<VXIchar> vxistring;
class VXMLElement;
class VXMLNodeRef;
class VXMLNodeIterator;
class VXMLNodeIteratorData;
class VXMLDocumentRep;


class VXMLDocumentModel {
public:
  static bool Initialize();
  static void Deinitialize();

  static void CreateHiddenVariable(vxistring &);
  static bool IsHiddenVariable(const vxistring &);

  class OutOfMemory { };
  class InternalError { };
};


class VXMLNode {
  friend class VXMLNodeIterator;

public:
  enum VXMLNodeType {
    Type_VXMLContent,              // This node represents a leaf.
    Type_VXMLElement,              // This node represents a branch.
    Type_VXMLNode                  // This node is not initialized.
  };

  VXMLNode(const VXMLNodeRef * i = NULL) : internals(i) { }
  virtual ~VXMLNode()          { }

  VXMLNode(const VXMLNode & x);
  VXMLNode & operator=(const VXMLNode &);

public:
  VXMLNodeType GetType() const;
  VXMLElement GetParent() const;

  bool operator!=(const VXMLNode &x) const  { return internals != x.internals;}
  bool operator==(const VXMLNode &x) const  { return internals == x.internals;}

  bool operator==(int a) const               { if (a != 0) return false;
                                               return (internals == NULL); }

  bool operator!=(int a) const               { if (a != 0) return false;
                                               return (internals != NULL); }

protected:
  const VXMLNodeRef * internals;
};


class VXMLNodeIterator {
public: // Creators
  VXMLNodeIterator(const VXMLNode &);
  ~VXMLNodeIterator();

public: // Manipulators
  void operator++();                 // Increments iterator to next child
  VXMLNode operator*() const;        // Returns corresponding node
  operator const void *() const;     // Returns 0 if iteration is invalid
  void reset();                      // Resets iterator to first child

private: // not implemented
  VXMLNodeIterator(const VXMLNodeIterator &);
  VXMLNodeIterator & operator=(const VXMLNodeIterator &);

  VXMLNodeIteratorData * data;
};


class VXMLContent : public VXMLNode {
public:
  virtual ~VXMLContent() { }

public:
  const vxistring & GetValue() const;

private: // Not implemented
  VXMLContent();
  VXMLContent(const VXMLContent & x);
  VXMLContent & operator=(const VXMLNode &);
  VXMLContent(const VXMLElement * p);
};


class VXMLElement : public VXMLNode {
public: // Creation & comparison functions
  VXMLElement(const VXMLNodeRef * ref = NULL);
  VXMLElement(const VXMLElement & x);
  virtual ~VXMLElement() { }

  // Navigation functions (moving up & down the tree)
  bool hasChildren() const;

  // Information retrieval
  VXMLElementType GetName() const;
  bool GetAttribute(VXMLAttributeType key, vxistring & attr) const;

private:
  VXMLElement & operator=(const VXMLNode &);
};


class VXMLDocument {
public:
  VXMLDocument(VXMLDocumentRep *pRep = NULL);
  VXMLDocument(const VXMLDocument &);
  VXMLDocument & operator=(const VXMLDocument &);
  ~VXMLDocument();

  VXMLElement GetRoot() const;

private:
  VXMLDocumentRep * internals;
};

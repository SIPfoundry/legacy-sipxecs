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

#include "DocumentRep.hpp"
#include <sstream>                     // by CreateHiddenVariable()
#include <list>                        // by VXMLElementRef
#include <iostream>

//#############################################################################
// One mutex is shared across all the VXMLNodes.  The small performance
// reduction is offset by the large number of mutexes which would be otherwise
// created.

#include "InternalMutex.hpp"                    // by VXMLNodeRef
static InternalMutex * mutex = NULL;


bool VXMLDocumentModel::Initialize()
{
  mutex = new InternalMutex();
  return !(mutex == NULL || mutex->IsBad());
}


void VXMLDocumentModel::Deinitialize()
{
  if (mutex != NULL) delete mutex;
  mutex = NULL;
}

//#############################################################################

static const VXIchar * const HIDDEN_VAR_PREFIX   = L"$_internalName_";

// The cautious reader will notice that this routine is not thread safe.  This
// perfectly acceptible.  We don't really care what the numbers are so long
// as they keep incrementing.
//
// The 'dangerous' line is indicated with the asterisk.  A context switch
// would need to happen between the comparison and the assignment but only when
// the threshold is exceeded.  Even on those architectures which permit this
// context switch, the risk is near zero.
//
void VXMLDocumentModel::CreateHiddenVariable(vxistring & v)
{
  static long internalCount = 31415;

  // Increment counter or reset if necessary (LONG_MAX must be larger).
  long count = ++internalCount;
  if (internalCount > 2141234567) internalCount = 271828;  // *

  std::basic_stringstream<VXIchar> name;
  name << HIDDEN_VAR_PREFIX << count;

  v = name.str();
}


// As long as HIDDEN_VAR_PREFIX is sufficiently obscure, this should be good
// enough.
bool VXMLDocumentModel::IsHiddenVariable(const vxistring & v)
{
  return (v.find(HIDDEN_VAR_PREFIX) == 0);
}

//#############################################################################
// This defines the reference counted data for VXMLNodes.

class VXMLNodeRef {
public:
  const VXMLNodeRef * GetParent() const         { return parent; }
  VXMLNode::VXMLNodeType GetType() const        { return type; }

  virtual ~VXMLNodeRef() { }

protected:
  VXMLNodeRef(const VXMLNodeRef * p, VXMLNode::VXMLNodeType t)
    : parent(p), type(t) { }

  const VXMLNodeRef * parent;
  VXMLNode::VXMLNodeType type;
};

//#############################################################################
// Content nodes are simple - they consist of a block of data.

class VXMLContentRef : public VXMLNodeRef {
public:
  vxistring data;

  VXMLContentRef(const VXMLNodeRef * p)
    : VXMLNodeRef(p, VXMLNode::Type_VXMLContent) { }

  virtual ~VXMLContentRef() { }
};

//#############################################################################

class VXMLElementRef : public VXMLNodeRef {
public:
  class VXMLElementAttribute {
  public:
    VXMLAttributeType key;
    vxistring value;

    VXMLElementAttribute() { }
    VXMLElementAttribute(VXMLAttributeType k, const vxistring & attr)
      : key(k), value(attr) { }
  };

  typedef std::list<VXMLNodeRef *> CHILDREN;
  typedef std::list<VXMLElementAttribute> ATTRIBUTES;

  VXMLElementType name;
  ATTRIBUTES attributes;
  CHILDREN   children;

  bool GetAttribute(VXMLAttributeType key, vxistring & attr) const;

  VXMLElementRef(const VXMLNodeRef * p, VXMLElementType n)
    : VXMLNodeRef(p, VXMLNode::Type_VXMLElement), name(n) { }

  virtual ~VXMLElementRef() {
    for (CHILDREN::iterator i = children.begin(); i != children.end(); ++i)
    {
      delete (*i);
      (*i) = NULL;
    }
  }
};


bool VXMLElementRef::GetAttribute(VXMLAttributeType key,
                                  vxistring & attr) const
{
  ATTRIBUTES::const_iterator i;
  for (i = attributes.begin(); i != attributes.end(); ++i) {
    if ((*i).key == key) {
      attr = (*i).value;
      return true;
    }
  }

  attr.erase();
  return false;
}

//#############################################################################

class VXMLNodeIteratorData {
public:
  const VXMLElementRef & ref;
  VXMLElementRef::CHILDREN::const_iterator i;

  VXMLNodeIteratorData(const VXMLElementRef & r) : ref(r)
    { i = ref.children.begin(); }
};

VXMLNodeIterator::VXMLNodeIterator(const VXMLNode & n) : data(NULL)
{
  if (n.internals == NULL ||
      n.internals->GetType() != VXMLNode::Type_VXMLElement)  return;
  
  const VXMLElementRef * tmp = static_cast<const VXMLElementRef*>(n.internals);

  data = new VXMLNodeIteratorData(*tmp);
}


VXMLNodeIterator::~VXMLNodeIterator()
{
  if (data != NULL) delete data;
  data = NULL;
}


void VXMLNodeIterator::operator++()
{
  if (data != NULL) ++data->i;
}


void VXMLNodeIterator::reset()
{
  if (data != NULL) data->i = data->ref.children.begin();
}


VXMLNode VXMLNodeIterator::operator*() const
{
  if (data == NULL) return VXMLNode();
  return *(data->i);
}


VXMLNodeIterator::operator const void *() const
{
  if (data == NULL || data->i == data->ref.children.end()) return NULL;
  return reinterpret_cast<void *>(1);
}

//#############################################################################

void VXMLDocumentRep::AddRef(VXMLDocumentRep * t)
{
  if (t == NULL) return;

  mutex->Lock();
  ++t->count;
  mutex->Unlock();
}


void VXMLDocumentRep::Release(VXMLDocumentRep * & t)
{
  if (t == NULL) return;

  mutex->Lock();
  --t->count;
  mutex->Unlock();

  if (t->count == 0) delete t;

  t = NULL;
}


VXMLDocumentRep::VXMLDocumentRep()
  : root(NULL), pos(NULL), posType(VXMLNode::Type_VXMLNode), count(1)
{
}


VXMLDocumentRep::~VXMLDocumentRep()
{
  if (root != NULL) delete root;
  root = NULL;
}


void VXMLDocumentRep::AddContent(const VXIchar * c, unsigned int len)
{
  // (0) Parameter check.
  if (c == NULL || len == 0) return;

  // (1) Handle the case where we're just appending content to an existing node
  if (posType == VXMLNode::Type_VXMLContent) {
    (static_cast<VXMLContentRef *>(pos))->data += vxistring(c, len);
    return;
  }

  // (2) Create new content node.
  VXMLContentRef * temp = new VXMLContentRef(pos);
  if (temp == NULL) throw VXMLDocumentModel::OutOfMemory();
  temp->data = vxistring(c, len);

  AddChild(temp);
  pos = temp;
  posType = VXMLNode::Type_VXMLContent;
}


void VXMLDocumentRep::StartElement(VXMLElementType n)
{
  if (root != NULL) {
    if (pos == NULL)
      throw VXMLDocumentModel::InternalError();

    else if (posType == VXMLNode::Type_VXMLContent) {
      pos = const_cast<VXMLNodeRef *>(pos->GetParent());
      posType = pos->GetType();
    }
  }

  VXMLElementRef * temp = new VXMLElementRef(pos, n);
  if (temp == NULL) throw VXMLDocumentModel::OutOfMemory();

  AddChild(temp);
  pos = temp;
  posType = VXMLNode::Type_VXMLElement;
}


void VXMLDocumentRep::EndElement()
{
  if (pos == NULL)
    throw VXMLDocumentModel::InternalError();

  if (posType == VXMLNode::Type_VXMLContent) {
    posType = pos->GetType();
    pos = const_cast<VXMLNodeRef *>(pos->GetParent());
  }

  if (pos == NULL)
    throw VXMLDocumentModel::InternalError();
  else
  {
      posType = pos->GetType();
      pos = const_cast<VXMLNodeRef *>(pos->GetParent());
  }
}


VXMLElementType VXMLDocumentRep::GetParentType() const
{
  if (pos == NULL)
    throw VXMLDocumentModel::InternalError();
  else
   return static_cast<const VXMLElementRef *>(pos->GetParent())->name;
}


bool VXMLDocumentRep::GetAttribute(VXMLAttributeType key,
                                   vxistring & attr) const
{
  if (pos == NULL || posType != VXMLNode::Type_VXMLElement)
    throw VXMLDocumentModel::InternalError();
  else
  {
      const VXMLElementRef * temp = static_cast<const VXMLElementRef *>(pos);
      return temp->GetAttribute(key, attr);
  }
}


void VXMLDocumentRep::AddAttribute(VXMLAttributeType name,
                                   const vxistring & attr)
{
  if (pos == NULL || posType != VXMLNode::Type_VXMLElement)
    throw VXMLDocumentModel::InternalError();
  else
  {
      VXMLElementRef * temp = static_cast<VXMLElementRef *>(pos);
      if (temp)
         temp->attributes.push_back(VXMLElementRef::VXMLElementAttribute(name, attr));
  }
}


void VXMLDocumentRep::AddChild(VXMLNodeRef * c)
{
  if (root == NULL) {
    root = c;
    return;
  }

  if (pos == NULL || posType != VXMLNode::Type_VXMLElement)
    throw VXMLDocumentModel::InternalError();
  else
  {
      VXMLElementRef * temp = static_cast<VXMLElementRef *>(pos);
      if (temp)
         temp->children.push_back(c);
  }
}


const VXMLNodeRef * VXMLDocumentRep::GetRoot() const
{
  return root;
}

//#############################################################################

VXMLDocument::VXMLDocument(VXMLDocumentRep * x) : internals(x)
{
  if (internals == NULL) {
    internals = new VXMLDocumentRep();
    if (internals == NULL) throw VXMLDocumentModel::OutOfMemory();
  }
}


VXMLDocument::~VXMLDocument()
{
  VXMLDocumentRep::Release(internals);
}


VXMLDocument::VXMLDocument(const VXMLDocument & x)
{
  internals = x.internals;
  VXMLDocumentRep::AddRef(internals);
}


VXMLDocument & VXMLDocument::operator=(const VXMLDocument & x)
{
  if (this != &x) {
    VXMLDocumentRep::Release(internals);
    internals = x.internals;
    VXMLDocumentRep::AddRef(internals);
  }
  return *this;
}


VXMLElement VXMLDocument::GetRoot() const
{
  if (internals == NULL) return VXMLElement();
  const VXMLNodeRef * root = internals->GetRoot();
  if (root == NULL) return VXMLElement();
  return VXMLElement(root);
}

//#############################################################################

VXMLNode::VXMLNode(const VXMLNode & x)
{
  internals = x.internals;
}


VXMLNode & VXMLNode::operator=(const VXMLNode & x)
{
  if (this != &x) {
    internals = x.internals;
  }
  return *this;
}


VXMLElement VXMLNode::GetParent() const
{
  if (internals != NULL && internals->GetType() == VXMLNode::Type_VXMLNode)
    throw VXMLDocumentModel::InternalError();

  if (internals == NULL) return VXMLElement();
  return VXMLElement(internals->GetParent());
}


VXMLNode::VXMLNodeType VXMLNode::GetType() const
{
  if (internals == NULL) return VXMLNode::Type_VXMLNode;
  return internals->GetType();
}

//#############################################################################

const vxistring & VXMLContent::GetValue() const
{
  if (internals == NULL) throw VXMLDocumentModel::InternalError();
  const VXMLContentRef * ref = static_cast<const VXMLContentRef *>(internals);
  return ref->data;
}

//#############################################################################

VXMLElement::VXMLElement(const VXMLNodeRef * ref) : VXMLNode(ref)
{
}


VXMLElement::VXMLElement(const VXMLElement & x)
{
  internals = x.internals;
}


bool VXMLElement::hasChildren() const
{
  if (internals == NULL) throw VXMLDocumentModel::InternalError();
  const VXMLElementRef * ref = static_cast<const VXMLElementRef *>(internals);
  return !ref->children.empty();
}


VXMLElementType VXMLElement::GetName() const
{
  if (internals == NULL) throw VXMLDocumentModel::InternalError();
  const VXMLElementRef * ref = static_cast<const VXMLElementRef *>(internals);
  return ref->name;
}


bool VXMLElement::GetAttribute(VXMLAttributeType key, vxistring & attr) const
{
  if (internals == NULL) throw VXMLDocumentModel::InternalError();
  const VXMLElementRef * ref = static_cast<const VXMLElementRef *>(internals);
  return ref->GetAttribute(key, attr);
}

//#############################################################################

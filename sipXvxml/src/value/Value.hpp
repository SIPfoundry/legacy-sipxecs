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
 * Header for the implementation of VXIValue based types as defined in
 * vxivalue.h
 *
 ***********************************************************************/

#ifndef _VALUE_HPP
#define _VALUE_HPP

// Forward declarations, required because we mask out the struct definitions
// in VXIvalue.h with the define below
class VXIValue;
class VXIInteger;
class VXIFloat;
class VXIPtr;
class VXIString;
class VXIContent;
class VXIMap;
class VXIVector;
class VXIMapIterator;
class VXIContentData;

#define VXIVALUE_REAL_STRUCTS
#include "VXIvalue.h"

#ifndef WIN32
extern "C" struct VXItrdMutex;
#endif


/**
 * VXIValue base class
 */
class VXIValue {
 public:
  // Constructor and destructor
  VXIValue (VXIvalueType t) : type(t) { }
  virtual ~VXIValue( ) { }

  // Get the value type
  VXIvalueType GetType( ) const { return type; }

 private:
  VXIvalueType type;
};

/**
 * Basic VXIValue based data types
 */
class VXIInteger : public VXIValue {
 public:
  // Constructor and destructor
  VXIInteger (VXIint32 v) : VXIValue (VALUE_INTEGER), value(v) { }
  virtual ~VXIInteger( ) { }

  // Get the value
  VXIint32 GetValue( ) const { return value; }

 private:
  VXIint32  value;
};

class VXIFloat : public VXIValue {
 public:
  // Constructor and destructor
  VXIFloat (VXIflt32 v) : VXIValue (VALUE_FLOAT), value(v) { }
  virtual ~VXIFloat( ) { }

  // Get the value
  VXIflt32 GetValue( ) const { return value; }

 private:
  VXIflt32  value;
};

class VXIPtr : public VXIValue {
 public:
  // Constructor and destructor
  VXIPtr (VXIptr v) : VXIValue (VALUE_PTR), value(v) { }
  virtual ~VXIPtr( ) { }

  // Get the value
  VXIptr GetValue( ) const { return value; }

 private:
  VXIptr        value;
};

// Helper class for VXIContent, non-public
class VXIContentData {
 public:
  // Constructors and destructor
  VXIContentData (const VXIchar  *ct,
		  VXIbyte        *c,
		  VXIulong        csb,
		  void          (*Destroy)(VXIbyte **content, void *userData),
		  void           *ud);
  virtual ~VXIContentData( );

  // Add and remove references
  static void AddRef  (VXIContentData *data);
  static void Release (VXIContentData **data);

  // Get the data
  const VXIchar  *GetContentType( ) const { return contentType; }
  const VXIbyte  *GetContent( ) const { return content; }
  const VXIulong  GetContentSizeBytes( ) const { return contentSizeBytes; }

 private:
  // Disabled copy constructor and assignment operator
  VXIContentData (const VXIContent &c);
  VXIContent &operator= (const VXIContent &c);

 private:
#ifdef WIN32
  long         refCount;
#else
  VXItrdMutex *mutex;
  VXIulong     refCount;
#endif

  VXIchar     *contentType;
  VXIbyte     *content;
  VXIulong     contentSizeBytes;
  void       (*Destroy)(VXIbyte **content, void *userData);
  void        *userData;
};

class VXIContent : public VXIValue {
 public:
  // Constructors and destructor
  VXIContent (const VXIchar  *contentType,
	      VXIbyte        *content,
	      VXIulong        contentSizeBytes,
	      void          (*Destroy)(VXIbyte **content, void *userData),
	      void           *userData) : 
    VXIValue(VALUE_CONTENT), details(NULL) {
    details = new VXIContentData (contentType, content, contentSizeBytes,
				  Destroy, userData); }
  VXIContent (const VXIContent &c) : 
    VXIValue(VALUE_CONTENT), details(c.details) { 
    VXIContentData::AddRef (details); }
  virtual ~VXIContent( ) { VXIContentData::Release (&details); }

  // Get the data
  const VXIchar  *GetContentType( ) const {
    return (details ? details->GetContentType( ) : NULL); }
  const VXIbyte  *GetContent( ) const {
    return (details ? details->GetContent( ) : NULL); }
  const VXIulong  GetContentSizeBytes( ) const {
    return (details ? details->GetContentSizeBytes( ) : 0); }

  // Assignment operator
  VXIContent &operator= (const VXIContent &c) {
    if ( &c != this ) {
      VXIContentData::Release (&details);
      details = c.details;
      VXIContentData::AddRef (details);
    }
    return *this;
  }

 private:
  VXIContentData  *details;
};

#endif  /* include guard */

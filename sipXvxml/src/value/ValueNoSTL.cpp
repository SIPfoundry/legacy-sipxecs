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
 * Implementation of the complex VXIValue based types without using
 * the C++ Standard Template Library as defined in vxivalue.h:
 * VXIString, VXIMap, and VXIVector.
 *
 * See ValueSTL.cpp for the higher-performance and less complex STL
 * based implementation.
 *
 ***********************************************************************/

#define VXIVALUE_EXPORTS
#include "Value.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Abstractions for VXIchar
 */
#define _UNICODE
#ifdef _UNICODE
#define VXIcharStrlen  wcslen
#define VXIcharStrcpy  wcscpy
#define VXIcharStrcmp  wcscmp
#else
#define VXIcharStrlen  strlen
#define VXIcharStrcpy  strcpy
#define VXIcharStrcmp  strcmp
#endif

/**
 * Real VXIString class
 */

class VXIString : public VXIValue {
 public:
  // Constructor and destructor
  VXIString (const VXIchar *v)
    : VXIValue (VALUE_STRING), value(NULL), length(0) 
  {
    if (( v ) && ( (value = new VXIchar [VXIcharStrlen (v) + 1]) != NULL )) {
      VXIcharStrcpy (value, v);
      length = VXIcharStrlen (v);
    }
  }

  VXIString (const VXIchar *v, VXIunsigned u)
    : VXIValue (VALUE_STRING), value(NULL), length(u)
  {
    if (( v ) && ( (value = new VXIchar [u + 1]) != NULL )) {
      for (unsigned int i = 0; i < u; ++i) value[i] = v[i];
      value[u] = '\0';
    }
  }

  virtual ~VXIString( ) { if ( value ) { delete [] value; value = NULL; } }

  virtual const char *toString(char *ret, size_t len ) const { 
     snprintf(ret, len, "%ls", value); return ret;}


  // Get the length of the string
  VXIunsigned GetLength( ) const       { return length; }

  // Get and set the value
  const VXIchar *GetValue( ) const { return value; }
  void SetValue (const VXIchar *v) { 
    if ( value ) { delete [] value; value = NULL; length = 0; }
    if (( v ) && ( (value = new VXIchar [VXIcharStrlen (v) + 1]) != NULL )) {
      VXIcharStrcpy (value, v);
      length = VXIcharStrlen (v);
    }
  }

  // Comparison methods, equivalent to strcmp( )
  int Compare (const VXIString &s) const { 
    return VXIcharStrcmp (value, s.value); }
  int Compare (const VXIchar *s) const { 
    return VXIcharStrcmp (value, s); }

  // Copy constructor and assignment operator
  VXIString (const VXIString &v) : VXIValue (VALUE_STRING), value(NULL) {
    if (( v.value ) && 
	( (value = new VXIchar [v.length + 1]) != NULL )) {
      for (unsigned int i = 0; i < v.length + 1; ++i) value[i] = v.value[i];
      length = v.length;
    }
  }
  VXIString & operator= (const VXIString &v) {
    if ( this != &v ) {
      if ( value ) { delete [] value; value = NULL; length = 0; }
      if (( v.value ) && 
	  ( (value = new VXIchar [v.length + 1]) != NULL )) {
        for (unsigned int i = 0; i < v.length + 1; ++i) value[i] = v.value[i];
        length = v.length;
      }
    }
    return *this;
  }

  // Overrides of comparison operators for VXIMapElement use
  virtual bool operator< (const VXIString &s) const {
    return (Compare (s) < 0 ? true : false);
  }
  virtual bool operator== (const VXIString &s) const { 
    return (Compare (s) == 0 ? true : false);
  }
  virtual bool operator!= (const VXIString &s) const { 
    return (Compare (s) != 0 ? true : false);
  }

 private:
  VXIchar *value;
  unsigned int length;
};


/**
 * Real VXIMap and supporting classes
 */

class VXIElement {
 public:
  // Constructor and destructor
  VXIElement (const VXIchar *k, VXIValue *v) : 
    key(k), value(v), next(NULL) { }
  VXIElement (const VXIString &k, VXIValue *v) : 
    key(k), value(v), next(NULL) { }
  virtual ~VXIElement( ) { value = NULL; }

  // Get the key and value
  const VXIString &GetKey( ) const { return key; }
  const VXIValue *GetValue( ) const { return value; }

  // Destroy the value
  void DestroyValue( ) { if ( value ) VXIValueDestroy (&value); }

  // Get and set the next element pointer
  VXIElement *GetNext( ) const { return next; }
  void SetNext (VXIElement *e) { next = e; }

  // Overrides of comparison operators for finding elements based on
  // the key name
  virtual bool operator< (const VXIElement &e) const {
    return key < e.key;
  }
  virtual bool operator== (const VXIElement &e) const { 
    return key == e.key;
  }
  virtual bool operator!= (const VXIElement &e) const { 
    return key != e.key;
  }

 private:
  // Stubs to prevent copy constructor and assignment operator use
  VXIElement (const VXIElement &e);
  VXIElement & operator= (const VXIElement &e);

 private:
  VXIString      key;
  VXIValue      *value;
  VXIElement    *next;
};

class VXIMap : public VXIValue {
 public:
  // Constructor and destructor
  VXIMap( ) : VXIValue (VALUE_MAP), badState(false), size(0),
		 head(NULL) { }
  virtual ~VXIMap( );

  // Copy constructor
  VXIMap (const VXIMap &m);

 private:
  // Stub to prevent assignment operator use, use the copy constructor
  VXIMap & operator= (const VXIMap &m);

 public:
  bool              badState;
  unsigned long     size;
  VXIElement       *head;
};

VXIMap::~VXIMap( )
{
  VXIElement *cur = head, *next;
  while (cur) {
    next = cur->GetNext( );
    cur->DestroyValue( );
    delete cur;
    cur = next;
  }
}

VXIMap::VXIMap (const VXIMap &m) : 
  VXIValue (VALUE_MAP), badState(false), size(0), head(NULL)
{
  VXIElement *src = m.head, *prev = NULL, *dest;
  while (src) {
    dest = new VXIElement (src->GetKey( ), VXIValueClone (src->GetValue( )));
    if ( dest == NULL ) {
      badState = true;
      return;
    } else if (( dest->GetKey( ).GetLength( ) == 0 ) ||
	       ( dest->GetValue( ) == NULL )) {
      dest->DestroyValue( );
      delete dest;
      dest = NULL;
      badState = true;
      return;
    }

    if ( prev == NULL )
      head = dest;
    else
      prev->SetNext (dest);

    prev = dest;
    src = src->GetNext( );
  }

  size = m.size;
}


class VXIMapIterator {
 public:
  // Constructor and destructor
  VXIMapIterator(const VXIMap *o) : curElement(o->head) { }
  virtual ~VXIMapIterator( ) { }

  // Get the key and value at the iterator's position
  VXIvalueResult GetKeyValue (const VXIchar **key, 
			      const VXIValue **value) const {
    if ( curElement == NULL ) {
      *key = NULL;
      *value = NULL;
      return VXIvalue_RESULT_FAILURE;
    }

    *key = curElement->GetKey( ).GetValue( );
    *value = curElement->GetValue( );
    return VXIvalue_RESULT_SUCCESS;
  }

  // Increment the iterator
  VXIMapIterator &operator++(int) { 
    if ( curElement != NULL )
      curElement = curElement->GetNext( ); 
    return *this; }

 private:
  const VXIElement *curElement;
};


/**
 * Real VXIVector and supporting classes
 */

class VXIVector : public VXIValue {
 public:
  static const int GROW_SIZE;

 public:
  // Constructor and destructor
  VXIVector( ) : VXIValue (VALUE_VECTOR), badState(false), size(0), 
    allocated(0), data(NULL) { }
  virtual ~VXIVector( );

  // Copy constructor
  VXIVector (const VXIVector &v);

  // Add an element
  VXIvalueResult Add (VXIValue *v);

 private:
  // Stub to prevent assignment operator use, use the copy constructor
  VXIMap & operator= (const VXIMap &o);

 public:
  bool              badState;
  unsigned long     size;
  unsigned long     allocated;
  VXIElement       *data;
};

const int VXIVector::GROW_SIZE = 8;

// Special new operator required by VXIVector, used to simply allow
// us to allocate our own memory for the vector data storage, and
// the matching delete
inline void *operator new (size_t, void *p) { return p; }
inline void operator delete(void *, void *) { }

VXIVector::~VXIVector( )
{ 
  if ( data ) {
    // Explicit destructor of each held item which does the standard
    // destruction of delete( ) but skips the memory free since we
    // provided the memory ourselves
    for (unsigned long i = 0; i < size; i++) {
      data[i].DestroyValue( );
      data[i].~VXIElement( );
    }

    // Now free the memory for the vector
    free (data);
  }
}

static VXIElement *writeOnlyDummy = NULL; // write-only dummy, to get rid of warnings

VXIVector::VXIVector (const VXIVector &v) : 
  VXIValue (VALUE_VECTOR), badState(false), size(0), allocated(0), data(NULL)
{
  if ( v.size > 0 ) {
    // Allocate just enough memory to store the vector we're copying, we're
    // making a guess that they will just hold onto this clone instead of
    // adding additional elements to it
    data = (VXIElement *) malloc (v.size * sizeof (VXIElement));
    if ( data == NULL ) {
      badState = true;
      return;
    }
    allocated = v.size; 

    // Now copy the elements
    while ( size < v.size ) {
      // Set the new element using a new operator with placement syntax
      // so it does the standard initialization of new( ) but skips the
      // allocation, instead using the memory we provide
      writeOnlyDummy = new (&data[size]) 
	VXIElement (v.data[size].GetKey( ),
		    VXIValueClone (v.data[size].GetValue( )));
      size++;
    }
  }
}


VXIvalueResult VXIVector::Add (VXIValue *val)
{
  // Reallocate memory for the vector if required
  if ( allocated <= size ) {
    VXIElement *ptr = (VXIElement *)
      realloc (data, (size + VXIVector::GROW_SIZE) * 
	       sizeof (VXIElement));
    if ( ptr == NULL )
      return VXIvalue_RESULT_OUT_OF_MEMORY;
    
    data = ptr;
    allocated = size + VXIVector::GROW_SIZE;
  }

  // Set the new element using a new operator with placement syntax
  // so it does the standard initialization of new( ) but skips the
  // allocation, instead using the memory we provide
  writeOnlyDummy = new (&data[size]) VXIElement (NULL, val);
  size++;

  return VXIvalue_RESULT_SUCCESS;
}


/**
 * Create a String from a null-terminated character array
 *
 * @param   str   NULL-terminated character array
 * @return        String with the specified value on success, 
 *                NULL otherwise
 */
VXIVALUE_API VXIString *VXIStringCreate(const VXIchar *str)
{
  if ( str == NULL )
    return NULL;

  // Allocate the string
  VXIString *s = new VXIString (str);
  if (( s != NULL ) && ( s->GetValue( ) == NULL )) {
    delete s;
    s = NULL;
  }

  return s;
}


/**
 * Create a String from a known-length character array
 *
 * @param   str   Character array (null characters may be embedded in the 
 *                array)
 * @param   len   Number of characters which will be copied.
 * @return        String with the specified value on success, 
 *                NULL otherwise
 */
VXIVALUE_API VXIString *VXIStringCreateN(const VXIchar *str, VXIunsigned len)
{
  if ( str == NULL )
    return NULL;

  // Allocate the string
  VXIString *s = new VXIString (str, len);
  if (( s != NULL ) && ( s->GetValue( ) == NULL )) {
    delete s;
    s = NULL;
  }

  return s;
}


/**
 * String destructor
 *
 * @param   s   String to destroy
 */
VXIVALUE_API void VXIStringDestroy(VXIString **s)
{
  if (( s ) && ( *s ) && ( (*s)->GetType( ) == VALUE_STRING )) {
    delete *s;
    *s = NULL;
  }
}


/**
 * String clone
 *
 * Note: functionally redundant with VXIValueClone( ), but provided to
 * reduce the need for C casts for this common operation
 *
 * @param    s   String to clone
 * @return       Clone of the string on success, NULL otherwise
 */
VXIVALUE_API VXIString *VXIStringClone(const VXIString *s)
{
  if (( s == NULL ) || ( s->GetType( ) != VALUE_STRING ))
    return NULL;

  // Allocate the string
  VXIString *str = new VXIString (*s);
  if (( str != NULL ) && ( str->GetValue( ) == NULL )) {
    delete str;
    str = NULL;
  }

  return str;
}


/**
 * Set the value of a String from a null-terminated character array
 *
 * Note: this functionality is provided to allow defining interfaces
 * where the caller passes in a VXIString from VXIStringCreate( )
 * (typically with an empty string as its value) with the interface
 * changing that value to return a string as output. This avoids
 * having to define interfaces where the client has to provide a
 * fixed length buffer (and thus worry about "buffer too small" errors
 * and complicated handling).
 *
 * @param   s     String to change the value of
 * @param   str   NULL-terminated character array
 * @return        VXIvalue_RESULT_SUCCESS on success 
 */
VXIVALUE_API VXIvalueResult VXIStringSetValue(VXIString      *s, 
					      const VXIchar  *str)
{
  if (( s == NULL ) || ( s->GetType( ) != VALUE_STRING ) || 
      ( str == NULL ))
    return VXIvalue_RESULT_INVALID_ARGUMENT;

  s->SetValue (str);
  if ( s->GetValue( ) == NULL )
    return VXIvalue_RESULT_OUT_OF_MEMORY;
  
  return VXIvalue_RESULT_SUCCESS;
}


/**
 * Get the value of a String
 *
 * @param   s     String to access
 * @param   buf   Character buffer to copy the value into as a
 *                NULL-terminated character array.  The buffer size must be
 *                at least VXIStringLength() + 1.
 * @param   len   Size of the buffer, in characters
 * @return        Pointer to buf on success, NULL on failure (most likely
 *                buffer too small) 
 */
VXIVALUE_API VXIchar *VXIStringValue(const VXIString  *s, 
				     VXIchar          *buf, 
				     VXIunsigned       len)
{
  if (( s == NULL ) || ( s->GetType( ) != VALUE_STRING ) || 
      ( buf == NULL ))
    return NULL;

  // Make sure the buffer is large enough
  if ( len < s->GetLength( ) + 1 ) return NULL;

  const VXIchar * str = s->GetValue();
  unsigned int length = s->GetLength();
  for (unsigned int i = 0; i < length; ++i)
    *(buf + i) = str[i];

  *(buf + length) = '\0';

  return buf;
}


/**
 * Get direct access to the NULL-terminated character value
 *
 * Note: the returned buffer must never be modified, and is only
 * provided for transient use (i.e. immediately logging it, comparing
 * it, etc. rather than storing or returning the pointer for longer
 * term access).
 *
 * @param   s   String to retrieve the data from
 * @return      Pointer to the NULL-terminated character array retrieved
 */
VXIVALUE_API const VXIchar* VXIStringCStr(const VXIString *s)
{
  if (( s == NULL ) || ( s->GetType( ) != VALUE_STRING ))
    return NULL;
  return s->GetValue( );
}


/**
 * Get the number of characters in a String's value
 *
 * Note: Add one byte for the NULL terminator when using this to determine
 * the length of the array required for a VXIStringValue( ) call.
 *
 * @param   s   String to access
 * @return      Length of the string, in characters
 */
VXIVALUE_API VXIunsigned VXIStringLength(const VXIString *s)
{
  if (( s == NULL ) || ( s->GetType( ) != VALUE_STRING ))
    return 0;
  return s->GetLength( );
}


/**
 * Compares two Strings
 *
 * @param   s1   First String to compare
 * @param   s2   Second String to compare
 * @return       Returns a value that is less than, equal to, or greater
 *               than zero depending on whether s1 is lexicographically
 *               less than, equal to, or greater than s2
 */
VXIVALUE_API VXIint VXIStringCompare(const VXIString *s1, 
				     const VXIString *s2)
{
  if (( s1 == NULL ) || ( s1->GetType( ) != VALUE_STRING ))
    return -1;
  if (( s2 == NULL ) || ( s2->GetType( ) != VALUE_STRING ))
    return 1;
  return s1->Compare (*s2);
}


/**
 * Compares a String to a NULL-terminated character array
 *
 * @param   str   String to compare
 * @param   buf   NULL-terminated character array to compare
 * @return        Returns a value that is less than, equal to, or greater
 *                than zero depending on whether str is lexicographically
 *                less than, equal to, or greater than buf
 */
VXIVALUE_API VXIint VXIStringCompareC(const VXIString *str, 
				      const VXIchar   *buf)
{
  if (( str == NULL ) || ( str->GetType( ) != VALUE_STRING ))
    return -1;
  if ( buf == NULL )
    return 1;
  return str->Compare (buf);
}


/**
 * Create an empty Map
 *
 * @return   New map on success, NULL otherwise
 */
VXIVALUE_API VXIMap *VXIMapCreate(void)
{
  return new VXIMap;
}


/**
 * Map destructor
 *
 * Note: this recursively destroys all the values contained within the
 * Map, including all the values of Maps and Vectors stored
 * within this map. However, for Ptr values the user is
 * responsible for freeing the held memory if appropriate.
 *
 * @param m   Map to destroy 
 */
VXIVALUE_API void VXIMapDestroy(VXIMap **m)
{
  if (( m ) && ( *m ) && ( (*m)->GetType( ) == VALUE_MAP )) {
    delete *m;
    *m = NULL;
  }
}


/**
 * Map clone
 *
 * Recursively copies all values contained within the map,
 * including all the values of Maps and Vectors stored within this
 * map.
 *
 * Note: functionally redundant with VXIValueClone( ), but provided to
 * reduce the need for C casts for this common operation
 *
 * @param    m   Map to clone
 * @return       Clone of the Map on success, NULL otherwise 
 */
VXIVALUE_API VXIMap *VXIMapClone(const VXIMap *m)
{
  if (( m == NULL ) || ( m->GetType( ) != VALUE_MAP ))
    return NULL;

  VXIMap *theMap = new VXIMap (*m);
  if (( theMap ) && ( theMap->badState )) {
    delete theMap;
    theMap = NULL;
  }

  return theMap;
}


/**
 * Set a named property on an Map
 *
 * The value can be an Map so a tree can be constructed.
 *
 * If the property already exists, the existing value is first
 * destroyed using VXIValueDestroy( ) (thus recursively deleting held
 * values within it if it is an Map or Vector), then does the
 * set operation with the new value.
 *
 * @param   m     Map to access
 * @param   key   NULL terminated property name
 * @param   val   Value to set the property to, ownership is passed
 *                to the Map (a simple pointer copy is done), so on
 *                success the user must not delete, modify, or otherwise
 *                use this. Also be careful to not add a Map as a
 *                property of itself (directly or indirectly), otherwise
 *                infinite loops may occur on access or deletion.
 * @return        VXIvalue_RESULT_SUCCESS on success
 */
VXIVALUE_API VXIvalueResult VXIMapSetProperty(VXIMap         *m, 
					      const VXIchar  *key,
					      VXIValue       *val)
{
  if (( m == NULL ) || ( m->GetType( ) != VALUE_MAP ) ||
      ( key == NULL ) || ( key[0] == 0 ) || ( val == NULL ))
    return VXIvalue_RESULT_INVALID_ARGUMENT;

  // Create the element to set
  VXIElement *element = new VXIElement (key, val);
  if ( ! element ) {
    return VXIvalue_RESULT_OUT_OF_MEMORY;
  } else if ( element->GetKey( ).GetLength( ) == 0 ) {
    element->DestroyValue( );
    delete element;
    element = NULL;
    return VXIvalue_RESULT_OUT_OF_MEMORY;
  }

  // Search for the insertion point for that element
  VXIElement *cur = m->head, *prev = NULL;
  while (( cur ) && ( *cur < *element )) {
    prev = cur;
    cur = cur->GetNext( );
  }

  // Replace any existing element with that key or insert it if
  // completely new
  if (( cur ) && ( *cur == *element )) {
    // Replacement
    element->SetNext (cur->GetNext( ));
    if ( prev )
      prev->SetNext (element);
    else
      m->head = element;

    cur->DestroyValue( );
    delete cur;
    cur = NULL;
  } else {
    // Insertion
    if ( cur ) {
      element->SetNext (cur);
    } else {
    }
    if ( prev ) {
      prev->SetNext (element);
    } else {
      m->head = element;
    }

    m->size++;
  }

  return VXIvalue_RESULT_SUCCESS;
}


/**
 * Get a named property from an Map
 *
 * The property value is returned for read-only access and is
 * invalidated if the Map is modified. The client must clone it if
 * they wish to perform modifications or wish to retain the value even
 * afer modifying this Map.
 *
 * @param   m     Map to access
 * @param   key   NULL terminated property name
 * @return        On success the value of the property for read-only 
 *                access (invalidated if the Map is modified), NULL
 *                if the property was never set or was deleted 
 */
VXIVALUE_API const VXIValue *VXIMapGetProperty(const VXIMap    *m, 
					       const VXIchar   *key)
{
  if (( m == NULL ) || ( m->GetType( ) != VALUE_MAP ) || 
      ( key == NULL ) || ( key[0] == 0 ))
    return NULL;

  // Search for that element
  int rc;
  VXIElement *cur = m->head;
  while (( cur ) && ( (rc = cur->GetKey( ).Compare (key)) < 0 ))
    cur = cur->GetNext( );
  
  // Return it if found
  if (( cur ) && ( rc == 0 ))
    return cur->GetValue( );

  return NULL;
}


/**
 * Delete a named property from an Map
 *
 * This does a VXIValueDestroy( ) on the value for the named property
 * (thus recursively deleting held values within it if it is an Map
 * or Vector). However, for Ptr properties the user is responsible for
 * freeing the held memory if appropriate.
 *
 * @param   m     Map to access
 * @param   key   NULL terminated property name
 * @return        VXIvalue_RESULT_SUCCESS on success 
 */
VXIVALUE_API VXIvalueResult VXIMapDeleteProperty(VXIMap         *m, 
						 const VXIchar  *key)
{
  if (( m == NULL ) || ( m->GetType( ) != VALUE_MAP ) || 
      ( key == NULL ) || ( key[0] == 0 ))
    return VXIvalue_RESULT_INVALID_ARGUMENT;

  // Search for that element
  int rc;
  VXIElement *cur = m->head, *prev = NULL;
  while (( cur ) && ( (rc = cur->GetKey( ).Compare (key)) < 0 )) {
    prev = cur;
    cur = cur->GetNext( );
  }
  
  // Delete it if found
  if (( ! cur ) || ( rc != 0 ))
    return VXIvalue_RESULT_FAILURE;

  if ( prev )
    prev->SetNext (cur->GetNext( ));
  else
    m->head = cur->GetNext( );
  
  cur->DestroyValue( );
  delete cur;
  cur = NULL;
  m->size--;

  return VXIvalue_RESULT_SUCCESS;
}


/**
 * Return the number of properties for an Map
 *
 * Note: this only returns the number of properties that are direct
 * children of the Map, it does not include the number of properties
 * held in Maps and Vectors stored within this map.
 *
 * @param   m   Map to access
 * @return      Number of properties stored in the Map
 */
VXIVALUE_API VXIunsigned VXIMapNumProperties(const VXIMap *m)
{
  if (( m == NULL ) || ( m->GetType( ) != VALUE_MAP ))
    return 0;

  return m->size;
}


/**
 * Get the first property of an Map and an iterator
 *
 * Note: this is used to traverse all the properties within an map,
 * there is no guarantee on what order the properties will be
 * returned. The iterator must be eventually freed with
 * VXIMapIteratorDestroy( ), and is invalidated if the Map is
 * modified in any way.
 *
 * @param   m      Map to access
 * @param   key    Set to point at the property name for read-only 
 *                 access (must not be modified)                 
 * @param   value  Set to point at the property value for read-only 
 *                 access (must not be modified)
 * @return         Pointer to an iterator that may be used to get
 *                 additional properties via VXIMapGetNextProperty( ),
 *                 or NULL on failure (typically no properties in the map)
 */
VXIVALUE_API 
VXIMapIterator *VXIMapGetFirstProperty(const VXIMap     *m,
				       const VXIchar   **key,
				       const VXIValue  **value)
{
  if (( m == NULL ) || ( m->GetType( ) != VALUE_MAP ) ||
      ( m->size == 0 ) || ( key == NULL ) || ( value == NULL ))
    return NULL;

  // Allocate an iterator map
  VXIMapIterator *it = new VXIMapIterator (m);
  if ( it == NULL )
    return NULL;
  
  // Get the first property
  it->GetKeyValue (key, value);
  return it;
}


/**
 * Get the next property of an Map based on an iterator
 *
 * Note: this is used to traverse all the properties within an map,
 * there is no gaurantee on what order the properties will be
 * returned.
 *
 * @param   it     Iterator used to access the map as obtained
 *                 from VXIMapGetFirstProperty( ), this operation
 *                 will advance the iterator to the next property on
 *                 success
 * @param   key    Set to point at the property name for read-only 
 *                 access (must not be modified, invalidated if the
 *                 Map is modified)                 
 * @param   value  Set to point at the property value for read-only 
 *                 access (must not be modified, invalidated if the
 *                 Map is modified)
 * @return         VXIvalue_RESULT_SUCCESS on success (property name 
 *                 and value returned, iterator advanced), 
 *                 VXIvalue_RESULT_FAILURE if there are no more properties
 *                 to read, or a VXIvalueResult error code for severe errors
 */
VXIVALUE_API VXIvalueResult VXIMapGetNextProperty(VXIMapIterator  *it,
						  const VXIchar  **key,
						  const VXIValue **value)
{
  if (( it == NULL ) || ( key == NULL ) || ( value == NULL ))
    return VXIvalue_RESULT_INVALID_ARGUMENT;

  (*it)++;
  return it->GetKeyValue (key, value);
}


/**
 * Destroy an iterator
 *
 * @param   it     Iterator to destroy as obtained from 
 *                 VXIMapGetFirstProperty( )
 */
VXIVALUE_API void VXIMapIteratorDestroy(VXIMapIterator **it)
{
  if (( it ) && ( *it )) {
    delete *it;
    *it = NULL;
  }
}


/**
 * Create an empty Vector
 *
 * @return   New vector on success, NULL otherwise
 */
VXIVALUE_API VXIVector *VXIVectorCreate(void)
{
  return new VXIVector();
}


/**
 * Vector destructor
 *
 * Note: this recursively destroys all the values contained within the
 * Vector, including all the values of Vectors stored within this
 * vector. However, for Ptr values the user is responsible for
 * freeing the held memory if appropriate.
 *
 * @param   v   Vector to destroy 
 */
VXIVALUE_API void VXIVectorDestroy(VXIVector **v)
{
  if (( v ) && ( *v ) && ( (*v)->GetType( ) == VALUE_VECTOR )) {
    delete *v;
    *v = NULL;
  }
}


/**
 * Vector clone
 *
 * Recursively copies all values contained within the vector,
 * including all the values of Vectors and Maps stored within this
 * vector.
 *
 * Note: functionally redundant with VXIValueClone( ), but provided to
 * reduce the need for C casts for this common operation
 *
 * @param    v   Vector to clone
 * @return Clone of the Vector on success, NULL otherwise */
VXIVALUE_API VXIVector *VXIVectorClone(const VXIVector *v)
{
  if (( v == NULL ) || ( v->GetType( ) != VALUE_VECTOR ))
    return NULL;

  VXIVector *vec = new VXIVector (*v);
  if (( vec ) && ( vec->badState )) {
    delete vec;
    vec = NULL;
  }

  return vec;
}


/**
 * Adds an element to the end of the Vector
 *
 * The value can be a Vector so frames can be implemented.
 *
 * @param   v    Vector to access
 * @param   val  Value to append to the vector, ownership is passed
 *               to the Vector (a simple pointer copy is done), so on
 *               success the user must not delete, modify, or otherwise
 *               use this. Also be careful to not add a Vector as a
 *               element of itself (directly or indirectly), otherwise
 *               infinite loops may occur on access or deletion.
 * @return       VXIvalue_RESULT_SUCCESS on success
 */
VXIVALUE_API VXIvalueResult VXIVectorAddElement(VXIVector      *v, 
						VXIValue       *val)
{
  if (( v == NULL ) || ( v->GetType( ) != VALUE_VECTOR ) || 
      ( val == NULL ))
    return VXIvalue_RESULT_INVALID_ARGUMENT;

  return v->Add (val);
}


/**
 * Set an indexed vector element
 *
 * Overwrites the specified element with the new value. The existing
 * value is first destroyed using VXIValueDestroy( ) (thus recursively
 * deleting held values within it if it is an Map or Vector), then
 * does the set operation with the new value.
 *
 * The value can be a Vector so frames can be implemented.
 *
 * @param   v     Vector to access
 * @param   n     Element index to set, it is an error to pass a
 *                index that is greater then the number of values
 *                currently in the vector
 * @param   val   Value to set the element to, ownership is passed
 *                to the Vector (a simple pointer copy is done), so on
 *                success the user must not delete, modify, or otherwise
 *                use this. Also be careful to not add a Vector as a
 *                element of itself (directly or indirectly), otherwise
 *                infinite loops may occur on access or deletion.
 * @return        VXIvalue_RESULT_SUCCESS on success
 */
VXIVALUE_API VXIvalueResult VXIVectorSetElement(VXIVector      *v, 
						VXIunsigned     n, 
						VXIValue       *val)
{
  if (( v == NULL ) || ( v->GetType( ) != VALUE_VECTOR ) || 
      ( val == NULL ) || ( n >= v->size ))
    return VXIvalue_RESULT_INVALID_ARGUMENT;

  // Delete the old one, we allocated the memory so we call an explicit
  // destructor that doesn't free the memory
  v->data[n].DestroyValue( );
  v->data[n].~VXIElement( );

  // Create the new one
  writeOnlyDummy = new (&v->data[n]) VXIElement (NULL, val);
  return VXIvalue_RESULT_SUCCESS;
}


/**
 * Get an indexed vector element
 *
 * The element value is returned for read-only access and is
 * invalidated if the Vector is modified. The client must clone it if
 * they wish to perform modifications or wish to retain the value even
 * after modifying this Vector.
 *
 * @param   v     Vector to access
 * @param   n     Element index to set, it is an error to pass a
 *                index that is greater or equal to then the number of values
 *                currently in the vector (i.e. range is 0 to length-1)
 * @return        On success the value of the property for read-only 
 *                access (invalidated if the Vector is modified), NULL
 *                on error 
 */
VXIVALUE_API const VXIValue *VXIVectorGetElement(const VXIVector *v, 
						 VXIunsigned      n)
{
  if (( v == NULL ) || ( v->GetType( ) != VALUE_VECTOR ) ||
      ( n >= v->size ))
    return NULL;

  return v->data[n].GetValue( );
}


/**
 * Return number of elements in a Vector
 *
 * This computes only the length of the Vector, elements within
 * Vectors and Maps within it are not counted.
 *
 * @param   v    Vector to access
 * @return       Number of elements stored in the Vector
 */
VXIVALUE_API VXIunsigned VXIVectorLength(const VXIVector *v)
{
  if (( v == NULL ) || ( v->GetType( ) != VALUE_VECTOR ))
    return 0;

  return v->size;
}

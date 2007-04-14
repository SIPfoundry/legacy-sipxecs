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
 * Implementation of the basic VXIValue based types as defined in
 * vxivalue.h: VXIValue, VXIInteger, VXIFloat, VXIPtr, and VXIContent.
 *
 ***********************************************************************/

#include <stdio.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>                 // For InterlockedIncrement/Decrement
#else
#include "VXItrd.h"                  // For VXItrdMutex
#endif

#define VXIVALUE_EXPORTS
#include "Value.hpp"


/**
 * Get the type of a Value
 *
 * @param   v   Value to check
 * @return      Type of value
 */
VXIVALUE_API VXIvalueType VXIValueGetType(const VXIValue *v)
{
  if ( v == NULL ) return (VXIvalueType) 0xFFFFFFFF;
  return v->GetType( );
}


/**
 * Generic Value destructor
 *
 * This automatically invokes the appropriate type specific
 * destructor.
 *
 * @param   v   Value to destroy
 */
VXIVALUE_API void VXIValueDestroy(VXIValue **v)
{
  if (( v != NULL ) && ( *v != NULL )) {
    delete *v;
    *v = NULL;
  }
}


/**
 * Generic Value clone
 *
 * This automatically invokes the appropriate type specific clone
 * operation.
 *
 * @param   v   Value to clone
 * @return      Clone of v, NULL on error
 */
VXIVALUE_API VXIValue *VXIValueClone(const VXIValue *v)
{
  if ( v == NULL ) return NULL;

  VXIValue *obj = NULL;
  switch ( v->GetType( ) ) {
    case VALUE_INTEGER:
      obj = new VXIInteger (*(const VXIInteger *) v);
      break;
    case VALUE_FLOAT:
      obj = new VXIFloat (*(const VXIFloat *) v);
      break;
    case VALUE_STRING:
      obj = (VXIValue *) VXIStringClone ((const VXIString *) v);
      break;
    case VALUE_PTR:
      obj = new VXIPtr (*(const VXIPtr *) v);
      break;
    case VALUE_CONTENT: {
        VXIContent *c = new VXIContent (*(const VXIContent *) v);
	if (( c ) && ( c->GetContent( ) == NULL )) {
	  delete c;
	  c = NULL;
	}
	obj = (VXIValue *) c;
      } break;
    case VALUE_MAP:
      obj = (VXIValue *) VXIMapClone ((const VXIMap *) v);
      break;
    case VALUE_VECTOR:
      obj = (VXIValue *) VXIVectorClone ((const VXIVector *) v);
      break;
    default:
      ; // Error but nothing we can do
  }
  
  return obj;
}


/**
 * Create an Integer from a 32 bit integer
 *
 * @param   n   32 bit integer value
 * @return      Integer with the specified value on success, 
 *              NULL otherwise
 */
VXIVALUE_API VXIInteger *VXIIntegerCreate(VXIint32 n)
{
  return new VXIInteger (n);
}


/**
 * Integer destructor
 *
 * @param   i   Integer to destroy
 */
VXIVALUE_API void VXIIntegerDestroy(VXIInteger **i)
{
  if (( i != NULL ) && ( *i != NULL ) && 
      ( (*i)->GetType( ) == VALUE_INTEGER )) {
    delete *i;
    *i = NULL;
  }
}


/**
 * Get the value of an Integer
 *
 * @param   i   Integer to obtain the value from
 * @return      32 bit integer value
 */
VXIVALUE_API VXIint32 VXIIntegerValue(const VXIInteger *i)
{
  if (( i == NULL ) || ( i->GetType( ) != VALUE_INTEGER ))
    return (VXIint32) 0xFFFFFFFF;

  return i->GetValue( );
}


/**
 * Create a Float from a 32 bit floating point number
 *
 * @param   n   32 bit floating point value
 * @return      Float with the specified value on success, 
 *              NULL otherwise
 */
VXIVALUE_API VXIFloat *VXIFloatCreate(VXIflt32 n)
{
  return new VXIFloat (n);
}


/**
 * Float destructor
 *
 * @param   f   Float to destroy
 */
VXIVALUE_API void VXIFloatDestroy(VXIFloat **f)
{
  if (( f != NULL ) && ( *f != NULL ) && 
      ( (*f)->GetType( ) == VALUE_FLOAT )) {
    delete *f;
    *f = NULL;
  }
}


/**
 * Get the value of a Float
 *
 * @param   f   Float to get the value from
 * @return      32 bit floating point value
 */
VXIVALUE_API VXIflt32 VXIFloatValue(const VXIFloat *f)
{
  if (( f == NULL ) || ( f->GetType( ) != VALUE_FLOAT ))
    return (VXIflt32) 0xFFFFFFFF;

  return f->GetValue( );
}


/**
 * Create a Ptr from a C pointer
 *
 * Note: This only stores the pointer blindly, it does not perform a
 * deep copy and the reference memory is not freed on
 * destruction. Thus the user is responsible for ensuring the
 * referenced memory location remains valid, and for freeing memory
 * when appropriate on Ptr destruction.
 *
 * @param   n     Pointer to memory
 * @return        Ptr with the specified value and type on success, 
 *                NULL otherwise
 */
VXIVALUE_API VXIPtr *VXIPtrCreate(void *n)
{
  return new VXIPtr (n);
}


/**
 * Ptr destructor
 *
 * @param   p   Ptr to destroy
 */
VXIVALUE_API void VXIPtrDestroy(VXIPtr **p)
{
  if (( p != NULL ) && ( *p != NULL ) && ( (*p)->GetType( ) == VALUE_PTR )) {
    delete *p;
    *p = NULL;
  }
}


/**
 * Get the value of a Ptr
 *
 * @param   p   Ptr to retrieve the pointer from
 * @return      Pointer to memory retrieved
 */
VXIVALUE_API void *VXIPtrValue(const VXIPtr *p)
{
  if (( p == NULL ) || ( p->GetType( ) != VALUE_PTR ))
    return (void *) 0xFFFFFFFF;

  return p->GetValue( );
}


/**
 * Create a Content from MIME content typed data
 *
 * Thread-safe reference counting is used to allow sharing the data
 * (typically large) across multiple clones while minimizing memory
 * use. The passed Destroy( ) function is only called when the
 * reference count drops to zero.
 *
 * @param   contentType       MIME content type for the data
 * @param   content           Data to store
 * @param   contentSizeBytes  Size of the data, in bytes
 * @param   Destroy           Destructor called to release the data when
 *                            no longer needed
 * @param   userData          Optional user data pointer passed to destroy 
 */
VXIVALUE_API VXIContent *
VXIContentCreate(const VXIchar  *contentType,
		 VXIbyte        *content,
		 VXIulong        contentSizeBytes,
		 void          (*Destroy)(VXIbyte **content, void *userData),
		 void           *userData)
{
  if (( ! contentType ) || ( ! contentType[0] ) || ( ! content ) || 
      ( contentSizeBytes < 1 ) || ( ! Destroy ))
    return NULL;

  VXIContent *c = new VXIContent (contentType, content, contentSizeBytes,
				  Destroy, userData);
  if (( c ) && ( c->GetContent( ) == NULL )) {
    delete c;
    c = NULL;
  }

  return c;
}


/**
 * Content destructor
 *
 * @param   c   Content to destroy
 */
VXIVALUE_API void VXIContentDestroy(VXIContent **c)
{
  if (( c != NULL ) && ( *c != NULL ) && 
      ( (*c)->GetType( ) == VALUE_CONTENT )) {
    delete *c;
    *c = NULL;
  }
}


/**
 * Get the value of a Content
 *
 * @param   c                 Content to retrieve the data from
 * @param   contentType       Returns the MIME content type for the data
 * @param   content           Returns the pointer to the data
 * @param   contentSizeBytes  Returns the size of the data, in bytes
 * @return                    VXIvalue_RESULT_SUCCESS on success 
 */
VXIVALUE_API VXIvalueResult 
VXIContentValue(const VXIContent  *c,
		const VXIchar    **contentType,
		const VXIbyte    **content,
		VXIulong          *contentSizeBytes)
{
  if (( c == NULL ) || ( c->GetType( ) != VALUE_CONTENT ) ||
      ( ! contentType ) || ( ! content ) || ( ! contentSizeBytes )) {
    if ( contentType ) *contentType = NULL;
    if ( content ) *content = NULL;
    if ( contentSizeBytes ) *contentSizeBytes = 0;
    return VXIvalue_RESULT_INVALID_ARGUMENT;
  }
  
  *contentType = c->GetContentType( );
  *content = c->GetContent( );
  *contentSizeBytes = c->GetContentSizeBytes( );
  return VXIvalue_RESULT_SUCCESS;
}


// VXIContentData constructor
VXIContentData::VXIContentData (const VXIchar *ct,
				VXIbyte       *c,
				VXIulong       csb,
				void         (*D)(VXIbyte **content, void *ud),
				void          *ud) :
#ifndef WIN32
  mutex(NULL),
#endif
  refCount(1), contentType(NULL), content(c), contentSizeBytes(csb), 
  Destroy(D), userData(ud)
{
  if (( ct ) && ( *ct )) {
    contentType = new VXIchar [wcslen (ct) + 1];
    if ( contentType ) {
      wcscpy (contentType, ct);
    }
  }

  if ( ! contentType )
    content = NULL;    // Flag for failure

#ifndef WIN32
  if (( content ) && ( VXItrdMutexCreate (&mutex) != VXItrd_RESULT_SUCCESS ))
    content = NULL;
#endif
}


// VXIContentData destructor
VXIContentData::~VXIContentData( )
{
  if ( contentType )
  {
      delete [] contentType;
      contentType = NULL;
  }

  if ( content )
    (*Destroy)(&content, userData);

#ifndef WIN32
  if ( mutex )
    VXItrdMutexDestroy (&mutex);
#endif
}


// VXIContentData add reference
void VXIContentData::AddRef (VXIContentData *data)
{
  if ( data ) {
#ifdef WIN32
    InterlockedIncrement (&(data->refCount));
#else
    if ( VXItrdMutexLock (data->mutex) == VXItrd_RESULT_SUCCESS ) {
      data->refCount++;
      VXItrdMutexUnlock (data->mutex);
    }
#endif
  }
}


// VXIContentData release reference
void VXIContentData::Release (VXIContentData **data)
{
  if (( data ) && ( *data )) {
#ifdef WIN32
    if ( InterlockedDecrement (&((*data)->refCount)) == 0L )
      delete *data;
#else
    if ( VXItrdMutexLock ((*data)->mutex) == VXItrd_RESULT_SUCCESS ) {
      (*data)->refCount--;
      VXIulong refCount = (*data)->refCount;
      VXItrdMutexUnlock ((*data)->mutex);

      if ( refCount == 0 )
	delete *data;
    }
#endif
    
    *data = NULL;
  }
}

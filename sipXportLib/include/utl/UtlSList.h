//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _UtlSList_h_
#define _UtlSList_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlList.h"
#include "os/OsLock.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlContainable;

/**
 * UtlSList is a singularly linked list designed to contain any number
 * of UtlContainable derived object.  The list may contain non-like objects
 * (e.g. UtlInts and UtlVoidPtrs), however, sorting and comparison behavior
 * may be non-obvious.
 *
 * Most list accessors and inquiry methods are performed by equality as
 * opposed to by referencing (pointers).  For example, a list.contains(obj)
 * call will loop through all of the list objects and test equality by calling
 * the isEquals(...) method.  A  list.containsReference(obj) call will search
 * for a pointer match.
 *
 * @see UtlSListIterator
 * @see UtlList
 * @see UtlContainer
 * @see UtlContainable
 */
class UtlSList : public UtlList
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Default Constructor
     */
    UtlSList();


/* ============================ MANIPULATORS ============================== */

    /**
     * Append the designated containable object to the end of this list.
     *
     * @return the object if successful, otherwise null
     */
    virtual UtlContainable* append(UtlContainable* obj) ;

    /// Insert the designated containable object at the designated position.
    virtual UtlContainable* insertAt(size_t N,           ///< zero-based position obj should be
                                     UtlContainable* obj ///< object to insert at N
                                     );
    /**<
     * It is an error to specify N > entries()
     *
     * @return obj if successful, NULL if N > entries
     */

    /**
     * Inserts the designated containable object at the end postion (tailer).
     *
     * @return the object if successful, otherwise null
     */
    virtual UtlContainable* insert(UtlContainable* obj) ;

    /**
     * Remove the designated object by equality (as opposed to by reference).
     */
    virtual UtlContainable* remove(const UtlContainable *);

    /**
     * Removes the designated objects from the list and frees the object
     * by calling delete.
     */
    virtual UtlBoolean destroy(const UtlContainable *);

    /**
     * Creates a copy of all the elements contained in this list and appends them
     * into the list passed as a parameter.  The templated type supplied when
     * calling the method must match the type of object that is contained
     * in this list.  In order to work, the templated type must provide
     * a copy-constructor otherwise a compile-time error will be generated.
     *
     * Note: When copying objects from the source list to the destination list, a
     * new object instance is allocated and its value initialized to the object
     * being copied such that the source and destination lists do not share pointers.
     *
     * Example:
     *   UtlSList sourceList; //some list that will contain pointers to NameValuePair objects
     *   fillSListWithNameValuePairs( sourceList );
     *   UtlSList backupOfSourceList; // some list that will become a copy of sourceList
     *   sourceList.copyTo<NameValuePair>( backupOfSourceList );
     */
    template<typename T> UtlBoolean copyTo( UtlSList& listToCopyInto ) const
    {
       UtlLink* listNode;
       listToCopyInto.destroyAll();

       OsLock take1(mContainerLock);
       OsLock take2(listToCopyInto.mContainerLock);

       LIST_SANITY_CHECK;
       for (listNode = head(); listNode; listNode = listNode->next())
       {
          UtlContainable* visitNode = (UtlContainable*) listNode->data;
          T* visitNodeAsType = dynamic_cast<T*>( visitNode );
          if( visitNodeAsType )
          {
             UtlContainable* newNode = new T( *visitNodeAsType );
             UtlLink::listBefore(&listToCopyInto, NULL, newNode);
          }
       }
       LIST_SANITY_CHECK;

       return TRUE;
    }

/* ============================ ACCESSORS ================================= */


    /**
     * Find the first occurence of the designated object by equality (as
     * opposed to by reference).
     */
    virtual UtlContainable* find(const UtlContainable *) const ;

/* ============================ INQUIRY =================================== */


    /**
     * Return the number of occurrences of the designated object
     */
    virtual size_t occurrencesOf(const UtlContainable* obj) const ;

    /**
     * Return the list position of the designated object or UTL_NOT_FOUND  if
     * not found.
     */
    virtual ssize_t index(const UtlContainable* obj) const ;


    /**
     * Get the ContainableType for the hash bag as a contained object.
     */
    virtual UtlContainableType getContainableType() const;

    static const UtlContainableType TYPE; ///< constant for class type comparison.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    friend class UtlSListIterator;


    /**
     * insertAfter is used by UtlListIterator::insertAfterPoint
     */
    virtual UtlContainable* insertAfter(UtlLink* afterNode, UtlContainable* object);


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlSList_h_

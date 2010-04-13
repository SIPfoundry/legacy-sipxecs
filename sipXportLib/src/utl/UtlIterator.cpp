//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlIterator.h"
#include "utl/UtlContainer.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor

// in this case, we don't need to acquire the sIteratorConnectionLock or take the
// mContainerRefLock because this is constructor,
// so no one has the pointer to this iterator yet.

UtlIterator::UtlIterator(const UtlContainer& container)
   : mContainerRefLock(OsBSem::Q_PRIORITY, OsBSem::FULL),
     // This const_cast is a cheat, because UtlIterator can be used to iterate
     // over const UtlContainer's as well as UtlContainer's, and we leave it to
     // the user to not use the iterator to modify the underlying container
     // if the iterator was created with a const UtlContainer.
     // In a perfect world, there would be UtlIteratorConst separate from
     // UtlIterator, and UtlIteratorConst would not allow modifying the
     // underlying UtlContainer.
     mpMyContainer(const_cast<UtlContainer*>(&container))
{
}


// Copy constructor


// Destructor
UtlIterator::~UtlIterator()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/*****************************************************************
 * The following routines are here to provide (protected) access
 * to the mpIteratorList in the associated UtlContainer.  These
 * operations are invoked here from the derived iterator class
 * constructors and destructors rather than in the UtlIterator
 * constructor and destructor so that the locks can be taken and
 * released in the correct order.  (E.g., if invalidate() was called
 * from ~UtlContainer, then a subclass's data structure would be
 * released before the iterators were invalidated.)
 *****************************************************************/

void UtlIterator::addToContainer(const UtlContainer* container)
{
   // caller is already holding the mContainerLock
   container->addIterator(this);
}

/**
 * invalidate is called by the UtlContainer from its destructor.
 * It disconnects the iterator from its container object (sets mpMyContainer to
 * NULL).
 *
 * Any subsequent invocation of this iterator (other than its
 * destructor) must not attempt to access *mpMyContainer.
 */
void UtlIterator::invalidate()
{
   // Caller holds sIteratorConnectionLock.

   // The caller is holding the sIteratorConnectionLock, so it is OK to lock
   // this.
   OsLock takeContainer(mContainerRefLock);
   mpMyContainer = NULL;
   // it may be that more is needed in the subclasses, but this provides the failsafe
}

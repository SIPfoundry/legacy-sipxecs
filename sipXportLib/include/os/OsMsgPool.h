//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsMsgPool_h_
#define _OsMsgPool_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsMsg.h"
#include "os/OsMutex.h"

/*****************************************************************************
 *                          OS Message Pool
 * Container for a set of reusable OsMsg objects (actually, message objects
 * derived from the OsMsg base class).  All the OsMsg objects in any
 * particular pool must be of the same derived type.
 *
 * The client of the OsMsgPool is the thread or interrupt service routine
 * that retrieves available messages from the pool, fills in the payload,
 * and sends it into an OsMsgQ.  A pool with a single client thread is
 * unshared, while a pool that may be used from more than one thread is
 * shared.
 *
 * Note that an interrupt service routine must use a single client pool of
 * its very own since interrupt routines cannot Take or Give mutexes.  It
 * must also be completely populated when it is created, as an ISR is not
 * permitted to allocate memory, as would be necessary to expand the pool.
 *
 * Note, also, that the receiver of messages that may be from such a pool
 * must not delete the messages explicitly, but rather should always invoke
 * the new OsMsg::releaseMsg(void) method.  This method clears the in-use
 * flag on reusable messages, or deletes one-use messages.  It is a safe
 * and general rule that all OsMsg objects should only be disposed of by
 * way of releaseMsg() and should never be explicitly deleted.  In aid of
 * this rule the destructor, OsMsg::~OsMsg(), now checks that only objects
 * without the is-reusable flag set are deleted.
 *
 * To create the pool, the caller must supply a "model" message, one of the
 * type to be contained in the pool.  The OsMsgPool will clone that message
 * by calling its createCopy() method to allocate the messages contained in
 * the pool.  On return from the OsMsgPool constructor, the model message
 * should be deleted by the caller.
 *
 * The pool will initially be populated with the number of messages specified
 * by the initialCount constructor argument.  The pool may grow beyond that
 * initial size, if so indicated by the other constructor arguments.  To
 * indicate that the pool may be expanded, the caller specifies three more
 * values:
 *
 *   softLimit - a count, larger than initialCount, that will generate a
 *      warning if the automatic expansion increases the pool beyond this
 *      count.
 *
 *   hardLimit - a count, larger than initialCount, that is the absolute
 *      maximum to which the pool can grow.  It is a fatal error if this
 *      count is reached and another element is needed to satisfy a request.
 *      This is the size of the array of OsMsg* pointers allocated in the
 *      constructor.
 *
 *   increment - the number of message to create each time there is no
 *      message available to satisfy a request.
 */


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlString;

//:Manager of a collection of OsMsg objects
class OsMsgPool
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   // Shared among multiple clients, or used by only a single client
   enum OsMsgPoolSharing
   {
      MULTIPLE_CLIENTS,
      SINGLE_CLIENT
   };

/* ============================ CREATORS ================================== */

   OsMsgPool(const char* name, // for identification
      const OsMsg& model,    // message to clone to populate pool
      int initialCount,      // number of messages to create initially
      int softLimit=0,       // number of message without complaining
      int hardLimit=0,       // absolute maximum number of messages
      int increment=1,       // number of messages to allocate when expanding
      OsMsgPoolSharing sharing=MULTIPLE_CLIENTS);
     //:Default constructor.  model is a message of the single type that
     //:will be contained in the pool, and its createCopy virtual method
     //:will be used to populate the pool.  The caller disposes of model

   virtual
   ~OsMsgPool();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsMsg* findFreeMsg(void);
   //:Find and return an available element of the pool, creating more if
   //:necessary and permitted.  Return NULL if failure.

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

   int getNoInUse(void);
   //:Return the number of items in use.

   int getSoftLimit(void);
   //:Return the current soft limit.

   int getHardLimit(void);
   //:Return the current hard limit.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   int      mInitialCount;   // initial number of items
   int      mCurrentCount;   // current number of items
   int      mSoftLimit;      // soft limit, warn when expanding above this
   int      mHardLimit;      // hard limit AND length of mpElts
   int      mIncrement;      // number to create when expanding
   int      mNext;           // index to next element to examine
   OsMutex* mpMutex;         // NULL if single client
   OsMsg*   mpModel;         // model element to clone
   OsMsg**  mpElts;          // array of pointers to contained objects
   UtlString* mpName;         // for ID in error messages

   OsMsgPool(const OsMsgPool& rOsMsgPool);
     //:Copy constructor (not implemented for this class)

   OsMsgPool& operator=(const OsMsgPool& rhs);
     //:Assignment operator (not implemented for this class)

};

#endif  // _OsMsgPool_h_

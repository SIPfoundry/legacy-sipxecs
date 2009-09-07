//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _OsTimerTask_h_
#define _OsTimerTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsMsgQ.h"
#include "os/OsServerTask.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class OsTimer;
class OsTimerMsg;

/** Timer service request manager (runs as a separate task).
 *  This task is responsible for managing timer service requests.  Timer
 *  requests are received via a message queue.
 */
class OsTimerTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   static OsTimerTask* getTimerTask(void);
     //:Return a pointer to the timer task, creating it if necessary

   static void destroyTimerTask(void);
     //: Destroy the singleton instance of the sys timer
     // Should only be called when timers are not being started or stopped.
     // All current timers are stopped.

   virtual
   ~OsTimerTask();
     //:Destructor
     // Should not be called directly.  Use destroyTimerTask().

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /// Constructor (called only indirectly via getTimerTask())
   OsTimerTask();
   /**< We identify this as a protected (rather than a private) method so
    *   that gcc doesn't complain that the class only defines a private
    *   constructor and has no friends.
    */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const int TIMER_MAX_REQUEST_MSGS;   // Maximum number of request messages

   /// The entry point for the task
   virtual int run(void* pArg);
   /**< We replace OsServerTask::run() so that it will simultaneously wait
    *   for an incoming message or for the next timer to fire.
    */

   /// Handle a timer service request.
   virtual UtlBoolean handleMessage(OsMsg& rMsg);
   ///< Return TRUE if the request was handled, otherwise FALSE.

   /** Fire a timer because it has expired.
    *  Calls the if notification routine, if the timer hasn't been stopped
    *  already.
    *  If the timer is periodic and hasn't been stopped, reinserts it into
    *  the queue.
    *  Advances the timer's state if it is one-shot or has been stopped.
    */
   virtual void fireTimer(OsTimer* timer);

   /// Pointer to the single instance of the OsTimerTask class.
   static volatile OsTimerTask* spInstance;
   ///< Declare as volatile because it is set and tested concurrently.

   /// Semaphore used to protect manipulations of spInstance.
   static OsBSem *spLock;

   /// The queue of timer requests, ordered by increasing firing time.
   OsTimer* mTimerQueue;

   /// Insert a timer into the timer queue.
   void insertTimer(OsTimer* timer);

   /// Remove a timer from the timer queue.
   void removeTimer(OsTimer* timer);

   /// Copy constructor (not implemented for this class)
   OsTimerTask(const OsTimerTask& rOsTimerTask);

   /// Assignment operator (not implemented for this class)
   OsTimerTask& operator=(const OsTimerTask& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsTimerTask_h_

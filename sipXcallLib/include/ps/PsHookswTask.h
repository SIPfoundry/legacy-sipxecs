//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsHookswTask_h_
#define _PsHookswTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsgQ.h"
#include "os/OsRWMutex.h"
#include "os/OsServerTask.h"
#include "os/OsTime.h"
#include "ps/PsMsg.h"
#include "ps/PsHookswDev.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class OsEventMsg;
class OsQueuedEvent;
class OsTimer;

//:Task responsible for managing the phone hookswitch
class PsHookswTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum HookswitchState
   {
      ON_HOOK,        // The handset is on hook
      OFF_HOOK,       // The handset is off hook
   };

/* ============================ CREATORS ================================== */

   static PsHookswTask* getHookswTask(void);
     //:Return a pointer to the Hooksw task, creating it if necessary

   virtual
   ~PsHookswTask();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus postEvent(const int msg, void* source,
                              const int hookswState,
                              const OsTime& rTimeout=OsTime::OS_INFINITY);
     //:Create a hookswitch message and post it to the Hookswitch task
     // Return the result of the message send operation.

/* ============================ ACCESSORS ================================= */

   virtual const int getHookswitchState(void);
     //:Return the hookswitch state

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   PsHookswTask();
     //:Constructor (called only indirectly via getHookswTask())
     // We identify this as a protected (rather than a private) method so
     // that gcc doesn't complain that the class only defines a private
     // constructor and has no friends.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum DebounceState
   {
      WAIT_FOR_INTR,
      SHORT_DEBOUNCE,
      LONG_DEBOUNCE
   };

   virtual UtlBoolean handleMessage(OsMsg& rMsg);
     //:Handle an incoming message
     // Return TRUE if the message was handled, otherwise FALSE.

   virtual UtlBoolean handleEventMessage(const OsEventMsg& rMsg);
     //:Handle an incoming event message (timer expiration)
     // Return TRUE if the message was handled, otherwise FALSE.
     // A write lock should be acquired before calling this method.

   virtual UtlBoolean handlePhoneMessage(const PsMsg& rMsg);
     //:Handle an incoming phone message (HOOKSW_OFF or HOOKSW_ON)
     // Return TRUE if the message was handled, otherwise FALSE.
     // A write lock should be acquired before calling this method.

   virtual void startDebounceTimer(void);
     //:Start the debounce timer for the hookswitch

   virtual int readHwHookswState(void);
     //:Read the hookswitch state from the hardware

   OsRWMutex    mMutex;       // mutex for synchonizing access to data
   int          mHookswState; // hookswitch state
   PsHookswDev* mpHookswDev;  // hookswitch device
   int          mDebounceState;
   int          mDebounceTicks;
   int          mDebounceHookswState;
   OsTimer*     mpTimer;      // timer used to debounce the hookswitch
   OsQueuedEvent* mpTimerEvent; // event signaled when the timer expires

   // Static data members used to enforce Singleton behavior
   static PsHookswTask* spInstance; // pointer to the single instance of
                                    //  the PsHookswTask class
   static OsBSem        sLock;      // semaphore used to ensure that there
                                    //  is only one instance of this class

   PsHookswTask(const PsHookswTask& rPsHookswTask);
     //:Copy constructor (not implemented for this task)

   PsHookswTask& operator=(const PsHookswTask& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsHookswTask_h_

//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PsButtonTask_h_
#define _PsButtonTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsBSem.h"
#include "os/OsMsgQ.h"
#include "os/OsRWMutex.h"
#include "os/OsServerTask.h"
#include "os/OsTime.h"
#include "ps/PsKeybdDev.h"
#include "ps/PsMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class OsEventMsg;
class OsTimer;
class PsButtonInfo;

//:Task responsible for managing the phone set buttons
class PsButtonTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   static PsButtonTask* getButtonTask(void);
     //:Return a pointer to the Button task, creating it if necessary

   virtual
   ~PsButtonTask();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus init(const int maxButtonIndex);
     //:Cause the Button task to (re)initialize itself
     // The task will allocate an array [0..maxButtonIndex] of PsButtonInfo
     // objects to hold button state.

   virtual OsStatus postEvent(const int msg, void* source,
                              const int buttonIndex,
                              const OsTime& rTimeout=OsTime::OS_INFINITY);
     //:Create a button message and post it to the Button task
     // Return the result of the message send operation.

   virtual OsStatus setButtonInfo(const int index,
                                  const int buttonId,
                                  const char* buttonName,
                                  const int eventMask,
                                  const OsTime& repInterval=OsTime::OS_INFINITY);
     //:Set the button information for the button designated by "index"

/* ============================ ACCESSORS ================================= */

   virtual const PsButtonInfo& getButtonInfo(const int index);
     //:Return the button information for the button designated by "index"

   virtual int getButtonIndex(int buttonId);
     //:Return the button index for the given button ID value
     // Returns -1 if the key was not found.

        virtual int getButtonIndex(const char* buttonName);
     //:Return the button index for the given button name value
     // Returns -1 if the key was not found.

   virtual int getMaxButtonIndex();
     //:Return the bmax utton index

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   PsButtonTask();
     //:Constructor (called only indirectly via getButtonTask())
     // We identify this as a protected (rather than a private) method so
     // that gcc doesn't complain that the class only defines a private
     // constructor and has no friends.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   virtual UtlBoolean handleMessage(OsMsg& rMsg);
     //:Handle an incoming message
     // Return TRUE if the message was handled, otherwise FALSE.

   virtual UtlBoolean handleEventMessage(const OsEventMsg& rMsg);
     //:Handle an incoming event message (timer expiration)
     // Return TRUE if the message was handled, otherwise FALSE.

   virtual UtlBoolean handlePhoneMessage(PsMsg& rMsg);
     //:Handle an incoming message from the keyboard controller.
     // Return TRUE if the message was handled, otherwise FALSE.

   virtual void disableTimer(intptr_t index);
     //:Disable the repeat timer for the designated button
     // Do nothing if there is no repeat timer in effect for the button.
     // A write lock should be acquired before calling this method.

   virtual void enableTimer(intptr_t index);
     //:Enable the repeat timer for the designated button
     // A write lock should be acquired before calling this method.

   virtual void doCleanup(void);
     //:Release dynamically allocated storage
     // A write lock should be acquired before calling this method.

   int           mMaxBtnIdx;        // max button index
   OsRWMutex     mMutex;            // mutex for synchonizing access to data
   PsButtonInfo* mpButtonInfo;      // ptr to an array of PsButtonInfo objects
   PsKeybdDev*   mpKeybdDev;        // ptr to keyboard device
   OsTimer**     mpRepTimers;       // ptr to an array of button repeat timers

   // Static data members used to enforce Singleton behavior
   static PsButtonTask* spInstance; // pointer to the single instance of
                                    //  the PsButtonTask class
   static OsBSem        sLock;      // semaphore used to ensure that there
                                    //  is only one instance of this class

   PsButtonTask(const PsButtonTask& rPsButtonTask);
     //:Copy constructor (not implemented for this task)

   PsButtonTask& operator=(const PsButtonTask& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsButtonTask_h_

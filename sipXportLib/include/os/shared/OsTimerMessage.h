//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _OsTimerMessage_h_
#define _OsTimerMessage_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsTimer.h"
#include "os/OsMsg.h"
#include "os/OsStatus.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Message used to send timer operation requests
class OsTimerMessage : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum MsgSubType
   {
      UNSPECIFIED,   // Not yet initialized
      ADD,           // Add a new timer
      REMOVE,        // Remove an existing timer
      SHUTDOWN       // Shutdown the timer thread
   };
     //!enumcode: UNSPECIFIED - not yet initialized
     //!enumcode: ADD - add a new timer.
     //!enumcode: REMOVE - remove an existing timer
     //!enumcode: SHUTDOWN - shutdown the timer thread

/* ============================ CREATORS ================================== */

   OsTimerMessage(OsTimer* pTimer, OsBSem* pSem);
     //:Constructor for ADD messages

   OsTimerMessage(int ID, OsBSem* pSem);
     //:Constructor for REMOVE messages

   OsTimerMessage(OsBSem* pSem);
     //:Constructor for SHUTDOWN messages

   OsTimerMessage(const OsTimerMessage& rOsTimerMessage);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual
   ~OsTimerMessage();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsTimerMessage& operator=(const OsTimerMessage& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

   virtual int getMsgSize(void) const;
     //:Return the size of the message in bytes
     // This is a virtual method so that it will return the accurate size for
     // the message object even if that object has been upcast to the type of
     // an ancestor class.

   int getTimerID(void);
     //:Get the timer ID (for REMOVE messages)

   OsTimer* getTimer(void);
     //:Get the OsTimer (for ADD messages)

   OsBSem* getSynchSem(void);
     //:Get the synchronization semaphore (for ADD and REMOVE messages)

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   int      mID;
   OsTimer* mpTimer;
   OsBSem*  mpSynchSem;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsTimerMessage_h_

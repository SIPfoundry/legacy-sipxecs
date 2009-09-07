//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsEventMsg_h_
#define _OsEventMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsMsg.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class OsQueuedEvent;

//:Message used to send event notifications
class OsEventMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum MsgSubType
   {
      UNSPECIFIED,   // Not yet initialized
      NOTIFY,        // Event notification
      USER_START  = 128
   };
   //!enumcode: UNSPECIFIED - not yet initialized
   //!enumcode: NOTIFY - event notification.
   //!enumcode: USER_START - User defined message type categories start at USER_START

/* ============================ CREATORS ================================== */

   OsEventMsg(const unsigned char subType, const OsQueuedEvent& rEvent,
              intptr_t eventData, const OsTime& rTimestamp);
     //:Constructor
     // userData is rEvent.getUserData(...).

   OsEventMsg(unsigned char subType,
              intptr_t eventData,
              void* userData);
     //:Constructor

   OsEventMsg(const OsEventMsg& rOsEventMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual
   ~OsEventMsg();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsEventMsg& operator=(const OsEventMsg& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

   virtual int getMsgSize(void) const;
     //:Return the size of the message in bytes
     // This is a virtual method so that it will return the accurate size for
     // the message object even if that object has been upcast to the type of
     // an ancestor class.

   virtual OsStatus getEventData(intptr_t& rEventData) const;
     //:Return the event data that was signaled by the notifier task
     // Always returns OS_SUCCESS.

   virtual OsStatus getTimestamp(OsTime& rTimestamp) const;
     //:Return the timestamp associated with this event
     // Always returns OS_SUCCESS.

   virtual OsStatus getUserData(void*& rUserData) const;
     //:Return the user data specified when the OsQueuedEvent was constructed
     // Always returns OS_SUCCESS.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   intptr_t  mEventData;
   void*     mUserData;
   OsTime    mTimestamp;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsEventMsg_h_

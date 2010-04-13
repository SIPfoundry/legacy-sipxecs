//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsProtectedEvent_h_
#define _OsProtectedEvent_h_

#include "os/OsEvent.h"
#include "os/OsBSem.h"
#include "os/OsTime.h"
#include "utl/UtlString.h"

//#define TAO_DEBUG

class OsProtectedEvent : public OsEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
        OsProtectedEvent(void* userData=0);
        virtual ~OsProtectedEvent();

/* ============================ MANIPULATORS ============================== */


   virtual OsStatus signal(intptr_t eventData);
     //:Set the event data and signal the occurrence of the event
     // Return OS_ALREADY_SIGNALED if the event has already been signaled
     // (and has not yet been cleared), otherwise return OS_SUCCESS.

   virtual OsStatus reset(void);
     //:Reset the event so that it may be signaled again
     // Return OS_NOT_SIGNALED if the event has not been signaled (or has
     // already been cleared), otherwise return OS_SUCCESS.

   virtual OsStatus wait(int msgId, const OsTime& rTimeout=OsTime::OS_INFINITY);
     //:Wait for the event to be signaled
     // Return OS_BUSY if the timeout expired, otherwise return OS_SUCCESS.

        void setStringData(UtlString& rStringData);

        void setIntData(intptr_t rIntData);

        void setIntData2(intptr_t rIntData);

        void setInUse(UtlBoolean inUse);

/* ============================ ACCESSORS ================================= */

        OsStatus getStringData(UtlString& data);
         //:Return the user data specified when this object was constructed.
         // Always returns OS_SUCCESS.

        OsStatus getIntData(intptr_t& data);

        OsStatus getIntData2(intptr_t& data);
         //:Return the user data specified when this object was constructed.
         // Always returns OS_SUCCESS.

/* ============================ INQUIRY =================================== */

   virtual UtlBoolean isInUse();
     //:Return TRUE if the event has been signaled, otherwise FALSE

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

        OsBSem          mRefSem;   // semaphore used to protect mRef
        UtlString               mStringData;
        intptr_t                mIntData;
        intptr_t                mIntData2;
        int                     mRef;                   // reference count


};

#endif // _OsProtectedEvent_h_

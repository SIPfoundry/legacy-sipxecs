//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoEvent_h_
#define _TaoEvent_h_

#include <utl/UtlString.h>

#include "os/OsEvent.h"
#include "os/OsMutex.h"
#include "tao/TaoDefs.h"

//#define TAO_DEBUG

class TaoEvent : public OsEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
        TaoEvent(const void* userData=0);

        TaoEvent(const TaoEvent& rTaoEvent);
     //:Copy constructor (not implemented for this class)

        virtual ~TaoEvent();

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus reset(void);
     //:Reset the event so that it may be signaled again
     // Return OS_NOT_SIGNALED if the event has not been signaled (or has
     // already been cleared), otherwise return OS_SUCCESS.

   virtual OsStatus wait(int msgId, const OsTime& rTimeout=OsTime::OS_INFINITY);
     //:Wait for the event to be signaled
     // Return OS_BUSY if the timeout expired, otherwise return OS_SUCCESS.

   void setMutex(OsMutex* pMutex);

        void setStringData(UtlString& rStringData);

        void setIntData(int rIntData);

        void setIntData2(int rIntData);


/* ============================ ACCESSORS ================================= */

        TaoStatus getStringData(UtlString& data);
         //:Return the user data specified when this object was constructed.
         // Always returns OS_SUCCESS.

        TaoStatus getIntData(int& data);

        TaoStatus getIntData2(int& data);
         //:Return the user data specified when this object was constructed.
         // Always returns OS_SUCCESS.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        UtlString       mStringData;
        int                     mIntData;
        int                     mIntData2;
        OsMutex*        mpMutex;
#ifdef TAO_DEBUG
        int                     mWaits;
#endif


};

#endif // _TaoEvent_h_

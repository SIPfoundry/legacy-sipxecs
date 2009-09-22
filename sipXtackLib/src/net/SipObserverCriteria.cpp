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
#include <assert.h>

// APPLICATION INCLUDES
#include <net/SipObserverCriteria.h>
#include <net/SipSession.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipObserverCriteria::SipObserverCriteria(void* observerData,
                                         OsMsgQ* messageQueue,
                                          const char* sipMethod,
                                          UtlBoolean wantRequests,
                                          UtlBoolean wantResponses,
                                          UtlBoolean wantIncoming,
                                          UtlBoolean wantOutGoing,
                                          const char* eventName,
                                          SipSession* pSession
                                          ) :
UtlString(sipMethod ? sipMethod : "")
{
   mObserverData = observerData;
   mpMessageObserverQueue = messageQueue;
   mWantsRequests = wantRequests;
   mWantsResponses = wantResponses;
   mWantsIncoming = wantIncoming;
   mWantsOutGoing = wantOutGoing;
   mEventName = eventName ? eventName : "";

   // Make a copy of the session
   if (pSession != NULL)
      mpSession = new SipSession(*pSession) ;
   else
      mpSession = NULL ;
}

// Copy constructor
SipObserverCriteria::SipObserverCriteria(const SipObserverCriteria& rSipObserverCriteria)
{
}


// Destructor
SipObserverCriteria::~SipObserverCriteria()
{
   if (mpSession != NULL)
   {
      delete mpSession ;
      mpSession = NULL ;
   }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipObserverCriteria&
SipObserverCriteria::operator=(const SipObserverCriteria& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */
OsMsgQ* SipObserverCriteria::getObserverQueue()
{
    return(mpMessageObserverQueue);
}

const char* SipObserverCriteria::getSipMethod() {
   return data();
}

void* SipObserverCriteria::getObserverData()
{
    return(mObserverData);
}

void SipObserverCriteria::getEventName(UtlString& eventName)
{
    eventName = mEventName;
}

SipSession* SipObserverCriteria::getSession()
{
    return (mpSession);
}

/* ============================ INQUIRY =================================== */

UtlBoolean SipObserverCriteria::wantsRequests()
{
    return(mWantsRequests);
}

UtlBoolean SipObserverCriteria::wantsResponses()
{
    return(mWantsResponses);
}

UtlBoolean SipObserverCriteria::wantsIncoming()
{
    return(mWantsIncoming);
}

UtlBoolean SipObserverCriteria::wantsOutGoing()
{
    return(mWantsOutGoing);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

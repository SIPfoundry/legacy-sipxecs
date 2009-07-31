//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtCallEvent_h_
#define _PtCallEvent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <ptapi/PtEvent.h>
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtCall;
class TaoClientTask;

//:PtCallEvent contains PtCall-associated event data

class PtCallEvent : public PtEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    PT_CLASS_INFO_MEMBERS

/* ============================ CREATORS ================================== */

   PtCallEvent(PtEvent::PtEventId eventId = EVENT_INVALID);
     //:Default constructor

        PtCallEvent(PtEventId eventId,
                                int metaCode,
                                const char* callId,
                                TaoClientTask *pClient,
                                int sipResponseCode,
                                const char* sipResponseText,
                                const char* newCallId = 0,
                                const char** oldCallIds = 0,
                                int numOldCalls = 0);
        //: Constructor.

        PtCallEvent(TaoClientTask *pClient);
        //: Constructor.

        PtCallEvent(const PtCallEvent& rPtCallEvent);
     //:Copy constructor

   virtual
   ~PtCallEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtCallEvent& operator=(const PtCallEvent& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

   PtStatus getCall(PtCall& rCall) const;
     //:Returns the call object associated with this event.
     //!param: (out) rCall - The reference used to return the call pointer
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   static const char* className();
     //:Returns the name of this class.
     //!returns: Returns the string representation of the name of this class

/* ============================ INQUIRY =================================== */

   virtual PtBoolean isClass(const char* pClassName);
     //:Determines if this object if of the specified type.
     //!param: (in) pClassName - the string to compare with the name of this class.
     //!retcode: TRUE - if the given string contains the class name of this class.
     //!retcode: FALSE - if the given string does not match that of this class

   virtual PtBoolean isInstanceOf(const char* pClassName);
     //:Determines if this object is either an instance of or is derived from
     //:the specified type.
     //!param: (in) pClassName - the string to compare with the name of this class.
     //!retcode: TRUE - if this object is either an instance of or is derived from the specified class.
     //!retcode: FALSE - if this object is not an instance of the specified class.

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtCallEvent_h_

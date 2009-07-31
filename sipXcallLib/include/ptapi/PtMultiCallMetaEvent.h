//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtMultiCallMetaEvent_h_
#define _PtMultiCallMetaEvent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtMetaEvent.h"
#include "ptapi/PtCall.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The <CODE>MultiCallMetaEvent</CODE> interface is the base interface for
// all multiple-call Call Meta events (multi-call MetaEvent).
// All multi-call MetaEvent's must extend this interface.
// Events which extend this interface are reported via the
// <CODE>CallListener</CODE> interface.
// <p>
// An individual <CODE>MultiCallMetaEvent</CODE>
// conveys, directly and with necessary details,
// what an application needs to know to respond to
// a multiple-call higher-level call event.
// The specific MultiCallMetaEvent event is indicated by
// the <CODE>Event.getID()</CODE>
// value returned by the event.
// <p>
// The core package defines events which are reported when
// high-level actions occur. The event IDs
// (as returned by <CODE>Event.getID()</CODE>) are:
// <CODE>MULTICALL_META_MERGE_STARTED</CODE>,
// <CODE>MULTICALL_META_MERGE_ENDED</CODE>,
// <CODE>MULTICALL_META_TRANSFER_STARTED</CODE> and
// <CODE>MULTICALL_META_TRANSFER_ENDED</CODE>.


class PtMultiCallMetaEvent : public PtMetaEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

        PT_CLASS_INFO_MEMBERS

    enum PtMultiCallMetaEventId
    {
                MULTICALL_META_MERGE_START              = 600,
                MULTICALL_META_MERGE_END                = 601,
                MULTICALL_META_TRANSFER_START   = 602,
                MULTICALL_META_TRANSFER_END             = 603
        };

/* ============================ CREATORS ================================== */
   PtMultiCallMetaEvent(PtEventId eventId = EVENT_INVALID);
   //:Default constructor

   PtMultiCallMetaEvent(PtEventId eventId,
                                                int metaCode,
                                                TaoClientTask *pClient,
                                                int sipResponseCode,
                                                const char*  sipResponseText,
                                                const char*  callId = 0,
                                                const char*  newCallId = 0,
                                                const char** oldCallIds = 0,
                                                int numOldCalls = 0);

   PtMultiCallMetaEvent(const PtMultiCallMetaEvent& rPtMultiCallMetaEvent);
     //:Copy constructor

   virtual
   ~PtMultiCallMetaEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtMultiCallMetaEvent& operator=(const PtMultiCallMetaEvent& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

        PtStatus getNewCall(PtCall& rCall) const;
                //:Returns the Call object associated with the result of the multiple-call operation
                // reported by this MultiCallMetaEvent event.
                //!param: (out) rCalls - The result Call associated with this event.
                //!retcode: PT_SUCCESS - Success
                //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

        PtStatus getOldCalls(PtCall rCalls[], int size, int& rNumItems) const;
                //:Returns an array of Call objects which were considered input to the multiple-call operation
                // reported by this MultiCallMetaEvent event.
                //!param: (out) rCalls - An array of old Calls associated with this event's operation.
                //!param: (in) size - Size of the array.
                //!retcode: PT_SUCCESS - Success
                //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   PtStatus numOldCalls(int& count) const;
     //:Returns the number of calls involved with this multicall event.
     //!param: (out) count - The number of calls
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

        static const char* className();
     //:Returns the name of this class.
     //!returns: Returns the string representation of the name of this class

/* ============================ INQUIRY =================================== */

   virtual PtBoolean isClass(const char* pClassName);
     //:Determines if this object if of the specified type.
     //!param: (in) pClassName - The string to compare with the name of this class.
     //!retcode: TRUE - If the given string contains the class name of this class.
     //!retcode: FALSE - If the given string does not match that of this class

   virtual PtBoolean isInstanceOf(const char* pClassName);
     //:Determines if this object is either an instance of or is derived from
     //:the specified type.
     //!param: (in) pClassName - The string to compare with the name of this class.
     //!retcode: TRUE - If this object is either an instance of or is derived from the specified class.
     //!retcode: FALSE - If this object is not an instance of the specified class.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtMultiCallMetaEvent_h_

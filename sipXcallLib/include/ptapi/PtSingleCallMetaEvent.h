//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtSingleCallMetaEvent_h_
#define _PtSingleCallMetaEvent_h_

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

//:The <CODE>SingleCallMetaEvent</CODE> interface is the base interface for
// all single-call Call Meta events.
// All single-call MetaEvent's must extend this interface.
// Events which extend this interface are reported via the
// <CODE>CallListener</CODE> interface.
// <p>
// An individual <CODE>SingleCallMetaEvent</CODE>
// conveys, directly and with necessary details,
// what an application needs to know to respond to
// a higher-level call event that only effects a single call.
// The specific SingleCallMetaEvent event is indicated by
// the <CODE>Event.getID()</CODE>
// value returned by the event.
// <p>
// Since all these Meta events relate to a single JTAPI Call entity,
// this interface provides the method getCall to grant the application access
// to the affected Call.
// <p>
// The core package defines events which are reported when
// high-level actions occur. The event IDs
// (as returned by <CODE>Event.getID()</CODE>) are:
// <CODE>SINGLECALL_META_PROGRESS_STARTED</CODE>,
// <CODE>SINGLECALL_META_PROGRESS_ENDED</CODE>,
// <CODE>MULTICALL_META_TRANSFER_STARTED</CODE> and
// <CODE>MULTICALL_META_TRANSFER_ENDED</CODE>.

class PtSingleCallMetaEvent : public PtMetaEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

        PT_CLASS_INFO_MEMBERS

        // Event id codes returned for this interface
        enum PtSingleCallMetaEventId
        {
                SINGLECALL_META_PROGRESS_START  = 610,
                SINGLECALL_META_PROGRESS_END    = 611,
                SINGLECALL_META_SNAPSHOT_START  = 612,
                SINGLECALL_META_SNAPSHOT_END    = 613,

        CALL_META_ADD_PARTY_STARTED                     = 620,
        CALL_META_ADD_PARTY_ENDED                       = 621,
        CALL_META_REMOVE_PARTY_STARTED          = 622,
        CALL_META_REMOVE_PARTY_ENDED            = 623,

        CALL_META_CALL_STARTING_STARTED         = 624,
        CALL_META_CALL_STARTING_ENDED           = 625,
        CALL_META_CALL_ENDING_STARTED           = 626,
        CALL_META_CALL_ENDING_ENDED                     = 627,
        };
    //!enumcode: SINGLECALL_META_PROGRESS_STARTED - event indicates that the current call in the telephony platform has changed state, and events will follow which indicate the changes to this call. <br>This constant indicates a specific event passed via a <CODE>SingleCallMetaEvent</CODE> event, and is reported on the <CODE>CallListener</CODE> interface.
    //!enumcode: SINGLECALL_META_PROGRESS_ENDED - event indicates that the current call in the telephony platform has changed state, and all the events that were associated with that change have now been reported.<p> This constant indicates a specific event passed via a <CODE>SingleCallMetaEvent</CODE> event, and is reported on the <CODE>CallListener</CODE> interface.
    //!enumcode: SINGLECALL_META_SNAPSHOT_STARTED - event indicates that the JTAPI implementation is reporting to the application the current state of the call on the associated telephony platform, by reporting a set of simulated state changes that in effect construct the current state of the call. The events which follow convey that current state.<p> This constant indicates a specific event passed via a <CODE>SingleCallMetaEvent</CODE> event, and is reported on the <CODE>CallListener</CODE> interface.
    //!enumcode: SINGLECALL_META_SNAPSHOT_ENDED - event indicates that the JTAPI implementation has finished reporting a set of simulated state changes that in effect construct the current state of the call.<p> This constant indicates a specific event passed via a <CODE>SingleCallMetaEvent</CODE> event, and is reported on the <CODE>CallListener</CODE> interface.

/* ============================ CREATORS ================================== */
   PtSingleCallMetaEvent(PtEventId eventId,
                                                int metaCode,
                                                TaoClientTask *pClient,
                                                int sipResponseCode,
                                                const char* sipResponseText,
                                                const char* callId = 0);
     //:Default constructor

   PtSingleCallMetaEvent(const PtSingleCallMetaEvent& rPtSingleCallMetaEvent);
     //:Copy constructor

   virtual
   ~PtSingleCallMetaEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtSingleCallMetaEvent& operator=(const PtSingleCallMetaEvent& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

        PtStatus getCall(PtCall& rCall) const ;
                //:Returns the Call object associated with the higher-level operation
                // reported by this SingleCallMetaEvent event.
                //!param: (out) rCall - The result Call associated with this event.
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

#endif  // _PtSingleCallMetaEvent_h_

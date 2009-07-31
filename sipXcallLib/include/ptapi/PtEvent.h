//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtEvent_h_
#define _PtEvent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include "ptapi/PtDefs.h"

// DEFINES
#define MAX_OLD_CALLS 10

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtMetaEvent;
class TaoClientTask;

//! The PtEvent encapulates data associated with an event notification.
//! The PtEvent is sent to an application implementing a PtEventListener
//! by the Pingtel object in which the event occurs.
/*!
 * <H3>Event/Listener Model</H3>
 * PtEvent is subclassed to contain the PTAPI object(s) and data
 * involved with the event.  An application that is interested in events
 * from a specific object must implement a PtEventListener that handles
 * the specific type of event(s) of interest.  In order to start the
 * reporting of events, the application adds its listener to the object
 * instance that will report those events.  When the
 * events for that object occur, a method on the listener is called,
 * specific to that event type.
 * <p>
 *
 * The application implementing the listener knows which specific event
 * has occurred based upon the listener method that was invoked.
 * However the PtEvent also contains a PtEventId which also defines
 * the specific event that occured.  In addition, the PtEvent contains
 * a PtEventCause which provides information as to why the event occurred.
 * <p>
 *
 * The methods on the PtEventListener, that are called when a specific event
 * occurs, take a PtEvent as an argument.  The application is likely
 * to want to up-cast the PtEvent to access data specific to the
 * PtEvent subtype.  To ease this, the PtEvent provides methods to query
 * its class name and inheritance.
 *
 * <H3>Meta Events</H3>
 * Events very often occur in logical groups of related events.  To provide
 * this grouping, the concept of meta events is used.  Meta events occur in
 * pairs to indicate the start and end of the logical group of events.
 * Events that belong to the logical group (including the end meta event)
 * have a meta event set to the start meta event.  Note that due to the
 * distributed nature of the Pingtel system, the events may not all be
 * received as a group or sequentially.  That is events, which do not
 * belong to this or perhaps any meta group of events may be received
 * after the start meta event and before end meta event is received.
 */

class PtEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    PT_CLASS_INFO_MEMBERS

    enum PtEventId
    {
        EVENT_INVALID = 0,

                // core
        ADDRESS_EVENT_TRANSMISSION_ENDED        = 100,

        CALL_ACTIVE                                                     = 101,
        CALL_INVALID                                            = 102,
        CALL_EVENT_TRANSMISSION_ENDED           = 103,

        CONNECTION_ALERTING                                     = 104,
        CONNECTION_CONNECTED                            = 105,
        CONNECTION_CREATED                                      = 106,
        CONNECTION_DISCONNECTED                         = 107,
        CONNECTION_FAILED                                       = 108,
        CONNECTION_IN_PROGRESS                          = 109,
        CONNECTION_UNKNOWN                                      = 110,

        PROVIDER_IN_SERVICE                                     = 111,
        PROVIDER_EVENT_TRANSMISSION_ENDED       = 112,
        PROVIDER_OUT_OF_SERVICE                         = 113,
        PROVIDER_SHUTDOWN                                       = 114,

                TERMINAL_CONNECTION_ACTIVE                      = 115,
                TERMINAL_CONNECTION_CREATED                     = 116,
                TERMINAL_CONNECTION_DROPPED                     = 117,
                TERMINAL_CONNECTION_PASSIVE                     = 118,
                TERMINAL_CONNECTION_RINGING                     = 119,
                TERMINAL_CONNECTION_UNKNOWN                     = 120,

                TERMINAL_EVENT_TRANSMISSION_ENDED       = 121,

                  // call control
        ADDRESS_DO_NOT_DISTURB_ENABLED          = 200,
        ADDRESS_FORWARDING_CHANGED                      = 201,
        ADDRESS_MESSAGES_WAITING                        = 202,
        ADDRESS_DO_NOT_DISTURB_DISABLED         = 241,
        ADDRESS_NO_MESSAGES_WAITING                     = 244,

        CONNECTION_DIALING                                      = 204,
        CONNECTION_ESTABLISHED                          = 206,
        CONNECTION_IDLE                                         = 207,
        CONNECTION_INITIATED                            = 208,
        CONNECTION_NETWORK_ALERTING                     = 209,
        CONNECTION_NETWORK_REACHED                      = 210,
        CONNECTION_OFFERED                                      = 211,
        CONNECTION_QUEUED                                       = 212,

        TERMINAL_CONNECTION_BRIDGED                     = 214,
        TERMINAL_CONNECTION_HELD                        = 216,
        TERMINAL_CONNECTION_IN_USE                      = 217,
        TERMINAL_CONNECTION_TALKING                     = 219,
        TERMINAL_CONNECTION_DO_NOT_DISTURB      = 221,
        TERMINAL_CONNECTION_IDLE                        = 222,

        PHONE_BUTTON_INFO_CHANGED                       = 500,
        PHONE_BUTTON_DOWN                                       = 501,
        PHONE_DISPLAY_CHANGED                           = 502,
        PHONE_HOOKSWITCH_OFFHOOK                        = 503,
        PHONE_LAMP_MODE_CHANGED                         = 504,
        PHONE_MICROPHONE_GAIN_CHANGED           = 505,
        PHONE_RINGER_PATTERN_CHANGED            = 506,
        PHONE_RINGER_VOLUME_CHANGED                     = 507,
        PHONE_SPEAKER_VOLUME_CHANGED            = 508,

                PHONE_BUTTON_UP                                         = 510,
                PHONE_BUTTON_REPEAT                                     = 511,
                PHONE_EXTSPEAKER_CONNECTED                      = 512,
                PHONE_EXTSPEAKER_DISCONNECTED           = 513,
                PHONE_HANDSET_VOLUME_CHANGED            = 514,
                PHONE_HANDSETMIC_GAIN_CHANGED           = 515,
                PHONE_HOOKSWITCH_ONHOOK                         = 516,
                PHONE_RINGER_INFO_CHANGED                       = 517,

                MULTICALL_META_MERGE_STARTED            = 600,
                MULTICALL_META_MERGE_ENDED                      = 601,
                MULTICALL_META_TRANSFER_STARTED         = 602,
                MULTICALL_META_TRANSFER_ENDED           = 603,
                MULTICALL_META_REPLACE_STARTED          = 604,
                MULTICALL_META_REPLACE_ENDED            = 605,

                SINGLECALL_META_PROGRESS_STARTED        = 610,
                SINGLECALL_META_PROGRESS_ENDED          = 611,
                SINGLECALL_META_SNAPSHOT_STARTED        = 612,
                SINGLECALL_META_SNAPSHOT_ENDED          = 613,

        CALL_META_ADD_PARTY_STARTED                     = 620,
        CALL_META_ADD_PARTY_ENDED                       = 621,
        CALL_META_REMOVE_PARTY_STARTED          = 622,
        CALL_META_REMOVE_PARTY_ENDED            = 623,

        CALL_META_CALL_STARTING_STARTED         = 624,
        CALL_META_CALL_STARTING_ENDED           = 625,
        CALL_META_CALL_ENDING_STARTED           = 626,
        CALL_META_CALL_ENDING_ENDED                     = 627,

        PROVIDER_ADDRESS_ADDED                          = 630,
        PROVIDER_ADDRESS_REMOVED                        = 631,
        PROVIDER_TERMINAL_ADDED                         = 632,
        PROVIDER_TERMINAL_REMOVED                       = 633
    };
    //:Event Ids
    // The following table defines the event ids for all of the events
    // supported by the Pingtel system.
    //
    /* Provider Events */
    //!enumcode: PROVIDER_EVENT_TRANSMISSION_ENDED - Indicates that the application will no longer receive provider events on this instance of the PtProviderListener.
    //!enumcode: PROVIDER_IN_SERVICE - Indicates that the state of the PtProvider object has changed to PtProvider::IN_SERVICE.
    //!enumcode: PROVIDER_OUT_OF_SERVICE - Indicates that the state of the Provider object has changed to PtProvider::OUT_OF_SERVICE.
    //!enumcode: PROVIDER_SHUTDOWN - Indicates that the state of the PtProvider object has changed to PtProvider::SHUTDOWN.
    //!enumcode: PROVIDER_ADDRESS_ADDED - Indicates that a new PtAddress has been added to the provider.
    //!enumcode: PROVIDER_ADDRESS_REMOVED - Indicates that a PtAddress has been removed from the provider.
    //!enumcode: PROVIDER_TERMINAL_ADDED - Indicates that a new PtTerminal has been added to the provider.
    //!enumcode: PROVIDER_TERMINAL_REMOVED - Indicates that a PtTerminal has been removed from the provider.

    /* Address Events */
    //!enumcode: ADDRESS_EVENT_TRANSMISSION_ENDED - Indicates that the application will no longer receive address events on this instance of the PtAddressListener.
    /* call control: */
    //!enumcode: ADDRESS_DO_NOT_DISTURB_ENABLED - Indicates the state of the do-not-disturb feature has changed to enabled for the PtAddress.
    //!enumcode: ADDRESS_DO_NOT_DISTURB_DISABLED - Indicates the state of the do-not-disturb feature has changed to disabled for the PtAddress.
    //!enumcode: ADDRESS_FORWARDING_CHANGED - Indicates the state of the forward feature has changed for the PtAddress.
    //!enumcode: ADDRESS_MESSAGES_WAITING - Indicates the state of the message waiting feature has changed to messages waiting for the PtAddress.
    //!enumcode: ADDRESS_NO_MESSAGES_WAITING - Indicates the state of the message waiting feature has changed to no messages waiting for the PtAddress.

    /* Terminal Events */
    //!enumcode: TERMINAL_EVENT_TRANSMISSION_ENDED - Indicates that the application will no longer receive terminal events on this instance of the PtTerminalListener.

    /* Call Events */
    //!enumcode: CALL_EVENT_TRANSMISSION_ENDED - Indicates that the application will no longer receive call events on this instance of the PtCallListener.
    //!enumcode: CALL_ACTIVE - Indicates that the state of the call object has changed to PtCall::ACTIVE
    //!enumcode: CALL_INVALID - Indicates that the state of the call object has changed to PtCall::INVALID.
    //!enumcode: CALL_META_PROGRESS_STARTED - Indicates that the current call in the telephony platform has changed state, and events will follow which indicate the changes to this call.
    //!enumcode: CALL_META_PROGRESS_ENDED - Indicates that the current call in the telephony platform has changed state, and all the events that were associated with that change have now been reported.
    //!enumcode: CALL_META_SNAPSHOT_STARTED - Indicates that the Pingtel implementation is reporting to the application the current state of the call on the associated telephony platform, by reporting a set of simulated state changes that, in effect, construct the current state of the call.
    //!enumcode: CALL_META_SNAPSHOT_ENDED - Indicates that the Pingtel implementation has finished reporting a set of simulated state changes that, in effect, construct the current state of the call.
    /* call control */
    //!enumcode: CALL_META_ADD_PARTY_STARTED - Indicates that a party has been added to the call. A "party" corresponds to a PtConnection being added. Note that if a PtTerminalConnection is added, it carries a meta event of CALL_META_PROGRESS_STARTED.
    //!enumcode: CALL_META_ADD_PARTY_ENDED - Indicates the end of the group of events related to the add party meta event.
    //!enumcode: CALL_META_REMOVE_PARTY_STARTED - Indicates that a party (i.e., connection) has been removed from the call by moving into the PtConnection::DISCONNECTED state.
    //!enumcode: CALL_META_REMOVE_PARTY_ENDED - Indicates the end of the group of events related to the remove party meta event.

    /* Multi-Call Events */
    //!enumcode: MULTICALL_META_MERGE_STARTED - Indicates that calls are merging, and events will follow which indicate the changes to those calls.
    //!enumcode: MULTICALL_META_MERGE_ENDED - Indicates that calls have merged, and that all state change events resulting from this merge have been reported.
    //!enumcode: MULTICALL_META_TRANSFER_STARTED - Indicates that a transfer is occurring, and events will follow which indicate the changes to the affected calls.
    //!enumcode: MULTICALL_META_TRANSFER_ENDED - Indicates that a transfer has completed, and that all state change events resulting from this transfer have been reported.

    /* Connection Events */
    //!enumcode: CONNECTION_CREATED - Indicates that a new PtConnection object has been created.
    /* replaced by callctrl CONNECTION_IN_PROGRESS - Indicates that the state of the PtConnection object has changed to PtConnection::IN_PROGRESS. */
    //!enumcode: CONNECTION_ALERTING - Indicates that the state of the PtConnection object has changed to PtConnection::ALERTING.
    /* replaced by callctrl CONNECTION_CONNECTED - Indicates that the state of the PtConnection object has changed to PtConnection::CONNECTED. */
    //!enumcode: CONNECTION_DISCONNECTED - Indicates that the state of the PtConnection object has changed to PtConnection::DISCONNECTED.
    //!enumcode: CONNECTION_FAILED - Indicates that the state of the PtConnection object has changed to PtConnection::FAILED.
    //!enumcode: CONNECTION_UNKNOWN - Indicates that the state of the PtConnection object has changed to PtConnection::UNKNOWN.
    /* call control */
    //!enumcode: CONNECTION_DIALING - Indicates that the state of the PtConnection object has changed to PtConnection::DIALING.
    //!enumcode: CONNECTION_ESTABLISHED - Indicates that the state of the PtConnection object has changed to PtConnection::ESTABLISHED.
    //!enumcode: CONNECTION_INITIATED -  Indicates that the state of the PtConnection object has changed to PtConnection::INITIATED.
    //!enumcode: CONNECTION_NETWORK_ALERTING - Indicates that the state of the PtConnection object has changed to PtConnection::NETWORK_ALERTING.
    //!enumcode: CONNECTION_NETWORK_REACHED - Indicates that the state of the PtConnection object has changed to PtConnection::NETWORK_REACHED.
    //!enumcode: CONNECTION_OFFERED - Indicates that the state of the PtConnection object has changed to PtConnection::OFFERED.
    //!enumcode: CONNECTION_QUEUED - Indicates that the state of the PtConnection object has changed to PtConnection::QUEUED.

    /* Terminal Connection Events */
    //!enumcode: TERMINAL_CONNECTION_CREATED - Indicates that a new PtTerminalConnection object has been created.
    /* replaced by callctrl TERMINAL_CONNECTION_ACTIVE - Indicates that the state of the PtTerminalConnection object has changed to PtTerminalConnection::ACTIVE. */
    /* TERMINAL_CONNECTION_PASSIVE - Indicates that the state of the PtTerminalConnection object has changed to PtTerminalConnection::PASSIVE. */
    //!enumcode: TERMINAL_CONNECTION_RINGING - Indicates that the state of the PtTerminalConnection object has changed to PtTerminalConnection::RINGING.
    //!enumcode: TERMINAL_CONNECTION_DROPPED - Indicates that the state of the PtTerminalConnection object has changed to PtTerminalConnection::DROPPED.
    //!enumcode: TERMINAL_CONNECTION_UNKNOWN - Indicates that the state of the PtTerminalConnection object has changed to PtTerminalConnection::UNKNOWN.
    /* call control */
    /* TERMINAL_CONNECTION_BRIDGED - Indicates that the state of the PtTerminalConnection object has changed to PtTerminalConnection::BRIDGED. */
    //!enumcode: TERMINAL_CONNECTION_HELD -  Indicates that the state of the PtTerminalConnection object has changed to PtTerminalConnection::HELD.
    /* TERMINAL_CONNECTION_IN_USE -  Indicates that the state of the PtTerminalConnection object has changed to PtTerminalConnection::INUSE. */
    //!enumcode: TERMINAL_CONNECTION_TALKING - Indicates that the state of the PtTerminalConnection object has changed to PtTerminalConnection::TALKING.

    /* PhoneComponent events */
    //!enumcode: PHONE_RINGER_VOLUME_CHANGED - Indicates that the PtPhoneRinger volume has changed.
    //!enumcode: PHONE_RINGER_PATTERN_CHANGED - Indicates that the PtPhoneRinger audio pattern to be played when ringing has changed.
    //!enumcode: PHONE_RINGER_INFO_CHANGED - Indicates that the PtPhoneRinger info string has changed.
    //!enumcode: PHONE_SPEAKER_VOLUME_CHANGED - Indicates that the associated PtPhoneSpeaker volume has changed.
    //!enumcode: PHONE_MICROPHONE_GAIN_CHANGED - Indicates that the associated PtPhoneMicrophone gain has changed.
    //!enumcode: PHONE_LAMP_MODE_CHANGED - Indicates that the associated PtPhoneLamp mode has changed.
    //!enumcode: PHONE_BUTTON_INFO_CHANGED - Indicates that the associated PtPhoneButton info string has changed.
    //!enumcode: PHONE_BUTTON_UP - Indicates that the associated PtPhoneButton has changed to the up (released) position.
    //!enumcode: PHONE_BUTTON_DOWN - Indicates that the associated PtPhoneButton has changed to the down (pressed) position.
    //!enumcode: PHONE_HOOKSWITCH_OFFHOOK - Indicates that the PtPhoneHookswitch has changed to the offhook state.
    //!enumcode: PHONE_HOOKSWITCH_ONHOOK - Indicates that the PtPhoneHookswitch has changed to the onhook state.
    //!enumcode: PHONE_DISPLAY_CHANGED - Indicates that the PtPhoneDisplay has changed.

    enum PtEventCause
    {
                /*
                 * WARNING: Do not change the order of these causes as they must match the JTAPI constants
                 */

                CAUSE_NORMAL                                                    = 100,
                CAUSE_UNKNOWN                                                   = 101,
                CAUSE_CALL_CANCELLED                                    = 102,
                CAUSE_DESTINATION_NOT_OBTAINABLE        = 103,
                CAUSE_INCOMPATIBLE_DESTINATION  = 104,
                CAUSE_LOCKOUT                                                   = 105,
                CAUSE_NEW_CALL                                                  = 106,
                CAUSE_RESOURCES_NOT_AVAILABLE           = 107,
                CAUSE_NETWORK_CONGESTION                        = 108,
                CAUSE_NETWORK_NOT_OBTAINABLE            = 109,
                CAUSE_SNAPSHOT                                                  = 110,

                // Call control
        CAUSE_ALTERNATE                                         = 202,
        CAUSE_BUSY                                                      = 203,
        CAUSE_CALL_BACK                                         = 204,
        CAUSE_CALL_NOT_ANSWERED                 = 205,
        CAUSE_CALL_PICKUP                                       = 206,
        CAUSE_CONFERENCE                                        = 207,
        CAUSE_DO_NOT_DISTURB                            = 208,
        CAUSE_PARK                                                      = 209,
        CAUSE_REDIRECTED                                        = 210,
        CAUSE_REORDER_TONE                                      = 211,
        CAUSE_TRANSFER                                          = 212,
        CAUSE_TRUNKS_BUSY                                       = 213,
        CAUSE_UNHOLD                                                    = 214,


                /*
                 * These are our own constants, make sure the map in JTAPI
                 */
                CAUSE_NOT_ALLOWED                                               = 1000,
                CAUSE_NETWORK_NOT_ALLOWED                       = 1001
    };
    //:Event causes
    // Specific event cause descriptions ...
    //!enumcode: CAUSE_CALL_CANCELLED - Cause code indicating the user has terminated call without going on-hook.
    //!enumcode: CAUSE_DESTINATION_NOT_OBTAINABLE - Cause code indicating the destination is not available.
    //!enumcode: CAUSE_INCOMPATIBLE_DESTINATION - Cause code indicating that a call has encountered an incompatible destination.
    //!enumcode: CAUSE_LOCKOUT - Cause code indicating that a call encountered inter-digit timeout while dialing.
    //!enumcode: CAUSE_NETWORK_CONGESTION - Cause code indicating call encountered network congestion.
    //!enumcode: CAUSE_NETWORK_NOT_OBTAINABLE - Cause code indicating call could not reach a destination network.
    //!enumcode: CAUSE_NEW_CALL - Cause code indicating the event is related to a new call.
    //!enumcode: CAUSE_NORMAL - Cause code indicating normal operation.
    //!enumcode: CAUSE_RESOURCES_NOT_AVAILABLE - Cause code indicating resources were not available.
    //!enumcode: CAUSE_SNAPSHOT - Cause code indicating that the event is part of a snapshot of the current state of the call.
    //!enumcode: CAUSE_UNKNOWN - Cause code indicating the cause was unknown.
    /* call control */
    //!enumcode: CAUSE_ALTERNATE - Cause code indicating that the call was put on hold and another retrieved in an atomic operation, typically on single line telephones.
    //!enumcode: CAUSE_BUSY - Cause code indicating that the call encountered a busy endpoint.
    //!enumcode: CAUSE_CALL_BACK - Cause code indicating the event is related to the callback feature.
    //!enumcode: CAUSE_CALL_NOT_ANSWERED - Cause code indicating that the call was not answered before a timer elapsed.
    //!enumcode: CAUSE_CALL_PICKUP - Cause code indicating that the call was redirected by the call pickup feature.
    //!enumcode: CAUSE_CONFERENCE - Cause code indicating the event is related to the conference feature.
    //!enumcode: CAUSE_DO_NOT_DISTURB - Cause code indicating the event is related to the do-not-disturb feature.
    //!enumcode: CAUSE_PARK - Cause code indicating the event is related to the park feature.
    //!enumcode: CAUSE_REDIRECTED - Cause code indicating the event is related to the redirect feature.
    //!enumcode: CAUSE_REORDER_TONE - Cause code indicating that the call encountered a reorder tone.
    //!enumcode: CAUSE_TRANSFER - Cause code indicating the event is related to the transfer feature.
    //!enumcode: CAUSE_TRUNKS_BUSY - Cause code indicating that the call encountered the "all trunks busy" condition.
    //!enumcode: CAUSE_UNHOLD - Cause code indicating the event is related to the unhold feature.
    //!enumcode: CAUSE_NOT_ALLOWED - PINGTEL SPECIFIC: Cause code indicating the call is unauthorized by endpoint.
    //!enumcode: CAUSE_NETWORK_NOT_ALLOWED - PINGTEL SPECIFIC: Cause code indicating the call is unauthorized by network/servers.

         // Meta events
    enum PtMetaCode
         {
                META_EVENT_NONE                                         = 0x00,
                META_CALL_STARTING                                      = 0x80,
                META_CALL_PROGRESS                                      = 0x81,
                META_CALL_ADDITIONAL_PARTY                      = 0x82,
                META_CALL_REMOVING_PARTY                        = 0x83,
                META_CALL_ENDING                                        = 0x84,
                META_CALL_MERGING                                       = 0x85,
                META_CALL_TRANSFERRING                          = 0x86,
                META_SNAPSHOT                                           = 0x87,
                META_CALL_REPLACING                             = 0x88,
                META_UNKNOWN                                            = 0x89,
        };
        /* meta events */
    //!enumcode: META_CALL_STARTING - Meta code description for the initiation or starting of a call. This implies that the call is a new call and in the active state with at least one Connection added to it.
    //!enumcode: META_CALL_PROGRESS - Meta code description for the progress of a call. This indicates an update in state of certain objects in the call, or the addition of TerminalConnections (but not Connections).
    //!enumcode: META_CALL_ADDITIONAL_PARTY - Meta code description for the addition of a party to call. This includes adding a connection to the call.
    //!enumcode: META_CALL_REMOVING_PARTY - Meta code description for a party leaving the call. This includes exactly one Connection moving to the <CODE>Connection.DISCONNECTED</CODE> state.
    //!enumcode: META_CALL_ENDING - Meta code description for the entire call ending. This includes the call going to <CODE>Call.INVALID</CODE>, all of the Connections moving to the <CODE>Connection.DISCONNECTED</CODE> state.
    //!enumcode: META_CALL_MERGING - Meta code description for an action of merging two calls. This involves the removal of one party from one call and the addition of the same party to another call.
    //!enumcode: META_CALL_TRANSFERRING - Meta code description for an action of transferring one call to another. This involves the removal of parties from one call and the addition to another call, and the common party dropping off completely.
    //!enumcode: META_SNAPSHOT - Meta code description for a snapshot of events.
    //!enumcode: META_UNKNOWN - Meta code is unknown.



/* ============================ CREATORS ================================== */
   PtEvent(int eventId = EVENT_INVALID,
                        int metaCode = 0,
                        int numOldCalls = 0,
                        const char* callId = NULL,
                        TaoClientTask *pClient = NULL,
                        int sipResponseCode = 0,
                        const char* sipResponseText = 0,
                        const char** pMetaEventCallIds = 0,
                        const char* newCallId = NULL,
                        PtEventCause cause = CAUSE_NORMAL,
                        int isLocal = -1);
     //:Default constructor

   PtEvent(const PtEvent& rPtEvent);
     //:Copy constructor

   virtual
   ~PtEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtEvent& operator=(const PtEvent& rhs);
     //:Assignment operator

   virtual void setEventId(PtEventId eventId);
   virtual void setMetaCode(PtMetaCode metaCode);
   virtual void setEventCallId(const char* callId);                     // call id
   virtual void setEventSipResponseCode(int sipResponseCode);           // SIP response code
   virtual void setEventSipResponseText(const char* sipResponseText);           // SIP response text
   virtual void setEventNewCallId(const char* newCallId);               // new call id
   virtual void setEventOldCallIds(int numOldCalls, UtlString* oldCallIds); // old call ids
   virtual void setEventCause(PtEventCause cause);              // cause
   virtual void setEventLocal(int isLocal);             // islocal?


/* ============================ ACCESSORS ================================= */

   PtStatus getId(PtEventId& rId);
     //:Returns the event identifier.
     // The <i>rId</i> argument is set to indicate the specific event that
     // has occurred.  For detailed descriptions of the event ids see the
     // PtEventId enumeration.
     //!param: (out) rId - The reference used to return the id
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   PtStatus getCause(PtEventCause& rCause);
     //:Returns the cause of this event.
     // For detailed descriptions of the event causes see the PtEventCause
     // enumeration.
     //!param: (out) rCause - The reference used to return the cause
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getMetaEvent(PtBoolean& rMetaEventExists, PtMetaEvent*& pEvent) const ;
     //:Returns meta event for this event.
     // Meta events are used to group a set of related events.  Meta events
     // occur in pairs to indicate the start and end of the set of events.
     // The events between, which belong to the logical set of events, will
     // all have a meta event set to the start meta event.
     //!param: (out) rMetaEventExists - TRUE if a meta event is associated with this event.
     //!param: (out) rEvent - The reference used to return the meta event
     // If this event is not associated with a meta event, <i>pEvent</i>
     // has a eventID set to EVENT_INVALID.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

        PtStatus getMetaCode(int& rMetaCode);
                //:Returns the meta code associated with this event. The meta code provides
                // a higher-level description of the event.
                //!param: (out) rMetaCode - The  meta code for this event.
                //!retcode: PT_SUCCESS - Success
                //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

        PtStatus  isNewMetaEvent(PtBoolean& rNewMetaEvent);
                //:Returns true when this event is the start of a meta code group. This
                // method is used to distinguish two contiguous groups of events bearing
                // the same meta code.
                //!param: (out) rNewMetaEvent - True if this event represents a new meta code grouping, false otherwise.
                //!retcode: PT_SUCCESS - Success
                //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   static const char* className();
     //:Returns the name of this class.
     //!returns: Returns the string representation of the name of this class

   PtStatus getSipResponseCode(int& responseCode, UtlString& responseText);
                //:Return the SIP response code associated with this event
                //!param: (out) responseCode - The SIP response code.
                //!param: (out) responseText - The SIP response text.
                //!retcode: PT_SUCCESS - Success

/* ============================ INQUIRY =================================== */
   int isLocal() const ;
                //:Return 0 if connection not local, 1 local.

        static PtBoolean isCallEvent(int eventId);

        static PtBoolean isConnectionEvent(int eventId);

        static PtBoolean isTerminalEvent(int eventId);

        static PtBoolean isTerminalComponentEvent(int eventId);

        static PtBoolean isTerminalConnectionEvent(int eventId);

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

   virtual PtBoolean isSame(const PtEvent& rEvent);
     //:Compares events to determine if they are the same.
     // Compares this event with <i>rEvent</i> to determine if they are the same event.
     // Events are the same if they represent the same logical event occuring
     // at the same instant on the same object.
     //!param: (in) rEvent - Event to compare with this object.
     //!retcode: TRUE - If the events are the same.
     //!retcode: FALSE - If the events differ.

        static PtBoolean isStateTransitionAllowed(int newState, int oldState);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

        int                             mIsLocal;
        PtEventId               mEventId;
        PtEventCause    mEventCause;
        PtMetaCode              mMetaCode;

        int                mNumOldCalls;
   char*       mOldCallIds[MAX_OLD_CALLS];
        // UtlString            mOldCallIds[MAX_OLD_CALLS];

   UtlString            mNewCallId;

        TaoClientTask*  mpClient;
   UtlString            mCallId;

        int                     mSipResponseCode;
        UtlString               mSipResponseText;


//      PtEvent(PtEventId eventId, PtEventCause cause = CAUSE_NORMAL);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtEvent_h_

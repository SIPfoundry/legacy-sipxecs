//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


/**
 * @file sipXtapiEvents.h
 * sipXtapi event declarations
 *
 * The sipXtapiEvents.h header file defines all of the events fired as part of
 * receiving inbound calls and placing outbound calls.  Each event notification
 * is comprised of a major event and a minor event.  The major event identifies
 * a significant state transition (e.g. from connected to disconnected.  The minor
 * event identifies the reason for the change (cause code).
 *
 * Below you will find state diagrams that show the typical event transitions
 * for both outbound and inbound calls.
 *
 * @image html callevents_inbound.gif
 *
 * Figure 1: Event flows for an inbound call (received call)
 *
 * @image html callevents_outbound.gif
 *
 * Figure 2: Event flows for an outbound call (placed call)
 */
#ifndef _SIPXTAPIEVENT_H
#define _SIPXTAPIEVENT_H


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "sipXtapi.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS


/**
 * Enum with all of the possible event types.
 */
typedef enum SIPX_EVENT_CATEGORY
{
    EVENT_CATEGORY_CALLSTATE,       /**< CALLSTATE events signify a change in state of a
                                         call.  States range from the notification of a
                                         new call to ringing to connection established to
                                         changes in audio state (starting sending, stop
                                         sending) to termination of a call. */
    EVENT_CATEGORY_LINESTATE,       /**< LINESTATE events indicate changes in the status
                                         of a line appearance.  Lines identify inbound
                                         and outbound identities and can be either
                                         provisioned (hardcoded) or configured to
                                         automatically register with a registrar.
                                         Lines also encapsulate the authentication
                                         criteria needed for dynamic registrations. */
    EVENT_CATEGORY_INFO_STATUS,     /**< INFO_STATUS events are sent when the application
                                         requests sipXtapi to send an INFO message to
                                         another user agent.  The status event includes
                                         the response for the INFO method.  Application
                                         developers should look at this event to determine
                                         the outcome of the INFO message. */
    EVENT_CATEGORY_INFO,            /**< INFO events are sent to the application whenever
                                         an INFO message is received by the sipXtapi user
                                         agent.  INFO messages are sent to a specific call.
                                         sipXtapi will automatically acknowledges the INFO
                                         message at the protocol layer. */
    EVENT_CATEGORY_SUB_STATUS,      /**< Subscription events: OK or expired, notify, etc.*/
    EVENT_CATEGORY_NOTIFY           /**< Notification of subscribed-to events */
} SIPX_EVENT_CATEGORY;

/**
 * Signature for event callback/observer.  Application developers should
 * not block the calling thread.
 *
 * @param category The category of the event (call, line, subscription, notify, etc.).
 * @param pInfo Pointer to the event info structure.
 * @param pUserData User data provided when listener was added
 */
typedef bool (*SIPX_EVENT_CALLBACK_PROC)(SIPX_EVENT_CATEGORY category,
                                         void* pInfo,
                                         void* pUserData);


/**
 * Major call state events identify significant changes in the state of a
 * call.
 */
typedef enum SIPX_CALLSTATE_EVENT
{
    CALLSTATE_UNKNOWN   = 0,    /**< An UNKNOWN event is generated when the state for a call
                                 is no longer known.  This is generally an error
                                 condition; see the minor event for specific causes. */
    CALLSTATE_NEWCALL         = 1000, /**< The NEWCALL event indicates that a new call has been
                                 created automatically by the sipXtapi.  This event is
                                 most frequently generated in response to an inbound
                                 call request.  */
	CALLSTATE_DIALTONE        = 2000, /**< The DIALTONE event indicates that a new call has been
                                 created for the purpose of placing an outbound call.
                                 The application layer should determine if it needs to
                                 simulate dial tone for the end user. */
	CALLSTATE_REMOTE_OFFERING = 2500, /**< The REMOTE_OFFERING event indicates that a call setup
                                 invitation has been sent to the remote party.  The
                                 invitation may or may not every receive a response.  If
                                 a response is not received in a timely manor, sipXtapi
                                 will move the call into a disconnected state.  If
                                 calling another sipXtapi user agent, the reciprocate
                                 state is OFFER. */
	CALLSTATE_REMOTE_ALERTING = 3000, /**< The REMOTE_ALERTING event indicates that a call setup
                                 invitation has been accepted and the end user is in the
                                 alerting state (ringing).  Depending on the SIP
                                 configuration, end points, and proxy servers involved,
                                 this event should only last for 3 minutes.  Afterwards,
                                 the state will automatically move to DISCONNECTED.  If
                                 calling another sipXtapi user agent, the reciprocate
                                 state is ALERTING. */
	CALLSTATE_CONNECTED       = 4000, /**< The CONNECTED state indicates that call has been setup
                                 between the local and remote party.  Audio should be
                                 flowing provided and the microphone and speakers should
                                 be engaged. */
	CALLSTATE_DISCONNECTED    = 5000, /**< The DISCONNECTED state indicates that a call was
                                 disconnected or failed to connect.  A call may move
                                 into the DISCONNECTED states from almost every other
                                 state.  Please review the DISCONNECTED minor events to
                                 understand the cause. */
	CALLSTATE_OFFERING        = 6000, /**< An OFFERING state indicates that a new call invitation
                                 has been extended this user agent.  Application
                                 developers should invoke sipxCallAccept(),
                                 sipxCallReject() or sipxCallRedirect() in response.
                                 Not responding will result in an implicit call
                                 sipXcallReject(). */
	CALLSTATE_ALERTING        = 7000, /**< An ALERTING state indicates that an inbound call has
                                 been accepted and the application layer should alert
                                 the end user.  The alerting state is limited to 3
                                 minutes in most configurations; afterwards the call
                                 will be canceled.  Applications will generally play
                                 some sort of ringing tone in response to this event. */
	CALLSTATE_DESTROYED       = 8000, /**< The DESTORYED event indicates the underlying resources
                                 have been removed for a call.  This is the last event
                                 that the application will receive for any call.  The
                                 call handle is invalid after this event is received. */
} SIPX_CALLSTATE_EVENT;


/**
 * Callstate cuase events identify the reason for a Callstate event or
 * provide more detail.
 */
typedef enum SIPX_CALLSTATE_CAUSE
{
	CALLSTATE_NEW_CALL_NORMAL		  = CALLSTATE_NEWCALL + 1,	        /**< See NEWCALL callstate event */
	CALLSTATE_NEW_CALL_TRANSFERED	  = CALLSTATE_NEWCALL + 2,	        /**< Call created because a transfer has is
	                                                                         occurring */
	CALLSTATE_DIALTONE_UNKNOWN        = CALLSTATE_DIALTONE + 1,         /**< See DIALTONE callstate event */
    CALLSTATE_DIALTONE_CONFERENCE     = CALLSTATE_DIALTONE,             /**< Call created as part of conference */
	CALLSTATE_REMOTE_OFFERING_NORMAL  = CALLSTATE_REMOTE_OFFERING + 1,  /**< See REMOTE_OFFERING callstate event */
	CALLSTATE_REMOTE_ALERTING_NORMAL  = CALLSTATE_REMOTE_ALERTING + 1,  /**< Remote party is alerting, play ringback
                                                                             locally */
	CALLSTATE_REMOTE_ALERTING_MEDIA,                                    /**< Remote party is alerting and providing
                                                                             ringback audio*/
	CALLSTATE_CONNECTED_ACTIVE		  = CALLSTATE_CONNECTED + 1,        /**< Call is connected and active (playing
                                                                             local media)*/
	CALLSTATE_CONNECTED_ACTIVE_HELD,                                    /**< Call is connected, held (not playing local
                                                                             media), and bridging media for a
                                                                             conference */
    CALLSTATE_CONNECTED_INACTIVE,                                       /**< Call is held (not playing local media) and
                                                                             is not bridging any other calls */
	CALLSTATE_DISCONNECTED_BADADDRESS = CALLSTATE_DISCONNECTED + 1,     /**< Disconnected: Invalid or unreachable
                                                                             address */
	CALLSTATE_DISCONNECTED_BUSY,                              /**< Disconnected: Caller or Callee was busy*/
	CALLSTATE_DISCONNECTED_NORMAL,                            /**< Disconnected: Normal call tear down (either
                                                                   local or remote)*/
    CALLSTATE_DISCONNECTED_RESOURCES,                         /**< Disconnected: Not enough resources
                                                                   available to complete call*/
    CALLSTATE_DISCONNECTED_NETWORK,                           /**< Disconnected: A network error cause call
                                                                   to fail*/
	CALLSTATE_DISCONNECTED_REDIRECTED,                        /**< Disconnected: Call was redirected a
                                                                   different user agent */
	CALLSTATE_DISCONNECTED_NO_RESPONSE,                       /**< Disconnected: No response was received */
    CALLSTATE_DISCONNECTED_AUTH,                              /**< Disconnected: Unable to authenticate */
    CALLSTATE_DISCONNECTED_UNKNOWN,                           /**< Disconnected: Unknown reason */

	CALLSTATE_OFFERING_ACTIVE		  = CALLSTATE_OFFERING + 1,         /**< See OFFERING callstate event */
	CALLSTATE_ALERTING_NORMAL		  = CALLSTATE_ALERTING + 1,         /**< See ALERTING callstate event */
	CALLSTATE_DESTROYED_NORMAL        = CALLSTATE_DESTROYED + 1,        /**< See DESTROYED callstate event */
} SIPX_CALLSTATE_CAUSE ;


/**
 * Enumeration of possible linestate Events.
 */
 typedef enum SIPX_LINESTATE_EVENT
{
    LINESTATE_UNKNOWN  = -1,             /**< This is the initial Line event state. */
    LINESTATE_REGISTERING   = 10000,     /**< The REGISTERING event is fired when sipXtapi
                                              has successfully sent a REGISTER message,
                                              but has not yet received a success response from the
                                              registrar server */
    LINESTATE_REGISTERED  = 11000,      /**< The REGISTERED event is fired after sipXtapi has received
                                             a response from the registrar server, indicating a successful
                                             registration. */
    LINESTATE_UNREGISTERING    = 12000, /**< The UNREGISTERING event is fired when sipXtapi
                                             has successfully sent a REGISTER message with an expires=0 parameter,
                                             but has not yet received a success response from the
                                             registrar server */
    LINESTATE_UNREGISTERED     = 13000, /**< The UNREGISTERED event is fired after sipXtapi has received
                                             a response from the registrar server, indicating a successful
                                             un-registration. */
    LINESTATE_REGISTER_FAILED  = 14000, /**< The REGISTER_FAILED event is fired to indicate a failure of REGISTRATION.
                                             It is fired in the following cases:
                                    The client could not connect to the registrar server.
                                    The registrar server challenged the client for authentication credentials,
                                    and the client failed to supply valid credentials.
                                    The registrar server did not generate a success response (status code == 200)
                                    within a timeout period.  */
    LINESTATE_UNREGISTER_FAILED  = 15000,  /**< The UNREGISTER_FAILED event is fired to indicate a failure of un-REGISTRATION.
                                    It is fired in the following cases:
                                    The client could not connect to the registrar server.
                                    The registrar server challenged the client for authentication credentials,
                                    and the client failed to supply valid credentials.
                                    The registrar server did not generate a success response (status code == 200)
                                    within a timeout period.  */
    LINESTATE_PROVISIONED      = 16000,    /**<  The PROVISIONED event is fired when a sipXtapi Line is added, and Registration is not
                                    requested (i.e. - sipxLineAdd is called with a bRegister parameter of false. */
} SIPX_LINESTATE_EVENT;

enum SIPX_INFOSTATUS_EVENT
{
    INFOSTATUS_UNKNOWN = -1,            /**< This is the initial value for an INFOSTATUS event. */
    INFOSTATUS_RESPONSE = 1000,         /**< This event is fired if a response is received after an
                                             INFO message has been sent */
    INFOSTATUS_NETWORK_ERROR = 2000     /**< This event is fired in case a network error was encountered
                                             while trying to send an INFO event. */
};

/**
 * Enumeration of possible linestate Event causes.
 */
enum SIPX_LINESTATE_CAUSE
{
    LINESTATE_CAUSE_UNKNOWN          = -1,       /**< No cause specified. */
};

/**
 * Callstate event information structure
 */
typedef struct
{
    size_t    nSize;                /**< The size of this structure. */
    SIPX_CALL hCall;                /**< Call handle associated with the callstate event. */
    SIPX_LINE hLine;                /**< Line handle associated with the callstate event. */
    SIPX_CALLSTATE_EVENT event;     /**< Callstate event enum code.
                                         Identifies the callstate event. */
    SIPX_CALLSTATE_CAUSE cause;     /**< Callstate cause enum code.
                                         Identifies the cause of the callstate event. */

} SIPX_CALLSTATE_INFO;

/**
 * Linestate event information structure
 */
typedef struct
{
    size_t    nSize ;               /**< The size of this structure. */
    SIPX_LINE hLine;                /**< Line handle associated with the linestate event. */
    SIPX_LINESTATE_EVENT event ;    /**< Callstate event enum code.
                                         Identifies the linestate event. */
    SIPX_LINESTATE_CAUSE cause ;    /**< Callstate cause enum code.
                                         Identifies the cause of the linestate event. */
} SIPX_LINESTATE_INFO ;


/**
 *  Major classifications of response statuses for a SIP message.
 */
typedef enum
{
    SIPX_MESSAGE_OK,                  /**< The message was successfully processed (200) */
    SIPX_MESSAGE_FAILURE,             /**< The server received the message, but could or would
                                           not process it. */
    SIPX_MESSAGE_SERVER_FAILURE,      /**< The server encountered an error while trying to process
                                           the message. */
    SIPX_MESSAGE_GLOBAL_FAILURE,      /**< Fatal error encountered. */
} SIPX_MESSAGE_STATUS ;

/**
 * An INFOSTATUS event informs that application layer of the status
 * of an outbound INFO requests;
 */
typedef struct
{
    size_t              nSize ;             /**< the size of this structure in bytes */
    SIPX_INFO           hInfo ;             /**< the handle used to make the outbound info request. */
    SIPX_MESSAGE_STATUS status ;            /**< Emumerated status for this
                                                 request acknowledgement. */
    int                 responseCode ;      /**< Numerical status code for this
                                                 request acknowledgement. */
    const char*         szResponseText ;    /**< The text of the request acknowledgement. */
    SIPX_INFOSTATUS_EVENT event;            /**< Event code for this INFO STATUS message */
} SIPX_INFOSTATUS_INFO ;


/**
 * An INFO event signals the application layer that an INFO message
 * was sent to this user agent.  If the INFO message was sent to a
 * call context (session) hCall will desiginate the call session.
 */
typedef struct
{
    size_t      nSize ;             /**< Size of structure */
    SIPX_CALL   hCall ;             /**< Call handle if available */
    SIPX_LINE   hLine ;             /**< Line handle if available */
    const char* szFromURL ;         /**< the URL of the host that originated
                                         the INFO message */
    const char* szUserAgent;        /**< the User Agent string of the source agent */
    const char* szContentType ;     /**< string indicating the info content type */
    const char* pContent ;          /**< pointer to the INFO message content */
    size_t      nContentLength ;    /**< length of the INFO message content */

} SIPX_INFO_INFO ;

/**
 * Enumeration of the possible subscription states visible to the client.
 */
typedef enum
{
    SIPX_SUBSCRIPTION_ACTIVE ,      /**< The subscription is currently active. */
    SIPX_SUSBSCRITION_FAILED ,      /**< The subscription is not active due to a failure.*/
    SIPX_SUBSCRIPTION_EXPIRED ,     /**< The subscription's lifetime has expired. */
    // TBD
} SIPX_SUBSCRIPTION_STATE;


/**
 * An SUBSTATUS event informs that application layer of the status
 * of an outbound SUBSCRIPTION requests;
 */
typedef struct
{
    size_t                  nSize ;     /**< the size of this structure in bytes */
    SIPX_SUBSCRIPTION_STATE state ;     /**< Enum state value indicating the current
                                             state of the subscription. */
    const char* szSubServerUserAgent;   /**< the User Agent header field value from
                                             the SIP SUBSCRIBE response (may be NULL) */

} SIPX_SUBSTATUS_INFO ;



/**
 * A NOTIFY_INFO event signifies that a NOTIFY message was received for
 * an active subscription.
 */
typedef struct
{
    size_t      nSize ;             /**< the size of this structure in bytes */
    SIPX_SUB    hSub ;              /**< a handle to handle to the subscrption which
                                         caused this NOTIFY event to be received. */
    const char* szNotiferUserAgent; /**< the User-Agent header field value from
                                         the SIP NOTIFY response (may be NULL) */
    const char* szContentType ;     /**< string indicating the info content type */
    const void* pContent ;          /**< pointer to the NOTIFY message content */
    size_t      nContentLength ;    /**< length of the NOTIFY message content */
} SIPX_NOTIFY_INFO ;



/* ============================ FUNCTIONS ================================= */

/**
 * Add a callback/observer for the purpose of receiving sipXtapi events
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param pCallbackProc Function to receive sipx events
 * @param pUserData user data passed along with event data
 */
SIPXTAPI_API SIPX_RESULT sipxEventListenerAdd(const SIPX_INST hInst,
                                             SIPX_EVENT_CALLBACK_PROC pCallbackProc,
                                             void *pUserData);

/**
 * Remove a sipXtapi event callback/observer.  Supply the same
 * pCallbackProc and pUserData values as sipxEventListenerAdd.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param pCallbackProc Function used to receive sipx events
 * @param pUserData user data specified as part of sipxListenerAdd
 */
SIPXTAPI_API SIPX_RESULT sipxEventListenerRemove(const SIPX_INST hInst,
                                            SIPX_EVENT_CALLBACK_PROC pCallbackProc,
                                            void* pUserData) ;



/* ============================ DEPRECATED DEFININTIONS ======================== */


/**
 * Major call state events identify significant changes in the state of a
 * call.
 * For backward compatibility.
 * @deprecated Use the new callback/event mechanism instead
 */
typedef enum SIPX_CALLSTATE_MAJOR
{
    UNKNOWN         = 0,    /**< An UNKNOWN event is generated when the state for a call
                                 is no longer known.  This is generally an error
                                 condition; see the minor event for specific causes. */
    NEWCALL                 = 1000, /**< The NEWCALL event indicates that a new call has been
                                 created automatically by the sipXtapi.  This event is
                                 most frequently generated in response to an inbound
                                 call request.  */
        DIALTONE        = 2000, /**< The DIALTONE event indicates that a new call has been
                                 created for the purpose of placing an outbound call.
                                 The application layer should determine if it needs to
                                 simulate dial tone for the end user. */
        REMOTE_OFFERING = 2500, /**< The REMOTE_OFFERING event indicates that a call setup
                                 invitation has been sent to the remote party.  The
                                 invitation may or may not every receive a response.  If
                                 a response is not received in a timely manor, sipXtapi
                                 will move the call into a disconnected state.  If
                                 calling another sipXtapi user agent, the reciprocate
                                 state is OFFER. */
        REMOTE_ALERTING = 3000, /**< The REMOTE_ALERTING event indicates that a call setup
                                 invitation has been accepted and the end user is in the
                                 alerting state (ringing).  Depending on the SIP
                                 configuration, end points, and proxy servers involved,
                                 this event should only last for 3 minutes.  Afterwards,
                                 the state will automatically move to DISCONNECTED.  If
                                 calling another sipXtapi user agent, the reciprocate
                                 state is ALERTING. */
        CONNECTED       = 4000, /**< The CONNECTED state indicates that call has been setup
                                 between the local and remote party.  Audio should be
                                 flowing provided and the microphone and speakers should
                                 be engaged. */
        DISCONNECTED    = 5000, /**< The DISCONNECTED state indicates that a call was
                                 disconnected or failed to connect.  A call may move
                                 into the DISCONNECTED states from almost every other
                                 state.  Please review the DISCONNECTED minor events to
                                 understand the cause. */
        OFFERING        = 6000, /**< An OFFERING state indicates that a new call invitation
                                 has been extended this user agent.  Application
                                 developers should invoke sipxCallAccept(),
                                 sipxCallReject() or sipxCallRedirect() in response.
                                 Not responding will result in an implicit call
                                 sipXcallReject(). */
        ALERTING        = 7000, /**< An ALERTING state indicates that an inbound call has
                                 been accepted and the application layer should alert
                                 the end user.  The alerting state is limited to 3
                                 minutes in most configurations; afterwards the call
                                 will be canceled.  Applications will generally play
                                 some sort of ringing tone in response to this event. */
        DESTROYED       = 8000, /**< The DESTORYED event indicates the underlying resources
                                 have been removed for a call.  This is the last event
                                 that the application will receive for any call.  The
                                 call handle is invalid after this event is received. */


} SIPX_CALLSTATE_MAJOR ;



/**
 * Minor call events identify the reason for a SIPX_CALLSTATE_MAJOR event or
 * provide more detail.
 * For backward compatibility.
 * @deprecated Use the new callback/event mechanism instead
 */
typedef enum SIPX_CALLSTATE_MINOR
{
    NEW_CALL_NORMAL            = NEWCALL + 1,          /**< See NEWCALL major event */
    NEW_CALL_TRANSFERED		   = NEWCALL + 2,	       /**< Call created because a transfer has is
	                                                        occurring */
    DIALTONE_UNKNOWN           = DIALTONE + 1,         /**< See DIALTONE major event */
    DIALTONE_CONFERENCE        = DIALTONE,             /**< Call created as part of conference */
    REMOTE_OFFERING_NORMAL     = REMOTE_OFFERING + 1,  /**< See REMOTE_OFFERING major event */
    REMOTE_ALERTING_NORMAL     = REMOTE_ALERTING + 1,  /**< Remote party is alerting, play ringback
                                                            locally */
    REMOTE_ALERTING_MEDIA,                             /**< Remote party is alerting and providing
                                                            ringback audio*/
    CONNECTED_ACTIVE           = CONNECTED + 1,        /**< Call is connected and active (playing
                                                            local media)*/
    CONNECTED_ACTIVE_HELD,                             /**< Call is connected, held (not playing local
                                                            media), and bridging media for a
                                                            conference */
    CONNECTED_INACTIVE,                                /**< Call is held (not playing local media) and
                                                            is not bridging any other calls */
    DISCONNECTED_BADADDRESS    = DISCONNECTED + 1,     /**< Disconnected: Invalid or unreachable
                                                            address */
    DISCONNECTED_BUSY,                                 /**< Disconnected: Caller or Callee was busy*/
    DISCONNECTED_NORMAL,                               /**< Disconnected: Normal call tear down (either
                                                            local or remote)*/
    DISCONNECTED_RESOURCES,                            /**< Disconnected: Not enough resources
                                                            available to complete call*/
    DISCONNECTED_NETWORK,                              /**< Disconnected: A network error cause call
                                                            to fail*/
    DISCONNECTED_REDIRECTED,                           /**< Disconnected: Call was redirected a
                                                            different user agent */
    DISCONNECTED_NO_RESPONSE,                          /**< Disconnected: No response was received */
    DISCONNECTED_AUTH,                                 /**< Disconnected: Unable to authenticate */
    DISCONNECTED_UNKNOWN,                              /**< Disconnected: Unknown reason */

    OFFERING_ACTIVE            = OFFERING + 1,         /**< See OFFERING major event */
    ALERTING_NORMAL            = ALERTING + 1,         /**< See ALERTING major event */
    DESTROYED_NORMAL           = DESTROYED + 1,        /**< See DESTROYED major event */

} SIPX_CALLSTATE_MINOR ;


/**
 * Enumeration of possible Line Events.
 */
typedef enum SIPX_LINE_EVENT_TYPE_MAJOR
{
   SIPX_LINE_EVENT_UNKNOWN          = -1,       /**< This is the initial Line event state. */
   SIPX_LINE_EVENT_REGISTERING      = 10000,    /**< The REGISTERING event is fired when sipXtapi
                                                     has successfully sent a REGISTER message,
                                                     but has not yet received a success response from the
                                                     registrar server */
   SIPX_LINE_EVENT_REGISTERED       = 11000,    /**< The REGISTERED event is fired after sipXtapi has received
                                                     a response from the registrar server, indicating a successful
                                                     registration. */
   SIPX_LINE_EVENT_UNREGISTERING    = 12000,    /**< The UNREGISTERING event is fired when sipXtapi
                                                     has successfully sent a REGISTER message with an expires=0 parameter,
                                                     but has not yet received a success response from the
                                                     registrar server */
   SIPX_LINE_EVENT_UNREGISTERED     = 13000,    /**< The UNREGISTERED event is fired after sipXtapi has received
                                                     a response from the registrar server, indicating a successful
                                                     un-registration. */
   SIPX_LINE_EVENT_REGISTER_FAILED  = 14000,    /**< The REGISTER_FAILED event is fired to indicate a failure of REGISTRATION.
                                                     It is fired in the following cases:
                                                     The client could not connect to the registrar server.
                                                     The registrar server challenged the client for authentication credentials,
                                                     and the client failed to supply valid credentials.
                                                     The registrar server did not generate a success response (status code == 200)
                                                     within a timeout period.  */
   SIPX_LINE_EVENT_UNREGISTER_FAILED  = 15000, /**< The UNREGISTER_FAILED event is fired to indicate a failure of un-REGISTRATION.
                                                     It is fired in the following cases:
                                                     The client could not connect to the registrar server.
                                                     The registrar server challenged the client for authentication credentials,
                                                     and the client failed to supply valid credentials.
                                                     The registrar server did not generate a success response (status code == 200)
                                                     within a timeout period.  */
   SIPX_LINE_EVENT_PROVISIONED      = 16000    /**<  The PROVISIONED event is fired when a sipXtapi Line is added, and Registration is not
                                                     requested (i.e. - sipxLineAdd is called with a bRegister parameter of false. */
} SIPX_LINE_EVENT_TYPE_MAJOR;


/**
 * Signature for event callback/observer.  Application developers should
 * not block the calling thread.
 *
 * @deprecated Use SIPX_EVENT_CALLBACK_PROC instead
 * @param hCall Source of event
 * @param hLine Line call was made on
 * @param eMajor Major event ID
 * @param eMinor Minor event ID
 * @param pUserData User data provided when listener was added
 */
typedef void (*CALLBACKPROC)( SIPX_CALL hCall,
                                                      SIPX_LINE hLine,
                                                      SIPX_CALLSTATE_MAJOR eMajor,
                                                      SIPX_CALLSTATE_MINOR eMinor,
                                                      void* pUserData) ;
/**
 * Signature for line event callback/observer.  Application developers should
 * not block the calling thread.
 *
 * @deprecated Use the new callback/event mechanism instead
 * @param hLine Line call was made on
 * @param eMajor Major event ID
 * @param pUserData User data provided when listener was added
 */
typedef void (*LINECALLBACKPROC)(SIPX_LINE hLine,
						      SIPX_LINE_EVENT_TYPE_MAJOR eMajor,
						      void* pUserData);


/* ============================ FUNCTIONS ================================= */

/**
 * Add a callback/observer for the purpose of receiving call events
 *
 * @deprecated Use sipxEventListenerAdd instead
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param pCallbackProc Function to receive sipx events
 * @param pUserData user data passed along with event data
 */
SIPXTAPI_API SIPX_RESULT sipxListenerAdd(const SIPX_INST hInst,
                                         CALLBACKPROC pCallbackProc,
                                         void* pUserData) ;

/**
 * Remove a call event callback/observer.  Supply the same
 * pCallbackProc and pUserData values as sipxListenerAdd.
 *
 * @deprecated Use sipxEventListenerRemove instead
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param pCallbackProc Function used to receive sipx events
 * @param pUserData user data specified as part of sipxListenerAdd
 */
SIPXTAPI_API SIPX_RESULT sipxListenerRemove(const SIPX_INST hInst,
                                            CALLBACKPROC pCallbackProc,
                                            void* pUserData) ;


/**
 * Create a printable string version of the designated call state event ids.
 * This is generally used for debugging.
 *
 * @param eMajor sipxtapi major event code
 * @param eMinor sipxtapi minor event code
 * @param szBuffer buffer to store event string
 * @param nBuffer length of string buffer szBuffer
 */
SIPXTAPI_API char* sipxEventToString(SIPX_CALLSTATE_MAJOR eMajor,
                                     SIPX_CALLSTATE_MINOR eMinor,
                                     char*  szBuffer,
                                     size_t nBuffer) ;

/**
 * Add a callback/observer for the purpose of receiving Line events
 *
 *
 * @deprecated Use sipxEventListenerAdd instead
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param pCallbackProc Function to receive sipx events
 * @param pUserData user data passed along with event data
 */
SIPXTAPI_API SIPX_RESULT sipxLineListenerAdd(const SIPX_INST hInst,
                                         LINECALLBACKPROC pCallbackProc,
                                         void* pUserData);

/**
 * Remove a Line event callback/observer.  Supply the same
 * pCallbackProc and pUserData values as sipxLineListenerAdd.
 *
 * @deprecated Use sipxEventListenerRemove instead
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param pCallbackProc Function used to receive sipx Line events
 * @param pUserData user data specified as part of sipxListenerAdd
 */
SIPXTAPI_API SIPX_RESULT sipxLineListenerRemove(const SIPX_INST hInst,
                                                LINECALLBACKPROC pCallbackProc,
                                                void* pUserData);
/**
 * Create a printable string version of the designated line event ids.
 * This is generally used for debugging.
 *
 * @deprecated Use the new callback/event mechanism instead
 * @param lineTypeMajor major event type id
 * @param szBuffer buffer to store event string
 * @param nBuffer length of string buffer szBuffer
 */
SIPXTAPI_API char* sipxLineEventToString(SIPX_LINE_EVENT_TYPE_MAJOR lineTypeMajor,
                                         char*  szBuffer,
                                         size_t nBuffer);




#endif /* ifndef _sipXtapiEvents_h_ */

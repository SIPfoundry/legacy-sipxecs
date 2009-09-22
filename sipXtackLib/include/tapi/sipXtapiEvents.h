//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
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
    EVENT_CATEGORY_NOTIFY,          /**< Notification of subscribed-to events */
    EVENT_CATEGORY_CONFIG           /**< CONFIG events signify changes in configuration.
                                         For example, when requesting STUN support, a
                                         notification is sent with the STUN outcome (either
                                         SUCCESS or FAILURE) */
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
    CALLSTATE_UNKNOWN         = 0,/**< An UNKNOWN event is generated when the state for a call
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

/** VIDEO: The CALLSTATE_INFO should contain supported remote audio and video codecs offered */


    CALLSTATE_ALERTING        = 7000, /**< An ALERTING state indicates that an inbound call has
                                 been accepted and the application layer should alert
                                 the end user.  The alerting state is limited to 3
                                 minutes in most configurations; afterwards the call
                                 will be canceled.  Applications will generally play
                                 some sort of ringing tone in response to this event. */

/** VIDEO: The CALLSTATE_INFO should contain supported remote audio and video codecs offered */


    CALLSTATE_DESTROYED       = 8000, /**< The DESTORYED event indicates the underlying resources
                                 have been removed for a call.  This is the last event
                                 that the application will receive for any call.  The
                                 call handle is invalid after this event is received. */
	CALLSTATE_AUDIO_EVENT      = 9000, /**< The  AUDIO_EVENT event indicates the RTP session has
                                 either started or stopped. */
    CALLSTATE_TRANSFER         = 10000, /**< The transfer state indicates a state change in a
                                 transfer attempt.  Please see the CALLSTATE_TRANSFER cause
                                 codes for details on each state transition */
    CALLSTATE_SECURITY_EVENT   = 11000 /** The SECURITY_EVENT is sent to the application
                                           when S/MIME or SRTP events occur which the application
                                           should know about. */
} SIPX_CALLSTATE_EVENT;


/**
 * Callstate cuase events identify the reason for a Callstate event or
 * provide more detail.
 */
typedef enum SIPX_CALLSTATE_CAUSE
{
	CALLSTATE_NEW_CALL_NORMAL		  = CALLSTATE_NEWCALL + 1,	        /**< See NEWCALL callstate event */
	CALLSTATE_NEW_CALL_TRANSFERRED	  = CALLSTATE_NEWCALL + 2,	        /**< Call created because a transfer has is
	                                                                         occurring */
	CALLSTATE_NEW_CALL_TRANSFER	      = CALLSTATE_NEWCALL + 3,	        /**< Call created because a transfer has
                                                                             been initiated locally. */
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
    CALLSTATE_CONNECTED_REQUEST_NOT_ACCEPTED,                           /**< The callee rejected a hold request, or some other
                                                                             request for a change in the session, and the connection
                                                                             is still active. */
	CALLSTATE_DISCONNECTED_BADADDRESS = CALLSTATE_DISCONNECTED + 1,     /**< Disconnected: Invalid or unreachable
                                                                             address */
	CALLSTATE_DISCONNECTED_BUSY,                                        /**< Disconnected: Caller or Callee was busy*/
	CALLSTATE_DISCONNECTED_NORMAL,                                      /**< Disconnected: Normal call tear down (either
                                                                             local or remote)*/
    CALLSTATE_DISCONNECTED_RESOURCES,                                   /**< Disconnected: Not enough resources
                                                                             available to complete call*/
    CALLSTATE_DISCONNECTED_NETWORK,                                     /**< Disconnected: A network error cause call
                                                                             to fail*/
	CALLSTATE_DISCONNECTED_REDIRECTED,                                  /**< Disconnected: Call was redirected a
                                                                             different user agent */
	CALLSTATE_DISCONNECTED_NO_RESPONSE,                                 /**< Disconnected: No response was received */
    CALLSTATE_DISCONNECTED_AUTH,                                        /**< Disconnected: Unable to authenticate */
    CALLSTATE_DISCONNECTED_UNKNOWN,                                     /**< Disconnected: Unknown reason */
	CALLSTATE_DISCONNECTED_GHOST,                                       /**< Disconnected: Ghost call tear down */

	CALLSTATE_OFFERING_ACTIVE		  = CALLSTATE_OFFERING + 1,         /**< See OFFERING callstate event */
	CALLSTATE_ALERTING_NORMAL		  = CALLSTATE_ALERTING + 1,         /**< See ALERTING callstate event */
	CALLSTATE_DESTROYED_NORMAL        = CALLSTATE_DESTROYED + 1,        /**< See DESTROYED callstate event */

    CALLSTATE_AUDIO_START             = CALLSTATE_AUDIO_EVENT + 1,      /**< RTP session started */
    CALLSTATE_AUDIO_STOP              = CALLSTATE_AUDIO_START + 1,      /**< RTP session stopped */

    CALLSTATE_TRANSFER_INITIATED      = CALLSTATE_TRANSFER + 1,         /**< A transfer attempt has been initiated.  This event
                                                                             is sent when a user agent attempts either a blind
                                                                             or consultative transfer. */
    CALLSTATE_TRANSFER_ACCEPTED,                                        /**< A transfer attempt has been accepted by the remote
                                                                             transferee.  This event indicates that the
                                                                             transferee supports transfers (REFER method).  The
                                                                             event is fired upon a 2xx class response to the SIP
                                                                             REFER request. */
    CALLSTATE_TRANSFER_TRYING,                                          /**< The transfer target is attempting the transfer.
                                                                             This event is sent when transfer target (or proxy /
                                                                             B2BUA) receives the call invitation, but before the
                                                                             the tranfer target accepts is. */
    CALLSTATE_TRANSFER_RINGING,                                         /**< The transfer target is ringing.  This event is
                                                                             generally only sent during blind transfer.
                                                                             Consultative transfer should proceed directly to
                                                                             TRANSFER_SUCCESS or TRANSFER_FAILURE. */
    CALLSTATE_TRANSFER_SUCCESS,                                         /**< The transfer was completed successfully.  The
                                                                             original call to transfer target will
                                                                             automatically disconnect.*/
    CALLSTATE_TRANSFER_FAILURE,                                         /**< The transfer failed.  After a transfer fails,
                                                                             the application layer is responsible for
                                                                             recovering original call to the transferee.
                                                                             That call is left on hold. */
    CALLSTATE_SECURITY_SELF_SIGNED_CERT = CALLSTATE_SECURITY_EVENT + 1, /**< A self-signed certificate is being used for S/MIME. */
    CALLSTATE_SECURITY_SESSION_NOT_SECURED,                             /**< Fired if a secure session could not be made. */
    CALLSTATE_SECURITY_REMOTE_SMIME_UNSUPPORTED,                        /**< Fired if the remote party's user-agent does not
                                                                             support S/MIME. */
} SIPX_CALLSTATE_CAUSE ;

/**
 * Enumeration of possible linestate Events.
 */
 typedef enum SIPX_LINESTATE_EVENT
{
    LINESTATE_UNKNOWN  = -1,            /**< This is the initial Line event state. */
    LINESTATE_REGISTERING   = 20000,    /**< The REGISTERING event is fired when sipXtapi
                                             has successfully sent a REGISTER message,
                                             but has not yet received a success response from the
                                             registrar server */
    LINESTATE_REGISTERED  = 21000,      /**< The REGISTERED event is fired after sipXtapi has received
                                             a response from the registrar server, indicating a successful
                                             registration. */
    LINESTATE_UNREGISTERING    = 22000, /**< The UNREGISTERING event is fired when sipXtapi
                                             has successfully sent a REGISTER message with an expires=0 parameter,
                                             but has not yet received a success response from the
                                             registrar server */
    LINESTATE_UNREGISTERED     = 23000, /**< The UNREGISTERED event is fired after sipXtapi has received
                                             a response from the registrar server, indicating a successful
                                             un-registration. */
    LINESTATE_REGISTER_FAILED  = 24000, /**< The REGISTER_FAILED event is fired to indicate a failure of REGISTRATION.
                                             It is fired in the following cases:
                                             The client could not connect to the registrar server.
                                             The registrar server challenged the client for authentication credentials,
                                             and the client failed to supply valid credentials.
                                             The registrar server did not generate a success response (status code == 200)
                                             within a timeout period.  */
    LINESTATE_UNREGISTER_FAILED  = 25000,/**< The UNREGISTER_FAILED event is fired to indicate a failure of un-REGISTRATION.
                                             It is fired in the following cases:
                                             The client could not connect to the registrar server.
                                             The registrar server challenged the client for authentication credentials,
                                             and the client failed to supply valid credentials.
                                             The registrar server did not generate a success response (status code == 200)
                                             within a timeout period.  */
    LINESTATE_PROVISIONED      = 26000, /**< The PROVISIONED event is fired when a sipXtapi Line is added, and Registration is not
                                             requested (i.e. - sipxLineAdd is called with a bRegister parameter of false. */
} SIPX_LINESTATE_EVENT;


/**
 * Enumeration of possible INFO status events
 */
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
typedef enum SIPX_LINESTATE_CAUSE
{
    LINESTATE_CAUSE_UNKNOWN                           = -1,                                /**< No cause specified. */
    LINESTATE_REGISTERING_NORMAL                      = LINESTATE_REGISTERING + 1,         /**< See LINESTATE_REGISTERING
                                                                                                event. */
    LINESTATE_REGISTERED_NORMAL                       = LINESTATE_REGISTERED + 1,          /**< See LINESTATE_REGISTERED
                                                                                                event. */
    LINESTATE_UNREGISTERING_NORMAL                    = LINESTATE_UNREGISTERING + 1,       /**< See LINESTATE_UNREGISTERING
                                                                                                event. */
    LINESTATE_UNREGISTERED_NORMAL                     = LINESTATE_UNREGISTERED + 1,        /**< See LINESTATE_UNREGISTERED
                                                                                                event. */
    LINESTATE_REGISTER_FAILED_COULD_NOT_CONNECT       = LINESTATE_REGISTER_FAILED + 1,     /**< Failed to register because
                                                                                                of a connectivity problem. */
    LINESTATE_REGISTER_FAILED_NOT_AUTHORIZED          = LINESTATE_REGISTER_FAILED + 2,     /**< Failed to register because
                                                                                                of an authorization /
                                                                                                authentication failure. */
    LINESTATE_REGISTER_FAILED_TIMEOUT                 = LINESTATE_REGISTER_FAILED + 3,     /**< Failed to register because of
                                                                                                a timeout. */
    LINESTATE_UNREGISTER_FAILED_COULD_NOT_CONNECT     = LINESTATE_UNREGISTER_FAILED + 1,   /**< Failed to unregister because of
                                                                                                a connectivity problem. */
    LINESTATE_UNREGISTER_FAILED_NOT_AUTHORIZED        = LINESTATE_UNREGISTER_FAILED + 2,   /**< Failed to unregister because of
                                                                                                of an authorization /
                                                                                                authentication failure. */
    LINESTATE_UNREGISTER_FAILED_TIMEOUT               = LINESTATE_UNREGISTER_FAILED + 3,   /**< Failed to register because of
                                                                                                a timeout. */
    LINESTATE_PROVISIONED_NORMAL                      = LINESTATE_PROVISIONED + 1          /**< See LINESTATE_PROVISIONED
                                                                                                event. */
} SIPX_LINESTATE_CAUSE;


/**
 * Enumeration of possible configuration events
 */
enum SIPX_CONFIG_EVENT
{
    CONFIG_UNKNOWN = -1,    /**< Unknown configuration event */
    CONFIG_STUN_SUCCESS,    /**< A STUN binding has been obtained for signaling purposes.
                                 For a SIPX_CONFIG_EVENT type of CONFIG_STUN_SUCCESS,
                                 the pData pointer of the info structure will point to a
                                 SIPX_CONTACT_ADDRESS structure. */
    CONFIG_STUN_FAILURE,    /**< Unable to obtain a STUN binding for signaling purposes. */
} ;


/**
 * Callstate event information structure
 */
typedef struct
{
    // TODO: Add a bitmask that identified which structure items are valid.  For
    //       example, codec and hAssociatedCall are only valid for certain event
    //       sequences.

    size_t    nSize;                /**< The size of this structure. */
    SIPX_CALL hCall;                /**< Call handle associated with the callstate event. */
    SIPX_LINE hLine;                /**< Line handle associated with the callstate event. */
    SIPX_CALLSTATE_EVENT event;     /**< Callstate event enum code.
                                         Identifies the callstate event. */
    SIPX_CALLSTATE_CAUSE cause;     /**< Callstate cause enum code.
                                         Identifies the cause of the callstate event. */
    SIPX_CODEC_INFO codecs;         /**< Structure containing audio and video codec information */
    SIPX_CALL hAssociatedCall ;     /**< Call associated with this event.  For example, when
                                         a new call is created as part of a consultative
                                         transfer, this handle contains the handle of the
                                         original call. */
    const char* remoteAddress;    /**< Remote address of the call. This information is
					 useful when call transfer happens. */
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
    SIPX_SUBSCRIPTION_PENDING,      /**< THe subscription is being set up, but not yet active. */
    SIPX_SUBSCRIPTION_ACTIVE ,      /**< The subscription is currently active. */
    SIPX_SUBSCRIPTION_FAILED ,      /**< The subscription is not active due to a failure.*/
    SIPX_SUBSCRIPTION_EXPIRED ,     /**< The subscription's lifetime has expired. */
    // TBD
} SIPX_SUBSCRIPTION_STATE;

/**
 * Enumeration of cause codes for state subscription state changes.
 */
typedef enum
{
    SUBSCRIPTION_CAUSE_UNKNOWN = -1, /**< No cause specified. */
    SUBSCRIPTION_CAUSE_NORMAL     /**< Normal cause for state change. */
} SIPX_SUBSCRIPTION_CAUSE;

/**
 * An SUBSTATUS event informs that application layer of the status
 * of an outbound SUBSCRIPTION requests;
 */
typedef struct
{
    size_t                  nSize ;     /**< the size of this structure in bytes */
    SIPX_SUB    hSub ;              /**< a handle to the subscription to which
                                         this state change occurred. */
    SIPX_SUBSCRIPTION_STATE state ;     /**< Enum state value indicating the current
                                             state of the subscription. */
    SIPX_SUBSCRIPTION_CAUSE cause;      /**< Enum cause for the state change in this
                                             event. */
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
    SIPX_SUB    hSub ;              /**< a handle to the subscrption which
                                         caused this NOTIFY event to be received. */
    const char* szNotiferUserAgent; /**< the User-Agent header field value from
                                         the SIP NOTIFY response (may be NULL) */
    const char* szContentType ;     /**< string indicating the info content type */
    const void* pContent ;          /**< pointer to the NOTIFY message content */
    size_t      nContentLength ;    /**< length of the NOTIFY message content */
} SIPX_NOTIFY_INFO ;


/**
 * SIPX_CONFIG_INFO events signifies that a change in configuration was
 * observed.
 *
 * NOTE: This structure is subject to change.
 */
typedef struct
{
    size_t            nSize ;   /**< the size of this structure in bytes */
    SIPX_CONFIG_EVENT event ;   /**< event code -- see SIPX_CONFIG_EVENT for
                                     details. */
    void*             pData;    /**< pointer to event data -- SEE SIPX_CONFIG_EVENT
                                     for details. */
} SIPX_CONFIG_INFO ;



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
    NEWCALL         = 1000, /**< The NEWCALL event indicates that a new call has been
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
                                 has been extended to this user agent.  Application
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
    AUDIO_EVENT     = 9000, /**< The  AUDIO_EVENT event indicates the RTP session has
                                 either started or stopped. */
    TRANSFER        = 10000 /**< The transfer state indicates a state change in a
                                 transfer attempt.  Please see the TRANSFER cause
                                 codes for details on each state transition */


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
    NEW_CALL_TRANSFERRED       = NEWCALL + 2,	       /**< Call created because a transfer has is
	                                                        occurring */
    NEW_CALL_TRANSFER          = NEWCALL + 3,          /**< Call created because a transfer has
                                                            been initiated locally. */
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

    AUDIO_START                = AUDIO_EVENT + 1,      /**< RTP session started */
    AUDIO_STOP                 = AUDIO_START + 1,      /**< RTP session stopped */

    TRANSFER_INITIATED         = TRANSFER + 1,         /**< A transfer attempt has been initiated.  This event
                                                            is sent when a user agent attempts either a blind
                                                            or consultative transfer. */
    TRANSFER_ACCEPTED,                                 /**< A transfer attempt has been accepted by the remote
                                                            transferee.  This event indicates that the
                                                            transferee supports transfers (REFER method).  The
                                                            event is fired upon a 2xx class response to the SIP
                                                            REFER request. */
    TRANSFER_TRYING,                                   /**< The transfer target is attempting the transfer.
                                                            This event is sent when transfer target (or proxy /
                                                            B2BUA) receives the call invitation, but before the
                                                            the tranfer target accepts is. */
    TRANSFER_RINGING,                                  /**< The transfer target is ringing.  This event is
                                                            generally only sent during blind transfer.
                                                            Consultative transfer should proceed directly to
                                                            TRANSFER_SUCCESS or TRANSFER_FAILURE. */
    TRANSFER_SUCCESS,                                  /**< The transfer was completed successfully.  The
                                                            original call to transfer target will
                                                            automatically disconnect.*/
    TRANSFER_FAILURE,                                  /**< The transfer failed.  After a transfer fails,
                                                            the application layer is responsible for
                                                            recovering original call to the transferee.
                                                            That call is left on hold. */
} SIPX_CALLSTATE_MINOR ;


/**
 * Enumeration of possible Line Events.
 */
typedef enum SIPX_LINE_EVENT_TYPE_MAJOR
{
   SIPX_LINE_EVENT_UNKNOWN          = -1,       /**< This is the initial Line event state. */
   SIPX_LINE_EVENT_REGISTERING      = 20000,    /**< The REGISTERING event is fired when sipXtapi
                                                     has successfully sent a REGISTER message,
                                                     but has not yet received a success response from the
                                                     registrar server */
   SIPX_LINE_EVENT_REGISTERED       = 21000,    /**< The REGISTERED event is fired after sipXtapi has received
                                                     a response from the registrar server, indicating a successful
                                                     registration. */
   SIPX_LINE_EVENT_UNREGISTERING    = 22000,    /**< The UNREGISTERING event is fired when sipXtapi
                                                     has successfully sent a REGISTER message with an expires=0 parameter,
                                                     but has not yet received a success response from the
                                                     registrar server */
   SIPX_LINE_EVENT_UNREGISTERED     = 23000,    /**< The UNREGISTERED event is fired after sipXtapi has received
                                                     a response from the registrar server, indicating a successful
                                                     un-registration. */
   SIPX_LINE_EVENT_REGISTER_FAILED  = 24000,    /**< The REGISTER_FAILED event is fired to indicate a failure of REGISTRATION.
                                                     It is fired in the following cases:
                                                     The client could not connect to the registrar server.
                                                     The registrar server challenged the client for authentication credentials,
                                                     and the client failed to supply valid credentials.
                                                     The registrar server did not generate a success response (status code == 200)
                                                     within a timeout period.  */
   SIPX_LINE_EVENT_UNREGISTER_FAILED  = 25000, /**< The UNREGISTER_FAILED event is fired to indicate a failure of un-REGISTRATION.
                                                     It is fired in the following cases:
                                                     The client could not connect to the registrar server.
                                                     The registrar server challenged the client for authentication credentials,
                                                     and the client failed to supply valid credentials.
                                                     The registrar server did not generate a success response (status code == 200)
                                                     within a timeout period.  */
   SIPX_LINE_EVENT_PROVISIONED      = 26000    /**<  The PROVISIONED event is fired when a sipXtapi Line is added, and Registration is not
                                                     requested (i.e. - sipxLineAdd is called with a bRegister parameter of false. */
} SIPX_LINE_EVENT_TYPE_MAJOR;

/**
 * Enumeration of possible minor Line Events.
 */
typedef enum SIPX_LINE_EVENT_TYPE_MINOR
{
    LINE_EVENT_UNKNOWN                                 = -1,       /**< No cause specified. */
    LINE_EVENT_REGISTERING_NORMAL                      = SIPX_LINE_EVENT_REGISTERING + 1,
    LINE_EVENT_REGISTERED_NORMAL                       = SIPX_LINE_EVENT_REGISTERED + 1,
    LINE_EVENT_UNREGISTERING_NORMAL                    = SIPX_LINE_EVENT_UNREGISTERING + 1,
    LINE_EVENT_UNREGISTERED_NORMAL                     = SIPX_LINE_EVENT_UNREGISTERED + 1,
    LINE_EVENT_REGISTER_FAILED_COULD_NOT_CONNECT       = SIPX_LINE_EVENT_REGISTER_FAILED + 1,
    LINE_EVENT_REGISTER_FAILED_NOT_AUTHORIZED          = SIPX_LINE_EVENT_REGISTER_FAILED + 2,
    LINE_EVENT_REGISTER_FAILED_TIMEOUT                 = SIPX_LINE_EVENT_REGISTER_FAILED + 3,
    LINE_EVENT_UNREGISTER_FAILED_COULD_NOT_CONNECT     = SIPX_LINE_EVENT_UNREGISTER_FAILED + 1,
    LINE_EVENT_UNREGISTER_FAILED_NOT_AUTHORIZED        = SIPX_LINE_EVENT_UNREGISTER_FAILED + 2,
    LINE_EVENT_UNREGISTER_FAILED_TIMEOUT               = SIPX_LINE_EVENT_UNREGISTER_FAILED + 3,
    LINE_EVENT_PROVISIONED_NORMAL                      = SIPX_LINE_EVENT_PROVISIONED + 1
} SIPX_LINE_EVENT_TYPE_MINOR;

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
SIPXTAPI_API char* sipxCallEventToString(SIPX_CALLSTATE_MAJOR eMajor,
                                     SIPX_CALLSTATE_MINOR eMinor,
                                     char*  szBuffer,
                                     size_t nBuffer) ;

/**
 * Create a printable string version of the designated event.
 * This is generally used for debugging.
 *
 * @param category Event category code
 * @param pEvent Pointer to the Event.
 * @param szBuffer buffer to store event string
 * @param nBuffer length of string buffer szBuffer
 */
SIPXTAPI_API char* sipxEventToString(const SIPX_EVENT_CATEGORY category,
                                     const void* pEvent,
                                     char*  szBuffer,
                                     size_t nBuffer);
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
 * @param lineTypeMinor minor event type id
 * @param szBuffer buffer to store event string
 * @param nBuffer length of string buffer szBuffer
 */
SIPXTAPI_API char* sipxLineEventToString(SIPX_LINE_EVENT_TYPE_MAJOR lineTypeMajor,
                                         SIPX_LINE_EVENT_TYPE_MINOR lineTypeMinor,
                                         char*  szBuffer,
                                         size_t nBuffer);

/**
 * Create a printable string version of the designated config event.
 * This is generally used for debugging.
 *
 * @param event Configuration event id
 * @param szBuffer Buffer to store event string
 * @param nBuffer Length of string buffer szBuffer
 */
SIPXTAPI_API char* sipxConfigEventToString(SIPX_CONFIG_EVENT event,
                                           char* szBuffer,
                                           size_t nBuffer) ;

#endif /* ifndef _sipXtapiEvents_h_ */

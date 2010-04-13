//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


/**
 * @mainpage sipXtapi SDK Overview
 *
 * @htmlinclude sipXtapi-overview.html
 */

/**
 * @file sipXtapi.h
 * sipXtapi main API declarations
 **/


#ifndef _sipXtapi_h_
#define _sipXtapi_h_

#include <stddef.h>       // size_t

// SYSTEM INCLUDES
// APPLICATION INCLUDES

// DEFINES
#define DEFAULT_UDP_PORT        5060    /**< Default UDP port */
#define DEFAULT_TCP_PORT        5060    /**< Default TCP port */
#define DEFAULT_TLS_PORT        5061    /**< Default TLS port */
#define DEFAULT_RTP_START_PORT  9000    /**< Starting RTP port for RTP port range.
                                             The user agent will use ports ranging
                                             from the start port to the start port
                                             + (default connections * 2). */
#define DEFAULT_CONNECTIONS     32      /**< Default number of max sim. conns. */
#define DEFAULT_IDENTITY        "sipx"  /**< sipx@<IP>:UDP_PORT used as identify if lines
                                             are not defined.  This define only controls
                                             the userid portion of the SIP url. */
#define DEFAULT_BIND_ADDRESS    "0.0.0.0" /**< Bind to all interfaces */

#define CODEC_G711_PCMU         "258"   /**< ID for PCMU vocodec */
#define CODEC_G711_PCMA         "257"   /**< ID for PCMA vocodec*/
#define CODEC_DTMF_RFC2833      "128"   /**< ID for RFC2833 DMTF (out of band DTMF codec) */

#define GAIN_MIN                1       /**< Min acceptable gain value */
#define GAIN_MAX                50     /**< Max acceptable gain value */
#define GAIN_DEFAULT            35      /**< Nominal gain value */

#define VOLUME_MIN              1       /**< Min acceptable volume value */
#define VOLUME_MAX              10      /**< Max acceptable volume value */
#define VOLUME_DEFAULT          7       /**< Nominal volume value */

#define MAX_AUDIO_DEVICES       16      /**< Max number of input/output audio devices */

#define CONF_MAX_CONNECTIONS    4       /**< Max number of conference participants */

#define SIPXTAPI_VERSION_STRING "SIPxua SDK %s.%s (built %s)" /**< Version string format string */
#define SIPXTAPI_VERSION        "2.9.0"      /**< sipXtapi API version */
#define SIPXTAPI_BUILDNUMBER    "0"          /**< Default build number */
#define SIPXTAPI_BUILDDATE      "2005-03-23" /**< Default build date */


#if defined(_WIN32)
#  ifdef SIPXTAPI_EXPORTS
#    define SIPXTAPI_API extern "C" __declspec(dllexport)  /**< Used for Win32 imp lib creation */
#  else
#    define SIPXTAPI_API extern "C" __declspec(dllimport)  /**< Used for Win32 imp lib creation */
#  endif
#else
#  define SIPXTAPI_API extern "C"   /**< Assume extern "C" for non-win32 platforms */
#endif

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// FORWARD DECLARATIONS
class UtlString ;
class Url ;

// STRUCTS
class SipUserAgent ;
class SdpCodecFactory ;
class CallManager ;
class SipLineMgr ;
class SipRefreshMgr ;


// TYPEDEFS
/**
 * Speaker output types are used to differentiate between the logical ringer
 * (used to alert user of in inbound call) and speaker (in call audio device).
 */
typedef enum SPEAKER_TYPE
{
    SPEAKER,    /**< Speaker / in call device */
    RINGER      /**< Ringer / alerting device */
} SPEAKER_TYPE ;

/**
 * Codec bandwidth ids are used to select a group of codecs with equal or lower
 * bandwidth requirements
 */
typedef enum SIPX_BANDWIDTH_ID
{
    AUDIO_CODEC_BW_VARIABLE=0,   /**< ID for codecs with variable bandwidth requirements */
    AUDIO_CODEC_BW_LOW,          /**< ID for codecs with low bandwidth requirements */
    AUDIO_CODEC_BW_NORMAL,       /**< ID for codecs with normal bandwidth requirements */
    AUDIO_CODEC_BW_HIGH          /**< ID for codecs with high bandwidth requirements */
} SIPX_BANDWIDTH_ID;

/**
 * Signature for a log callback function that gets passed three strings,
 * first string is the priority level, second string is the source id of
 * the subsystem that generated the message, and the third string is the
 * message itself.
 */
typedef void (*sipxLogCallback)(const char* szPriority,
                                const char* szSource,
                                const char* szMsg);


/**
 * SIPX_RESULT is an enumeration with all the possible result/return codes.
 */
typedef enum SIPX_RESULT
{
    SIPX_RESULT_SUCCESS = 0,         /**< Success */
    SIPX_RESULT_FAILURE,             /**< Generic Failure*/
    SIPX_RESULT_NOT_IMPLEMENTED,     /**< Method/API not implemented */
    SIPX_RESULT_OUT_OF_MEMORY,       /**< Unable to allocate enough memory to perform operation*/
    SIPX_RESULT_INVALID_ARGS,        /**< Invalid arguments; bad handle, argument out of range,
                                          etc.*/
    SIPX_RESULT_BAD_ADDRESS,         /**< Invalid SIP address */
    SIPX_RESULT_OUT_OF_RESOURCES,    /**< Out of resources (hit some max limit) */
    SIPX_RESULT_INSUFFICIENT_BUFFER, /**< Buffer too short for this operation */
} SIPX_RESULT ;

/**
 * DTMF/other tone ids used with sipxCallStartTone/sipxCallStopTone
 */
typedef enum TONE_ID
{
    ID_DTMF_0              = '0',   /**< DMTF 0 */
    ID_DTMF_1              = '1',   /**< DMTF 1 */
    ID_DTMF_2              = '2',   /**< DMTF 2 */
    ID_DTMF_3              = '3',   /**< DMTF 3 */
    ID_DTMF_4              = '4',   /**< DMTF 4 */
    ID_DTMF_5              = '5',   /**< DMTF 5 */
    ID_DTMF_6              = '6',   /**< DMTF 6 */
    ID_DTMF_7              = '7',   /**< DMTF 7 */
    ID_DTMF_8              = '8',   /**< DMTF 8 */
    ID_DTMF_9              = '9',   /**< DMTF 9 */
    ID_DTMF_STAR           = '*',   /**< DMTF * */
    ID_DTMF_POUND          = '#',   /**< DMTF # */
    ID_TONE_DIALTONE  = 512,        /**< Dialtone */
    ID_TONE_BUSY,                   /**< Call-busy tone */
    ID_TONE_RINGBACK,               /**< Remote party is ringing feedback tone */
    ID_TONE_RINGTONE,               /**< Default ring/alert tone */
    ID_TONE_CALLFAILED,             /**< Fasy Busy / call failed tone */
    ID_TONE_SILENCE,                /**< Silence */
    ID_TONE_BACKSPACE,              /**< Backspace tone */
    ID_TONE_CALLWAITING,            /**< Call waiting alert tone*/
    ID_TONE_CALLHELD,               /**< Call held feedback tone */
    ID_TONE_LOUD_FAST_BUSY          /**< Off hook / fast busy tone */
} TONE_ID ;


/**
 * Various log levels available for the sipxConfigEnableLog method.
 * Developers can choose the amount of detail available in the log.
 * Each level includes messages generated at lower levels.  For
 * example, LOG_LEVEL_EMERG will limit the log to emergency messsages,
 * while LOG_LEVEL_ERR includes emergency messages, alert messages,
 * critical messages, and errors.  LOG_LEVEL_ERR is probably best for
 * general runtime situations.  LOG_LEVEL_INFO or LOG_LEVEL_DEBUG is
 * best for diagnosing problems.
 */
typedef enum SIPX_LOG_LEVEL
{
    LOG_LEVEL_DEBUG,     /**< debug-level messages */
    LOG_LEVEL_INFO,      /**< informational messages */
    LOG_LEVEL_NOTICE,    /**< normal, but significant, conditions */
    LOG_LEVEL_WARNING,   /**< warning conditions */
    LOG_LEVEL_ERR,       /**< error conditions */
    LOG_LEVEL_CRIT,      /**< critical conditions */
    LOG_LEVEL_ALERT,     /**< action must be taken immediately */
    LOG_LEVEL_EMERG,     /**< system is unusable */
    LOG_LEVEL_NONE,      /**< disable logging */
} SIPX_LOG_LEVEL ;


/**
 * SIPX_CONTACT_TYPE is an enumeration of possible address type for use with
 * SIP contacts and SDP connection information.
 */
typedef enum
{
    CONTACT_LOCAL,      /**< Local address for a particular interface */
    CONTACT_NAT_MAPPED, /**< NAT mapped address (e.g. STUN)           */
    CONTACT_RELAY,      /**< Relay address (e.g. TURN)                */
    CONTACT_CONFIG,     /**< Manually configured address              */

    CONTACT_AUTO = -1,  /**< Automatic contact selection; used for API
                             parameters */
} SIPX_CONTACT_TYPE ;


/**
 * The ContactAddress structure includes contact information (ip and port),
 * address source type, and interface.
 */
typedef struct
{
    SIPX_CONTACT_TYPE eContactType ;   /**< Address type/source */
    char*             cInterface[32] ; /**< Source interface    */
    char              cIpAddress[32] ; /**< IP Address          */
    int               iPort ;          /**< Port                */
} SIPX_CONTACT_ADDRESS ;


/**
 * The SIPX_AUDIO_CODEC structure includes codec name and bandwidth info,
 * address source type, and interface.
 */
typedef struct
{
#define SIPXTAPI_CODEC_NAMELEN 32
    char              cName[SIPXTAPI_CODEC_NAMELEN];  /**< Codec name    */
    SIPX_BANDWIDTH_ID iBandWidth;             /**< Bandwidth requirement */
} SIPX_AUDIO_CODEC ;

/**
 * The SIPX_INST handle represents an instance of a user agent.  A user agent
 * includes a SIP stack and media processing framework.  sipXtapi does support
 * multiple instances of user agents in the same process space, however,
 * certain media processing features become limited or ambiguous.  For
 * example, only one user agent should control the local system's input and
 * output audio devices. */
typedef void* SIPX_INST ;

/**
 * The SIPX_LINE handle represents an inbound or outbound identity.  When
 * placing outbound the application programmer must define the outbound
 * line.  When receiving inbound calls, the application can query the
 * line.
 */
typedef unsigned int SIPX_LINE ;

/**
 * The SIPX_CALL handle represents a call or connection between the user
 * agent and another party.  All call operations require the call handle
 * as a parameter.
 */
typedef unsigned int SIPX_CALL ;

/**
 * The SIPX_CONF handle represents a collection of CALLs that have bridge
 * (mixed) audio.  Application developers can manipulate each leg of the
 * conference through various conference functions.
 */
typedef unsigned int SIPX_CONF ;

/**
 * The SIPX_INFO handle represents a handle to an INFO message sent by
 * a sipXtapi instance.  INFO messages are useful for communicating
 * information between user agents within a logical call.  The SIPX_INFO
 * handle is returned when sending an INFO message via
 * sipxCallSendInfo(...).  The handle is references as part of the
 * EVENT_CATEGORY_INFO_STATUS event callback/observer.  sipXtapi will
 * automatically deallocate this handle immediately after the status
 * call back.
 */
typedef intptr_t SIPX_INFO;


/**
 * A publisher handle.  Refers to a publishing context.
 * SIPX_PUB handles are created by using sipxCreatePublisher.
 * SIPX_PUB handles should be torn down using sipxDestroyPublisher.
 */
typedef unsigned int SIPX_PUB;

/**
 * A subscription handle which refers to a notification subscription
 * associated with a call.
 * SIPX_SUB handles are created by using the sipxCallSubscribe function.
 * SIPX_SUB handles should be destroyed using the sipxCallUnsubscribe function.
 */
typedef intptr_t SIPX_SUB ;

/**
 * A handle referring to an SIP NOTIFY message.
 */
typedef unsigned int SIPX_NOTIFY ;

/**
 * Typedef for audio source (microphone) hook procedure.  This typedef
 * coupled with the sipxConfigSetMicAudioHook API allows developers to
 * view, modify or substitute microphone data.
 *
 * @param nSamples number of 16 bit unsigned PCM samples
 * @param pSamples pointer to array of samples.
 */
typedef void (*fnMicAudioHook)(const int nSamples, short* pSamples) ;

/**
 * Typedef for audio target(speaker) hook procedure.  This typedef
 * coupled with the sipxConfigSetSpkrAudioHook API allows developers to
 * intercept and modify audio headed for the speaker.
 *
 * @param nSamples number of 16 bit unsigned samples
 * @param pSamples pointer to array of samples
 */
typedef void (*fnSpkrAudioHook)(const int nSamples, short* pSamples) ;

/* ============================ FUNCTIONS ================================= */

/** @name Initialization */
//@{


/**
 * Initialize the sipX tapi-like API layer.  This method initialized the
 * basic SIP stack and media process resources and must be called before
 * any other sipxXXX methods.  Additionally, this method fills in a
 * SIPX_INST parameter which must be passed to a number of sipX methods.
 *
 * @param phInst A pointer to a hInst that must be various other
 *        sipx routines.
 * @param udpPort The default UDP port for the SIP protocol stack.  The
 *        port cannot be changed after initialization.  Pass a value of 0
 *        (zero) to automatically select an open port or -1 to disable
 *        UDP.
 * @param tcpPort The default TCP port for the SIP protocol stack.  The
 *        port cannot be changed after initialization.  Pass a value of 0
 *        (zero) to automatically select an open port or -1 to disable
 *        TCP.
 * @param tlsPort **NOT YET SUPPORTED**
 * @param rtpPortStart The starting port for inbound RTP traffic.  The
 *        sipX layer will use ports starting at rtpPortStart and ending
 *        at (rtpPortStart + 2 * maxConnections) - 1.  Pass a value of 0
 *        (zero) to automatically select an open port.
 * @param maxConnections The maximum number of simultaneous connections
 *        that the sipX layer will support.
 * @param szIdentity The default outbound identity used by the SIP stack
 *        if no line appears are defined.
 * @param szBindToAddr Defines which IP/address the user agent / rtp
 *        stack will listen on.  The default "0.0.0.0" listens on all
 *        interfaces.  The address must be in dotted decimal form --
 *        hostnames will not work.
 * @param bUseSequentialPorts If unable to bind to the udpPort, tcpPort,
 *        or tlsPort, try sequential ports until a successful port is
 *        found.  If enabled, sipXtapi will try 10 sequential port
 *        numbers after the initial port.
 */
SIPXTAPI_API SIPX_RESULT sipxInitialize(SIPX_INST* phInst,
                                        const int udpPort = DEFAULT_UDP_PORT,
                                        const int tcpPort = DEFAULT_TCP_PORT,
                                        const int tlsPort = DEFAULT_TLS_PORT,
                                        const int rtpPortStart = DEFAULT_RTP_START_PORT,
                                        const int maxConnections = DEFAULT_CONNECTIONS,
                                        const char* szIdentity = DEFAULT_IDENTITY,
                                        const char* szBindToAddr = DEFAULT_BIND_ADDRESS,
                                        bool      bUseSequentialPorts = false) ;


/**
 * Un-initialize the sipX tapi-like API layer.  This method tears down the
 * basic SIP stack and media process resources and should be called before
 * exiting the process.
 *
 * @param hInst An instance handle obtained from sipxInitialize.
 */
SIPXTAPI_API SIPX_RESULT sipxUnInitialize(SIPX_INST hInst);

//@}
/** @name Call Methods */
//@{

/**
 * Accepts an inbound call and proceed immediately to alerting.  This method
 * is invoked in response to a NEWCALL event.  Whenever a new call is received,
 * the application developer should ACCEPT (proceed to ringing), REJECT (send
 * back busy), or REDIRECT the call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param contactType The desired contact type to use for this call.  The
 *        contactType allows you to control whether the user agent and
 *        media processing advertises the local address (e.g. 10.1.1.x or
 *        192.168.x.x) or the NAT-derived address to the target party.
 */
SIPXTAPI_API SIPX_RESULT sipxCallAccept(const SIPX_CALL   hCall,
                                        SIPX_CONTACT_TYPE contactType = CONTACT_AUTO) ;

/**
 * Reject an inbound call (prior to alerting the user).  This method must
 * be invoked before the end user is alerted (before sipXcallAccept).
 * Whenever a new call is received, the application developer should ACCEPT
 * (proceed to ringing), REJECT (send back busy), or REDIRECT the call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */
SIPXTAPI_API SIPX_RESULT sipxCallReject(const SIPX_CALL hCall) ;


/**
 * Redirect an inbound call (prior to alerting the user).  This method must
 * be invoked before the end user is alerted (before sipXcallAccept).
 * Whenever a new call is received, the application developer should ACCEPT
 * (proceed to ringing), REJECT (send back busy), or REDIRECT the call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szForwardURL SIP url to forward/redirect the call to.
 */
SIPXTAPI_API SIPX_RESULT sipxCallRedirect(const SIPX_CALL hCall,
                                          const char* szForwardURL) ;

/**
 * Answer an alerting call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */
SIPXTAPI_API SIPX_RESULT sipxCallAnswer(const SIPX_CALL hCall) ;


/**
 * Create a new call for the purpose of creating an outbound connection/call.
 * As a side effect, a DIALTONE event is fired to simulate the PSTN world.
 * Generally an application would simulate dialtone in reaction to that
 * event.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param hLine Line Identity for the outbound call.  The line identity
 *        helps defines the "From" caller-id.
 * @param phCall Pointer to a call handle.  Upon success, this value is
 *        replaced with a valid call handle.  Success is determined by
 *        the SIPX_RESULT result code.
 */
SIPXTAPI_API SIPX_RESULT sipxCallCreate(const SIPX_INST hInst,
                                        const SIPX_LINE hLine,
                                        SIPX_CALL*  phCall) ;


/**
 * Get the local contact address available for outbound/inbound signaling and
 * audio.  The local contact addresses will always include the local IP
 * addresses.  The local contact addresses may also include external NAT-
 * derived addresses (e.g. STUN).  See the definition of SIPX_CONTACT_ADDRESS
 * for more details.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param addresses A pre-allocated list of SIPX_CONTACT_ADDRESS
 *        structures.  This data will be filled in by the API call.
 * @param nMaxAddresses The maximum number of addresses supplied by the
 *        addresses parameter.
 * @param nActualAddresses The actual number of addresses populated in
 *        the addresses parameter.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetLocalContacts(const SIPX_CALL hCall,
                                                  SIPX_CONTACT_ADDRESS addresses[],
                                                  size_t nMaxAddresses,
                                                  size_t& nActualAddresses) ;

/**
 * Connects an idle call to the designated target address
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szAddress SIP url of the target party
 * @param contactType The desired contact type to use for this call.  The
 *        contactType allows you to control whether the user agent and
 *        media processing advertises the local address (e.g. 10.1.1.x or
 *        192.168.x.x) or the NAT-derived address to the target party.
 */
SIPXTAPI_API SIPX_RESULT sipxCallConnect(const SIPX_CALL hCall,
                                         const char* szAddress,
                                         SIPX_CONTACT_TYPE contactType = CONTACT_AUTO) ;


/**
 * Placed the specified call on hold.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */
SIPXTAPI_API SIPX_RESULT sipxCallHold(const SIPX_CALL hCall) ;


/**
 * Take the specified call off hold.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */
SIPXTAPI_API SIPX_RESULT sipxCallUnhold(const SIPX_CALL hCall) ;


/**
 * Drop/Destroy the specified call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */
SIPXTAPI_API SIPX_RESULT sipxCallDestroy(SIPX_CALL& hCall) ;


/**
 * Get the SIP call ID of the call represented by the specified call handle.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szId Buffer to store the ID.  A zero-terminated string will be
 *        copied into this buffer on success.
 * @param iMaxLength Max length of the ID buffer
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetID(const SIPX_CALL hCall,
                                       char* szId,
                                       const size_t iMaxLength) ;

/**
 * Get the SIP identity of the local connection.  The identity represents
 * either 1) who was called in the case of a inbound call, or 2) the
 * line identity used in an outbound call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szId Buffer to store the ID.  A zero-terminated string will be
 *        copied into this buffer on success.
 * @param iMaxLength Max length of the ID buffer.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetLocalID(const SIPX_CALL hCall,
                                            char* szId,
                                            const size_t iMaxLength) ;


/**
 * Get the SIP identity of the remote connection.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szId Buffer to store the ID.  A zero-terminated string will be
 *        copied into this buffer on success.
 * @param iMaxLength Max length of the ID buffer.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetRemoteID(const SIPX_CALL hCall,
                                             char* szId,
                                             const size_t iMaxLength) ;


/**
 * Gets the media interface connectionid.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param connectionId Reference to the returned connection identifier.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetConnectionId(const SIPX_CALL hCall,
                                                 int& connectionId);



/**
 * Play a tone (DTMF, dialtone, ring back, etc) to the local and/or
 * remote party.  See the DTMF_ constants for built-in tones.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param toneId ID of the tone to play
 * @param bLocal Should the tone be played locally?
 * @param bRemote Should the tone be played to the remote party?
 */
SIPXTAPI_API SIPX_RESULT sipxCallStartTone(const SIPX_CALL hCall,
                                           const TONE_ID toneId,
                                           const bool bLocal,
                                           const bool bRemote) ;

/**
 * Stop playing a tone (DTMF, dialtone, ring back, etc). to local
 * and remote parties.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */
SIPXTAPI_API SIPX_RESULT sipxCallStopTone(const SIPX_CALL hCall) ;


/**
 * Play the designed file.  The file may be a raw 16 bit signed PCM at
 * 8000 samples/sec, mono, little endian.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio files can only be played in the
 *        context of a call.
 * @param szFile Filename for the audio file to be played.
 * @param bLocal True if the audio file is to be rendered locally.
 * @param bRemote True if the audio file is to be rendered by the remote
 *                endpoint.
 */
SIPXTAPI_API SIPX_RESULT sipxCallPlayFile(const SIPX_CALL hCall,
                                          const char* szFile,
                                          const bool bLocal,
                                          const bool bRemote) ;

/**
 * Subscribe for NOTIFY events which may be published by the other end-point of the Call.
 *
 * @param hCall The call handle of the call associated with the subscription.
 * @param szEventType A string representing the type of event that can be published.
 * @param szAcceptType A string representing the types of NOTIFY events that this client will accept.
 * @param phSub Pointer to a subscription handle whose value is set by this funtion.
 */
SIPXTAPI_API SIPX_RESULT sipxCallSubscribe(const SIPX_CALL hCall,
                                           const char* szEventType,
                                           const char* szAcceptType,
                                           SIPX_SUB* phSub) ;

/**
 * Unsubscribe from previously subscribed NOTIFY events.
 *
 * @param hSub The subscription handle obtained from the call to sipXCallSubscribe.
 */
SIPXTAPI_API SIPX_RESULT sipxCallUnsubscribe(const SIPX_SUB hSub) ;

/**
 * Creates a publishing context, which perfoms the processing necessary
 * to accept SUBSCRIBE requests, and to publish NOTIFY messages to subscribers.
 * The resource may be specific to a single call, conference or global
 * to this user agent.  The naming of the resource ID determines the scope.
 *
 * @param phPub Pointer to a publisher handle - this method modifies the value
 *              to refer to the newly created publishing context.
 * @param szResourceId The resourceId to the state information being published.
 *               This must match the request URI of the incoming SUBSCRIBE request
 *               (only the user ID, host and port are significant in matching the
 *               request URI).  Examples: fred\@10.0.0.1:5555,
 *               sip:conference1\@192.160.0.1, sip:kate\@example.com
 * @param szEventType A string representing the type of event that can be
 *               published.
 * @param szContentType String representation of the content type being published.
 * @param pContent Pointer to the NOTIFY message's body content.
 * @param nContentLength Size of the content to be published.
 * @return If the resource already has a a publisher created for the given
 *               event type, SIPX_RESULT_INVALID_ARGS is returned.
 */
SIPXTAPI_API SIPX_RESULT sipxCreatePublisher(SIPX_PUB* phPub,
                                             const char* szResourceId,
                                             const char* szEventType,
                                             const char* szContentType,
                                             const char* pContent,
                                             const size_t nContentLength);

/**
 * Tears down the publishing context.  Any existing subscriptions
 * are sent a final NOTIFY request.  If pFinalContent is not NULL and
 * nContentLength > 0 the given publish state is given otherwise
 * the final NOTIFY requests are sent with no body or state.
 *
 * @param hPub Handle of the publishing context to destroy
 *              (returned from a call to sipxCreatePublisher)
 * @param szContentType String representation of the content type being published
 * @param pFinalContent Pointer to the NOTIFY message's body content
 * @param nContentLength Size of the content to be published
 */
SIPXTAPI_API SIPX_RESULT sipxDestroyPublisher(const SIPX_PUB hPub,
                                              const char* szContentType,
                                              const char* pFinalContent,
                                              const size_t nContentLength);

/**
 * Publishes an updated state to specific event via NOTIFY to its subscribers.
 *
 * @param hPub Handle of the publishing context
 *              (returned from a call to sipxCreatePublisher)
 * @param szContentType String representation of the content type being published
 * @param pContent Pointer to the NOTIFY message's body content
 * @param nContentLength Size of the content to be published
 */
SIPXTAPI_API SIPX_RESULT sipxCallPublish(const SIPX_PUB hPub,
                                         const char* szContentType,
                                         const char* pContent,
                                         const size_t nContentLength);

/**
 * Sends an INFO event to the other end-point(s) on a Call.
 *
 * @param phInfo Pointer to an INFO message handle, whose value is set by this method.
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szContentType String representation of the INFO content type
 * @param szContent Pointer to the INFO messasge's content
 * @param nContentLength Size of the INFO content
 */
SIPXTAPI_API SIPX_RESULT sipxCallSendInfo(SIPX_INFO* phInfo,
                                          const SIPX_CALL hCall,
                                          const char* szContentType,
                                          const char* szContent,
                                          const size_t nContentLength);

/**
 * Blind transfer the specified call to another party.
 * WARNING: STILL UNDER DEVELOPMENT
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szAddress SIP url identifing the transfer target (who the call
 *        identified by hCall will be transfered to).
 */
SIPXTAPI_API SIPX_RESULT sipxCallBlindTransfer(const SIPX_CALL hCall,
                                               const char* szAddress) ;
//@}

/** @name Conference Methods */
//@{


/**
 * Create a conference handle.  Conferences are an association of calls
 * where the audio media is mixed.  sipXtapi supports conferences up to
 * 4 (CONF_MAX_CONNECTIONS) parties in its default configuration.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param phConference Pointer to a conference handle.  Upon success,
 *        this value is replaced with a valid conference handle.
 *        Success is determined by the SIPX_RESULT result code.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceCreate(const SIPX_INST hInst,
                                              SIPX_CONF*      phConference) ;

/**
 * Join (add) an existing call into a conference.
 * The call must already exist and the conference must already
 * have been created.
 *
 * @param hConf Conference handle obtained by calling sipxConferenceCreate.
 * @param hCall Call handle of the call to join into the conference.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceJoin(const SIPX_CONF hConf,
                                            const SIPX_CALL hCall) ;

/**
 * Split (remove) an existing call from a conference (NOT IMPLEMENTED).
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceSplit(const SIPX_CONF hConf,
                                             const SIPX_CALL hCall) ;

/**
 * Add a new party to an existing conference.  A connection is automatically
 * initiated for the specified address.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained
 *        by invoking sipxConferenceCreate.
 * @param hLine Line Identity for the outbound call.  The line identity
 *        helps defines the "From" caller-id.
 * @param szAddress SIP url of the conference partipant to add
 * @param phNewCall Pointer to a call handle to store new call.
 * @param contactType The desired contact type to use for this call.  The
 *        contactType allows you to control whether the user agent and
 *        media processing advertises the local address (e.g. 10.1.1.x or
 *        192.168.x.x) or the NAT-derived address to the target party.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceAdd(const SIPX_CONF hConf,
                                           const SIPX_LINE hLine,
                                           const char* szAddress,
                                           SIPX_CALL* phNewCall,
                                           SIPX_CONTACT_TYPE contactType = CONTACT_AUTO) ;

/**
 * Removes a participant from conference by hanging up on them.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained
 *        by invoking sipxConferenceCreate.
 * @param hCall Call handle identifying which call to remove from the
 *        conference by hanging up.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceRemove(const SIPX_CONF hConf,
                                              const SIPX_CALL hCall) ;


/**
 * Gets all of the calls participating in a conference.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained
 *        by invoking sipxConferenceCreate.
 * @param calls An array of call handles filled in by the API.
 * @param iMax The maximum number of call handles to return.
 * @param nActual The actual number of call handles returned.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceGetCalls(const SIPX_CONF hConf,
                                                SIPX_CALL calls[],
                                                const size_t iMax,
                                                size_t& nActual) ;

/**
 * Destroys a conference.  All participants within a conference are
 * dropped.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained
 *        by invoking sipxConferenceCreate.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceDestroy(SIPX_CONF hConf) ;

//@}

/** @name Audio Methods */
//@{

#if defined(_WIN32)

/**
 * Set the local microphone gain.  If the microphone is muted,
 * resetting the gain will not enable audio -- you must unmute
 * the microphone.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param iLevel The level of the local microphone gain
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetGain(const SIPX_INST hInst,
                                          const int iLevel) ;


/**
 * Get the current microphone gain.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param iLevel The level of the gain of the microphone
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetGain(const SIPX_INST hInst,
                                          int& iLevel) ;


/**
 * Mute or unmute the microphone.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param bMute True if the microphone is to be muted and false if it
 *        is not to be muted
 */
SIPXTAPI_API SIPX_RESULT sipxAudioMute(const SIPX_INST hInst,
                                       const bool bMute) ;


/**
 * Gets the mute state of the microphone.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param bMute True if the microphone has been muted and false if it
 *        is not mute
 */
SIPXTAPI_API SIPX_RESULT sipxAudioIsMuted(const SIPX_INST hInst,
                                          bool &bMuted) ;


/**
 * Enables one of the speaker outputs.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param type The type of the speaker either the logical ringer
 *		  (used to alert user of in inbound call) or speaker
 *        (in call audio device).
 */
SIPXTAPI_API SIPX_RESULT sipxAudioEnableSpeaker(const SIPX_INST hInst,
                                                const SPEAKER_TYPE type) ;


/**
 * Gets the enabled speaker selection.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param type The type of the speaker either the logical ringer
 *	      (used to alert user of in inbound call) or speaker
 *        (in call audio device).
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetEnabledSpeaker(const SIPX_INST hInst,
                                                    SPEAKER_TYPE& type) ;


/**
 * Sets the audio level for the designated speaker type.  If the speaker type
 * is enabled, the change it audio will be heard instantly.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param type The type of the speaker either the logical ringer
 *	      (used to alert user of in inbound call) or speaker
 *        (in call audio device).
 * @param iLevel The level of the gain of the microphone
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetVolume(const SIPX_INST hInst,
                                            const SPEAKER_TYPE type,
                                            const int iLevel) ;


/**
 * Gets the audio level for the designated speaker type
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param type The type of the speaker either the logical ringer
 *		  (used to alert user of in inbound call) or speaker
 *        (in call audio device).
 * @param iLevel The level of the gain of the microphone
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetVolume(const SIPX_INST hInst,
                                            const SPEAKER_TYPE type,
                                            int& iLevel) ;


/**
 * Enables or disables Acoustic Echo Cancellation (AEC).
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param enable True if AEC is to be enabled and false if
 *        AEC is to be disabled.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioEnableAEC(const SIPX_INST hInst,
                                            const bool enable) ;


/**
 * Gets the enabled or disabled state of Acoustic Echo Cancellation (AEC).
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param enable True if AEC is enabled and false if AEC is disabled.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioIsAECEnabled(const SIPX_INST hInst,
                                               bool& enabled) ;

#endif /* defined(_WIN32) */

/**
 * Get the number of input devices available on this system.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param numDevices The number of input devices available
 *        on this system.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetNumInputDevices(const SIPX_INST hInst,
                                                     size_t& numDevices) ;

/**
 * Get the name/identifier for input device at position index
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param index Zero based index of the input device to be queried.
 * @param szDevice Reference an character string pointer to receive
 *                 the device name.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetInputDevice(const SIPX_INST hInst,
                                                 const int index,
                                                 const char*& szDevice) ;

/**
 * Get the number of output devices available on this system
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param numDevices The number of output devices available
 *        on this system.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetNumOutputDevices(const SIPX_INST hInst,
                                                      size_t& numDevices) ;

/**
 * Get the name/identifier for output device at position index
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param index Zero based index of the output device to be queried.
 * @param szDevice Reference an character string pointer to receive
 *                 the device name.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetOutputDevice(const SIPX_INST hInst,
                                                  const int index,
                                                  const char*& szDevice) ;

/**
 * Set the call input device (in-call microphone).
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szDevice Character string pointer to be set to
 *                 a string name of the output device.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetCallInputDevice(const SIPX_INST hInst,
                                                     const char* szDevice) ;

/**
 * Set the call ringer/alerting device.
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szDevice The call ringer/alerting device.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetRingerOutputDevice(const SIPX_INST hInst,
                                                        const char* szDevice) ;


/**
 * Set the call output device (in-call speaker).
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szDevice The call output device.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetCallOutputDevice(const SIPX_INST hInst,
                                                      const char* szDevice) ;
//@}
/** @name Line / Identity Methods*/
//@{


/**
 * Adds a line appearance.  A line appearance defines your address of record
 * and is used both as your "From" caller-id and as the public identity to
 * which you will receive calls for.  Directing calls to a particular user
 * agent is achieved using registrations.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szLineURL The address of record for the line identity.
 * @param phLine Pointer to a line handle.  Upon success, a handle to the
 *        newly added line is returned.
 * @param contactType The desired contact type to use for this call.  The
 *        contactType allows you to control whether the user agent and
 *        media processing advertises the local address (e.g. 10.1.1.x or
 *        192.168.x.x) or the NAT-derived address to the target party.
 */
SIPXTAPI_API SIPX_RESULT sipxLineAdd(const SIPX_INST hInst,
                                     const char* szLineURL,
                                     SIPX_LINE* phLine,
                                     SIPX_CONTACT_TYPE contactType = CONTACT_AUTO) ;

/**
 *  Registers a line with the proxy server.  Registrations will be re-registered
 *  automatically, before they expire.
 *  Unless your user agent is designated a static IP
 *        address or DNS name and that routing information is provisioned
 *        into a SIP server, you should register the line by calling this function.
 * @param hLine Handle to a line appearance.  Line handles are obtained by
 *        creating a line using the sipxLineAdd function or by receiving
 *        a line event notification.
 * @param bRegister true if Registration is desired, otherwise, an Unregister is performed.
 */
SIPXTAPI_API SIPX_RESULT sipxLineRegister(const SIPX_LINE hLine, const bool bRegister);

/**
 * Remove the designated line appearence.
 *
 * @param hLine Handle to a line appearance.  Line handles are obtained by
 *        creating a line using the sipxLineAdd function or by receiving
 *        a line event notification.
 */
SIPXTAPI_API SIPX_RESULT sipxLineRemove(SIPX_LINE hLine) ;


/**
 * Adds authentication credentials to the designated line appearance.
 * Credentials are often required by registration services to verify that the
 * line is being used by the line appearance/address of record owner.
 *
 * @param hLine Handle to a line appearance.  Line handles are obtained by
 *        creating a line using the sipxLineAdd function or by receiving
 *        a line event notification.
 * @param szUserID user id used for the line appearance.
 * @param szPasswd passwd used for the line appearance.
 * @param szRealm realm for which the user and passwd are valid.
 */
SIPXTAPI_API SIPX_RESULT sipxLineAddCredential(const SIPX_LINE hLine,
                                               const char* szUserID,
                                               const char* szPasswd,
                                               const char* szRealm) ;

/**
 * Gets the active list of line identities.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param lines Pre-allocated array of line handles.
 * @param max Maximum number of lines to return.
 * @param actual Actual number of valid lines returned.
 */

SIPXTAPI_API SIPX_RESULT sipxLineGet(const SIPX_INST hInst,
                                     SIPX_LINE lines[],
                                     const size_t max,
                                     size_t& actual) ;

/**
 * Get the Line URI for the designated line handle
 *
 * @param hLine Handle to a line appearance.  Line handles are obtained by
 *        creating a line using the sipxLineAdd function or by receiving
 *        a line event notification.
 * @param szBuffer Buffer to place line URL.  A NULL value will return
 *        the amount of storage needed in nActual.
 * @param nBuffer Size of szBuffer in bytes (not to exceed)
 * @param nActual Actual number of bytes written
 */
SIPXTAPI_API SIPX_RESULT sipxLineGetURI(const SIPX_LINE hLine,
                                        char*  szBuffer,
                                        const size_t nBuffer,
                                        size_t& nActual) ;

//@}
/** @name Configuration Methods*/
//@{


/**
 * The sipxConfigEnableLog method enables logging for the sipXtapi API,
 * media processing, call processing, SIP stack, and OS abstraction layer.
 * Logging is disabled by default.  The underlying framework makes no attempts
 * to bound the log file to a fixed size.
 *
 * Log Format:
 *    time:event id:facility:priority:host name:task name:task id:process id:log message
 *
 * The log message escapes backslashes, Cr, Lfs, and quotes.  See
 * OsSysLog.h/cpp in the sipXportLib project for more details.  A viewer is
 * also available as part of the sipXportLib project.
 *
 * @param logLevel Designates the amount of detail includes in the log.  See
 *        SIPX_LOG_LEVEL for more details.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetLogLevel(SIPX_LOG_LEVEL logLevel) ;


/**
 * The sipxConfigSetlogFile method sets the filename of the log file and
 * directs output to that file
 *
 * NOTE: At this time no validation is performed on the specified filename.
 * Please make sure the directories exist and the appropriate permissions
 * are available.
 *
 * @param szFileName The filename for the log file.  Designated a NULL
 *        filename will disable logging, however, threads/resources will not
 *        be deallocated.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetLogFile(const char *szFilename) ;


/**
 * Set a callback function to collect logging information. This function
 * directs logging output to the specfied function.
 *
 * @param pCallback is a pointer to a callback function. This callback function
 *        gets passed three strings, first string is the priority level,
 *        second string is the source id of the subsystem that generated
 *        the message, and the third string is the message itself.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetLogCallback(sipxLogCallback pCallback);


/**
 * Designate a callback routine as a microphone replacement or supplement.
 * The callback is invoked with microphone data and the data can be reviewed,
 * modified, replaced, or discarded.
 *
 * This callback proc must *NOT* block and must return data quickly.
 * Additionally, the method should not call any blocking function (i.e. IO
 * operations, malloc, new, etc).
 *
 * Data must be formatted as mono 16-bit signed PCM, little endian, 8000
 * samples per second. The callback is handed 80 samples (10ms) of data at
 * a time.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetMicAudioHook(fnMicAudioHook hookProc) ;


/**
 * Designate a callback routine for post-mixing audio data (e.g. to speaker
 * data).  The hook may review, modify, replace, or discard data.
 *
 * This callback proc must *NOT* block and must return data quickly.
 * Additionally, the method should not call any blocking function (i.e. IO
 * operations, malloc, new, etc).
 *
 * Data must be formatted as mono 16-bit signed PCM, little endian, 8000
 * samples per second. The callback is handed 80 samples (10ms) of data at
 * a time.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetSpkrAudioHook(fnSpkrAudioHook hookProc) ;


/**
 * Defines the SIP proxy used for outbound requests.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szProxy the new outbound proxy
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetOutboundProxy(const SIPX_INST hInst,
                                                    const char* szProxy) ;

/**
 * Modifies the timeout values used for DNS SRV lookups.  In generally,
 * you shouldn't need to modified these, however, if you find yourself
 * in a situation where a router/network fails to send responses to
 * DNS SRV requests these values can be tweaked.  Note, failing to send
 * responses is different then a receiving an no-such-animal response.
 * <p>
 * The default values are initialTimeout = 5 seconds, and 4 retries.  The
 * time waited is doubled after each timeout, so with the default settings,
 * a single DNS SRV can block for 75 seconds (5 + 10 + 20 + 40).  In general,
 * 4 DNS SRV requests are made for each hostname (e.g. domain.com):
 * <ul>
 *   <li> _sip._udp.domain.com</li>
 *   <li> _sip._tcp.domain.com</li>
 *   <li> _sip._udp.domain.com.domain.com</li>
 *   <li> _sip._tcp.domain.com.domain.com</li>
 * </ul>
 *
 * If DNS response are dropped in the network (or your DNS server is down),
 * the API will block for 3 minutes.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetDnsSrvTimeouts(const int initialTimeoutInSecs,
                                                     const int retries) ;

/**
 * Specifies the time to wait before trying the next DNS SRV record.  The user
 * agent will attempt to obtain DNS SRV resolutions for the child DNS SRV
 * records.  This setting is the time allowed for attempting a lookup - if the
 * time expires without a lookup, then next child is attempted.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param failoverTimeoutInSecs Number of seconds until the next DNS SRV
 *        record is tried.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetDnsSrvFailoverTimeout(const SIPX_INST hInst,
                                                            const int failoverTimeoutInSecs);


/**
 * Enable or disable the use of "rport".  If rport is included on a via,
 * responses should be sent back to the originating port -- not what is
 * advertised as part of via.  Additionally, the sip stack will not
 * receive messages sent to the originating port unless this is enabled.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param bEnable Enable or disable the use of rport.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableRport(const SIPX_INST hInst,
                                               const bool bEnable) ;

/**
 * Specifies the expiration period for registration.  After setting this
 * configuration, all subsequent REGISTER messages will be sent with the new
 * registration period.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param nRegisterExpirationSecs Number of seconds until the expiration of a
 *        REGISTER message
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetRegisterExpiration(const SIPX_INST hInst,
                                                         const int nRegisterExpirationSecs);

/**
 * Specifies the expiration period for subscription.  After setting this
 * configuration, all subsequent SUBSCRIBE messages will be sent with the new
 * subscribe period.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param nSubscribeExpirationSecs Number of seconds until the expiration of a
 *        SUBSCRIBE message
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetSubscribeExpiration(const SIPX_INST hInst,
                                                          const int nSubscribeExpirationSecs);

/**
 * Enables STUN (Simple Traversal of UDP through NAT) support for both
 * UDP SIP signaling and UDP audio/video (RTP).  STUN helps user agents
 * determine thier external IP address from the inside of NAT/Firewall.
 * This method should be invoked immediately after calling sipxInitialize
 * and before creating any lines or calls.  Enabling STUN while calls are
 * setup should not effect the media path of existing calls.  The "contact"
 * address uses for UDP signaling may change on the next request.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szServer The stun server that should be used for discovery.
 * @param iKeepAliveSecs This setting controls how often to refresh the stun
 *        binding.  The most aggressive NAT/Firewall solutions free port
 *        mappings after 30 seconds of non-use.  We recommend a value of 28
 *        seconds to be safe.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableStun(const SIPX_INST hInst,
                                              const char* szServer,
                                              int iKeepAliveSecs) ;

/**
 * Disable the use of STUN.  See sipxConfigEnableStun for details on STUN.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 */
SIPXTAPI_API SIPX_RESULT sipxConfigDisableStun(const SIPX_INST hInst) ;


/**
 * Enable/disable sending of out-of-band DTMF tones. If disabled the tones
 * will be sent inband.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param enable Enable or disable out-of-band DTMF tones.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableOutOfBandDTMF(const SIPX_INST hInst,
                                                       const bool enable) ;


/**
 * Returns if sending of out-of-band DTMF tones is enabled or disabled.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param enabled Out-of-band DTMF tones enabled or disabled.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigIsOutOfBandDTMFEnabled(const SIPX_INST hInst,
                                                          bool& enabled) ;


/**
 * Get the sipXtapi API version string.
 *
 * @param szVersion Buffer to store the version string. A zero-terminated
 *        string will be copied into this buffer on success.
 * @param nBuffer Size of szBuffer in bytes (not to exceed). A size of 48 bytes
 *        should be sufficient in most cases.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetVersion(char* szVersion,
											  const size_t nBuffer) ;

/**
 * Get the local UDP port for SIP signaling.  The port is supplied in the
 * call to sipXinitialize; however, the port may be allocated dynamically.
 * This method will return SIPX_RESULT_SUCCESS if able to return the port
 * value.  SIPX_RESULT_FAILURE is returned if the protocol is not enabled.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pPort Pointer to a port number.  This value must not be NULL.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalSipUdpPort(SIPX_INST hInst, int* pPort) ;


/**
 * Get the local TCP port for SIP signaling.  The port is supplied in the
 * call to sipXinitialize; however, the port may be allocated dynamically.
 * This method will return SIPX_RESULT_SUCCESS if able to return the port
 * value.  SIPX_RESULT_FAILURE is returned if the protocol is not enabled.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pPort Pointer to a port number.  This value must not be NULL.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalSipTcpPort(SIPX_INST hInst, int* pPort) ;


/**
 * Get the local TLS port for SIP signaling.  The port is supplied in the
 * call to sipXinitialize; however, the port may be allocated dynamically.
 * This method will return SIPX_RESULT_SUCCESS if able to return the port
 * value.  SIPX_RESULT_FAILURE is returned if the protocol is not enabled.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pPort Pointer to a port number.  This value must not be NULL.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalSipTlsPort(SIPX_INST hInst, int* pPort) ;


/**
 * Set the preferred bandwidth requirement for codec selection. Whenever
 * possible a codec matching that requirement will be selected for a call.
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec
 * preferences.  SIPX_RESULT_FAILURE is returned if the preference is not set.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bandWidth Valid bandwidth requirements  are AUDIO_CODEC_BW_LOW,
 *        AUDIO_CODEC_BW_NORMAL, and AUDIO_CODEC_BW_HIGH.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetAudioCodecPreferences(const SIPX_INST hInst,
                                                            SIPX_BANDWIDTH_ID bandWidth) ;

/**
 * Get the current codec preference.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pBandWidth pointer to an integer that will contain AUDIO_CODEC_BW_LOW,
 *        AUDIO_CODEC_BW_NORMAL, or AUDIO_CODEC_BW_HIGH.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetAudioCodecPreferences(const SIPX_INST hInst,
                                                            SIPX_BANDWIDTH_ID *pBandWidth);

/**
 * Get the number of audio codecs.
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec
 * preferences.  SIPX_RESULT_FAILURE is returned if the number of codecs can
 * no be retrieved.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pNumCodecs Pointer to the number of codecs.  This value must not be NULL.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetNumAudioCodecs(const SIPX_INST hInst,
                                                     int* pNumCodecs) ;


/**
 * Get the audio codec at a certain index in the list of codecs. Use this
 * function in conjunction with sipxConfigGetNumAudioCodecs to enumerate
 * the list of audio codecs.
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec
 * preferences.  SIPX_RESULT_FAILURE is returned if the audio codec can not
 * be retrieved.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param index Index in the list of codecs
 * @param pCodec SIPX_AUDIO_CODEC structure that holds information
 *        (name, bandwidth requirement) about the codec.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetAudioCodec(const SIPX_INST hInst,
                                                 const int index,
                                                 SIPX_AUDIO_CODEC* pCodec) ;

//@}

#endif // _sipXtapi_h_

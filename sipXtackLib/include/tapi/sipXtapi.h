//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////

/**
 * @mainpage sipXtapi SDK Overview
 *
 * @htmlinclude sipXtapi-overview.html
 */

/**
 * sipXtapi main API declarations
 **/


#ifndef _sipXtapi_h_
#define _sipXtapi_h_

#include <memory.h>
#include <string.h>
#include <stddef.h>       // size_t
#include <stdint.h>       // intptr_t

// SYSTEM INCLUDES
#ifdef VIDEO
#ifdef _WIN32
#if !defined __strmif_h__
    #include <strmif.h>
#endif
#endif
#endif
#if !defined (_WIN32) || !defined (VIDEO)
    struct IBaseFilter;
#endif

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
#define DEFAULT_BIND_ADDRESS    "0.0.0.0" /**< Bind to the first physical interface discovered */

#define CODEC_G711_PCMU         "258"   /**< ID for PCMU vocodec */
#define CODEC_G711_PCMA         "257"   /**< ID for PCMA vocodec*/
#define CODEC_DTMF_RFC2833      "128"   /**< ID for RFC2833 DMTF (out of band DTMF codec) */

#define GAIN_MIN                1       /**< Min acceptable gain value */
#define GAIN_MAX                100      /**< Max acceptable gain value */
#define GAIN_DEFAULT            70      /**< Nominal gain value */

#define VOLUME_MIN              1       /**< Min acceptable volume value */
#define VOLUME_MAX              100      /**< Max acceptable volume value */
#define VOLUME_DEFAULT          70       /**< Nominal volume value */

#define MAX_AUDIO_DEVICES       16      /**< Max number of input/output audio devices */

#define CONF_MAX_CONNECTIONS    32      /**< Max number of conference participants */
#define SIPX_MAX_IP_ADDRESSES   32      /**< Maximum number of IP addresses on the host */

#define SIPX_STUN_NORMAL            0   /** Default STUN options: Do not request change port
                                            or change address. */
#define SIPX_STUN_CHANGE_PORT       1   /** When sending stun requests for public IP discovery
                                            (not ICE), ask the STUN server to send the response
                                            from a different port. */
#define SIPX_STUN_CHANGE_ADDRESS    2   /** When sending stun requests for for public IP
                                            discovery (not ICE), ask the STUN server to send the
                                            response from a different IP address. */

#define SIPX_PORT_DISABLE       -1      /**< Special value that disables the transport
                                             type (e.g. UDP, TCP, or TLS) when passed
                                             to sipXinitialize */
#define SIPX_PORT_AUTO          -2      /**< Special value that instructs sipXtapi to
                                             automatically select an open port for
                                             signaling or audio when passed to
                                             sipXinitialize */

#define SIPXTAPI_VERSION_STRING "SIPxua SDK %s.%s %s (built %s)" /**< Version string format string */
#define SIPXTAPI_VERSION        "2.9.0"      /**< sipXtapi API version -- automatically filled in
                                                  during release process */
#define SIPXTAPI_BUILDNUMBER    "0"          /**< Default build number -- automatically filled in
                                                  during release process*/
#define SIPXTAPI_BUILD_WORD     2,9,0,0      /**< Default build word -- automatically filled in
                                                  during release process */
#define SIPXTAPI_FULL_VERSION   "2.9.0.X"    /**< Default full version number -- automatically filled in
                                                  during release process*/
#define SIPXTAPI_BUILDDATE      "2005-03-23" /**< Default build date -- automatically filled in
                                                  during release process*/

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
typedef enum SIPX_AUDIO_BANDWIDTH_ID
{
    AUDIO_CODEC_BW_VARIABLE=0,   /**< ID for codecs with variable bandwidth requirements */
    AUDIO_CODEC_BW_LOW,          /**< ID for codecs with low bandwidth requirements */
    AUDIO_CODEC_BW_NORMAL,       /**< ID for codecs with normal bandwidth requirements */
    AUDIO_CODEC_BW_HIGH,         /**< ID for codecs with high bandwidth requirements */
    AUDIO_CODEC_BW_CUSTOM
} SIPX_AUDIO_BANDWIDTH_ID;

typedef enum SIPX_VIDEO_BANDWIDTH_ID
{
    VIDEO_CODEC_BW_VARIABLE=0,   /**< ID for codecs with variable bandwidth requirements */
    VIDEO_CODEC_BW_LOW,          /**< ID for codecs with low bandwidth requirements */
    VIDEO_CODEC_BW_NORMAL,       /**< ID for codecs with normal bandwidth requirements */
    VIDEO_CODEC_BW_HIGH,         /**< ID for codecs with high bandwidth requirements */
    VIDEO_CODEC_BW_CUSTOM
} SIPX_VIDEO_BANDWIDTH_ID;

typedef enum SIPX_VIDEO_QUALITY_ID
{
    VIDEO_QUALITY_LOW=1,         /**< Low quality video */
    VIDEO_QUALITY_NORMAL=2,      /**< Normal quality video */
    VIDEO_QUALITY_HIGH=3         /**< High quality video */
} SIPX_VIDEO_QUALITY_ID;


/**
 * Format definitions for memory resident audio data
 */
typedef enum SIPX_AUDIO_DATA_FORMAT
{
    RAW_PCM_16=0                 /**< Signed 16 bit PCM data, mono, 8KHz, no header */
} SIPX_AUDIO_DATA_FORMAT;


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
    SIPX_RESULT_OUT_OF_RESOURCES,    /**< Out of resources (hit some max limit or resource unavailble) */
    SIPX_RESULT_INSUFFICIENT_BUFFER, /**< Buffer too short for this operation */
    SIPX_RESULT_EVAL_TIMEOUT,        /**< The evaluation version of this product has expired */
    SIPX_RESULT_BUSY,                /**< The operation failed because the system was busy */
    SIPX_RESULT_INVALID_STATE,       /**< The operation failed because the object was in
                                          the wrong state.  For example, attempting to split
                                          a call from a conference before that call is
                                          connected. */
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
    ID_DTMF_FLASH          = '!',   /**< DTMF Flash */
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


class SIPX_SECURITY_ATTRIBUTES
{
public:
    SIPX_SECURITY_ATTRIBUTES()
    {
    }

    SIPX_SECURITY_ATTRIBUTES(const SIPX_SECURITY_ATTRIBUTES& ref)
    {
    }

    SIPX_SECURITY_ATTRIBUTES& operator=(const SIPX_SECURITY_ATTRIBUTES& ref)
    {
        // Need to be implemented!!!
        if (this == &ref)
            return *this;

        return *this;
    }

private:
};

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
 * Type for storing a "window object handle" - in Windows,
 * the application should cast their HWND to a SIPX_WINDOW_HANDLE.
 */
typedef void* SIPX_WINDOW_HANDLE;

/**
 * Enum for specifying the type of display object
 * to be used for displaying video
 */
typedef enum SIPX_VIDEO_DISPLAY_TYPE
{
    SIPX_WINDOW_HANDLE_TYPE,     /**< A handle to the window for
                                      the remote video display */
    DIRECT_SHOW_FILTER           /**< A DirectShow render filter object for
                                      handling the remote video display */
} SIPX_VIDEO_DISPLAY_TYPE;

struct SIPX_VIDEO_DISPLAY
{
    SIPX_VIDEO_DISPLAY()
    {
        cbSize = sizeof(SIPX_VIDEO_DISPLAY);
        type = SIPX_WINDOW_HANDLE_TYPE;
        handle = NULL;
    }
    SIPX_VIDEO_DISPLAY(const SIPX_VIDEO_DISPLAY& ref)
    {
        this->cbSize = ref.cbSize;
        this->type = ref.type;
        this->handle = ref.handle;
    }

    int cbSize;
    SIPX_VIDEO_DISPLAY_TYPE type;
    union
    {
    SIPX_WINDOW_HANDLE handle;
    IBaseFilter* filter;
    };
};

/** Type for storing Contact Record identifiers */
typedef int SIPX_CONTACT_ID;

/**
 * The ContactAddress structure includes contact information (ip and port),
 * address source type, and interface.
 */
struct SIPX_CONTACT_ADDRESS
{
    /**
     * Default constructor for a SIPX_CONTACT_ADDRESS.
     */
    SIPX_CONTACT_ADDRESS()
    {
        memset((void*)cInterface, 0, sizeof(cInterface));
        memset((void*)cIpAddress, 0, sizeof(cIpAddress));
        eContactType = CONTACT_AUTO;
        id = 0;
        iPort = -1;
    }

    /**
     * Copy constructor for SIPX_CONTACT_ADDRESS (deep copy)
     */
    SIPX_CONTACT_ADDRESS(const SIPX_CONTACT_ADDRESS& ref)
    {
        strcpy(cInterface, ref.cInterface);
        strcpy(cIpAddress, ref.cIpAddress);
        eContactType = ref.eContactType;
        id = ref.id;
        iPort = ref.iPort;
    }

    /**
     * Assignment operator for SIPX_CONTACT_ADDRESS (deep copy).
     */
    SIPX_CONTACT_ADDRESS& operator=(const SIPX_CONTACT_ADDRESS& ref)
    {
        // check for assignment to self
        if (this == &ref) return *this;

        strcpy(this->cInterface, ref.cInterface);
        strcpy(this->cIpAddress, ref.cIpAddress);
        this->eContactType = ref.eContactType;
        this->id = ref.id;
        this->iPort = ref.iPort;

        return *this;
    }


    SIPX_CONTACT_ID   id;              /**< Contact record Id */
    SIPX_CONTACT_TYPE eContactType ;   /**< Address type/source */
    char              cInterface[32] ; /**< Source interface    */
    char              cIpAddress[32] ; /**< IP Address          */
    int               iPort ;          /**< Port                */
};


/**
 * The SIPX_AUDIO_CODEC structure includes codec name and bandwidth info.
 */
typedef struct
{
#define SIPXTAPI_CODEC_NAMELEN 32       /**< Maximum length for codec name */
    char              cName[SIPXTAPI_CODEC_NAMELEN];  /**< Codec name    */
    SIPX_AUDIO_BANDWIDTH_ID iBandWidth; /**< Bandwidth requirement */
    int               iPayloadType;     /**< Payload type          */
} SIPX_AUDIO_CODEC ;


/**
 * The SIPX_VIDEO_CODEC structure includes codec name and bandwidth info.
 */
typedef struct
{
#define SIPXTAPI_CODEC_NAMELEN 32        /**< Maximum length for codec name */
    char              cName[SIPXTAPI_CODEC_NAMELEN];  /**< Codec name    */
    SIPX_VIDEO_BANDWIDTH_ID iBandWidth;  /**< Bandwidth requirement */
    int               iPayloadType;      /**< Payload type          */
} SIPX_VIDEO_CODEC ;


/**
* In a CALLSTATE_AUDIO_EVENT the SIPX_CODEC_INFO structure is being passed up
* to the event handler and contains information about the negotiated audio
* and video codec.
*/
typedef struct
{
    SIPX_AUDIO_CODEC audioCodec;     /**< Audio codec */
    SIPX_VIDEO_CODEC videoCodec;     /**< Video codec */
} SIPX_CODEC_INFO;


/**
 * The SIPX_INST handle represents an instance of a user agent.  A user agent
 * includes a SIP stack and media processing framework.  sipXtapi does support
 * multiple instances of user agents in the same process space, however,
 * certain media processing features become limited or ambiguous.  For
 * example, only one user agent should control the local system's input and
 * output audio devices. */
typedef void* SIPX_INST ;
const SIPX_INST SIPX_INST_NULL = 0; /**< Represents a null instance handle */

/**
 * The SIPX_LINE handle represents an inbound or outbound identity.  When
 * placing outbound the application programmer must define the outbound
 * line.  When receiving inbound calls, the application can query the
 * line.
 */
typedef unsigned int SIPX_LINE ;
const SIPX_LINE SIPX_LINE_NULL = 0; /**< Represents a null line handle */

/**
 * The SIPX_CALL handle represents a call or connection between the user
 * agent and another party.  All call operations require the call handle
 * as a parameter.
 */
typedef unsigned int SIPX_CALL ;
const SIPX_CALL SIPX_CALL_NULL = 0; /**< Represents a null call handle */

/**
 * The SIPX_CONF handle represents a collection of CALLs that have bridge
 * (mixed) audio.  Application developers can manipulate each leg of the
 * conference through various conference functions.
 */
typedef unsigned int SIPX_CONF ;
const SIPX_CONF SIPX_CONF_NULL = 0; /**< Represents a null conference handle */

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
const SIPX_PUB SIPX_PUB_NULL = 0; /**< Represents a null publisher handle */

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
 *        port cannot be changed after initialization.  Right now,
 *        the UDP port and TCP port numbers MUST be equal.  Pass a value of
 *        SIPX_PORT_DISABLE (-1) to disable disable UDP or a value of
 *        SIPX_PORT_AUTO (-2) to automatically select an open UDP port.
 * @param tcpPort The default TCP port for the SIP protocol stack.  The
 *        port cannot be changed after initialization.    Right now,
 *        the UDP port and TCP port numbers MUST be equal.  Pass a value of
 *        SIPX_PORT_DISABLE (-1) to disable disable TCP or a value of
 *        SIPX_PORT_AUTO (-2) to automatically select an open TCP port.
 * @param tlsPort **NOT YET SUPPORTED**
 * @param rtpPortStart The starting port for inbound RTP traffic.  The
 *        sipX layer will use ports starting at rtpPortStart and ending
 *        at (rtpPortStart + 2 * maxConnections) - 1.  Pass a value of
 *        SIPX_PORT_AUTO (-2) to automatically select an open port.
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
 *
 * @return SIPX_RESULT_OUT_OF_RESOURCES if unable to bind to the
 *         requested port configuration, otherwise SIPX_RESULT_SUCCESS.
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
 * exiting the process.  Users are responsible for ending all calls and
 * unregistering line appearances before calling sipxUnInitialize.  Failing
 * to end calls/conferences or remove lines will result in a
 * SIPX_RESULT_BUSY return code.
 *
 * @param hInst An instance handle obtained from sipxInitialize.
 */
SIPXTAPI_API SIPX_RESULT sipxUnInitialize(SIPX_INST hInst);

SIPXTAPI_API SIPX_RESULT sipxCallEnableSecurity(const SIPX_INST hInst,
                                                  const SIPX_CALL hCall,
                                                  const bool bEnable,
                                                  const SIPX_SECURITY_ATTRIBUTES attrib);


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
 * @param pDislay Pointer to an object describing the display object for
 *        rendering remote video.
 */
SIPXTAPI_API SIPX_RESULT sipxCallAccept(const SIPX_CALL hCall, SIPX_VIDEO_DISPLAY* const pDisplay = NULL);

/** VIDEO: Insert SIPX_WINDOW_HANDLE here */


/**
 * Reject an inbound call (prior to alerting the user).  This method must
 * be invoked before the end user is alerted (before sipXcallAccept).
 * Whenever a new call is received, the application developer should ACCEPT
 * (proceed to ringing), REJECT (send back busy), or REDIRECT the call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param errorCode RFC specified error code.
 * @param szErrorText null terminated text string to explain the error code.
 */
SIPXTAPI_API SIPX_RESULT sipxCallReject(const SIPX_CALL hCall,
                                        const int errorCode = 400,
                                        const char* szErrorText = "Bad Request") ;


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
 * Connects an idle call to the designated target address
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szAddress SIP url of the target party
 * @param contactId Id of the desired contact record to use for this call.
 *        The id refers to a Contact Record obtained by a call to
 *        sipxConfigGetLocalContacts.  The application can choose a
 *        contact record of type LOCAL, NAT_MAPPED, CONFIG, or RELAY.
 *        The Contact Type allows you to control whether the
 *        user agent and  media processing advertises the local address
 *         (e.g. LOCAL contact of 10.1.1.x or
 *        192.168.x.x), the NAT-derived address to the target party,
 *        or, local contact addresses of other types.
 * @param pDislay Pointer to an object describing the display object for
 *        rendering remote video.
 * @param Call-Id to use for this connection.  If not specified, one is generated.
 * @param szFrom SIP url of the from party to use for this connection.  If not
 *        specified, the one from the line is used
 */
SIPXTAPI_API SIPX_RESULT sipxCallConnect(const SIPX_CALL hCall,
                                         const char* szAddress,
                                         SIPX_CONTACT_ID contactId = 0,
                                         SIPX_VIDEO_DISPLAY* const pDisplay = NULL,
                                         const char* szCallId = NULL,
                                         const char* szFrom = NULL,
                                         const bool sendPAIheader = 0) ;

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
 * Get the SIP request uri.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szUri Buffer to store the request uri.  A zero-terminated string will be
 *        copied into this buffer on success.
 * @param iMaxLength Max length of the request uri buffer.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetRequestURI(const SIPX_CALL hCall,
                                               char* szUri,
                                               const size_t iMaxLength) ;



/**
 * Get the SIP remote contact.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szContact Buffer to store the remote contact.  A zero-terminated string will be
 *        copied into this buffer on success.
 * @param iMaxLength Max length of the remote contact buffer.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetRemoteContact(const SIPX_CALL hCall,
                                                  char* szContact,
                                                  const size_t iMaxLength) ;


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
 * 8000 samples/sec, mono, little endian or a .WAV file.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio files can only be played in the
 *        context of a call.
 * @param szFile Filename for the audio file to be played.
 * @param bLocal True if the audio file is to be rendered locally.
 * @param bRemote True if the audio file is to be rendered by the remote
 *                endpoint.
 * For backward compatibility.
 * @deprecated Use sipxCallPlayFileStart instead
 */
SIPXTAPI_API SIPX_RESULT sipxCallPlayFile(const SIPX_CALL hCall,
                                          const char* szFile,
                                          const bool bLocal,
                                          const bool bRemote) ;

/** VIDEO: sipxCallPlay APIs MUST be renamed to sipxCallAudioPlay */

/**
 * Play the designed file.  The file may be a raw 16 bit signed PCM at
 * 8000 samples/sec, mono, little endian or a .WAV file.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio files can only be played in the
 *        context of a call.
 * @param szFile Filename for the audio file to be played.
 * @param bRepeat True if the file is supposed to be played repeatedly
 * @param bLocal True if the audio file is to be rendered locally.
 * @param bRemote True if the audio file is to be rendered by the remote
 *                endpoint.
 */
SIPXTAPI_API SIPX_RESULT sipxCallPlayFileStart(const SIPX_CALL hCall,
                                               const char* szFile,
                                               const bool bRepeat,
                                               const bool bLocal,
                                               const bool bRemote) ;

/**
 * Stop playing a file started with sipxCallPlayFileStart
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio files can only be played and stopped
 *        in the context of a call.
 */
SIPXTAPI_API SIPX_RESULT sipxCallPlayFileStop(const SIPX_CALL hCall) ;


/**
 * Play the specified audio data.  Currently the only data format that
 * is supported is raw 16 bit signed PCM at 8000 samples/sec, mono,
 * little endian.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio can only be played in the context
 *        of a call.
 * @param szBuffer Pointer to the audio data to be played.
 * @param bufSize Length, in bytes, of the audio data.
 * @param bufType The audio encoding format for the data as specified
 *                by the SIPX_AUDIO_DATA_FORMAT enumerations.  Currently
 *                only RAW_PCM_16 is supported.
 * @param bRepeat True if the audio is supposed to be played repeatedly
 * @param bLocal True if the audio is to be rendered locally.
 * @param bRemote True if the audio is to be rendered by the remote endpoint.
 */
SIPXTAPI_API SIPX_RESULT sipxCallPlayBufferStart(const SIPX_CALL hCall,
                                                 const char* szBuffer,
                                                 const int  bufSize,
                                                 const int  bufType,
                                                 const bool bRepeat,
                                                 const bool bLocal,
                                                 const bool bRemote) ;


/**
 * Stop playing the audio started with sipxCallPlayBufferStart
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio can only be played and stopped
 *        in the context of a call.
 */
SIPXTAPI_API SIPX_RESULT sipxCallPlayBufferStop(const SIPX_CALL hCall) ;


/**
 * Subscribe for NOTIFY events which may be published by the other end-point of the Call.
 *
 * @param hCall The call handle of the call associated with the subscription.
 * @param szEventType A string representing the type of event that can be published.
 * @param szAcceptType A string representing the types of NOTIFY events that this client will accept.
 * @param phSub Pointer to a subscription handle whose value is set by this funtion.
 * @param bRemoteContactIsGruu indicates whether the Contact for the remote
 *        side of the call can be assumed to be a Globally Routable Unique URI
 *        (GRUU).  Normally one cannot assume that a contact is a GRUU and the
 *        To or From address for the remote side is assumed to be an Address Of
 *        Record (AOR) that is globally routable.
 */
SIPXTAPI_API SIPX_RESULT sipxCallSubscribe(const SIPX_CALL hCall,
                                           const char* szEventType,
                                           const char* szAcceptType,
                                           SIPX_SUB* phSub,
                                           bool bRemoteContactIsGruu = false);

/**
 * Unsubscribe from previously subscribed NOTIFY events.
 *
 * @param hSub The subscription handle obtained from the call to sipXCallSubscribe.
 */
SIPXTAPI_API SIPX_RESULT sipxCallUnsubscribe(const SIPX_SUB hSub) ;


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
 * Blind transfer the specified call to another party.  Monitor the
 * TRANSFER state events for details on the transfer attempt.  If the
 * call is not already on hold, the party will be placed on hold.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szAddress SIP url identifing the transfer target (who the call
 *        identified by hCall will be transfered to).
 *
 * @see SIPX_CALLSTATE_EVENT
 * @see SIPX_CALLSTATE_CAUSE
 */
SIPXTAPI_API SIPX_RESULT sipxCallBlindTransfer(const SIPX_CALL hCall,
                                               const char* szAddress) ;

/**
 * Transfer the source call to the target call.  This method can be used
 * to implement consultative transfer (transfer initiator can speak with
 * the transfer target prior to transferring.  If you wish to consult
 * privately, create a new call to the transfer target.  If you wish
 * consult and allow the source (transferee) to participant in the
 * converstation, create a conference and then transfer one leg to
 * another.
 *
 * If not already on hold, parties are placed on hold as part of the
 * transfer operation.
 *
 * @param hSourceCall Handle to the source call (transferee).
 * @param hTargetCall Handle to the target call (transfer target).
 *
 * @see SIPX_CALLSTATE_EVENT
 * @see SIPX_CALLSTATE_CAUSE
 */
SIPXTAPI_API SIPX_RESULT sipxCallTransfer(const SIPX_CALL hSourceCall,
                                          const SIPX_CALL hTargetCall) ;

//@}

/** @name Publishing Methods */
//@{


/**
 * Creates a publishing context, which perfoms the processing necessary
 * to accept SUBSCRIBE requests, and to publish NOTIFY messages to subscribers.
 * The resource may be specific to a single call, conference or global
 * to this user agent.  The naming of the resource ID determines the scope.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param phPub Pointer to a publisher handle - this method modifies the value
 *              to refer to the newly created publishing context.
 * @param szResourceId The resourceId to the state information being
 *        published.  This must match the request URI of the incoming
 *        SUBSCRIBE request (only the user ID, host and port are significant
 *        in matching the request URI).  Examples: fred\@10.0.0.1:5555,
 *               sip:conference1\@192.160.0.1, sip:kate\@example.com
 * @param szEventType A string representing the type of event that can be
 *               published.
 * @param szContentType String representation of the content type being
 *        published.
 * @param pContent Pointer to the NOTIFY message's body content.
 * @param nContentLength Size of the content to be published.
 *
 * @return If the resource already has a a publisher created for the given
 *               event type, SIPX_RESULT_INVALID_ARGS is returned.
 */
SIPXTAPI_API SIPX_RESULT sipxPublisherCreate(const SIPX_INST hInst,
                                             SIPX_PUB* phPub,
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
 * @param szContentType String representation of the content type being
 *        published
 * @param pFinalContent Pointer to the NOTIFY message's body content
 * @param nContentLength Size of the content to be published
 */
SIPXTAPI_API SIPX_RESULT sipxPublisherDestroy(const SIPX_PUB hPub,
                                              const char* szContentType,
                                              const char* pFinalContent,
                                              const size_t nContentLength);

/**
 * Publishes an updated state to specific event via NOTIFY to its subscribers.
 *
 * @param hPub Handle of the publishing context
 *              (returned from a call to sipxCreatePublisher)
 * @param szContentType String representation of the content type being
 *        published
 * @param pContent Pointer to the NOTIFY message's body content
 * @param nContentLength Size of the content to be published
 */
SIPXTAPI_API SIPX_RESULT sipxPublisherUpdate(const SIPX_PUB hPub,
                                             const char* szContentType,
                                             const char* pContent,
                                             const size_t nContentLength);

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
                                              SIPX_CONF* phConference) ;

/**
 * Join (add) an existing held call into a conference.
 *
 * An existing call can be added to a virgin conference without restriction.
 * Additional calls, must be connected and on remote hold for this operation
 * to succeed.   A remote hold can be accomplished by calling sipxCallHold on
 * the joining party.  The application layer must wait for the
 * CALLSTATE_CONNECTION_INACTIVE event prior to calling join. No events
 * are fired as part of the operation and the newly joined call is left on
 * hold.  The application layer should call sipxCallUnhold on the new
 * participant to finalize the join.
 *
 * @param hConf Conference handle obtained by calling sipxConferenceCreate.
 * @param hCall Call handle of the call to join into the conference.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceJoin(const SIPX_CONF hConf,
                                            const SIPX_CALL hCall,
                                            bool mFlagTransfer = false) ;

/**
 * Split (remove) a held call from a conference.  This method will remove
 * the specified call from the conference.
 *
 * The call must be connected and on remote hold for this operation to
 * succeed.   A remote hold can be accomplished by calling sipxCallHold on
 * the conference participant or by placing the entire conference on hold
 * with bridging disabled.  The application layer must wait for the
 * CALLSTATE_CONNECTION_INACTIVE event prior to calling split. No events
 * are fired as part of the operation and the split call is left on hold.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained
 *        by invoking sipxConferenceCreate.
 * @param hCall Call handle of the call that should be removed from the
 *        the conference.
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
 * @param contactId Id of the desired contact record to use for this call.
 *        The id refers to a Contact Record obtained by a call to
 *        sipxConfigGetLocalContacts.  The application can choose a
 *        contact record of type LOCAL, NAT_MAPPED, CONFIG, or RELAY.
 *        The Contact Type allows you to control whether the
 *        user agent and  media processing advertises the local address
 *         (e.g. LOCAL contact of 10.1.1.x or
 *        192.168.x.x), the NAT-derived address to the target party,
 *        or, local contact addresses of other types.
 * @param pDislay Pointer to an object describing the display object for
 *        rendering remote video.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceAdd(const SIPX_CONF hConf,
                                           const SIPX_LINE hLine,
                                           const char* szAddress,
                                           SIPX_CALL* phNewCall,
                                           SIPX_CONTACT_ID contactId = 0,
                                           SIPX_VIDEO_DISPLAY* const pDisplay = NULL);

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
 * Puts conference members on hold.  Either a bridging conference hold
 * (conference members can talk to each other while on hold), or
 * a non-bridging conference hold (conference members cannot talk to
 * anyone while on hold).
 *
 * @param hConf Handle to a conference.  Conference handles are obtained
 *        by invoking sipxConferenceCreate.
 * @param bBridging true for a bridging conference hold,
 *        false for a non-bridging conference hold.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceHold(const SIPX_CONF hConf,
                                            bool bBridging = true);

/**
 * Removes conference members from a held state.
 *
 * @deprecated Still under development
 *
 * @param hConf Handle to a conference.  Conference handles are obtained
 *        by invoking sipxConferenceCreate.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceUnhold(const SIPX_CONF hConf);

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
 * @param bMuted True if the microphone has been muted and false if it
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
 * @param enabled True if AEC is enabled and false if AEC is disabled.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioIsAECEnabled(const SIPX_INST hInst,
                                               bool& enabled) ;

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


/** VIDEO: Add Video config stuff here */

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
 * @param szLineURL The address of record for the line identity.  Can be
 *        prepended with a Display Name.
 *        e.g. -    "Zaphod Beeblebrox" <sip:zaphb@fourty-two.net>
 * @param phLine Pointer to a line handle.  Upon success, a handle to the
 *        newly added line is returned.
 * @param contactId Id of the desired contact record to use for this line.
 *        The id refers to a Contact Record obtained by a call to
 *        sipxConfigGetLocalContacts.  The application can choose a
 *        contact record of type LOCAL, NAT_MAPPED, CONFIG, or RELAY.
 *        The Contact Type allows you to control whether the
 *        user agent and  media processing advertises the local address
 *         (e.g. LOCAL contact of 10.1.1.x or
 *        192.168.x.x), the NAT-derived address to the target party,
 *        or, local contact addresses of other types.
 */
SIPXTAPI_API SIPX_RESULT sipxLineAdd(const SIPX_INST hInst,
                                     const char* szLineURL,
                                     SIPX_LINE* phLine,
                                     SIPX_CONTACT_ID contactId = 0) ;

/**
 * Adds an alias for a line definition.  Line aliases are used to map an
 * inbound call request to an existing line definition.  You should only
 * need to an a aliase if your network infrastructure directs calls to this
 * user agent using multiple identities.  For example, if user agent
 * registers as "sip:bandreasen@example.com"; however, calls can also be
 * directed to you via an exention (e.g. sip:122@example.com).
 *
 * If sipXtapi receives a call with an unknown line, you can still answer
 * and interact wtih the call; however, the line handle will be SIPX_LINE_NULL
 * in all event callbacks.  Adding an aliases allows you to correlate another
 * line url with your line definition and receive real line handles with event
 * callbacks.
 *
 * Line aliases are not used for outbound calls.
 */
SIPXTAPI_API SIPX_RESULT sipxLineAddAlias(const SIPX_LINE hLine, const char* szLineURL) ;

/**
 *  Registers a line with the proxy server.  Registrations will be re-registered
 *  automatically, before they expire.  A sipxLineRegister operation should not be
 *  started until a events have confirmed that any previous sipxLineRegister operation
 *  has completed.
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
 * Adds authentication credentials to the designated line appearance.
 * Credentials are often required by registration services to verify that the
 * line is being used by the line appearance/address of record owner.
 *
 * @param hLine Handle to a line appearance.  Line handles are obtained by
 *        creating a line using the sipxLineAdd function or by receiving
 *        a line event notification.
 * @param szUserID user id used for the line appearance.
 * @param szAuthHash the hex-string encoded value of md5(user:realm:password)
 * @param szRealm realm for which the user and passwd are valid.
 */
SIPXTAPI_API SIPX_RESULT sipxLineAddDigestCredential(const SIPX_LINE hLine,
                                                     const char* szUserID,
                                                     const char* szAuthHash,
                                                     const char* szRealm);


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

/**
 * Look up a line handle given a URI
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szURI URI to map to a line definition
 * @param hLine the line handle matching the URI or SIPX_LINE_NULL if not
 *        found
 */
SIPXTAPI_API SIPX_RESULT sipxLookupLine(const SIPX_INST hInst,
                                        const char* szURI,
                                        SIPX_LINE& hLine);

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
 * @param szFilename The filename for the log file.  Designated a NULL
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
 * Sets the User-Agent name to be used with outgoing SIP messages.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szName The user-agent name.
 * @param bIncludePlatformName Indicates whether or not to append the
 *        platform description onto the user agent name.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetUserAgentName(const SIPX_INST hInst,
                                                    const char* szName,
                                                    const bool bIncludePlatformName = true);

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
 * @param stunOptions This setting is used to modify the bahavior the STUN
 *        client when using STUN to discover its public IP address.  See
 *        the SIPX_STUN defines for details.  Multiple options can be
 *        combined using "|".  For example:
 *        SIPX_STUN_CHANGE_PORT | SIPX_STUN_CHANGE_ADDRESS
 *
 * @see SIPX_STUN_NORMAL
 * @see SIPX_STUN_CHANGE_PORT
 * @see SIPX_STUN_CHANGE_ADDRESS
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableStun(const SIPX_INST hInst,
                                              const char* szServer,
                                              int iKeepAliveSecs,
                                              int stunOptions = SIPX_STUN_NORMAL) ;

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
 * Enables/disables sending of DNS SRV request for all sipXtapi instances.
 *
 * @param enable Enable or disable DNS SRV resolution.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableDnsSrv(const bool enable);

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
                                                            SIPX_AUDIO_BANDWIDTH_ID bandWidth) ;

/**
 * Set the codec by name. The name must match one of the supported codecs
 * otherwise this functon will fail.
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec.
 * SIPX_RESULT_FAILURE is returned if the codec is not set.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szCodecName codec name
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetAudioCodecByName(const SIPX_INST hInst,
                                                       const char* szCodecName) ;

/**
 * Get the current codec preference.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pBandWidth pointer to an integer that will contain AUDIO_CODEC_BW_LOW,
 *        AUDIO_CODEC_BW_NORMAL, or AUDIO_CODEC_BW_HIGH. AUDIO_CODEC_BW_CUSTOM
 *        will be returned if a specific codec was et using the
 *        sipxConfigSetAudioCodecByName function.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetAudioCodecPreferences(const SIPX_INST hInst,
                                                            SIPX_AUDIO_BANDWIDTH_ID *pBandWidth);

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

/**
 * Set the preferred bandwidth requirement for codec selection. Whenever
 * possible a codec matching that requirement will be selected for a call.
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec
 * preferences.  SIPX_RESULT_FAILURE is returned if the preference is not set.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bandWidth Valid bandwidth requirements  are VIDEO_CODEC_BW_LOW,
 *        VIDEO_CODEC_BW_NORMAL, and VIDEO_CODEC_BW_HIGH.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoCodecPreferences(const SIPX_INST hInst,
                                                            SIPX_VIDEO_BANDWIDTH_ID bandWidth) ;

/**
 * Set the codec by name. The name must match one of the supported codecs
 * otherwise this functon will fail.
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec.
 * SIPX_RESULT_FAILURE is returned if the codec is not set.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szCodecName codec name
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoCodecByName(const SIPX_INST hInst,
                                                       const char* szCodecName) ;

/**
 * Get the current codec preference.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pBandWidth pointer to an integer that will contain AUDIO_CODEC_BW_LOW,
 *        AUDIO_CODEC_BW_NORMAL, or AUDIO_CODEC_BW_HIGH. AUDIO_CODEC_BW_CUSTOM
 *        will be returned if a specific codec was et using the
 *        sipxConfigSetAudioCodecByName function.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetVideoCodecPreferences(const SIPX_INST hInst,
                                                            SIPX_VIDEO_BANDWIDTH_ID *pBandWidth);

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
SIPXTAPI_API SIPX_RESULT sipxConfigGetNumVideoCodecs(const SIPX_INST hInst,
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
SIPXTAPI_API SIPX_RESULT sipxConfigGetVideoCodec(const SIPX_INST hInst,
                                                 const int index,
                                                 SIPX_VIDEO_CODEC* pCodec) ;

/**
 * Set the codec by name. The name must match one of the supported codecs
 * otherwise this functon will fail.
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec.
 * SIPX_RESULT_FAILURE is returned if the codec is not set.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szCodecName codec name
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoCodecByName(const SIPX_INST hInst,
                                                       const char* szCodecName) ;


/**
 * Get the local contact address available for outbound/inbound signaling and
 * audio.  The local contact addresses will always include the local IP
 * addresses.  The local contact addresses may also include external NAT-
 * derived addresses (e.g. STUN).  See the definition of SIPX_CONTACT_ADDRESS
 * for more details.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param addresses A pre-allocated list of SIPX_CONTACT_ADDRESS
 *        structures.  This data will be filled in by the API call.
 * @param nMaxAddresses The maximum number of addresses supplied by the
 *        addresses parameter.
 * @param nActualAddresses The actual number of addresses populated in
 *        the addresses parameter.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalContacts(const SIPX_INST hInst,
                                                    SIPX_CONTACT_ADDRESS addresses[],
                                                    size_t nMaxAddresses,
                                                    size_t& nActualAddresses) ;


/**
 * Populates an array of IP Addresses in UtlString* form.  The array must be preallocated to
 * contain MAX_IP_ADDRESSES elements.
 *
 * @param arrAddresses Pre-allocated array to be popluated with ip addresses.
 * @param arrAddressAdapter For each record in arrAddresses, there is a corresponding record,
 *        with the same index, in arrAddressAdpater which represents the
 *        "sipx adapter name" for that address
 * @param numAddresses Reference to the number of IPs found by the system.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetAllLocalNetworkIps(const char* arrAddresses[],
                                                         const char* arrAddressAdapter[],
                                                         int &numAddresses);
#ifdef VIDEO

/**
 * Sets the display object for the "video preview".
 *
 * @param pDisplay Pointer to a video preview display object.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoPreviewDisplay(const SIPX_INST hInst, SIPX_VIDEO_DISPLAY* const pDisplay);

#ifdef VIDEO
/**
 * Updates the Preview window with a new frame buffer.  Should be called
 * when the window receives a PAINT message.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param hWnd Window handle of the video preview window.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigUpdatePreviewWindow(const SIPX_INST hInst, const SIPX_WINDOW_HANDLE hWnd);
#endif


/**
 * Updates the Video window with a new frame buffer.  Should be called
 * when the window receives a PAINT message.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param hWnd Window handle of the video preview window.
 */
SIPXTAPI_API SIPX_RESULT sipxCallUpdateVideoWindow(const SIPX_CALL hCall, const SIPX_WINDOW_HANDLE hWnd);

/**
 * Sets the video quality.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param quality Id setting the video quality.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoQuality(const SIPX_INST hInst, const SIPX_VIDEO_QUALITY_ID quality);

/**
 * Sets the bit rate and frame rate parameters for video.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bitRate Bit rate parameter
 * @param frameRate Frame rate parameter
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoParameters(const SIPX_INST hInst,
                                                      const int bitRate,
                                                      const int frameRate);


#endif

//@}

#endif // _sipXtapi_h_

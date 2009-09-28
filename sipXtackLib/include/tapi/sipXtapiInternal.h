//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SIPXTAPIINTERNAL_H /* [ */
#define _SIPXTAPIINTERNAL_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "tapi/sipXtapi.h"
#include "tapi/SipXMessageObserver.h"
#include "net/SipSession.h"
#include "net/SipUserAgent.h"
#include "net/SipSubscribeClient.h"
#include "utl/UtlDefs.h"
#include "utl/UtlContainable.h"
#include "os/OsRWMutex.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "os/OsSysLog.h"
#include "os/OsMutex.h"

// DEFINES
/** sipXtapi can be configured to expire after a certain date */
// #define SIPXTAPI_EVAL_EXPIRATION

#ifdef SIPXTAPI_EVAL_EXPIRATION
#  define EVAL_EXPIRE_MONTH     9
#  define EVAL_EXPIRE_DAY       1
#  define EVAL_EXPIRE_YEAR      2005
#endif

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class SipSubscribeServer;
class SipSubscribeClient;

// STRUCTS

// TYPEDEFS
typedef struct
{
    bool bInitialized ;     /**< Is the data valid */
    bool bMuted ;           /**< Muted state (regain gain) */
    int  iGain ;            /**< Gain setting (GAIN_MIN-GAIN_MAX) */
    UtlString device;       /**< Desired auto device */
} MIC_SETTING ;

typedef struct
{
    bool bInitialized ;     /**< Is the data valid */
    int  iVol ;             /**< Gain setting (VOLUME_MIN-VOLUME_MAX) */
    UtlString device;       /**< Desired auto device */
} SPEAKER_SETTING ;

typedef struct
{
    bool bInitialized ;     /**< Is the data valid */
    bool bEnabled ;         /**< Is AEC enabled? */
} AEC_SETTING ;

typedef struct
{
    bool              bInitialized;    /**< Is the data valid */
    int               numCodecs;       /**< Number of codecs */
    SIPX_AUDIO_BANDWIDTH_ID codecPref; /**< Numeric Id of codec preference */
    SIPX_AUDIO_BANDWIDTH_ID fallBack;  /**< Fallback id if codec setting fails */
    UtlString         sPreferences;    /**< List of preferred codecs */
    SdpCodec**        sdpCodecArray;   /**< Pointer to an array of codecs */
} AUDIO_CODEC_PREFERENCES;

typedef struct
{
    bool              bInitialized;    /**< Is the data valid */
    int               numCodecs;       /**< Number of codecs */
    SIPX_VIDEO_BANDWIDTH_ID codecPref; /**< Numeric Id of codec preference */
    SIPX_VIDEO_BANDWIDTH_ID fallBack;  /**< Fallback id if codec setting fails */
    UtlString         sPreferences;    /**< List of preferred codecs */
    SdpCodec**        sdpCodecArray;   /**< Pointer to an array of codecs */
} VIDEO_CODEC_PREFERENCES;

typedef struct
{
    bool        bInitialized;    /**< Is the data valid */
    bool        bEnabled;        /**< Is SRTP enabled */
    int         iCipherType;     /**< Cipher type */
    int         iCipherKeyLen;   /**< Cipher key length */
    int         iAuthType;       /**< Authentication type */
    int         iAuthKeyLen;     /**< Authentication key length */
    int         iAuthTagLen;     /**< Tag length */
    int         iSecurity;       /**< Protection level */
    UtlString   sKey;            /**< Key */
} SRTP_SETTING;

typedef struct
{
    bool             bInitialized;
    bool             tonePlaying;
} TONE_STATES;

typedef struct
{
    SipUserAgent*    pSipUserAgent ;
    SdpCodecFactory* pCodecFactory ;
    CallManager*     pCallManager ;
    SipLineMgr*      pLineManager ;
    SipRefreshMgr*   pRefreshManager ;
    SipSubscribeServer* pSubscribeServer;
    SipSubscribeClient* pSubscribeClient;

    MIC_SETTING      micSetting ;
    SPEAKER_SETTING  speakerSettings[2] ;
    AEC_SETTING      aecSetting ;
    SPEAKER_TYPE     enabledSpeaker ;
    AUDIO_CODEC_PREFERENCES
                     audioCodecSetting;
    VIDEO_CODEC_PREFERENCES
                     videoCodecSetting;
    TONE_STATES      toneStates;

    char*            inputAudioDevices[MAX_AUDIO_DEVICES] ;
    char*            outputAudioDevices[MAX_AUDIO_DEVICES] ;
    SipXMessageObserver* pMessageObserver;
    OsNotification   *pStunNotification ;   /**< Signals the initial stun success/failure
                                                 when calling sipXconfigEnableStun */

    OsMutex*         pLock ;
    int              nCalls ;       /**< Counter for inprocess calls */
    int              nConferences ; /**< Counter for inprocess conferences */
    int              nLines ;       /**< Counter for inprocess lines */
} SIPX_INSTANCE_DATA ;


typedef enum
{
    SIPX_INTERNAL_CALLSTATE_UNKNOWN,            /** Unknown call state */
    SIPX_INTERNAL_CALLSTATE_OUTBOUND_ATTEMPT,   /** Early dialog: outbound */
    SIPX_INTERNAL_CALLSTATE_INBOUND_ATEMPT,     /** Early dialog: inbound */
    SIPX_INTERNAL_CALLSTATE_CONNECTED,          /** Active call - remote audio */
    SIPX_INTERNAL_CALLSTATE_HELD,               /** Remotely held call */
    SIPX_INTERNAL_CALLSTATE_BRIDGED,            /** Locally held call */
    SIPX_INTERNAL_CALLSTATE_DISCONNECTED,       /** Disconnected or failed */
} SIPX_INTERNAL_CALLSTATE ;


typedef struct
{
    UtlString* callId;
    UtlString* sessionCallId;
    UtlString* ghostCallId;
    UtlString* remoteAddress ;
    UtlString* lineURI ;
    SIPX_LINE  hLine ;
    SIPX_INSTANCE_DATA* pInst ;
    OsRWMutex* pMutex ;
    SIPX_CONF hConf ;
    SIPX_VIDEO_DISPLAY display;
    UtlBoolean bRemoveInsteadOfDrop ;   /** Remove the call instead of dropping it
                                            -- this is used as part of consultative
                                            transfer when we are the transfer target
                                            and need to replace a call leg within
                                            the same CpPeerCall. */
    SIPX_CALLSTATE_EVENT lastCallstateEvent ;
    SIPX_CALLSTATE_CAUSE lastCallstateCause ;

    SIPX_INTERNAL_CALLSTATE state ;
    UtlBoolean bInFocus ;
    UtlString* transferCallId;
} SIPX_CALL_DATA ;

typedef enum CONF_HOLD_STATE
{
    CONF_STATE_UNHELD = 0,
    CONF_STATE_BRIDGING_HOLD,
    CONF_STATE_NON_BRIDGING_HOLD,
} CONF_HOLD_STATE;

typedef struct
{
    UtlString*          strCallId ;
    SIPX_INSTANCE_DATA* pInst ;
    size_t              nCalls ;
    SIPX_CALL           hCalls[CONF_MAX_CONNECTIONS] ;
    CONF_HOLD_STATE     confHoldState;
    OsRWMutex*          pMutex ;
} SIPX_CONF_DATA ;


typedef struct
{
    Url* lineURI ;
    SIPX_INSTANCE_DATA* pInst ;
    OsRWMutex*          pMutex ;
    SIPX_CONTACT_TYPE   contactType ;
    UtlSList*           pLineAliases ;
} SIPX_LINE_DATA ;

typedef struct
{
    SIPX_INFO_INFO infoData;
    SIPX_INSTANCE_DATA* pInst;
    SipSession* pSession;
    OsRWMutex*          pMutex;
} SIPX_INFO_DATA;

typedef struct
{
    SIPX_INSTANCE_DATA* pInst;
    UtlString* pResourceId;
    UtlString* pEventType;
} SIPX_PUBLISH_DATA;

typedef struct
{
    SIPX_INSTANCE_DATA* pInst;
    UtlString* pDialogHandle;
} SIPX_SUBSCRIPTION_DATA;

/**
 * internal sipXtapi structure that binds a
 * an event callback proc
 * with an instance pointer and user data
 */
typedef struct
{
    SIPX_EVENT_CALLBACK_PROC pCallbackProc;
    void* pUserData;
    SIPX_INSTANCE_DATA* pInst;
} EVENT_LISTENER_DATA;

/**
 * internal sipXtapi structure that binds a
 * an call callback proc
 * with an instance pointer and user data
 */
typedef struct
{
        CALLBACKPROC    pCallbackProc ;
        void*           pUserData ;
    SIPX_INSTANCE_DATA* pInst ;
} CALL_LISTENER_DATA ;

/**
 * internal sipXtapi structure that binds a
 * an line callback proc
 * with an instance pointer and user data
 */
typedef struct
{
    LINECALLBACKPROC    pCallbackProc;
    void*               pUserData;
    SIPX_INSTANCE_DATA* pInst;
} LINE_LISTENER_DATA;

typedef enum SIPX_LOCK_TYPE
{
    SIPX_LOCK_NONE,
    SIPX_LOCK_READ,
    SIPX_LOCK_WRITE
} SIPX_LOCK_TYPE ;

/* ============================ FUNCTION POINTER DEFINITIONS =============== */

typedef void (*sipxCallEventCallbackFn)(const void* pSrc,
                                        const char* szCallId,
                                        SipSession* pSession,
                                        const char* szRemoteAddress,
                                        SIPX_CALLSTATE_MAJOR eMajorState,
                                        SIPX_CALLSTATE_MINOR eMinorState,
                                        void* pEventData);

typedef void (*sipxLineEventCallbackFn)(const void* pSrc,
                                        const char* szLineIdentifier,
                                        SIPX_LINE_EVENT_TYPE_MAJOR major,
                                        SIPX_LINE_EVENT_TYPE_MINOR minor);

typedef void (*sipxEventCallbackFn)(const void* pSrc,
                                    SIPX_EVENT_CATEGORY category,
                                    void* pInfo);


/* ============================ FUNCTIONS ================================= */


/**
 * Fire events to interested listeners (call events only).
 */
void sipxFireCallEvent(const void* pSrc,
                       const char* szCallId,
                       SipSession* pSession,
                       const char* szRemoteAddress,
                       SIPX_CALLSTATE_MAJOR eMajorState,
                       SIPX_CALLSTATE_MINOR eMinorState,
                       void* pEventData=NULL) ;

/**
 * Fires a Line Event to the listeners.
 */
void sipxFireLineEvent(const void* pSrc,
                       const char* szLineIdentifier,
                       SIPX_LINE_EVENT_TYPE_MAJOR major,
                       SIPX_LINE_EVENT_TYPE_MINOR minor);

/**
 * Bubbles up all non-line and non-call events to the application layer
 */
void sipxFireEvent(const void* pSrc,
                   SIPX_EVENT_CATEGORY category,
                   void* pInfo);

SIPX_INSTANCE_DATA* findSessionByCallManager(const void* pCallManager) ;

SIPX_CALL_DATA* sipxCallLookup(const SIPX_CALL hCall, SIPX_LOCK_TYPE type);
void sipxCallReleaseLock(SIPX_CALL_DATA*, SIPX_LOCK_TYPE type);
void sipxCallObjectFree(const SIPX_CALL hCall);
SIPX_CALL sipxCallLookupHandle(const UtlString& callID, const void* pSrc);
void destroyCallData(SIPX_CALL_DATA* pData);
UtlBoolean validCallData(SIPX_CALL_DATA* pData);
UtlBoolean sipxCallGetCommonData(SIPX_CALL hCall,
                                 SIPX_INSTANCE_DATA** pInst,
                                 UtlString* pStrCallId,
                                 UtlString* pStrRemoteAddress,
                                 UtlString* pLineId,
                                 UtlString* pGhostCallId = NULL) ;

SIPX_CONF sipxCallGetConf(SIPX_CALL hCall) ;

UtlBoolean sipxCallGetState(SIPX_CALL hCall,
                            SIPX_CALLSTATE_EVENT& lastEvent,
                            SIPX_CALLSTATE_CAUSE& lastCause,
                            SIPX_INTERNAL_CALLSTATE& state) ;

UtlBoolean sipxCallSetState(SIPX_CALL hCall,
                            SIPX_CALLSTATE_EVENT event,
                            SIPX_CALLSTATE_CAUSE cause) ;

SIPX_CONTACT_TYPE sipxCallGetLineContactType(SIPX_CALL hCall) ;

SIPX_LINE_DATA* sipxLineLookup(const SIPX_LINE hLine, SIPX_LOCK_TYPE type);
void sipxLineReleaseLock(SIPX_LINE_DATA* pData, SIPX_LOCK_TYPE type) ;
void sipxLineObjectFree(const SIPX_LINE hLine) ;
SIPX_LINE sipxLineLookupHandle(SIPX_INSTANCE_DATA* pInst,
                               const char* szLineURI,
                               const char* requestUri);
SIPX_LINE sipxLineLookupHandleByURI(SIPX_INSTANCE_DATA* pInst,
                                    const char* szURI);
UtlBoolean validLineData(const SIPX_LINE_DATA*) ;

UtlBoolean sipxRemoveCallHandleFromConf(SIPX_CONF_DATA *pConfData,
                                        const SIPX_CALL hCall) ;

SIPX_CONF_DATA* sipxConfLookup(const SIPX_CONF hConf, SIPX_LOCK_TYPE type) ;
void sipxConfReleaseLock(SIPX_CONF_DATA* pData, SIPX_LOCK_TYPE type) ;
void sipxConfFree(const SIPX_CONF hConf) ;
UtlBoolean validConfData(const SIPX_CONF_DATA* pData) ;

void sipxIncSessionCount();
void sipxDecSessionCount();
int sipxGetSessionCount();

UtlBoolean sipxIsCallInFocus() ;

/**
 * Frees the INFO structure allocated by a call to sipxCallSendInfo
 *
 * @param pData Pointer to SIPX_INFO_DATA structure
 */
void sipxInfoFree(SIPX_INFO_DATA* pData);

/**
 * Releases the INFO handle created by a call to sipxCallSendInfo.
 * Also cals sipxInfoFree.
 *
 * @param hInfo Handle to the Info object
 */
void sipxInfoObjectFree(SIPX_INFO hInfo);

void sipxGetContactHostPort(SIPX_INSTANCE_DATA* pData,
                            SIPX_CONTACT_TYPE   contactType,
                            Url&                uri) ;
  //: Get the external host and port given the contact preference

/**
 * Looks up the SIPX_INFO_DATA structure pointer, given the SIPX_INFO handle.
 * @param hInfo Info Handle
 * @param type Lock type to use during lookup.
 */
SIPX_INFO_DATA* sipxInfoLookup(const SIPX_INFO hInfo, SIPX_LOCK_TYPE type);

/**
 * Unlocks the mutex associated with the INFO DATA
 *
 * @param pData pointer to the SIPX_INFO structure
 * @param type Type of lock (read or write)
 */
void sipxInfoReleaseLock(SIPX_INFO_DATA* pData, SIPX_LOCK_TYPE type);

/**
 * Adds a log entry to the system log - made necessary to add logging
 * capability on the API level.
 *
 * @param logLevel priority of the log entry
 * @param format a format string for the following variable argument list
 */
SIPXTAPI_API void sipxLogEntryAdd(OsSysLogPriority logLevel,
                     const char *format,
                     ...);

/**
 * Utility function for setting allowed methods on a
 * instance's user-agent.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigAllowMethod(const SIPX_INST hInst, const char* method, const bool bAllow = true);

/**
 * Get the list of active calls for the specified call manager instance
 */
SIPXTAPI_API SIPX_RESULT sipxGetActiveCallIds(SIPX_INST hInst, int maxCalls, int& actualCalls, UtlString callIds[]) ;

/**
 * Callback for subscription client state
 */
void sipxSubscribeClientSubCallback(enum SipSubscribeClient::SubscriptionState newState,
                                   const char* earlyDialogHandle,
                                   const char* dialogHandle,
                                   void* applicationData,
                                   int responseCode,
                                   const char* responseText,
                                   long expiration,
                                   const SipMessage* subscribeResponse);

/**
 * Callback for subscription client NOTIFY content
 */
bool sipxSubscribeClientNotifyCallback(const char* earlyDialogHandle,
                                     const char* dialogHandle,
                                     void* applicationData,
                                     const SipMessage* notifyRequest);

/**
 * Look for leaks in internal handles
 */
SIPXTAPI_API SIPX_RESULT sipxCheckForHandleLeaks() ;


/**
 * Translate tone ids to implementation specific codes
 *
 * @param toneId sipx-internal tone id
 * @param xlateId implementation-specific tone id
 */
SIPXTAPI_API SIPX_RESULT sipxTranslateToneId(const TONE_ID toneId,
                                             TONE_ID& xlateId) ;


/**
 * Gets an CpMediaInterface pointer, associated with the call connection.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param ppInstData pointer to a memory address that is set to the media interface
 *        pointer.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetConnectionMediaInterface(const SIPX_CALL hCall,
                                                           void** ppInstData);


#ifdef VOICE_ENGINE
    #include "GipsVoiceEngineLib.h"
    #include "GIPSAECTuningWizardAPI.h"
#ifdef VIDEO
#ifdef _WIN32
    #include <windows.h>
    #include "GipsVideoEngineWindows.h"
#endif
#endif
    /**
     * For Gips VoiceEngine versions of sipXtapi, this method will
     * return the GipsVoiceEngineLib pointer associated with the
     * call.
     *
     * @param hCall Handle to a call.  Call handles are obtained either by
     *        invoking sipxCallCreate or passed to your application through
     *        a listener interface.
     */
    SIPXTAPI_API GipsVoiceEngineLib* sipxCallGetVoiceEnginePtr(const SIPX_CALL hCall);


    /**
     * For Gips VoiceEngine versions of sipXtapi, this method will
     * return the GipsVoiceEngineLib pointer associated with the
     * factory implementation.
     *
     * @param hInst Instance pointer obtained by sipxInitialize
     */
    SIPXTAPI_API GipsVoiceEngineLib* sipxConfigGetVoiceEnginePtr(const SIPX_INST hInst);

    /**
     * For Gips VoiceEngine versions of sipXtapi, this method will
     * return a Audio Tuning Wizard pointer associated with the
     * factory implementation.
     *
     * @param hInst Instance pointer obtained by sipxInitialize
     */
    SIPXTAPI_API GIPSAECTuningWizard* sipxConfigGetVoiceEngineAudioWizard();

#ifdef VIDEO
    /**
     * For Gips VoiceEngine versions of sipXtapi, this method will
     * return the GipsVoiceEngineLib pointer associated with the
     * factory implementation.
     *
     * @param hInst Instance pointer obtained by sipxInitialize
     */
    SIPXTAPI_API GipsVideoEngineWindows* sipxConfigGetVideoEnginePtr(const SIPX_INST hInst);
#endif VIDEO

    /**
     * For Gips VoiceEngine versions of sipXtapi, this method will
     * enable or disable insertion of VoiceEngine trace output into the
     * sipXtapi log.
     *
     * @param hInst Instance pointer obtained by sipxInitialize
     * @param bEnable Enable or disable VoceEngine trace output
     */
    SIPXTAPI_API SIPX_RESULT sipxEnableAudioLogging(const SIPX_INST hInst, bool bEnable);
#endif

/**
 * Subscribes for Message Waiting Indicator NOTIFY messages from the Voicemail
 * server.
 *
 * NOTE: This API will change
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szSubscribeURL Voicemail subscription string - the voicemail server
 *        to subscribe to.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigVoicemailSubscribe(const SIPX_INST hInst,
                                                      const char* szSubscribeURL);


UtlBoolean sipxCallSetRemoveInsteadofDrop(SIPX_CALL hCall) ;
UtlBoolean sipxCallIsRemoveInsteadOfDropSet(SIPX_CALL hCall) ;
UtlBoolean sipxAddCallHandleToConf(const SIPX_CALL hCall,
                                   const SIPX_CONF hConf);
void sipxDumpCalls();


#endif /* ] _SIPXTAPIINTERNAL_H */

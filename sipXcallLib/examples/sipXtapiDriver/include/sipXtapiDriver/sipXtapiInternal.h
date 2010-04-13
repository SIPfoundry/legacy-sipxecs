//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
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
#include "utl/UtlDefs.h"
#include "utl/UtlContainable.h"
#include "os/OsRWMutex.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "os/OsSysLog.h"

// DEFINES
/**< sipXtapi can be configured to expire after a certain date */
#define SIPXTAPI_EVAL_EXPIRATION
#undef SIPXTAPI_EVAL_EXPIRATION

#ifdef SIPXTAPI_EVAL_EXPIRATION
#  define EVAL_EXPIRE_MONTH     6
#  define EVAL_EXPIRE_DAY       15
#  define EVAL_EXPIRE_YEAR      2005
#endif

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
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
    bool              bInitialized;  /**< Is the data valid */
    int               numCodecs;     /**< Number of codecs */
    SIPX_BANDWIDTH_ID codecPref;     /**< Numeric Id of codec preference */
    UtlString         sPreferences;  /**< List of preferred codecs */
    SdpCodec**        sdpCodecArray; /**< Pointer to an array of codecs */
} AUDIO_CODEC_PREFERENCES;

typedef struct
{
    SipUserAgent*    pSipUserAgent ;
    SdpCodecFactory* pCodecFactory ;
    CallManager*     pCallManager ;
    SipLineMgr*      pLineManager ;
    SipRefreshMgr*   pRefreshManager ;

    MIC_SETTING      micSetting ;
    SPEAKER_SETTING  speakerSettings[2] ;
    AEC_SETTING      aecSetting ;
    SPEAKER_TYPE     enabledSpeaker ;
    AUDIO_CODEC_PREFERENCES
                     audioCodecSetting;

    char*            inputAudioDevices[MAX_AUDIO_DEVICES] ;
    char*            outputAudioDevices[MAX_AUDIO_DEVICES] ;
    SipXMessageObserver* pMessageObserver;
} SIPX_INSTANCE_DATA ;

typedef struct
{
    UtlString* callId ;
    UtlString* ghostCallId;
    UtlString* remoteAddress ;
    UtlString* lineURI ;
    SIPX_LINE  hLine ;
    SIPX_INSTANCE_DATA* pInst ;
    OsRWMutex*          pMutex ;
} SIPX_CALL_DATA ;


typedef struct
{
    UtlString*          strCallId ;
    SIPX_INSTANCE_DATA* pInst ;
    size_t              nCalls ;
    SIPX_CALL           hCalls[CONF_MAX_CONNECTIONS] ;
    OsRWMutex*          pMutex ;
} SIPX_CONF_DATA ;


typedef struct
{
    Url* lineURI ;
    SIPX_INSTANCE_DATA* pInst ;
    OsRWMutex*          pMutex ;
    SIPX_CONTACT_TYPE   contactType ;
} SIPX_LINE_DATA ;

typedef struct
{
    SIPX_INFO_INFO infoData;
    SIPX_INSTANCE_DATA* pInst;
    SipSession* pSession;
    OsRWMutex*          pMutex;
} SIPX_INFO_DATA;

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
                                        SIPX_CALLSTATE_MINOR eMinorState);

typedef void (*sipxLineEventCallbackFn)(const void* pSrc,
                                        const char* szLineIdentifier,
                                        SIPX_LINE_EVENT_TYPE_MAJOR major);

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
                       SIPX_CALLSTATE_MINOR eMinorState) ;

/**
 * Fires a Line Event to the listeners.
 */
void sipxFireLineEvent(const void* pSrc,
                       const char* szLineIdentifier,
                       SIPX_LINE_EVENT_TYPE_MAJOR major);

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
SIPX_CONTACT_TYPE sipxCallGetLineContactType(SIPX_CALL hCall) ;

SIPX_LINE_DATA* sipxLineLookup(const SIPX_LINE hLine, SIPX_LOCK_TYPE type);
void sipxLineReleaseLock(SIPX_LINE_DATA* pData, SIPX_LOCK_TYPE type) ;
void sipxLineObjectFree(const SIPX_LINE hLine) ;
SIPX_LINE sipxLineLookupHandle(const char* szLineURI);
UtlBoolean validLineData(const SIPX_LINE_DATA*) ;

UtlBoolean sipxRemoveCallHandleFromConf(const SIPX_CONF hConf,
                                        const SIPX_CALL hCall) ;

SIPX_CONF_DATA* sipxConfLookup(const SIPX_CONF hConf, SIPX_LOCK_TYPE type) ;
void sipxConfReleaseLock(SIPX_CONF_DATA* pData, SIPX_LOCK_TYPE type) ;
void sipxConfFree(const SIPX_CONF hConf) ;
UtlBoolean validConfData(const SIPX_CONF_DATA* pData) ;

void sipxIncSessionCount();
void sipxDecSessionCount();
int sipxGetSessionCount();

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
 * Look for leaks in internal handles
 */
SIPXTAPI_API SIPX_RESULT sipxCheckForHandleLeaks() ;


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
    #include "VoiceEngine/interface/GipsVoiceEngineLib.h"
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


#endif /* ] _SIPXTAPIINTERNAL_H */

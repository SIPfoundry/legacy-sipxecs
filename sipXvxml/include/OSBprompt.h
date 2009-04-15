/****************License************************************************
 *
 * Copyright 2000-2001.  SpeechWorks International, Inc.  
 *
 * Use of this software is subject to notices and obligations set forth
 * in the SpeechWorks Public License - Software Version 1.1 which is
 * included with this software.
 *
 * SpeechWorks is a registered trademark, and SpeechWorks Here, 
 * DialogModules and the SpeechWorks logo are trademarks of SpeechWorks 
 * International, Inc. in the United States and other countries. 
 * 
 ***********************************************************************
 *
 * 
 * 
 ************************************************************************
 */

#ifndef _OSBPROMPT_H
#define _OSBPROMPT_H

#include "VXIprompt.h"                 /* For VXIprompt base interface */
#include "prompt/osbprompt_playerlistener.h"
#include "VXIlog.h"                    /* For VXIlog interface */
#include "VXIinet.h"                   /* For VXIinet interface */
#include "cp/CallManager.h"

#include "VXIheaderPrefix.h"
#ifdef OSBPROMPT_EXPORTS
#define OSBPROMPT_API SYMBOL_EXPORT_DECL
#else
#define OSBPROMPT_API SYMBOL_IMPORT_DECL
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name OSBprompt
 * @memo OSBprompt Interface
 * @doc
 * OSBprompt interface, a simulator implementation of the VXIprompt abstract
 * interface for Prompting functionality. Prompts are constructed as a
 * series of audio segments, where segments may be in-memory audio
 * samples, paths to on-disk audio files, URLs to remote audio, text
 * for playback via a Text-to-Speech engine, or text for playback
 * using concatenative audio routines (123 played as "1.ulaw 2.ulaw
 * 3.ulaw"). This implementation merely logs what a real VXIprompt
 * implementation should do to the diagnostic log. <p>
 *
 * There is one prompt interface per thread/line.
 */

  /*@{*/

/**
 * Global platform initialization of OSBprompt
 *
 * @param log            VXI Logging interface used for error/diagnostic 
 *                       logging, only used for the duration of this 
 *                       function call
 * @param diagLogBase    Base tag number for diagnostic logging purposes.
 *                       All diagnostic tags for OSBprompt will start at this
 *                       ID and increase upwards.
 *
 * @result VXIprompt_RESULT_SUCCESS on success
 */
OSBPROMPT_API VXIpromptResult OSBpromptInit (VXIlogInterface  *log,
					     VXIunsigned       diagLogBase,
               const VXIchar     *baseUrl);

/**
 * Global platform shutdown of OSBprompt
 *
 * @param log    VXI Logging interface used for error/diagnostic logging,
 *               only used for the duration of this function call
 *
 * @result VXIprompt_RESULT_SUCCESS on success
 */
OSBPROMPT_API VXIpromptResult OSBpromptShutDown (VXIlogInterface  *log);

/**
 * Preparing to shut down the session by setting live to 0
 *
 * @param log    VXI Logging interface used for error/diagnostic logging,
 *               only used for the duration of this function call
 *
 * @result VXIprompt_RESULT_SUCCESS on success
 */
OSBPROMPT_API VXIpromptResult OSBpromptExiting (VXIlogInterface  *log,
                                                 VXIpromptInterface **prompt);

/**
 * Create a new prompt service handle
 *
 * @param log    VXI Logging interface used for error/diagnostic logging,
 *               must remain a valid pointer throughout the lifetime of
 *               the resource (until OSBpromptDestroyResource( ) is called)
 * @param inet   VXI Internet interface used for URL fetches,
 *               must remain a valid pointer throughout the lifetime of
 *               the resource (until OSBpromptDestroyResource( ) is called)
 *
 * @result VXIprompt_RESULT_SUCCESS on success 
 */
OSBPROMPT_API 
VXIpromptResult OSBpromptCreateResource (VXIunsigned         channelNum,
					 VXIlogInterface     *log,
					 VXIinetInterface    *inet,
					 VXIpromptInterface **prompt,
					 CallManager *pCallMgr);

/**
 * Add call manager to the prompt service handle
 *
 * @param pCallMgr    call manager (until OSBpromptDestroyResource( ) is called)
 *
 * @result VXIprompt_RESULT_SUCCESS on success 
 */
OSBPROMPT_API 
VXIpromptResult OSBpromptAddResource (VXIpromptInterface **prompt,
					 CallManager *pCallMgr);

/**
 * Destroy the interface and free internal resources. Once this is
 *  called, the logging and Internet interfaces passed to
 *  OSBpromptCreateResource( ) may be released as well.
 *
 * @result VXIprompt_RESULT_SUCCESS on success 
 */
OSBPROMPT_API 
VXIpromptResult OSBpromptDestroyResource (VXIpromptInterface **prompt);


/* Structure for global player listeners list */
typedef struct  PlayerListenerDB {
        OSBPlayerListener*	pListener;
        VXIunsigned     	channelNum;
        char*     		callId;
} PlayerListenerDB;

#if defined(_WIN32)
PlayerListenerDB** glbPlayerListenerTable;
int glbMaxNumListeners;
int glbPlayerListenerCnt;
#else
extern PlayerListenerDB** glbPlayerListenerTable;
extern int glbMaxNumListeners;
extern int glbPlayerListenerCnt;
#endif

OSBPROMPT_API int VXIReleasePlayerListener(const char* callId, VXIunsigned channel);

OSBPROMPT_API OSBPlayerListener* VXIGetPlayerListener(const char* callId, VXIunsigned channel);

/*@}*/

#ifdef __cplusplus
}
#endif

#include "VXIheaderSuffix.h"

#endif  /* include guard */

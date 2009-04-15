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

#ifndef _OSBREC_H
#define _OSBREC_H

#include "VXIrec.h"                 /* For VXIrec base interface */
#include "VXIlog.h"                 /* For VXIlog interface */

#include "cp/CallManager.h"

#include "VXIheaderPrefix.h"
#include "VXIprompt.h"
#ifdef OSBREC_EXPORTS
#define OSBREC_API SYMBOL_EXPORT_DECL
#else
#define OSBREC_API SYMBOL_IMPORT_DECL
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name OSBrec
 * @memo OSBrec Interface
 * @doc
 * OSBrec provides a simulator implementation of the VXIrec abstract interface
 * for recognition functionality.   Recognition is done with all
 * blocking calls, because VoiceXML is essentially a blocking
 * protocol.  One VXIrec interface should be constructed per line.
 */

/*@{*/

/**
 * Global platform initialization of OSBrec
 *
 * @param log            VXI Logging interface used for error/diagnostic 
 *                       logging, only used for the duration of this 
 *                       function call
 * @param diagLogBase    Base tag number for diagnostic logging purposes.
 *                       All diagnostic tags for OSBprompt will start at this
 *                       ID and increase upwards.
 * 
 * @result VXIrec_RESULT_SUCCESS on success
 */
OSBREC_API VXIrecResult OSBrecInit(VXIlogInterface *log,
				   VXIunsigned      diagLogBase);

/**
 * Global platform shutdown of Recognizer
 *
 * @param log    VXI Logging interface used for error/diagnostic logging,
 *               only used for the duration of this function call
 *
 * @result VXIrec_RESULT_SUCCESS on success
 */
OSBREC_API VXIrecResult OSBrecShutDown(VXIlogInterface *log);

/**
 * Prepareing to shutdown the session by setting live to 0
 *
 * @param log    VXI Logging interface used for error/diagnostic logging,
 *               only used for the duration of this function call
 *
 * @result VXIrec_RESULT_SUCCESS on success
 */
OSBREC_API VXIrecResult OSBrecExiting (VXIlogInterface  *log,
                                       VXIrecInterface  **rec);

/**
 * Create a new recognizer service handle
 *
 * @param log    VXI Logging interface used for error/diagnostic logging,
 *               must remain a valid pointer throughout the lifetime of
 *               the resource (until OSBrecDestroyResource( ) is called)
 * @param rec    VXIrecInterface pointer that will be allocated within this
 *               function. Call OSBrecDestroyResource( ) to delete the 
 *               resource.
 *
 * @result VXIrec_RESULT_SUCCESS on success 
 */
OSBREC_API VXIrecResult OSBrecCreateResource(VXIunsigned      channelNum,
					     VXIlogInterface   *log,
					     VXIrecInterface  **rec);

/**
 * Add the call manager to the recognizer service handle
 *
 * @param pCallMgr    Call manager pointer.
 * @param rec    VXIrecInterface pointer that will be allocated within this
 *               function. Call OSBrecDestroyResource( ) to delete the 
 *               resource.
 *
 * @result VXIrec_RESULT_SUCCESS on success 
 */
OSBREC_API VXIrecResult OSBrecAddResource (VXIrecInterface **rec,
					 CallManager *pCallMgr,
           VXIpromptInterface *prompt);

/**
 * Destroy the interface and free internal resources. Once this is
 *  called, the logging and Internet interfaces passed to
 *  VXIrecognizerCreateResource( ) may be released as well.
 *
 * @param rec    VXIrecInterface pointer that will be deallocated.  It will
 *               be set to NULL when deallocated.
 *
 * @result VXIrec_RESULT_SUCCESS on success 
 */
OSBREC_API VXIrecResult OSBrecDestroyResource(VXIrecInterface **rec);

/*@}*/

#ifdef __cplusplus
}
#endif

#include "VXIheaderSuffix.h"

#endif  /* include guard */

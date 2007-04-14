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

#ifndef _OSBTEL_H
#define _OSBTEL_H

#include "cp/CallManager.h"
#include <VXIlog.h>
#include <VXItel.h>
#include "ivr/IvrTelListener.h"

#include <VXIheaderPrefix.h>
#ifdef OSBTEL_EXPORTS
#define OSBTEL_API SYMBOL_EXPORT_DECL
#else
#define OSBTEL_API SYMBOL_IMPORT_DECL
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name OSBtel
 * @memo OSBtel Interface
 * @doc
 * OSBtel provides a simulator implementation of the VXItel abstract interface
 * for call control functionality.   All calls are blocking with the
 * results returned by the implementation being used to determine the
 * outcome of transers. One VXItel interface should be constructed per line.
 */

/*@{*/

/**
 * OSBtel interface, extends the VXItel interface to add a wait for
 * call method.
 */
typedef struct OSBtelInterface
{
  /* Base interface, must be the first member */
  VXItelInterface intf;

  /**
   * EnableCall
   *
   * Re-arms the hardware interface to allow it to accept a call. The
   * telephony interface starts up in an out-of-service (OOS) state.
   * In this state, calls will not be excepted. The best
   * implementation is to arrange for the line to be out of service
   * back to the telephony switch so that failover to the next line
   * occurs at the switch level. For some protocols this can be done
   * by setting the line to a busy state.  This call is blocking. When
   * it returns the interface will be ready to accept calls.
   *
   * A hangup or error on the line will place the hardware back into
   * the out-of-service state so that no calls come into the line
   * until it is explictly re-armed.
   *
   * @return VXItel_RESULT_SUCCESS if sucess, different value on failure 
   */
  VXItelResult (*EnableCall)(struct OSBtelInterface  *pThis);

  /**
   * WaitForCall
   *
   * Wait for and answer a call, EnableCall( ) must be called prior
   * to this to enable calls on the channel.
   *
   * @param telephonyProps [OUT] Used to return the telephony properties
   *                             that should be made available to the
   *                             application as "session.telephone.*" 
   *                             variables, such as the called and calling
   *                             party telephone numbers. The platform is
   *                             responsible for destroying these on
   *                             success.
   *
   * @return VXItel_RESULT_SUCCESS if sucess, different value on failure 
   */
  VXItelResult (*WaitForCall)(struct OSBtelInterface  *pThis,
			      VXIMap                 **telephonyProps);

} OSBtelInterface;


/**
 * Initializes an OSBtel implementation of the VXItel interface
 *
 * @param log              VXI Logging interface used for error/diagnostic 
 *                         logging, only used for the duration of this 
 *                         function call
 * @param diagLogBase      Logging base for diagnostic logging. All 
 *                         diagnostics are based on this offset.
 *
 * @return VXItel_RESULT_SUCCESS if sucess, different value on failure
 */
OSBTEL_API VXItelResult OSBtelInit (VXIlogInterface  *log,
				    VXIunsigned       diagLogBase);

/**
 * Shutdown an OSBtel implementation of the VXItel interface
 *
 * @param log              VXI Logging interface used for error/diagnostic 
 *                         logging, only used for the duration of this 
 *                         function call
 *
 * @return VXItel_RESULT_SUCCESS if sucess, different value on failure
 */
OSBTEL_API VXItelResult OSBtelShutDown (VXIlogInterface  *log);

/**
 * Prepareing to shutdown the session by setting live to 0
 *
 * @param log    VXI Logging interface used for error/diagnostic logging,
 *               only used for the duration of this function call
 *
 * @result VXItel_RESULT_SUCCESS on success
 */
OSBTEL_API VXItelResult OSBtelExiting (VXIlogInterface  *log,
                                       VXItelInterface  **tel);

/**
 * Creates an OSBtel implementation of the VXItel interface
 *
 * @param log            [IN]  VXI Logging interface used for error/diagnostic
 *                             logging, must remain a valid pointer throughout
 *                             the lifetime of the resource (until 
 *                             OSBpromptDestroyResource( ) is called)
 * @param tel            [OUT] Pointer that returns the newly created VXItel
 *                             interface.
 *
 * @return VXItel_RESULT_SUCCESS if sucess, different value on failure
 */
OSBTEL_API VXItelResult OSBtelCreateResource(VXIunsigned     channelNum,
					     VXIlogInterface *log,
					     VXItelInterface **tel);

/**
 * Adds the call manager handle to the VXItel interface
 *
 * @param pCallMgr       [IN]  call manager
 * @param tel            Pointer that returns the VXItel interface.
 *
 * @return VXItel_RESULT_SUCCESS if sucess, different value on failure
 */
OSBTEL_API VXItelResult OSBtelAddResource (VXItelInterface **tel,
					 CallManager *pCallMgr,
           IvrTelListener *pListener);



/**
 * Destroys the specified OSBtel implementation
 *
 * @param tel  [IN/OUT] Pointer to the OSBtel object to be destroyed.  Set
 *                      to NULL on return.
 *
 * @return VXItel_RESULT_SUCCESS if sucess, different value on failure
 */
OSBTEL_API VXItelResult OSBtelDestroyResource(VXItelInterface **tel);


/*@}*/
#include <VXIheaderSuffix.h>

#ifdef __cplusplus
}
#endif

#endif  /* include guard */

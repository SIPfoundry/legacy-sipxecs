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
 ************************************************************************
 *
 * 
 *
 ************************************************************************
 */

#ifndef _VXIINTERPRETER_H
#define _VXIINTERPRETER_H

#include "VXItypes.h"                  /* For VXIchar */
#include "VXIvalue.h"                  /* For VXIValue */

#ifdef __cplusplus
struct VXIlogInterface;
struct VXIinetInterface;
struct VXIjsiInterface;
struct VXIrecInterface;
struct VXIpromptInterface;
struct VXItelInterface;
struct VXIobjectInterface;
#else
#include "VXIlog.h"
#include "VXIinet.h"
#include "VXIjsi.h"
#include "VXIrec.h"
#include "VXIprompt.h"
#include "VXItel.h"
#include "VXIobject.h"
#endif

#include "VXIheaderPrefix.h"
#ifdef VXI_EXPORTS
#define VXI_INTERPRETER SYMBOL_EXPORT_DECL
#else
#define VXI_INTERPRETER SYMBOL_IMPORT_DECL
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
  @name VXI 
  @memo VXI interpreter interface
  @doc The OpenSpeech Browser core is the OpenVXI.  The
  VXIinterpreter interface implements the VXI interface function to run
  the interface. In addition a set of process and thread initialization
  routines are provided to set-up and destroy the interpreter per thread.
  */

/*@{*/

/**
 * Keys identifying properties for SetProperties.
 */
#define VXI_BEEP_AUDIO        L"vxi.property.beep.uri"          /* VXIString */
#define VXI_PLATFORM_DEFAULTS L"vxi.property.platform.defaults" /* VXIString */

/**
 * @name VXIinterpreterResult
 * @memo Result codes for interface methods
 * @doc
 * Result codes less then zero are severe errors (likely to be
 * platform faults), those greater then zero are warnings (likely to
 * be application issues) 
 */
typedef enum VXIinterpreterResult {
  /** Fatal error, terminate call    */
  VXIinterp_RESULT_FATAL_ERROR          = -100, 
  /** I/O error                      */
  VXIinterp_RESULT_IO_ERROR             =   -8,
  /** Out of memory                  */
  VXIinterp_RESULT_OUT_OF_MEMORY        =   -7, 
  /** System error, out of service   */
  VXIinterp_RESULT_SYSTEM_ERROR         =   -6, 
  /** Errors from platform services  */
  VXIinterp_RESULT_PLATFORM_ERROR       =   -5, 
  /** Return buffer too small        */
  VXIinterp_RESULT_BUFFER_TOO_SMALL     =   -4, 
  /** Property name is not valid     */
  VXIinterp_RESULT_INVALID_PROP_NAME    =   -3, 
  /** Property value is not valid    */
  VXIinterp_RESULT_INVALID_PROP_VALUE   =   -2, 
  /** Invalid function argument      */
  VXIinterp_RESULT_INVALID_ARGUMENT     =   -1, 
  /** Success                        */
  VXIinterp_RESULT_SUCCESS              =    0,
  /** Normal failure, nothing logged */
  VXIinterp_RESULT_FAILURE              =    1,
  /** Non-fatal non-specific error   */
  VXIinterp_RESULT_NON_FATAL_ERROR      =    2, 
  /** Document not found             */
  VXIinterp_RESULT_NOT_FOUND            =   50, 
  /** Document fetch timeout         */
  VXIinterp_RESULT_FETCH_TIMEOUT        =   51,
  /** Other document fetch error     */
  VXIinterp_RESULT_FETCH_ERROR          =   52,
  /** Not a valid VoiceXML document  */
  VXIinterp_RESULT_INVALID_DOCUMENT     =   53, 
  /** Document has syntax errors     */
  VXIinterp_RESULT_SYNTAX_ERROR         =   54,
  /** Uncaught fatal VoiceXML event  */
  VXIinterp_RESULT_UNCAUGHT_FATAL_EVENT =   55,
  /** ECMAScript syntax error        */
  VXIinterp_RESULT_SCRIPT_SYNTAX_ERROR  =   56,
  /** ECMAScript exception throw     */
  VXIinterp_RESULT_SCRIPT_EXCEPTION     =   57,
  /** Operation is not supported     */
  VXIinterp_RESULT_UNSUPPORTED          =  100
} VXIinterpreterResult;


/**
 * @name VXIresources
 * @memo Structure containing all the interfaces required by the VXI.
 * @doc This structure must be allocated and all the pointers filled
 * with created and initialized resources before creating the VXI
 * interface.
 */
typedef struct VXIresources {
  /** log interface */
  VXIlogInterface    * log;
  /** Internet interface */
  VXIinetInterface   * inet; 
  /** ECMAScript interface */
  VXIjsiInterface    * jsi;    
  /** Recognizer interface */
  VXIrecInterface    * rec;  
  /** Prompt interface */
  VXIpromptInterface * prompt;
  /** Telephony interface */
  VXItelInterface    * tel;
  /** object interface. May be NULL in which case objects will not
      function */
  VXIobjectInterface * object;
} VXIresources;


/*
** ==================================================
** VXIinterpreterInterface Interface definition
** ==================================================
*/

/**
 * @name VXIinterpreterInterface
 * @memo VXIinterpreter interface for VoiceXML execution
 * @version 1.0 <br>
 * @doc
 * Abstract interface for the VoiceXML intepreter, simply provides a
 * single method for running the intepreter on a document and getting
 * the document result.<p>
 *
 * There is one intepreter interface per thread/line.
 */
typedef struct VXIinterpreterInterface {
  /**
   * @name GetVersion
   * @memo Get the VXI interface version implemented
   *
   * @return  VXIint32 for the version number. The high high word is 
   *          the major version number, the low word is the minor version 
   *          number, using the native CPU/OS byte order. The current
   *          version is VXI_CURRENT_VERSION as defined in VXItypes.h.
   */ 
  VXIint32 (*GetVersion)(void);
  
  /**
   * @name GetImplementationName
   * @memo Get the name of the implementation
   *
   * @return  Implementation defined string that must be different from
   *          all other implementations. The recommended name is one
   *          where the interface name is prefixed by the implementator's
   *          Internet address in reverse order, such as com.xyz.rec for
   *          VXIrec from xyz.com. This is similar to how VoiceXML 1.0
   *          recommends defining application specific error types.
   */
  const VXIchar* (*GetImplementationName)(void);
  
  /**
   * Run a VoiceXML document and optionally return the result
   *
   * @param name    [IN] Name of the VoiceXML document to fetch and 
   *                  execute, may be a URL or a platform dependant path.
   *                  See the Open( ) method in VXIinet.h for details
   *                  about supported names, however for URLs this
   *                  must always be an absolute URL and any query arguments
   *                  must be embedded.
   * @param sessionArgs [IN] Any arguments to be passed to the VXI.  Some of
   *                  these, such as ANI, DNIS, etc. as required by VXML, but
   *                  anything may be passed in.  These values are available
   *                  through the session variable in ECMA script.
   * @param result  [OUT] (Optional, pass NULL if not desired.) Return
   *                  value for the VoiceXML document (from <exit/>), this
   *                  is allocated on success and when there is an
   *                  exit value (a NULL pointer is returned otherwise),
   *                  the caller is responsible for destroying the returned
   *                  value by calling VXIValueDestroy( ). If
   *                  VXIinterp_RESULT_UNCAUGHT_FATAL_EVENT is returned,
   *                  this will be a VXIString that provides the name
   *                  of the VoiceXML event that caused the interpreter
   *                  to exit.
   *
   * @return         VXIinterp_RESULT_SUCCESS on success
   */
  VXIinterpreterResult (*Run)(struct VXIinterpreterInterface  *pThis,
                              const VXIchar                   *name,
                              const VXIMap                    *sessionArgs,
                              VXIValue                       **result);

  /**
   * Specify runtime properties for the VoiceXML interpreter.
   *
   * @param props   [IN] Map containing a list of properties.  Currently there
   *                  are two of interest:
   *                  * VXI_BEEP_AUDIO           URI for the beep audio
   *                  * VXI_PLATFORM_DEFAULTS    URI for the platform defaults
   *
   * @return         VXIinterp_RESULT_SUCCESS on success
   */
  VXIinterpreterResult (*SetProperties)(struct VXIinterpreterInterface *pThis,
					const VXIMap                   *props);

  /**
   * Load and parse an VXML document.  This tests the validity.
   *
   * @param name    [IN] Name of the VoiceXML document to fetch and 
   *                  execute, may be a URL or a platform dependant path.
   *                  See the Open( ) method in VXIinet.h for details
   *                  about supported names, however for URLs this
   *                  must always be an absolute URL and any query arguments
   *                  must be embedded.
   *
   * @return        VXIinterp_RESULT_SUCCESS if document exists and is valid
   *                VXIinterp_RESULT_NOT_FOUND if document not found at uri
   *                VXIinterp_RESULT_FETCH_ERROR if document retrieval failed
   *                VXIinterp_RESULT_FAILURE if document is invalid VXML
   */
  VXIinterpreterResult (*Validate)(struct VXIinterpreterInterface  *pThis,
                                   const VXIchar *name);

} VXIinterpreterInterface;


/**
 * @name VXIinterpreterInit
 * @memo Per-process initialization for VXIinterpreter.
 * @doc This function should be called once at process startup. 
 *
 * @param log            [IN] VXI Logging interface used for error/diagnostic 
 *                             logging, only used for the duration of this 
 *                             function call
 * @param  diagLogBase   [IN] Base tag number for diagnostic logging purposes.
 *                             All diagnostic tags for the VXI will start at
 *                             this ID and increase upwards.
 *
 * @return     VXIinterp_RESULT_SUCCESS if resources may be created.
 * @return     VXIinterp_RESULT_FAILURE if interface is unavailable.
 */
VXI_INTERPRETER 
VXIinterpreterResult VXIinterpreterInit(VXIlogInterface  *log,
                                        VXIunsigned       diagLogBase);


/**
 * @name VXIinterpreterShutDown
 * @memo  Per-process de-initialization for VXIinterpreter.
 * @doc This function should be called once per process shutdown, after
 * all the interfaces for the process are destroyed.
 *
 * @param log [IN] VXI Logging interface used for error/diagnostic logging,
 *                  only used for the duration of this function call
 */
VXI_INTERPRETER void VXIinterpreterShutDown(VXIlogInterface  *log);


/**
 * @name VXIinterpreterCreateResource
 * @memo  Create an interface to the VoiceXML interpreter.
 * @doc Create a VXI interface given an interface structure that
 * contains all the resources required for the VXI.
 *
 * @param resource [IN] A pointer to a structure containing all the
 *                       interfaces requires by the VXI
 * @param pThis    [IN] A pointer to the VXI interface that is to be
 *                       allocated.  The pointer will be set if this call
 *                       is successful.
 *
 * @return     VXIinterp_RESULT_SUCCESS if interface is available for use
 * @return     VXIinterp_RESULT_OUT_OF_MEMORY if low memory is suspected
 */
VXI_INTERPRETER VXIinterpreterResult 
VXIinterpreterCreateResource(VXIresources *resource,
                             VXIinterpreterInterface ** pThis);

/**
 * @name VXIinterpreterDestroyResource
 * @memo Destroy and de-allocate a VXI interface
 * @doc
 * Destroy an interface returned from VXIinterpreterCreateResource.
 * The pointer is set to NULL on success.  
 *
 * @param pThis [IN] The pointer to the interface to be destroyed.
 */
VXI_INTERPRETER void
VXIinterpreterDestroyResource(VXIinterpreterInterface ** pThis);

/*@}*/

#ifdef __cplusplus
}
#endif

#include "VXIheaderSuffix.h"

#endif  /* include guard */

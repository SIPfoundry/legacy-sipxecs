/*****************************************************************************
 *****************************************************************************
 *
 * 
 *
 * Implementation of the SBjsi functions defined in SBjsi.h and the
 * OSBjsi functions defined in OSBjsi.h, see those headers for details.
 * Those headers are functionally redundant, the OSB version and entry
 * points are for the OpenVXI open source release, while the SB version
 * and entry points are for the OpenSpeech Browser PIK product release.
 *
 *****************************************************************************
 ****************************************************************************/


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
 */

static const char *rcsid = 0 ? (char *) &rcsid :
"";

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8


#include "SBjsiInternal.h"

#include <string.h>                     // For memset( )

#ifdef OPENVXI
#define SBJSI_API static
#else
#define SBJSI_EXPORTS
#include "SBjsi.h"                      // Header for this interface
#endif
#define OSBJSI_EXPORTS
#include "OSBjsi.h"                     // Header for this interface
#include "SBjsiAPI.h"                   // Header for the API functions

#include "SBjsiLog.h"                   // For logging
#include "JsiRuntime.hpp"               // For JsiRuntime class
#include "SBjsiInterface.h"             // For SBjsiInterface

#include "jsapi.h"                      // For JS_ShutDown( )

// Global variable to track whether this is initialized
static bool gblInitialized = false;

// Global runtime, used across the entire system
static JsiRuntime *gblJsiRuntime = NULL;

// Runtime and Context sizes in bytes for each new runtime/context
static long gblRuntimeSize = 0;
static long gblContextSize = 0;

// Maximum number of JavaScript branches per script evaluation
static long gblMaxBranches = 0;

// Offset for diagnostic logging
static VXIunsigned gblDiagTagBase = 0;


// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8


/**
 * Global platform initialization of JavaScript
 *
 * @param log           VXI Logging interface used for error/diagnostic 
 *                      logging, only used for the duration of this 
 *                      function call
 * @param  diagLogBase  Base tag number for diagnostic logging purposes.
 *                      All diagnostic tags for SBjsi will start at this
 *                      ID and increase upwards.
 * @param  runtimeSize  Size of the JavaScript runtime environment, 
 *                      in bytes, see above for a recommended default
 * @param  contextSize  Size of each JavaScript context, in bytes, see
 *                      above for a recommended default
 * @param  maxBranches  Maximum number of JavaScript branches for each
 *                      JavaScript evaluation, used to interrupt infinite
 *                      loops from (possibly malicious) scripts
 *
 * @result VXIjsiResult 0 on success
 */
SBJSI_API VXIjsiResult SBjsiInit (VXIlogInterface  *log,
				  VXIunsigned       diagTagBase,
				  VXIlong           runtimeSize,
				  VXIlong           contextSize,
				  VXIlong           maxBranches)
{
  static const wchar_t func[] = L"SBjsiInit";
  if ( log )
    log->Diagnostic (log, diagTagBase + SBJSI_LOG_API, func,
		     L"entering: 0x%p, %u, %ld, %ld, %ld",
		     log, diagTagBase, runtimeSize, contextSize, maxBranches);

  // Make sure this wasn't already called
  VXIjsiResult rc = VXIjsi_RESULT_SUCCESS;
  if ( gblInitialized == true ) {
    SBinetLogger::Error (log, MODULE_SBJSI, JSI_ERROR_INIT_FAILED, NULL);
    rc = VXIjsi_RESULT_FATAL_ERROR;
    if ( log )
      log->Diagnostic (log, diagTagBase + SBJSI_LOG_API, func,
		       L"exiting: returned %d", rc);
    return rc;
  }

  // Check arguments
  if (( ! log ) || ( runtimeSize <= 0 ) || ( contextSize <= 0 ) || 
      ( maxBranches <= 0 )) {
    SBinetLogger::Error (log, MODULE_SBJSI, JSI_ERROR_INIT_FAILED, NULL);
    rc = VXIjsi_RESULT_INVALID_ARGUMENT;
    if ( log )
      log->Diagnostic (log, diagTagBase + SBJSI_LOG_API, func,
		       L"exiting: returned %d", rc);
    return rc;
  }

  // Create the global runtime environment
  gblJsiRuntime = new JsiRuntime( );
  if ( gblJsiRuntime == NULL ){
    SBinetLogger::Error (log, MODULE_SBJSI, JSI_ERROR_OUT_OF_MEMORY, NULL);
    rc = VXIjsi_RESULT_OUT_OF_MEMORY;
  }
  else
    rc = gblJsiRuntime->Create (runtimeSize, log, diagTagBase);

  // Finish creation
  if ( rc == VXIjsi_RESULT_SUCCESS ) {
    gblRuntimeSize = runtimeSize;
    gblContextSize = contextSize;
    gblMaxBranches = maxBranches;
    gblDiagTagBase = diagTagBase;
    gblInitialized = true;
  } else if ( gblJsiRuntime ) {
    delete gblJsiRuntime;
    gblJsiRuntime = NULL;
  }

  log->Diagnostic (log, diagTagBase + SBJSI_LOG_API, func,
		   L"exiting: returned %d", rc);
  return rc;
}


OSBJSI_API VXIjsiResult OSBjsiInit (VXIlogInterface  *log,
				    VXIunsigned       diagTagBase,
				    VXIlong           runtimeSize,
				    VXIlong           contextSize,
				    VXIlong           maxBranches)
{
  return SBjsiInit (log, diagTagBase, runtimeSize, contextSize, maxBranches);
}


/**
 * Global platform shutdown of Jsi
 *
 * @param log    VXI Logging interface used for error/diagnostic logging,
 *               only used for the duration of this function call
 *
 * @result VXIjsiResult 0 on success
 */
SBJSI_API VXIjsiResult SBjsiShutDown (VXIlogInterface  *log)
{
  static const wchar_t func[] = L"SBjsiShutDown";
  if ( log )
    log->Diagnostic (log, gblDiagTagBase + SBJSI_LOG_API, func,
		     L"entering: 0x%p", log);

  // Make sure we've been created successfully
  VXIjsiResult rc = VXIjsi_RESULT_SUCCESS;
  if ( gblInitialized == false ) {
    SBinetLogger::Error (log, MODULE_SBJSI, JSI_ERROR_NOT_INITIALIZED, NULL);
    rc = VXIjsi_RESULT_FATAL_ERROR;
    if ( log )
      log->Diagnostic (log, gblDiagTagBase + SBJSI_LOG_API, func,
		       L"exiting: returned %d", rc);
    return rc;
  }

  if ( ! log )
    return VXIjsi_RESULT_INVALID_ARGUMENT;

  // Destroy the runtime environment
  if ( gblJsiRuntime )
    delete gblJsiRuntime;
  gblJsiRuntime = NULL;
  
  // Shut down SpiderMonkey 
  JS_ShutDown( );
  
  // Finish shutdown
  gblRuntimeSize = 0;
  gblContextSize = 0;
  gblMaxBranches = 0;
  gblInitialized = false;

  log->Diagnostic (log, gblDiagTagBase + SBJSI_LOG_API, func,
		   L"exiting: returned %d", rc);
  return rc;
}


OSBJSI_API VXIjsiResult OSBjsiShutDown (VXIlogInterface  *log)
{
  return SBjsiShutDown(log);
}


/**
 * Create a new jsi service handle
 *
 * @param log    VXI Logging interface used for error/diagnostic 
 *               logging, must remain a valid pointer throughout the 
 *               lifetime of the resource (until SBjsiDestroyResource( )
 *               is called)
 *
 * @result VXIjsiResult 0 on success 
 */
SBJSI_API 
VXIjsiResult SBjsiCreateResource(VXIlogInterface     *log,
				 VXIjsiInterface    **jsi)
{
  static const wchar_t func[] = L"SBjsiCreateResource";
  if ( log )
    log->Diagnostic (log, gblDiagTagBase + SBJSI_LOG_API, func,
		     L"entering: 0x%p, 0x%p", log, 
		     jsi);

  // Make sure we've been created successfully
  VXIjsiResult rc = VXIjsi_RESULT_SUCCESS;
  if ( gblInitialized == false ) {
    SBinetLogger::Error (log, MODULE_SBJSI, JSI_ERROR_NOT_INITIALIZED, NULL);
    rc = VXIjsi_RESULT_FATAL_ERROR;
    if ( log )
      log->Diagnostic (log, gblDiagTagBase + SBJSI_LOG_API, func,
		       L"exiting: returned %d", rc);
    return rc;
  }

  if (( log == NULL ) || ( jsi == NULL )) {
    SBinetLogger::Error (log, MODULE_SBJSI, JSI_ERROR_INVALID_ARG, NULL);
    rc = VXIjsi_RESULT_INVALID_ARGUMENT;
    if ( log )
      log->Diagnostic (log, gblDiagTagBase + SBJSI_LOG_API, func,
		       L"exiting: returned %d", rc);
    return rc;
  }

  // Get a new interface instance
  SBjsiInterface *newJsi = new SBjsiInterface;
  if ( ! newJsi ) {
    SBinetLogger::Error (log, MODULE_SBJSI, JSI_ERROR_INTERFACE_ALLOC_FAILED,
			 NULL);
    rc = VXIjsi_RESULT_OUT_OF_MEMORY;
  } else {
    memset (newJsi, 0, sizeof (SBjsiInterface));
    
    // Initialize the function pointers
    newJsi->jsi.GetVersion = SBjsiGetVersion;
    newJsi->jsi.GetImplementationName = SBjsiGetImplementationName;
    newJsi->jsi.CreateContext = SBjsiCreateContext;
    newJsi->jsi.DestroyContext = SBjsiDestroyContext;
    newJsi->jsi.CreateVarExpr = SBjsiCreateVarExpr;
    newJsi->jsi.CreateVarValue = SBjsiCreateVarValue;
    newJsi->jsi.SetVarExpr = SBjsiSetVarExpr;
    newJsi->jsi.SetVarValue = SBjsiSetVarValue;
    newJsi->jsi.GetVar = SBjsiGetVar;
    newJsi->jsi.CheckVar = SBjsiCheckVar;
    newJsi->jsi.Eval = SBjsiEval;
    newJsi->jsi.PushScope = SBjsiPushScope;
    newJsi->jsi.PopScope = SBjsiPopScope;
    newJsi->jsi.ClearScopes = SBjsiClearScopes;
    
    // Initialize the data members
    newJsi->contextSize = gblContextSize;
    newJsi->maxBranches = gblMaxBranches;
    newJsi->jsiRuntime = gblJsiRuntime;
    newJsi->log = log;
    newJsi->diagTagBase = gblDiagTagBase;
  }

  if ( rc != VXIjsi_RESULT_SUCCESS ) {
    if ( newJsi )
      delete newJsi;
    newJsi = NULL;
  } else {
    // Return the object
    *jsi = &(newJsi->jsi);
  }

  log->Diagnostic (log, gblDiagTagBase + SBJSI_LOG_API, func,
		   L"exiting: returned %d", rc);
  return rc;
}


OSBJSI_API 
VXIjsiResult OSBjsiCreateResource(VXIlogInterface     *log,
				  VXIjsiInterface    **jsi)
{
  return SBjsiCreateResource (log, jsi);
}


/**
 * Destroy the interface and free internal resources. Once this is
 *  called, the logging interface passed to SBjsiCreateResource( ) may
 *  be released as well.
 *
 * @result VXIjsiResult 0 on success 
 */
SBJSI_API 
VXIjsiResult SBjsiDestroyResource(VXIjsiInterface **jsi)
{
  static const wchar_t func[] = L"SBjsiDestroyResource";
  // Can't log yet, don't have a log handle

  // Make sure we've been created successfully
  if ( gblInitialized == false )
    return VXIjsi_RESULT_FATAL_ERROR;

  if (( jsi == NULL ) || ( *jsi == NULL ))
    return VXIjsi_RESULT_INVALID_ARGUMENT;

  // Get the real underlying interface
  VXIjsiResult rc = VXIjsi_RESULT_SUCCESS;
  SBjsiInterface *sbJsi = (SBjsiInterface *) *jsi;

  VXIlogInterface *log = sbJsi->log;
  log->Diagnostic (log, gblDiagTagBase + SBJSI_LOG_API, func,
		   L"entering: 0x%p (0x%p)", jsi, *jsi);

  // Delete the object
  delete sbJsi;
  sbJsi = NULL;
  *jsi = NULL;

  log->Diagnostic (log, gblDiagTagBase + SBJSI_LOG_API, func,
		   L"exiting: returned %d", rc);
  return rc;
}


OSBJSI_API 
VXIjsiResult OSBjsiDestroyResource(VXIjsiInterface **jsi)
{
  return SBjsiDestroyResource (jsi);
}

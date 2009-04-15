/*****************************************************************************
 *****************************************************************************
 *
 * 
 *
 * OSBjsi JavaScript (ECMAScript) Engine Interface
 *
 * OSBjsi interface, an implementation of the VXIjsi abstract interface
 * for interacting with a JavaScript (ECMAScript) engine.  This
 * provides functionality for creating JavaScript execution contexts,
 * manipulating JavaScript scopes, manipulating variables within those
 * scopes, and evaluating JavaScript expressions/scripts.
 *
 * There is one JavaScript interface per thread/line.
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

#ifndef _OSBJSI_H
#define _OSBJSI_H

#include "VXIjsi.h"                    /* For VXIjsi base interface */
#include "VXIlog.h"                    /* For VXIlog interface */

#include "VXIheaderPrefix.h"
#ifdef OSBJSI_EXPORTS
#define OSBJSI_API SYMBOL_EXPORT_DECL
#else
#define OSBJSI_API SYMBOL_IMPORT_DECL
#endif

  /* -- start docme interface -- */

/**
 * @name OSBjsi
 * @memo OSBjsi implementation of VXIjsi
 * @doc
 * OSBjsi interface, an implementation of the VXIjsi interface for
 * interacting with a ECMAScript (JavaScript) engine.  This provides
 * functionality for creating ECMAScript execution contexts,
 * manipulating ECMAScript scopes, manipulating variables within those
 * scopes, and evaluating ECMAScript expressions/scripts. <p>
 *
 * There is one VXIjsi interface per thread/line.  
 */

  /*@{*/


/* Recommended defaults for OSBjsiInit */
#define JSI_RUNTIME_SIZE_DEFAULT     (1024 * 1024 * 16)
#define JSI_CONTEXT_SIZE_DEFAULT     (1024 * 128)
#define JSI_MAX_BRANCHES_DEFAULT     100000

/**
 * Global platform initialization of JavaScript
 *
 * @param log             VXI Logging interface used for error/diagnostic 
 *                        logging, only used for the duration of this 
 *                        function call
 * @param  diagLogBase    Base tag number for diagnostic logging purposes.
 *                        All diagnostic tags for OSBjsi will start at this
 *                        ID and increase upwards.
 * @param  runtimeSize    Size of the JavaScript runtime environment, in 
 *                        bytes. There is one runtime per process. See 
 *                        above for a recommended default.
 * @param  contextSize    Size of each JavaScript context, in bytes. There
 *                        may be multiple contexts per channel, although
 *                        the VXI typically only uses one per channel.
 *                        See above for a recommended default.
 * @param  maxBranches    Maximum number of JavaScript branches for each 
 *                        JavaScript evaluation, used to interrupt infinite
 *                        loops from (possibly malicious) scripts
 *
 * @result VXIjsiResult 0 on success
 */
OSBJSI_API VXIjsiResult OSBjsiInit (VXIlogInterface  *log,
				    VXIunsigned       diagLogBase,
				    VXIlong           runtimeSize,
				    VXIlong           contextSize,
				    VXIlong           maxBranches);

/**
 * Global platform shutdown of JavaScript
 *
 * @param log    VXI Logging interface used for error/diagnostic logging,
 *               only used for the duration of this function call
 *
 * @result VXIjsiResult 0 on success
 */
OSBJSI_API VXIjsiResult OSBjsiShutDown (VXIlogInterface  *log);

/**
 * Create a new JavaScript service handle
 *
 * @param log    VXI Logging interface used for error/diagnostic 
 *               logging, must remain a valid pointer throughout the 
 *               lifetime of the resource (until OSBjsiDestroyResource( )
 *               is called)
 *
 * @result VXIjsiResult 0 on success 
 */
OSBJSI_API VXIjsiResult OSBjsiCreateResource(VXIlogInterface   *log,
					     VXIjsiInterface  **jsi);

/**
 * Destroy the interface and free internal resources. Once this is
 *  called, the logging interface passed to OSBjsiCreateResource( ) may
 *  be released as well.
 *
 * @result VXIjsiResult 0 on success 
 */
OSBJSI_API VXIjsiResult OSBjsiDestroyResource(VXIjsiInterface **jsi);

  /* -- end docme interface -- */
/*@}*/
#include "VXIheaderSuffix.h"

#endif  /* include guard */

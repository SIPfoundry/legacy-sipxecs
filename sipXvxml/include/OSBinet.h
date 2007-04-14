/*****************************************************************************
 *****************************************************************************
 *
 * 
 *
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

#ifndef _OSBINET_H
#define _OSBINET_H

#include "VXIinet.h"                   /* For VXIinet base interface */
#include "VXIlog.h"                    /* For VXIlog interface */

#include "VXIheaderPrefix.h"
#ifdef OSBINET_EXPORTS
#define OSBINET_API SYMBOL_EXPORT_DECL
#else
#define OSBINET_API SYMBOL_IMPORT_DECL
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name OSBinet
 * @memo OSBinet implementation of VXIinet
 * @doc
 * OSBinet interface, and implementation of the VXIinet abstract
 * interface for Internet functionality including HTTP requests, local
 * file access, URL caching, memory buffer caching, and cookie
 * access. <p>
 *
 * The interface is a synchronous interface based on the ANSI/ISO C
 * standard file I/O interface, the only exception is that pre-fetches are
 * asynchronous. The client of the interface may use this in an
 * asynchronous manner by using non-blocking I/O operations, creating
 * threads, or by invoking this from a separate server process. <p>
 * 
 * This implementation currently does NOT support INET_MODE_WRITE for
 * http:// access (HTTP PUT), and only supports http://, file://, and
 * OS dependant paths.
 *
 * There is one Internet interface per thread/line.  
 */

  /*@{*/

/**
 * Default value for the User Agent Name parameter of OSBinetInit()
 */
#define OSBINET_USER_AGENT_NAME_DEFAULT \
              L"OpenVXITestApp/" VXI_CURRENT_VERSION_STR


/**
 * Global platform initialization of OSBinet
 *
 * @param log              VXI Logging interface used for error/diagnostic 
 *                         logging, only used for the duration of this 
 *                         function call
 * @param cacheDir         Cache directory name
 * @param cacheSizeMB      Maximum size of the data in the cache directory,
 *                         in megabytes
 * @param entryMaxSizeMB   Maximum size of any individual cache entry, in
 *                         megabytes
 * @param entryExpTimeSec  Maximum amount of time any individual cache entry
 *                         will remain in the cache, in seconds
 * @param proxyServer      Name of the proxy server to use for HTTP access,
 *                         pass a server name or IP address, or NULL to
 *                         do direct HTTP access.
 * @param proxyPort        Port number for accessing the proxy server.
 * @param userAgentName    HTTP user agent name sent in all HTTP messages.
 *                         Must be of the form <app>/<version> with no
 *                         spaces, such as "OpenVXI/1.0". When using the
 *                         OpenVXI application name or a derrivative, use
 *                         use VXI_CURRENT_VERSION_STR for the version.
 * @param extensionRules   Rules for mapping file extensions to MIME content
 *                         types, used for that purpose when accessing
 *                         local files and file:// URLs. Each key in the
 *                         map must be an extension (period followed by the
 *                         extension such as ".txt") with the value being
 *                         the MIME content type for that extension.  Copied
 *                         internally so the pointer that is passed in still
 *                         belongs to the caller.
 * @param reserved         Reserved VXIVector, pass NULL
 *
 * @result VXIinet_RESULT_SUCCESS on success
 */
OSBINET_API VXIinetResult OSBinetInit (VXIlogInterface  *log,
				      const VXIunsigned diagLogBase,
				      const VXIchar    *cacheDir,
				      const int         cacheSizeMB,
				      const int         entryMaxSizeMB,
				      const int         entryExpTimeSec,
				      const VXIchar    *proxyServer,
				      VXIulong          proxyPort,
				      const VXIchar    *userAgentName,
				      const VXIMap     *extensionRules,
				      const VXIVector  *reserved);

/**
 * Global platform shutdown of OSBinet
 *
 * @param log    VXI Logging interface used for error/diagnostic logging,
 *               only used for the duration of this function call
 *
 * @result VXIinet_RESULT_SUCCESS on success
 */
OSBINET_API VXIinetResult OSBinetShutDown (VXIlogInterface  *log);

/**
 * Create a new inet service handle
 *
 * @param log            VXI Logging interface used for error/diagnostic 
 *                       logging, must remain a valid pointer throughout the 
 *                       lifetime of the resource (until 
 *                       OSBinetDestroyResource( ) is called)
 *
 * @result VXIinet_RESULT_SUCCESS on success 
 */
OSBINET_API 
VXIinetResult OSBinetCreateResource (VXIlogInterface     *log,
				     VXIinetInterface   **inet);
  
/**
 * Destroy the interface and free internal resources. Once this is
 *  called, the logging interface passed to OSBinetCreateResource( ) 
 *  may be released as well.
 *
 * @result VXIinet_RESULT_SUCCESS on success 
 */
OSBINET_API 
VXIinetResult OSBinetDestroyResource (VXIinetInterface **inet);

  /*@}*/

#ifdef __cplusplus
}
#endif

#include "VXIheaderSuffix.h"

#endif  /* include guard */

/*****************************************************************************
 *****************************************************************************
 *
 * 
 *
 * Logging for the SBinet Internet Interface
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

#ifndef _SBINET_LOG_H
#define _SBINET_LOG_H

#include "VXIlog.h"          /* Logging interface */
#ifdef __cplusplus
#include "SBinetLogger.hpp"  /* Logging base class */
#endif

/* Module defines */
#ifndef MODULE_PREFIX
#define MODULE_PREFIX  COMPANY_DOMAIN L"."
#endif

#ifdef OPENVXI
#define MODULE_SBINET                  MODULE_PREFIX L"OSBinet"
#define SBINET_IMPLEMENTATION_NAME     COMPANY_DOMAIN L".OSBinet"
#else
#define MODULE_SBINET                  MODULE_PREFIX L"SBinet"
#define SBINET_IMPLEMENTATION_NAME     COMPANY_DOMAIN L".SBinet"
#endif

#define MODULE_SBINET_TAGID            0

#define MODULE_SBINET_CHANNEL          MODULE_SBINET
#define MODULE_SBINET_CHANNEL_TAGID    1

#define MODULE_SBINET_STREAM           MODULE_SBINET
#define MODULE_SBINET_STREAM_TAGID     2

#define MODULE_SBINET_INTERFACE        MODULE_SBINET
#define MODULE_SBINET_INTERFACE_TAGID  3

#define MODULE_SBINET_BASE             MODULE_SBINET
#define MODULE_SBINET_BASE_TAGID       4

#define MODULE_SBINET_HTALERT          MODULE_SBINET
#define MODULE_SBINET_HTALERT_TAGID    5

#define MODULE_SBINET_HTPRINT          MODULE_SBINET
#define MODULE_SBINET_HTPRINT_TAGID    6

#define MODULE_SBINET_HTTRACE          MODULE_SBINET
#define MODULE_SBINET_HTTRACE_TAGID    7

#define MODULE_SBINET_HTTRACEDATA      MODULE_SBINET
#define MODULE_SBINET_HTTRACEDATA_TAGID 8

#define MODULE_SBINET_CACHE_LOCK       MODULE_SBINET
#define MODULE_SBINET_CACHE_LOCK_TAGID 9

#endif  /* _SBINET_LOG_H */

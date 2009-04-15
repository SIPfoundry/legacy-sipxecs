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
 * OS specific utilities.  Broken out here to provide uniform support
 * across operating systems
 *
 ************************************************************************
 */

#ifndef SBLOG_OS_UTILS
#define SBLOG_OS_UTILS

#include <stdarg.h>
#include <time.h>
#include <VXItypes.h>

#ifdef __cplusplus
extern "C" {
#endif

int SBlogGetTime(time_t *timestamp, 
		 VXIunsigned *timestampMsec);

int SBlogGetTimeStampStr(time_t          timestamp,
			 VXIunsigned     timestampMsec,
			 char*           timestampStr);

typedef struct SBlogFileStats
{
  long int st_size;
  time_t st_atim;
  time_t st_mtim;
  time_t st_ctim;

} SBlogFileStats;

int SBlogGetFileStats(const char *path, 
		      SBlogFileStats *fileStats);

int SBlogGetCPUTimes(long *userTime
		     /* ms spent in user mode */,
		     long *kernelTime
		     /* ms spent in kernel mode*/
		     );

int SBlogVswprintf(wchar_t* wcs, size_t maxlen, 
		   const wchar_t* format, va_list args);

#ifdef __cplusplus
}
#endif

#endif // include guard

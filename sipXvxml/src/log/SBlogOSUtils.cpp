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

static const char *rcsid = 0 ? (char *) &rcsid :
"";

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/times.h>                  // For times( )
#endif

#include <sys/timeb.h>                  // for ftime( )/_ftime( )
#include <sys/stat.h>                   // for stat( )

#ifndef CLK_TCK
#define CLK_TCK CLOCKS_PER_SEC
#endif

#include "SBlogOSUtils.h"

#define BUFSIZE (4096 + 1024) // typical maxlen is 4096, want room over that

// Convert wide to narrow characters
#define w2c(w) (((w) & 0xff00)?'\277':((unsigned char) ((w) & 0x00ff)))

/*****************************
// SBlogGetTime
*****************************/
extern "C" int SBlogGetTime(time_t *timestamp, 
			    VXIunsigned *timestampMsec)
{
#ifdef WIN32
  struct _timeb tbuf;
  _ftime(&tbuf);
  *timestamp = tbuf.time;
  *timestampMsec = (VXIunsigned) tbuf.millitm;
#else
  struct timeb tbuf;
  ftime(&tbuf);
  *timestamp = tbuf.time;
  *timestampMsec = (VXIunsigned) tbuf.millitm;
#endif

  return 0;
}


/*****************************
// SBlogGetTimeStampStr
*****************************/
extern "C" int SBlogGetTimeStampStr(time_t          timestamp,
				    VXIunsigned     timestampMsec,
				    char           *timestampStr)
{
  struct tm *gmt;
  gmt = gmtime(&timestamp);
  char *timeStr = asctime( gmt );

  if (timeStr) {
    strncpy(timestampStr, &timeStr[0], 3);
    sprintf(&timestampStr[3], ", %s", &timeStr[4]);
    strncpy(&timestampStr[12], &timeStr[20], 5);
    sprintf(&timestampStr[16], " %s", &timeStr[11]);
    sprintf(&timestampStr[25], ".%02u GMT", timestampMsec / 10);
  } else {
    timestampStr[0] = '\0';
    return -1;
  }

  return 0;
}

/*****************************
// SBlogGetFileStats
*****************************/
extern "C" int SBlogGetFileStats(const char *path, 
				 SBlogFileStats *fileStats)
{
  int rc;
  #ifdef WIN32
  struct _stat stats;
  #else
  struct stat stats;
  #endif

  if ((! path) || (! fileStats))
    return -1;
  
  #ifdef WIN32
  rc = _stat(path, &stats);
  #else
  rc = stat(path, &stats);
  #endif
  
  if (rc != 0) {
    return -1;
  }
  
  fileStats->st_size  = stats.st_size;
  fileStats->st_atim = stats.st_atime;
  fileStats->st_mtim = stats.st_mtime;
  fileStats->st_ctim = stats.st_ctime;
  
  return 0;
}

/*****************************
// SBlogGetCPUTimes
*****************************/
extern "C" int SBlogGetCPUTimes(long *userTime
				/* ms spent in user mode */,
				long *kernelTime
				/* ms spent in kernel mode*/
				)
{
#ifdef WIN32
  FILETIME dummy;
  FILETIME k, u;
  LARGE_INTEGER lk, lu;

  if ((! userTime) || (! kernelTime))
    return -1;

  if (GetThreadTimes(GetCurrentThread(), &dummy, &dummy, &k, &u) == FALSE)
    return -1;

  lk.LowPart  = k.dwLowDateTime;
  lk.HighPart = k.dwHighDateTime;
  *kernelTime = (long) (lk.QuadPart / 10000);

  lu.LowPart  = u.dwLowDateTime;
  lu.HighPart = u.dwHighDateTime;
  *userTime   = (long) (lu.QuadPart / 10000);


#else
  struct tms timeBuf;

  if ((! userTime) || (! kernelTime))
    return -1;

  times(&timeBuf);
  *userTime = (long)timeBuf.tms_utime * 1000 / CLK_TCK;
  *kernelTime = (long)timeBuf.tms_stime * 1000 / CLK_TCK;

#endif
  return 0;
}


/*****************************
// SBlogVswprintf
*****************************/
extern "C" int SBlogVswprintf(wchar_t* wcs, size_t maxlen,
			      const wchar_t* format, va_list args)
{
  int rc;

  if (maxlen < 1)
    return -1;
  wcs[0] = '\0';

#ifdef WIN32
  rc = _vsnwprintf(wcs, maxlen, format, args);
#else
  rc = vswprintf(wcs, maxlen, format, args);
#endif
  if ((size_t) rc >= maxlen - 1) /* overflow */
    wcs[maxlen - 1] = L'\0';

  return rc;
}

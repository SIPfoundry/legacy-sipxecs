//-< STDTP.H >-------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Standart type and macro definitions
//-------------------------------------------------------------------*--------*

#ifndef __STDTP_H__
#define __STDTP_H__

#ifdef FASTDB_DLL
#ifdef INSIDE_FASTDB
#define FASTDB_DLL_ENTRY __declspec(dllexport)
#else
#define FASTDB_DLL_ENTRY __declspec(dllimport)
#endif
#else
#define FASTDB_DLL_ENTRY
#endif

#ifdef _WIN32
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(disable:4800 4355 4146 4251)
#endif
#else
#ifdef _AIX
#define INT8_IS_DEFINED
#endif
#ifndef NO_PTHREADS
#ifndef _REENTRANT
#define _REENTRANT 
#endif
#endif
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stdarg.h>
#include <time.h>

#define DEBUG_NONE  0
#define DEBUG_CHECK 1
#define DEBUG_TRACE 2

#if FASTDB_DEBUG == DEBUG_TRACE
#define TRACE_MSG(x)  dbTrace x
#else
#define TRACE_MSG(x)
#endif

#ifndef EXTRA_DEBUG_NEW_PARAMS
#define EXTRA_DEBUG_NEW_PARAMS
#endif


#ifndef HAS_TEMPLATE_FRIENDS
#if !defined(_MSC_VER) || _MSC_VER >= 1300
#define HAS_TEMPLATE_FRIENDS
#endif
#endif

typedef void (*dbTraceFunctionPtr)(char* message);

extern dbTraceFunctionPtr dbTraceFunction;
extern FASTDB_DLL_ENTRY void dbTrace(char* message, ...);


#ifdef PHAR_LAP
#define PHAR_LAP 1
#endif

#ifdef __QNX__
#define USE_POSIX_API 1
#define POSIX_1003_1d 1
#endif

// Align value 'x' to boundary 'b' which should be power of 2
#define DOALIGN(x,b)   (((x) + (b) - 1) & ~((b) - 1))

typedef signed char    int1;
typedef unsigned char  nat1;

typedef signed short   int2;
typedef unsigned short nat2;

typedef signed int     int4;
typedef unsigned int   nat4;

typedef unsigned char  byte;

#if defined(_WIN32) && !defined(__MINGW32__)
typedef unsigned __int64 nat8;
typedef __int64          db_int8;
#if defined(__IBMCPP__)
#define INT8_FORMAT "%lld"
#else
#define INT8_FORMAT "%I64d"
#endif
#define CONST64(c)  c
#else
#if defined(__osf__ )
typedef unsigned long nat8;
typedef signed   long db_int8;
#define INT8_FORMAT "%ld"
#define CONST64(c)  c##L
#else
typedef unsigned long long nat8;
typedef signed   long long db_int8;
#define INT8_FORMAT "%lld"
#define CONST64(c)  c##LL
#endif
#endif

#if !defined(bool) && defined(__IBMCPP__)
#define bool  char
#define true  (1)
#define false (0)
#endif

#define nat8_low_part(x)  ((nat4)(x))
#define nat8_high_part(x) ((nat4)((nat8)(x)>>32))
#define int8_low_part(x)  ((int4)(x))
#define int8_high_part(x) ((int4)((db_int8)(x)>>32))
#define cons_nat8(hi, lo) ((((nat8)(hi)) << 32) | (nat4)(lo))
#define cons_int8(hi, lo) ((((db_int8)(hi)) << 32) | (nat4)(lo))
 
#define MAX_NAT8  nat8(-1)

#ifndef INT8_IS_DEFINED
typedef db_int8 int8;
#endif

typedef float  real4;
typedef double real8; 

#ifndef BIG_ENDIAN
#define BIG_ENDIAN      4321    /* most-significant byte first (IBM, net) */
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN   1234
#endif

#ifndef BYTE_ORDER
#if defined(__sparc__) || defined(__m68k__)
#define BYTE_ORDER      BIG_ENDIAN 
#else
#define BYTE_ORDER      LITTLE_ENDIAN
#endif
#endif

#ifdef _WIN32
typedef HANDLE descriptor_t; 
#else
typedef int descriptor_t; 
#endif

#if !defined(_fastcall) && (!defined(_WIN32) || defined(__IBMCPP__) || defined(__MINGW32__))
#define _fastcall
#endif

#if defined(_WIN32) || !defined(NO_PTHREADS)
#define THREADS_SUPPORTED 1
#else
#define THREADS_SUPPORTED 0
#endif

#define itemsof(array) (sizeof(array)/sizeof*(array))


extern FASTDB_DLL_ENTRY byte* dbMalloc(size_t size);
extern FASTDB_DLL_ENTRY void  dbFree(void* ptr);

#if defined(USE_SYSV_SHARED_MEMORY) && !defined(DISKLESS_CONFIGURATION)
#define DISKLESS_CONFIGURATION 1
#endif


#if !defined(_WIN32)
#define NO_STRICMP  1
#define NO_STRICOLL 1
#endif

#if defined(IGNORE_CASE) && defined(NO_STRICMP) 
#include <ctype.h>
inline int stricmp(const char* p, const char* q)
{
    while (toupper(*(unsigned char*)p) == toupper(*(unsigned char*)q)) { 
    if (*p == '\0') { 
        return 0;
    }
    p += 1;
    q += 1;
    }
    return toupper(*(unsigned char*)p) - toupper(*(unsigned char*)q);
}
#endif

#if defined(IGNORE_CASE) && defined(USE_LOCALE_SETTINGS) && defined(NO_STRICOLL) 
#include <ctype.h>
inline int stricoll(const char* p, const char* q)
{
    char   p_buf[256];
    char   q_buf[256];
    size_t p_len = strlen(p);
    size_t q_len = strlen(q);
    char*  p_dst = p_buf; 
    char*  q_dst = q_buf; 
    int    i;
    if (p_len >= sizeof(p_buf)) { 
        p_dst = new char[p_len+1];
    }
    if (q_len >= sizeof(q_buf)) { 
        q_dst = new char[q_len+1];
    }
    for (i = 0; p[i] != '\0'; i++) { 
        p_dst[i] = toupper(p[i] & 0xFF);
    }
    p_dst[i] = '\0';

    for (i = 0; q[i] != '\0'; i++) { 
        q_dst[i] = toupper(q[i] & 0xFF);
    }
    q_dst[i] = '\0';

    int diff = strcoll(p_dst, q_dst);
    if (p_dst != p_buf) { 
        delete[] p_dst;
    }
    if (q_dst != q_buf) { 
        delete[] q_dst;
    }
    return diff;
}
#endif

#endif





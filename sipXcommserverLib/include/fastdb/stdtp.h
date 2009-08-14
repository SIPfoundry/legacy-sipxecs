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

#include "config.h"

#ifdef FASTDB_DLL
#ifdef INSIDE_FASTDB
#define FASTDB_DLL_ENTRY __declspec(dllexport)
#else
#define FASTDB_DLL_ENTRY __declspec(dllimport)
#endif
#else
#define FASTDB_DLL_ENTRY
#endif

#ifdef USE_NAMESPACES
#define BEGIN_FASTDB_NAMESPACE namespace fastdb {
#define END_FASTDB_NAMESPACE }
#define USE_FASTDB_NAMESPACE using namespace fastdb;
#else
#define BEGIN_FASTDB_NAMESPACE
#define END_FASTDB_NAMESPACE
#define USE_FASTDB_NAMESPACE
#endif

#if defined(AUTO_DETECT_PROCESS_CRASH) && !defined(RECOVERABLE_CRITICAL_SECTION)
#define RECOVERABLE_CRITICAL_SECTION
#endif

#define GNUC_BEFORE(major,minor) (defined(__GNUC__) && (major > __GNUC__ || (major == __GNUC__ && minor > __GNUC_MINOR__)))

#if defined(__MINGW32__) && !defined(_WIN32)
#define _WIN32
#endif

#if defined(_WIN32_WCE) && !defined(_WINCE)
#define _WINCE
#endif

#if defined(__APPLE__) && !defined(__FreeBSD__)
// MAC OS X is Free BSD
#define __FreeBSD__ 5
#endif

#ifdef _WIN32

#ifdef USE_MFC
#include <afx.h>
#include <winsock2.h>
#else
#include <windows.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable:4800 4355 4146 4251 4996)
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

#if defined(__VACPP_MULTI__) // IBM compiler produce a lot of stupid warnings
#pragma report(disable, "CPPC1608")
#pragma report(disable, "CPPC1281")
#endif /* __VACPP_MULTI__ */


#ifdef _WINCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <tchar.h>
#include "wince.h"

#else

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
#include <malloc.h>
#endif

#endif

#ifdef PHAR_LAP
#define PHAR_LAP 1
#endif

#ifdef __QNX__
#define USE_POSIX_API 1
#define POSIX_1003_1d 1
#endif

#if defined(__MINGW32__) && !defined(_WIN32)
#define _WIN32
#endif

#if defined(__APPLE__) && !defined(__FreeBSD__)
// MAC OS X is Free BSD
#define __FreeBSD__ 4
#endif

#ifndef CLI_CALLBACK_CC /* CLI callbacks calling convention */
#define CLI_CALLBACK_CC
#endif

#define DEBUG_NONE            0
#define DEBUG_CHECK           1
#define DEBUG_TRACE_IMPORTANT 2
#define DEBUG_TRACE_ALL       3

#if FASTDB_DEBUG >= DEBUG_TRACE_IMPORTANT
#define TRACE_IMSG(x)  dbTrace x
#else
#define TRACE_IMSG(x)
#endif

#if FASTDB_DEBUG >= DEBUG_TRACE_ALL
#define TRACE_MSG(x)  dbTrace x
#else
#define TRACE_MSG(x)
#endif

#ifndef EXTRA_DEBUG_NEW_PARAMS
#define EXTRA_DEBUG_NEW_PARAMS
#endif


#ifndef HAS_TEMPLATE_FRIENDS
#if (!defined(_MSC_VER) || _MSC_VER >= 1300) && !defined(NO_MEMBER_TEMPLATES)
#define HAS_TEMPLATE_FRIENDS
#endif
#endif

#ifndef __IBMCPP__
BEGIN_FASTDB_NAMESPACE
#endif

// User defined trace functions
typedef void (CLI_CALLBACK_CC *dbTraceFunctionPtr)(char* message);

// Pointer to trace function (default implementation just prints message to stderr)
extern FASTDB_DLL_ENTRY dbTraceFunctionPtr dbTraceFunction;

extern FASTDB_DLL_ENTRY void dbTrace(char* message, ...);

#if defined(_WINCE) || defined(UNICODE)
inline wchar_t* convertStringToUnicode(wchar_t* dst, char const* src, size_t size) {
	mbstowcs(dst, src, size);
	return dst;
}
#define CNV_BUF_SIZE 256
#define W32_STRING(s) convertStringToUnicode((wchar_t*)alloca(sizeof(wchar_t)*CNV_BUF_SIZE), s, CNV_BUF_SIZE)
#else
#define W32_STRING(s) s
#endif


// Align value 'x' to boundary 'b' which should be power of 2
#define DOALIGN(x,b)   (((x) + (b) - 1) & ~((b) - 1))

typedef signed char    db_int1;
typedef unsigned char  db_nat1;

typedef signed short   db_int2;
typedef unsigned short db_nat2;

typedef signed int     db_int4;
typedef unsigned int   db_nat4;

typedef unsigned char  db_byte;

// This mapping is done for compatibility reasons
typedef db_int1 int1;
typedef db_nat1 nat1;
typedef db_int2 int2;
typedef db_nat2 nat2;
typedef db_int4 int4;
typedef db_nat4 nat4;

typedef db_byte byte;

#if defined(_WIN32) && !defined(__MINGW32__)
typedef unsigned __int64 db_nat8;
typedef __int64          db_int8;
#if defined(__IBMCPP__)
#define INT8_FORMAT "%lld"
#else
#define INT8_FORMAT "%I64d"
#endif
#define CONST64(c)  c
#else
#if defined(__osf__ )
typedef unsigned long db_nat8;
typedef signed   long db_int8;
#define INT8_FORMAT "%ld"
#define CONST64(c)  c##L
#ifndef SIZEOF_LONG
#define SIZEOF_LONG 8
#endif
#else
typedef unsigned long long db_nat8;
typedef signed   long long db_int8;
#if defined(__MINGW32__) && __GNUG__<3
#define INT8_FORMAT "%I64d"
#else
#define INT8_FORMAT "%lld"
#endif
#define CONST64(c)  c##LL
#endif
#endif

// Starting from which version IBM C++ compiler support bool type?
//#if !defined(bool) && ((defined(__SUNPRO_CC) && __SUNPRO_CC_COMPAT < 5) || defined(__IBMCPP__))
#if !defined(bool) && (defined(__SUNPRO_CC) && __SUNPRO_CC_COMPAT < 5)
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
typedef db_nat8 nat8;

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

// Enable or disable trace at runtime
extern FASTDB_DLL_ENTRY bool dbTraceEnable;

extern FASTDB_DLL_ENTRY byte* dbMalloc(size_t size);
extern FASTDB_DLL_ENTRY void  dbFree(void* ptr);

#if defined(FUZZY_CHECKPOINT) && !defined(NO_MMAP)
#define NO_MMAP 1
#endif

#if !defined(USE_POSIX_MMAP) && defined(DISKLESS_CONFIGURATION)
// To enable sharing of database in diskless configuration we have to use
// shmap instead of mmap, which anonymous region is private to process.
#define USE_POSIX_MMAP 0
#endif

#if defined(USE_POSIX_MMAP) && !USE_POSIX_MMAP
// allocate space for the database using Sys-V shmat
#define USE_SYSV_SHARED_MEMORY 1
#if !defined(NO_MMAP) && !defined(DISKLESS_CONFIGURATION)
#define NO_MMAP 1
#endif
#endif

#if !defined(_WIN32)
#define NO_STRICMP  1
#define NO_STRICOLL 1
#endif


#if defined(_WINCE) && defined(USE_LOCALE_SETTINGS)
#undef USE_LOCALE_SETTINGS
#endif

#if defined(IGNORE_CASE) && defined(NO_STRICMP)
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
#else
#ifdef _WIN32
#define stricoll(x,y) _stricoll(x,y)
#endif
#endif

#ifndef STRINCMP_DEFINED
inline int strincmp(const char* p, const char* q, size_t n)
{
    while (n > 0) {
        int diff = toupper(*(unsigned char*)p) - toupper(*(unsigned char*)q);
        if (diff != 0) {
            return diff;
        } else if (*p == '\0') {
            return 0;
        }
        p += 1;
        q += 1;
        n -= 1;
    }
    return 0;
}
#endif

#if defined(IGNORE_CASE) && defined(USE_LOCALE_SETTINGS) && defined(NO_STRICOLL)
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

#ifndef __IBMCPP__
END_FASTDB_NAMESPACE
#endif

#endif

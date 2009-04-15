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
 * Settings which should start all public headers
 *
 ************************************************************************
 */

/* (1) Platform specific macro which handles symbol exports & imports */

#ifndef _VXIHEADERPREFIX_ONE_TIME_
#ifdef WIN32
  #ifdef __cplusplus
    #define SYMBOL_EXPORT_DECL extern "C" __declspec(dllexport)
    #define SYMBOL_IMPORT_DECL extern "C" __declspec(dllimport)
    #define SYMBOL_EXPORT_CPP_DECL extern __declspec(dllexport)
    #define SYMBOL_IMPORT_CPP_DECL extern __declspec(dllimport)
    #define SYMBOL_EXPORT_CPP_CLASS_DECL __declspec(dllexport)
    #define SYMBOL_IMPORT_CPP_CLASS_DECL __declspec(dllimport)
  #else
    #define SYMBOL_EXPORT_DECL __declspec(dllexport)
    #define SYMBOL_IMPORT_DECL __declspec(dllimport)
  #endif
#else
  #ifdef __cplusplus
    #define SYMBOL_EXPORT_DECL extern "C"
    #define SYMBOL_IMPORT_DECL extern "C"
    #define SYMBOL_EXPORT_CPP_DECL extern
    #define SYMBOL_IMPORT_CPP_DECL extern
    #define SYMBOL_EXPORT_CPP_CLASS_DECL
    #define SYMBOL_IMPORT_CPP_CLASS_DECL
  #else
    #define SYMBOL_EXPORT_DECL extern
    #define SYMBOL_IMPORT_DECL extern
  #endif
#endif

#if !defined(SYMBOL_EXPORT_DECL) || !defined(SYMBOL_IMPORT_DECL)
#error Symbol import/export pair not defined.
#endif
#endif /* end of (1) */


/* Define structure packing conventions */

#ifdef WIN32
#if defined(_MSC_VER)            /* Microsoft Visual C++ */
  #pragma pack(push, 8)
#elif defined(__BORLANDC__)      /* Borland C++ */
  #pragma option -a8
#elif defined(__WATCOMC__)       /* Watcom C++ */
  #pragma pack(push, 8)
#else                            /* Anything else */
  #error Review the settings for your compiler.
#endif

/* Do other (one time) compiler specific work */

#ifndef _VXIHEADERPREFIX_ONE_TIME_
  #if defined(_MSC_VER)
    #include "VXIcompilerMsvc.h"
  #endif
#endif

#endif /*win32*/

/* Define compiler guard for one-time instructions */

#if !defined(_VXIHEADERPREFIX_ONE_TIME_)
#define _VXIHEADERPREFIX_ONE_TIME_
#endif

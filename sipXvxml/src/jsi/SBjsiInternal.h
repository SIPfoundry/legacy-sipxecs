/*****************************************************************************
 *****************************************************************************
 *
 * 
 *
 * SBjsiInternal.h, compiler/platform specific definitions
 *
 * This header is used to include standard compiler/platform specific
 * definitions, this is included by all SBjsi source files.
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

#ifndef _SBjsi_INTERNAL_H__
#define _SBjsi_INTERNAL_H__

#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifdef _MSC_VER           /* Microsoft Visual C++ */
#ifdef __cplusplus

namespace std { };
using namespace std;

#endif /* __cplusplus */
#endif /* _MSC_VER */

#endif /* _SBjsi_INTERNAL_H__ */

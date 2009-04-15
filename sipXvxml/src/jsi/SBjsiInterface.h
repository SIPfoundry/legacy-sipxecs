/*****************************************************************************
 *****************************************************************************
 *
 * 
 *
 * SBjsiInterface, definition of the real SBjsi resource object
 *
 * The SBjsiInterface object defines the SB implementation of a
 * VXIjsi resource that is used to provide JavaScript services to
 * a channel/thread.
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

/* -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8
 */

#ifndef _SBJSI_INTERFACE_H__
#define _SBJSI_INTERFACE_H__

#include "VXIjsi.h"            /* For VXIjsiInterface and VXIjsiResult codes */

#ifdef __cplusplus
class JsiRuntime;
extern "C" {
#else
typedef struct JsiRuntime { void * dummy; } JsiRuntime;
#endif

struct VXIlogInterface;

/* SBjsi interface, "inherits" from VXIjsiInterface */
typedef struct SBjsiInterface
{
  /* Base interface, must be the first member */
  VXIjsiInterface jsi;

  /* Context size in byts for each new context for this resource (currently
   * shared across the entire process) 
   */
  long contextSize;

  /* Maximum number of JavaScript branches for each JavaScript
   * evaluation for this resource (currently shared across the entire
   * process) 
   */
  long maxBranches;

  /* Logging interface for this resource */
  VXIlogInterface *log;
  
  /* Offset for diagnostic logging */
  VXIunsigned diagTagBase;

  /* JavaScript runtime environment for this resource */
  JsiRuntime *jsiRuntime;

} SBjsiInterface;

#ifdef __cplusplus
}
#endif

#endif  /* _SBJSI_INTERFACE_H__ */

//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifdef _VXWORKS /* [ */

/* SYSTEM INCLUDES */
#include <stdio.h>
#include <taskLib.h>
#include <sysLib.h>

/* APPLICATION INCLUDES */
#include "os/OsAssert.h"

#undef BUILDING_FOR_TORNADO_2_2
#if   CPU == ARMSA110 /* Tornado 2.0 */
#undef BUILDING_FOR_TORNADO_2_2
#elif (CPU == ARMARCH4) || (CPU == STRONGARM)  /* Tornado 2.0.1 */
#undef BUILDING_FOR_TORNADO_2_2
#elif (CPU == XSCALE)  /* Tornado 2.2.0 */
#define BUILDING_FOR_TORNADO_2_2
#else
#error Unexpected CPU value
#endif

#undef BUILDING_FOR_TORNADO_2_2

/* EXTERNAL FUNCTIONS */
/* EXTERNAL VARIABLES */
/* CONSTANTS */
/* STATIC VARIABLE INITIALIZATIONS */

static volatile int actionOnAssert = OS_ASSERT_SUSPEND;

/* FUNCTIONS */

/****************************************************************************
 * This is an improved version of the corresponding VxWorks function.
 * Note: (rschaaf) 09/06/99
 *       This function is defined in the top-level directory of the
 *       os library (.../src/os) rather than in the VxWorks specific
 *       subdirectory (.../src/os/vxw) to make it easier to build a
 *       version of the VxWorks kernel that uses the improved __assert()
 *       function and optionally allows the Pinger application to be linked
 *       in with the kernel.
 *
 *       The somewhat complicated for doing this is described below.
 *
 *       1) Building the VxWorks kernel presently entails linking it with a
 *          number of library files that are mostly specific to the Pinger
 *          application.  When the Pinger application is not being linked
 *          with the kernel, these application-specific libraries are largely
 *          ignored.  The one exception is the definition of the __assert()
 *          function.
 *       2) The os library is built in such a way that all of the
 *          platform-specific definitions (e.g., for VxWorks, all of
 *          src/os/vxw/?*.o) are combined into a single object module.
 *
 *       If the __assert() function were defined in the VxWorks object
 *       module, linking with the kernel would cause all of the other
 *       definitions in that object module (and their unresolved references)
 *       to also be pulled in.  The unresolved references, in turn, would
 *       cause more stuff from other application-specific libraries to be
 *       included in the VxWorks kernel.
 *
 *       We avoid this by defining the __assert() function in its own source
 *       file in the .../src/os directory.  It can therefore be included
 *       independently of the other application-specific portions of the
 *       VxWorks support for the OS abstraction layer.
 ***************************************************************************/

static int numberOfAsserts = 0;

#ifdef BUILDING_FOR_TORNADO_2_2 /* [ */
static volatile int* dataAbort = (volatile int*) 0x09000000;
static int lastAssertTask = 0;
#endif /* BUILDING_FOR_TORNADO_2_2 ] */

int assertCount(void) {return numberOfAsserts;}

void __takeAssertAction(int secondsToWait)
{
   int Hz = sysClkRateGet();

   if (OS_ASSERT_REBOOT == actionOnAssert) {
      while ((OS_ASSERT_REBOOT == actionOnAssert) && (secondsToWait-- > 0)) {
         taskDelay(Hz); /* wait one second */
      }
      if (OS_ASSERT_REBOOT == actionOnAssert) {
         sysToMonitor(0);
      }
   }
}

void __assert(const char *msg)
{

/* The user can set the PHONESET_ASSERT_ACTION to RESTART or SUSPEND
 * the default (which is set in initConfigDb) is to restart
 */

   taskLock();
   numberOfAsserts++;
#ifdef BUILDING_FOR_TORNADO_2_2 /* [ */
   lastAssertTask = taskIdSelf();
#endif /* BUILDING_FOR_TORNADO_2_2 ] */
   taskUnlock();

   fdprintf(2, "\n\n(Task: '%s', TaskId: 0x%X)\n  %s\n",
            taskName(0), taskIdSelf(), msg);

   __takeAssertAction(60);

#ifdef BUILDING_FOR_TORNADO_2_2 /* [ */
   {
      int iiii;
      iiii = *dataAbort;
   }
#else /* BUILDING_FOR_TORNADO_2_2 ] [ */
   taskSuspend(taskIdSelf());
#endif /* BUILDING_FOR_TORNADO_2_2 ] */
   fdprintf(2, "\n *** RESUMING *** (Task: '%s', TaskId: 0x%X)  %s\n"
      "   Good Luck, you're probably gonna need it!!!!",
      taskName(0), taskIdSelf(), msg);
}

int OsAssert_SetFailureAction(int action)
{
   int save = actionOnAssert;

   if (OS_ASSERT_ALWAYS_SUSPEND == actionOnAssert) return save;

   if ((OS_ASSERT_REBOOT == action)
    || (OS_ASSERT_SUSPEND == action)
    || (OS_ASSERT_ALWAYS_SUSPEND == action)) {
      actionOnAssert = action;
   } else {
      actionOnAssert = OS_ASSERT_SUSPEND;
      __assert(
        "\n\n ** WARNING:  Invalid argument to OsAssert_SetFailureAction()\n");
   }
   return save;
}

int NoAssertReboot() /* Console Commands */
{
   return OsAssert_SetFailureAction(OS_ASSERT_ALWAYS_SUSPEND);
}

int NAR() {return NoAssertReboot();}

#ifdef BUILDING_FOR_TORNADO_2_2 /* [ */
int bumpAssertPC(int tid, int resume)
{
   STATUS stat;
   REG_SET  Regs;
   REG_SET* pRegs = &Regs;

   if (0 != (tid & 3)) {
      if (0 != lastAssertTask) {
         printf("most recently asserted task is 0x%X ('%s')\n",
            lastAssertTask, taskName(lastAssertTask));
      } else {
         printf("no recently asserted task\n");
      }
      return lastAssertTask;
   }
   if (0 == tid) tid = lastAssertTask;
   if (VX_OK == (stat = taskRegsGet(tid, pRegs))) {
      (int) (Regs.pc) = (int) (Regs.pc) + 4;
      stat = taskRegsSet(tid, pRegs);
      if (resume) {
         stat = taskResume(tid);
         if (lastAssertTask == tid) lastAssertTask = 0;
      }
   } else {
      printf("taskRegs(0x%0x, 0x%0x) returned %d!\n", tid, (int) pRegs, stat);
   }
   return stat;
}

int bAP(int tid, int resume) {return bumpAssertPC(tid, resume);}
int bAPC(int tid, int resume) {return bumpAssertPC(tid, resume);}
#endif /* BUILDING_FOR_TORNADO_2_2 ] */


#endif /* _VXWORKS ] */

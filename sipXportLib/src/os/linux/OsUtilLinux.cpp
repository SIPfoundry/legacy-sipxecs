//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <sys/time.h>
#include <signal.h>
#include <os/linux/OsTaskLinux.h>

// APPLICATION INCLUDES
#include "os/linux/OsUtilLinux.h"
#include "os/OsDateTime.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
const int NSECS_PER_SEC = 1000000000;
const int NSECS_PER_USEC = 1000;

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

// Convert an abstraction layer task priority to a Linux task priority
int OsUtilLinux::cvtOsPrioToLinuxPrio(const int osPrio)
{
   int linuxPrio = 100; // set out of bounds to catch osPrio errors

   // The mapping flips the priorities upside down, and scales them.
   //
   // The OS abstraction layer allows task priorities to range from 0
   // to 255, where zero is the highest priority and 255 is the lowest.
   // Linux priorities range from 1 to 99 (for realtime processes) and
   // 1 is the lowest while 99 is the highest.

   //linuxPrio = (255 - osPrio) * 98 / 255 + 1;
   if(osPrio == 0)
   {
      linuxPrio = OsTaskLinux::RT_HIGHEST;
   }
   else if(osPrio >= 1 && osPrio <= 5)
   {
      linuxPrio = OsTaskLinux::RT_HIGH;
   }
   else if(osPrio >= 6 && osPrio <=10)
   {
      linuxPrio = OsTaskLinux::RT_NORMAL;
   }
   else if(osPrio >= 11 && osPrio <=100)
   {
      linuxPrio = OsTaskLinux::RT_LOW;
   }
   else if(osPrio >= 101 && osPrio <= 255)
   {
      linuxPrio = OsTaskLinux::RT_NO;
   }

   assert(linuxPrio >= 0 && linuxPrio <= 99);

   return linuxPrio;
}

// Convert a Linux task priority to an abstraction layer task priority
int OsUtilLinux::cvtLinuxPrioToOsPrio(const int linuxPrio)
{
   int osPrio;
   // The mapping flips the priorities upside down, and scales them.
   //
   // The OS abstraction layer allows task priorities to range from 0
   // to 255, where zero is the highest priority and 255 is the lowest.
   // Linux priorities range from 1 to 99 (for realtime processes) and
   // 1 is the lowest while 99 is the highest.

   //osPrio = 255 - (linuxPrio - 1) * 255 / 98;
   switch(linuxPrio)
   {
      case OsTaskLinux::RT_HIGHEST:
           osPrio = 0;
           break;

      case OsTaskLinux::RT_HIGH:
           osPrio = 5;
           break;

      case OsTaskLinux::RT_NORMAL:
           osPrio = 10;
           break;

      case OsTaskLinux::RT_LOW:
           osPrio = 100;
           break;

      default:
           osPrio = 128;
           break;
   }

   assert(osPrio >= 0 && osPrio <= 255);

   return osPrio;
}

// Convert an OsTime class relative to the current time to a struct
// timespec relative to epoch
void OsUtilLinux::cvtOsTimeToTimespec(OsTime time1, struct timespec * time2)
{
   struct timeval now;
   time2->tv_sec = time1.seconds();
   time2->tv_nsec = time1.usecs() * NSECS_PER_USEC;
   gettimeofday(&now, NULL);
   time2->tv_sec += now.tv_sec;
   time2->tv_nsec += now.tv_usec * NSECS_PER_USEC;
   if(time2->tv_nsec >= NSECS_PER_SEC)
   {
           time2->tv_nsec -= NSECS_PER_SEC;
           time2->tv_sec++;
   }
}

/**
 * This is a replacement for signal() which registers a signal handler but sets
 * a flag causing system calls ( namely read() or getchar() ) not to bail out
 * upon recepit of that signal. We need this behavior, so we must call
 * sigaction() manually.
 */
sighandler_t OsUtilLinux::signal(int signum, sighandler_t handler)
{
    struct sigaction action[2];
    action[0].sa_handler = handler;
    sigemptyset(&action[0].sa_mask);
    action[0].sa_flags = 0;
    sigaction(signum, &action[0], &action[1]);
    return action[1].sa_handler;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

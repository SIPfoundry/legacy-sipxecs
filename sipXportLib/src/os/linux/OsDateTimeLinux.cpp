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
#include <time.h>
#include <sys/time.h>

// APPLICATION INCLUDES
#include "os/OsTime.h"
#include "os/linux/OsLinuxDefs.h"
#include "os/linux/OsDateTimeLinux.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const int MICROSECS_PER_SEC = 1000000;
static double sSecondsSinceBoot = 0; // First call to get seconds since boot and store the number here.
static double sSecondsFirstCall = 0; // Store the seconds since Epoch at first call to get seconds since boot
                                      // and just call time() to calculate the seconds since boot, rather than
                                      // to read the /proc/ everytime.

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Default constructor
OsDateTimeLinux::OsDateTimeLinux()
 : OsDateTimeBase()
{
   // no further work required
   //  all of the work is done by the parent constructor
}

// Constructor
OsDateTimeLinux::OsDateTimeLinux(const unsigned short year,
                             const unsigned char  month,
                             const unsigned char  day,
                             const unsigned char  hour,
                             const unsigned char  minute,
                             const unsigned char  second,
                             const unsigned int   microsecond)
 : OsDateTimeBase(year, month, day, hour, minute, second, microsecond)
{
   // no further work required
   //  all of the work is done by the parent constructor
}

// Copy constructor
OsDateTimeLinux::OsDateTimeLinux(const OsDateTimeLinux& rOsDateTimeLinux)
 : OsDateTimeBase((const OsDateTimeBase&) rOsDateTimeLinux)
{
   // no further work required
   //  all of the work is done by the parent copy constructor
}

/// Convert an OsTime to an OsDateTime
OsDateTimeLinux::OsDateTimeLinux(const OsTime& toTime)
{
   struct tm dateTime;
   time_t seconds = toTime.seconds();
   gmtime_r(&seconds, &dateTime);

   mYear        = 1900 + dateTime.tm_year;
   mMonth       = (unsigned char) dateTime.tm_mon;
   mDay         = (unsigned char) dateTime.tm_mday;
   mHour        = (unsigned char) dateTime.tm_hour;
   mMinute      = (unsigned char) dateTime.tm_min;
   mSecond      = (unsigned char) dateTime.tm_sec;
   mMicrosecond = toTime.usecs();
}


// Destructor
OsDateTimeLinux::~OsDateTimeLinux()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsDateTimeLinux&
OsDateTimeLinux::operator=(const OsDateTimeLinux& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsDateTimeBase::operator=((const OsDateTimeBase&) rhs);
   // no further work required
   //  all of the "heavy lifting" is done by the parent assignment operator

   return *this;
}

// Set the system time -- this is a no-op under Linux for now.
void OsDateTimeLinux::setTime(struct timespec gmt, int tzOffsetSecs,
                            DstRule dstRule)
{
   // no-op for now
}

// Set the system timezone -- this is a no-op under Linux for now.
void OsDateTimeLinux::setTimeZone(int tzOffsetSecs, DstRule dstRule)
{
   // no-op for now
}

/* ============================ ACCESSORS ================================= */

// Convert the OsDateTimeBase value to an OsTime value
// The OsTime value is relative to when the system was booted.
OsStatus OsDateTimeLinux::cvtToTimeSinceBoot(OsTime& rTime) const
{
   unsigned long curTimeAsSecsSinceBoot;
   time_t        curTimeAsTimeT;
   struct tm     thisTime;
   long          thisTimeAsSecsSinceBoot;
   time_t        thisTimeAsTimeT;

   // convert "this" OsDateTime to a time_t representation
   thisTime.tm_year = mYear - 1900;
   thisTime.tm_mon  = mMonth;
   thisTime.tm_mday = mDay;
   thisTime.tm_hour = mHour;
   thisTime.tm_min  = mMinute;
   thisTime.tm_sec  = mSecond;
   thisTimeAsTimeT  = mktime(&thisTime);
   assert(thisTimeAsTimeT >= 0);

   // get the current time as a time_t
   curTimeAsTimeT   = time(NULL);
   assert(curTimeAsTimeT >= 0);

   // get the current time as seconds since boot
   curTimeAsSecsSinceBoot = (unsigned long)secondsSinceBoot();

   // convert "this" time to seconds since boot
   thisTimeAsSecsSinceBoot = (thisTimeAsTimeT - curTimeAsTimeT) +
                             curTimeAsSecsSinceBoot;

   OsTime deltaOsTime(thisTimeAsSecsSinceBoot, 0);
   rTime = deltaOsTime;

   return OS_SUCCESS;
}

// Return the current time as an OsTime value
void OsDateTimeLinux::getCurTime(OsTime& rTime)
{
   struct timeval theTime;
   gettimeofday( &theTime, NULL );

   OsTime curTime(theTime.tv_sec, theTime.tv_usec);

   rTime = curTime;
}

// Return the current time as an OsDateTime value
void OsDateTimeLinux::getCurTime(OsDateTimeLinux& rDateTime)
{
   struct timeval theTime;
   gettimeofday( &theTime, NULL );

   struct tm curDateTime;
   gmtime_r(&theTime.tv_sec, &curDateTime);

   rDateTime.mYear        = 1900 + curDateTime.tm_year;
   rDateTime.mMonth       = (unsigned char) curDateTime.tm_mon;
   rDateTime.mDay         = (unsigned char) curDateTime.tm_mday;
   rDateTime.mHour        = (unsigned char) curDateTime.tm_hour;
   rDateTime.mMinute      = (unsigned char) curDateTime.tm_min;
   rDateTime.mSecond      = (unsigned char) curDateTime.tm_sec;
   rDateTime.mMicrosecond = theTime.tv_usec;
}

// Return the current time as an OsTime value
// The OsTime value is relative to when the system was booted.
void OsDateTimeLinux::getCurTimeSinceBoot(OsTime& rTime)
{
   double seconds;
   int secs;
   int usecs;

   seconds = secondsSinceBoot();
   secs  = (int)seconds;
   usecs = (int)((seconds - secs) * MICROSECS_PER_SEC);

   OsTime timeSinceBoot(secs, usecs);
   rTime = timeSinceBoot;
}

// Current time as the number of seconds since midnight (0 hour) 01/01/1970
unsigned long OsDateTimeLinux::getSecsSinceEpoch(void)
{
   return (unsigned long)time(NULL);
}

// Get the number of seconds since boot using /proc/uptime
// This is somewhat of a cheezy hack but there doesn't seem to be a system
// call to do it. It will work on all Linux systems with the /proc filesystem
// enabled, which is just about every one of them.
double OsDateTimeLinux::secondsSinceBoot(void)
{
   double seconds = 0;   // default to 0 if we can't open the file
   OsTime curTime(time(NULL), 0);

   if (sSecondsSinceBoot != 0)
   {
        seconds = curTime.seconds() - sSecondsFirstCall + sSecondsSinceBoot;
   }
   else
   {
        sSecondsFirstCall = curTime.seconds();
        FILE * proc;
        proc = fopen("/proc/uptime","r");
        if(proc != NULL)
        {
           fscanf(proc, "%lf", &seconds);
           fclose(proc);
        }
        sSecondsSinceBoot = seconds;
   }
   return seconds;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

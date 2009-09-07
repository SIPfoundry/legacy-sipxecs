//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsTime.h"

// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
const OsTime OsTime::OS_INFINITY(0x7FFFFFFF,0x7FFFFFFF);
const OsTime OsTime::NO_WAIT(0,0);
const int OsTime::MSECS_PER_SEC   = 1000;
const int OsTime::USECS_PER_MSEC  = 1000;
const int OsTime::USECS_PER_SEC   = 1000000;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor (creates a zero duration interval)
OsTime::OsTime() :
   mSeconds(0),
   mUsecs(0)
{
}

// Constructor
OsTime::OsTime(const long msecs)
{
   // Calculate the number of seconds and microseconds
   mSeconds =  msecs / MSECS_PER_SEC;
   mUsecs   = (msecs % MSECS_PER_SEC) * USECS_PER_MSEC;

   // Make sure mUsecs >= 0, which might not happen if msecs < 0
   // and "/" is a "Fortran-style" divide.
   if (mUsecs < 0)
   {
      mSeconds -= 1;
      mUsecs   += USECS_PER_SEC;
   }
}

// Constructor
OsTime::OsTime(const long seconds, const long usecs)
{
   mSeconds = seconds;

   // if necessary, adjust seconds and usecs so that the following holds:
   //    0 <= usecs <= (USECS_PER_SEC-1)
   if (usecs < 0 || usecs >= USECS_PER_SEC)
   {
      mSeconds += usecs / USECS_PER_SEC;
      mUsecs   =  usecs % USECS_PER_SEC;

      if (usecs < 0)
      {
         mSeconds -= 1;
         mUsecs   += USECS_PER_SEC;
      }
   }
   else     // 0 <= usecs < USECS_PER_SEC
   {
      mUsecs = usecs;
   }
}

// Copy constructor
OsTime::OsTime(const OsTime& rOsTime)
{
   mSeconds = rOsTime.mSeconds;
   mUsecs   = rOsTime.mUsecs;
}

// Destructor
OsTime::~OsTime()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsTime&
OsTime::operator=(const OsTime& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   mSeconds = rhs.mSeconds;
   mUsecs   = rhs.mUsecs;

   return *this;
}

// Addition operator
OsTime OsTime::operator+(const OsTime& rhs)
{
   OsTime sum(this->mSeconds + rhs.mSeconds,
              this->mUsecs   + rhs.mUsecs);
   // Adjust for overflow.
   if (sum.mUsecs >= USECS_PER_SEC)
   {
      sum.mSeconds += 1;
      sum.mUsecs   -= USECS_PER_SEC;
   }
   return sum;
}

// Subtraction operator
OsTime OsTime::operator-(const OsTime& rhs)
{
   OsTime diff(this->mSeconds - rhs.mSeconds,
               this->mUsecs   - rhs.mUsecs);
   // Adjust for underflow.
   if (diff.mUsecs < 0)
   {
      diff.mSeconds -= 1;
      diff.mUsecs   += USECS_PER_SEC;
   }
   return diff;
}

// Increment operator
OsTime OsTime::operator+=(const OsTime& rhs)
{
   *this = *this + rhs;
   return *this;
}

// Decrement operator
OsTime OsTime::operator-=(const OsTime& rhs)
{
   *this = *this - rhs;
   return *this;
}

// Test for equality operator
bool OsTime::operator==(const OsTime& rhs) const
{
   return (this->mSeconds == rhs.mSeconds) &&
          (this->mUsecs   == rhs.mUsecs);
}

// Test for inequality operator
bool OsTime::operator!=(const OsTime& rhs) const
{
   return (this->mSeconds != rhs.mSeconds) ||
          (this->mUsecs   != rhs.mUsecs);
}

// Test for greater than
bool OsTime::operator>(const OsTime& rhs) const
{
   return
      this->mSeconds > rhs.mSeconds ||
      (this->mSeconds == rhs.mSeconds &&
       this->mUsecs > rhs.mUsecs);
}

// Test for greater than or equal
bool OsTime::operator>=(const OsTime& rhs) const
{
   return
      this->mSeconds > rhs.mSeconds ||
      (this->mSeconds == rhs.mSeconds &&
       this->mUsecs >= rhs.mUsecs);
}

// Test for less than
bool OsTime::operator<(const OsTime& rhs) const
{
   return
      this->mSeconds < rhs.mSeconds ||
      (this->mSeconds == rhs.mSeconds &&
       this->mUsecs < rhs.mUsecs);
}

// Test for less than or equal
bool OsTime::operator<=(const OsTime& rhs) const
{
   return
      this->mSeconds < rhs.mSeconds ||
      (this->mSeconds == rhs.mSeconds &&
       this->mUsecs <= rhs.mUsecs);
}

/* ============================ ACCESSORS ================================= */

// Convert the time interval to milliseconds
int OsTime::cvtToMsecs(void) const
{
   return (mSeconds * MSECS_PER_SEC) + (mUsecs / USECS_PER_MSEC);
}

/* ============================ INQUIRY =================================== */


// Return TRUE if the time interval is zero (no wait)
UtlBoolean OsTime::isNoWait(void) const
{
   if (this == &NO_WAIT ||
       (seconds() == 0 && usecs() == 0))
      return TRUE;
   else
      return FALSE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

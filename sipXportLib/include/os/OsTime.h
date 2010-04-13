//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsTime_h_
#define _OsTime_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "utl/UtlDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Time or time interval
// If necessary, this class will adjust the seconds and microseconds values
// that it reports such that 0 <= microseconds < USECS_PER_SEC.


class OsTime
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const OsTime OS_INFINITY;
   static const OsTime NO_WAIT;
   static const int MSECS_PER_SEC ;
   static const int USECS_PER_MSEC ;
   static const int USECS_PER_SEC ;

/* ============================ CREATORS ================================== */

   OsTime();
     //:Default constructor (creates a zero duration interval)

   OsTime(const long msecs);
     //:Constructor specifying time/duration in terms of milliseconds

   OsTime(const long seconds, const long usecs);
     //:Constructor specifying time/duration in terms of seconds and microseconds

   OsTime(const OsTime& rOsTime);
     //:Copy constructor

   virtual
   ~OsTime();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsTime& operator=(const OsTime& rhs);
     //:Assignment operator

   OsTime operator+(const OsTime& rhs);
     //:Addition operator

   OsTime operator-(const OsTime& rhs);
     //:Subtraction operator

   OsTime operator+=(const OsTime& rhs);
     //:Increment operator

   OsTime operator-=(const OsTime& rhs);
     //:Decrement operator

   bool operator==(const OsTime& rhs) const;
     //:Test for equality operator

   bool operator!=(const OsTime& rhs) const;
     //:Test for inequality operator

   bool operator>(const OsTime& rhs) const;
     //:Test for greater than

   bool operator>=(const OsTime& rhs) const;
     //:Test for greater than or equal

   bool operator<(const OsTime& rhs) const;
     //:Test for less than

   bool operator<=(const OsTime& rhs) const;
     //:Test for less than or equal

/* ============================ ACCESSORS ================================= */

   virtual int seconds(void) const
   {
      return mSeconds;
   }
   //:Return the seconds portion of the time interval

   virtual int usecs(void) const
   {
      return mUsecs;
   }
   //:Return the microseconds portion of the time interval

   virtual int cvtToMsecs(void) const;
     //:Convert the time interval to milliseconds

/* ============================ INQUIRY =================================== */

   // Return TRUE if the time interval is infinite
   // Method inlined as it is called about 40000 times per call.
   inline UtlBoolean isInfinite(void) const
   {
      return (this == &OS_INFINITY ||
              (seconds() == OS_INFINITY.seconds() &&
               usecs()   == OS_INFINITY.usecs()));
   }

   virtual UtlBoolean isNoWait(void) const;
     //:Return TRUE if the time interval is zero (no wait)

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   // The time interval denoted is (mSeconds + mUsecs/1,000,000) seconds.
   // This is effectively two's complement notation.
   int mSeconds;               /* May be negative, if the time
                                 * interval is negative */
   int mUsecs;                 /* 0 <= mUsecs < USECS_PER_SEC,
                                 * even if mSeconds < 0 */

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsTime_h_

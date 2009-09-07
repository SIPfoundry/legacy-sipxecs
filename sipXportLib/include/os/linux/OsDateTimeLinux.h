//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _OsDateTimeLinux_h_
#define _OsDateTimeLinux_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsDateTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:DateTime management functions for Linux
class OsDateTimeLinux : public OsDateTimeBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsDateTimeLinux();
     //:Default constructor

   OsDateTimeLinux(const unsigned short year, ///< 4-digit year
                 const unsigned char  month,
                 const unsigned char  day,
                 const unsigned char  hour,
                 const unsigned char  minute,
                 const unsigned char  second,
                 const unsigned int   microsecond);
     //:Constructor

   OsDateTimeLinux(const OsDateTimeLinux& rOsDateTimeLinux);
     //:Copy constructor

   /// Convert an OsTime to an OsDateTime
   OsDateTimeLinux(const OsTime& toTime);

   virtual
   ~OsDateTimeLinux();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsDateTimeLinux& operator=(const OsDateTimeLinux& rhs);
     //:Assignment operator

   static void setTime(struct timespec gmt, int tzOffsetSecs,
                       DstRule dstRule);
     //:Set the system time
     //!param: (in) gmt - time relative to the beginning of 1970 (GMT)
     //!param: (in) tzOffsetSecs - local time offset (seconds relative to GMT)
     //!param: (in) dstRule - daylight savings time rule

   static void setTimeZone(int tzOffsetSecs, DstRule dstRule);
     //:Set the time zone and daylight savings time information
     //!param: (in) tzOffsetSecs - local time offset (seconds relative to GMT)
     //!param: (in) dstRule - daylight savings time rule

/* ============================ ACCESSORS ================================= */

   virtual OsStatus cvtToTimeSinceBoot(OsTime& rTime) const;
     //:Convert the OsDateTimeBase value to an OsTime value
     // The OsTime value is relative to when the system was booted.

   static void getCurTime(OsDateTimeLinux& rDateTime);
     //:Return the current time as an OsDateTime value

   /// Return the current time as an OsTime value
   static void getCurTime(OsTime& rTime);

   static void getCurTimeSinceBoot(OsTime& rTime);
     //:Return the current time as an OsTime value
     // The OsTime value is relative to when the system was booted.

   static unsigned long getSecsSinceEpoch(void);
     //:Current time as the number of seconds since midnight (0 hour) 01/01/70

   static double secondsSinceBoot(void);
     //:Get seconds since boot under Linux using /proc/uptime

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsDateTimeLinux_h_

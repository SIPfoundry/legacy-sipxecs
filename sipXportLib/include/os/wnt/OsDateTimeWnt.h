//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsDateTimeWnt_h_
#define _OsDateTimeWnt_h_

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

//:DateTime management functions for Windows NT
class OsDateTimeWnt : public OsDateTimeBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsDateTimeWnt();
     //:Default constructor

   OsDateTimeWnt(const unsigned short year,
                 const unsigned char  month,
                 const unsigned char  day,
                 const unsigned char  hour,
                 const unsigned char  minute,
                 const unsigned char  second,
                 const unsigned int   microsecond);
     //:Constructor

   OsDateTimeWnt(const OsDateTimeWnt& rOsDateTimeWnt);
     //:Copy constructor

   /// Convert an OsTime to an OsDateTime
   OsDateTimeWnt(const OsTime& toTime);

   virtual
   ~OsDateTimeWnt();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsDateTimeWnt& operator=(const OsDateTimeWnt& rhs);
     //:Assignment operator

   static void setTime(struct timespec gmt, int tzOffsetSecs,
                       DstRule dstRule);
     //:Set the system time.  For now, this is a no-op for Windows platforms.
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

   static void getCurTime(OsDateTimeWnt& rDateTime);
     //:Return the current time as an OsDateTime value

   static void getCurTime(OsTime& rTime);
     //:Return the current time as an OsTime value

   static void getCurTimeSinceBoot(OsTime& rTime);
     //:Return the current time as an OsTime value
     // The OsTime value is relative to when the system was booted.

   static unsigned long getSecsSinceEpoch(void);
     //:Current time as the number of seconds since midnight (0 hour) 01/01/70

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsDateTimeWnt_h_

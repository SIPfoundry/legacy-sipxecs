//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsDateTime_h_
#define _OsDateTime_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STRUCTS
#if defined(_WIN32)
struct timespec
{
   time_t tv_sec;   // seconds
   long   tv_nsec;  // nanoseconds
};
#endif

// TYPEDEFS

// FORWARD DECLARATIONS
class OsTime;
class UtlString;

//:DateTime management functions

/* On Solaris, DST_NONE is defined to be 0! This causes a syntax error. Rather
 * than change it, we can just undefine it - if it ever is defined, even in
 * other operating systems, we don't want it to be. Since anyone who wants to
 * use OsDateTime::DST_NONE will include this file, they'll inherit the
 * undefinition. */
#ifdef DST_NONE
#undef DST_NONE
#endif

class OsDateTimeBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum DstRule
   {
      DST_NONE,
      DST_NORTH_AMERICA,
      DST_WESTERN_EUROPE
   };

   static const char* spMonthStrings[12];
   //: Array containing the 3 character month names

   static const char* spDayStrings[7];
   //: Array containing the 3 character day of the week names

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

   static time_t tm2Epoch(const struct tm *t);
     //:Convert tm struct to epoch time
     //! returns: the number of seconds since the begining of 1970.

   static time_t convertHttpDateToEpoch(const char *date);
     //:Parse HTTP text format date and convert to epoch time
     // See RFC 822, 1123, 850, 1036
     //! returns: the number of seconds since the begining of 1970.

   static void setTime(struct timespec gmt);
     //:Set the system time and retain the previous settings for
     //: time zone offset and daylight savings rules

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

   virtual OsStatus cvtToTimeSinceBoot(OsTime& rTime) const = 0;
     //:Convert the OsDateTimeBase value to an OsTime value.
     //:The OsTime value is relative to when the system was booted.

   virtual OsStatus cvtToTimeSinceEpoch(OsTime& rTime) const;
     //:Convert the OsDateTimeBase value to an OsTime value.
     //:The OsTime value is relative to midnight (0 hour) 01/01/70.

   static void getDayOfWeek(int year, int  month, int dayOfMonth, int& dayOfWeek);
     //:Get the day of the week given the date
     //!param: (in) year - 4 digit year
     //!param: (in) month - 0-11
     //!param: (in) dayOfMonth - the day of the month 1-31
     //!param: (out) dayOfWeek - the day of the week 0-6

   void getDayOfWeek(int& dayOfWeek);
     //:Get the day of the week for this OsDateTime
     //!param: (out) dayOfWeek - the day of the week 0-6

   int usecs();
     //: Get the number of microseconds since the beginning of this second

   virtual void getHttpTimeString(UtlString& dataString);
     //:Get the RFC 822/1123 format date string for this OsDateTime
     // E.g. Wed, 06 Mar 2002 05:51:44 GMT
     // Assumes this OsDateTime is GMT

   static void getLocalTimeString(UtlString& dateString);
     //:Return the current local time as an OsDateTime value in the
     // following format:
     // Mon, 8/26/2002 07:21:32 PM EST

   /// Set the dateString to the time as UTC time in a Postgres compatible format:
   ///   2002-08-26 19:21:32.000
   void getSqlTimeStringZ(UtlString& dateString);

   /// Set the dateString to the time as UTC time in the following format:
   ///   2002-08-26T19:21:32.000Z
   void getIsoTimeStringZ(UtlString& dateString);

   /// Set the dateString to the time as UTC time in the following format:
   ///   2002-08-26T19:21:32.000000Z
   void getIsoTimeStringZus(UtlString& dateString);

   static void getCurTime(OsDateTimeBase& rDateTime);
     //:Return the current time as an OsDateTime value

   static void getCurTime(OsTime& rTime);
     //:Return the current time as an OsTime value

   static void getCurTimeSinceBoot(OsTime& rTime);
     //:Return the current time as an OsTime value
     // The OsTime value is relative to when the system was booted.

   static unsigned long getSecsSinceEpoch(void);
     //:Current time as the number of seconds since 00:00:00, 1 Jan 1970, GMT
     // (in the usual Unix manner, excluding leap seconds)

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static DstRule sDstRule;        // the daylight savings time rule in effect
   static int     sDstYear;        // the year (relative to 1900) that was
                                   //  used to determine the start and end
                                   //  dates for daylight savings time
   static int     sTzOffsetSecs;   // time zone offset expressed as seconds
                                   //  east of the zero meridian.  For example
                                   //  US Eastern time is -18000

   OsDateTimeBase();
     //:Default constructor

   OsDateTimeBase(const unsigned short year,        /**< XXXX year */
                const unsigned char  month,         /**< 0-based month; 0=JAN, 1=FEB */
                const unsigned char  day,           /**< 1-based day (1..31) */
                const unsigned char  hour,          /**< Hour is 24-hour format (0..23) */
                const unsigned char  minute,        /**< Minute (0..59) */
                const unsigned char  second,        /**< Second (0..59) */
                const unsigned int   microsecond);  /**< Microseconds  (0 - 999999) */
     //:Constructor

   OsDateTimeBase(const OsDateTimeBase& rOsDateTime);
     //:Copy constructor

   virtual
      ~OsDateTimeBase();
     //:Destructor

   OsDateTimeBase& operator=(const OsDateTimeBase& rhs);
     //:Assignment operator

   unsigned int   mMicrosecond;
        //:Microsecond, valid range: 0 - 999999

   unsigned short mYear;
        //:4 digit year

   unsigned char  mMonth;
        //:January = 0, February = 1, and so on

   unsigned char  mDay;
        //:Day of month, valid range: 1-31

   unsigned char  mHour;
        //:Hour, valid range: 0 - 23

   unsigned char  mMinute;
        //:Minute, valid range 0 - 59

   unsigned char  mSecond;
        //:Second, valid range 0 - 59

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   static int checkmask(const char *data, const char *mask);
     //: parsing utility used to parse text dates and times

};

/* ============================ INLINE METHODS ============================ */

// Depending on the native OS that we are running on, we include the class
// declaration for the appropriate lower level implementation and use a
// "typedef" statement to associate the OS-independent class name (OsDateTime)
// with the OS-dependent realization of that type (e.g., OsDateTimeWnt).
#if defined(_WIN32)
#  include "os/Wnt/OsDateTimeWnt.h"
   typedef class OsDateTimeWnt OsDateTime;
#elif defined(_VXWORKS)
#  include "os/Vxw/OsDateTimeVxw.h"
   typedef class OsDateTimeVxw OsDateTime;
#elif defined(__pingtel_on_posix__)
#  include "os/linux/OsDateTimeLinux.h"
   typedef class OsDateTimeLinux OsDateTime;
#else
#  error Unsupported target platform.
#endif

#endif  // _OsDateTime_h_

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
#include <time.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// APPLICATION INCLUDES
#include "os/OsDateTime.h"
#include "os/OsTime.h"
#include "utl/UtlString.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define BAD_DATE (time_t)0
static const int MICROSECS_PER_MILLISEC = 1000;

// STATIC VARIABLE INITIALIZATIONS
OsDateTimeBase::DstRule OsDateTimeBase::sDstRule = OsDateTimeBase::DST_NONE;
int                     OsDateTimeBase::sDstYear = -1;
int                     OsDateTimeBase::sTzOffsetSecs = 0;

//:Represents a date and time.
// Uses individual members for the month, day, year, hour, minute, second
// second and microsecond.

const char* OsDateTimeBase::spMonthStrings[12] =
{
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

const char* OsDateTimeBase::spDayStrings[7] =
{
    "Sun","Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Default constructor
OsDateTimeBase::OsDateTimeBase()
 : mMicrosecond(0),
   mYear(0),
   mMonth(0),
   mDay(0),
   mHour(0),
   mMinute(0),
   mSecond(0)
{
   // no work required beyond what's been done by the initializers
}

// Constructor
OsDateTimeBase::OsDateTimeBase(const unsigned short year,
                               const unsigned char  month,
                               const unsigned char  day,
                               const unsigned char  hour,
                               const unsigned char  minute,
                               const unsigned char  second,
                               const unsigned int   microsecond)
 : mMicrosecond(microsecond),
   mYear(year),
   mMonth(month),
   mDay(day),
   mHour(hour),
   mMinute(minute),
   mSecond(second)
{
   // no work required beyond what's been done by the initializers

}

// Copy constructor
OsDateTimeBase::OsDateTimeBase(const OsDateTimeBase& rOsDateTimeBase)
{
   mYear        = rOsDateTimeBase.mYear;
   mMonth       = rOsDateTimeBase.mMonth;
   mDay         = rOsDateTimeBase.mDay;
   mHour        = rOsDateTimeBase.mHour;
   mMinute      = rOsDateTimeBase.mMinute;
   mSecond      = rOsDateTimeBase.mSecond;
   mMicrosecond = rOsDateTimeBase.mMicrosecond;
}

// Destructor
OsDateTimeBase::~OsDateTimeBase()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsDateTimeBase&
OsDateTimeBase::operator=(const OsDateTimeBase& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   mYear        = rhs.mYear;
   mMonth       = rhs.mMonth;
   mDay         = rhs.mDay;
   mHour        = rhs.mHour;
   mMinute      = rhs.mMinute;
   mSecond      = rhs.mSecond;
   mMicrosecond = rhs.mMicrosecond;

   return *this;
}


time_t OsDateTimeBase::tm2Epoch(const struct tm *t)
{
    /*
     * tm2sec converts a GMT tm structure into the number of seconds since
     * 1st January 1970 UT.  Note that we ignore tm_wday, tm_yday, and tm_dst.
     *
     * The return value is always a valid time_t value -- (time_t)0 is returned
     * if the input date is outside that capable of being represented by time(),
     * i.e., before Thu, 01 Jan 1970 00:00:00 for all systems and
     * beyond 2038 for 32bit systems.
     *
     * This routine is intended to be very fast, much faster than mktime().
     */
    int  year;
    time_t days;
    const int dayoffset[12] =
        {306, 337, 0, 31, 61, 92, 122, 153, 184, 214, 245, 275};

    year = t->tm_year;

    if (year < 70 || ((sizeof(time_t) <= 4) && (year >= 138)))
        return BAD_DATE;

    /* shift new year to 1st March in order to make leap year calc easy */

    if (t->tm_mon < 2) year--;

    /* Find number of days since 1st March 1900 (in the Gregorian calendar). */

    days  = year * 365 + year/4 - year/100 + (year/100 + 3)/4;
    days += dayoffset[t->tm_mon] + t->tm_mday - 1;
    days -= 25508; /* 1 jan 1970 is 25508 days since 1 mar 1900 */

    days = ((days * 24 + t->tm_hour) * 60 + t->tm_min) * 60 + t->tm_sec;

    if (days < 0)
        return BAD_DATE;       /* must have overflowed */
    else
        return days;           /* must be a valid time */
}


time_t OsDateTimeBase::convertHttpDateToEpoch(const char *date)
{
    /*
     * Parses an HTTP date in one of three standard forms:
     *
     *     Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
     *     Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
     *     Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format
     *
     * and returns the time_t number of seconds since 1 Jan 1970 GMT, or
     * 0 if this would be out of range or if the date is invalid.
     *
     * The restricted HTTP syntax is
     *
     *     HTTP-date    = rfc1123-date | rfc850-date | asctime-date
     *
     *     rfc1123-date = wkday "," SP date1 SP time SP "GMT"
     *     rfc850-date  = weekday "," SP date2 SP time SP "GMT"
     *     asctime-date = wkday SP date3 SP time SP 4DIGIT
     *
     *     date1        = 2DIGIT SP month SP 4DIGIT
     *                    ; day month year (e.g., 02 Jun 1982)
     *     date2        = 2DIGIT "-" month "-" 2DIGIT
     *                    ; day-month-year (e.g., 02-Jun-82)
     *     date3        = month SP ( 2DIGIT | ( SP 1DIGIT ))
     *                    ; month day (e.g., Jun  2)
     *
     *     time         = 2DIGIT ":" 2DIGIT ":" 2DIGIT
     *                    ; 00:00:00 - 23:59:59
     *
     *     wkday        = "Mon" | "Tue" | "Wed"
     *                  | "Thu" | "Fri" | "Sat" | "Sun"
     *
     *     weekday      = "Monday" | "Tuesday" | "Wednesday"
     *                  | "Thursday" | "Friday" | "Saturday" | "Sunday"
     *
     *     month        = "Jan" | "Feb" | "Mar" | "Apr"
     *                  | "May" | "Jun" | "Jul" | "Aug"
     *                  | "Sep" | "Oct" | "Nov" | "Dec"
     *
     * However, for the sake of robustness (and Netscapeness), we ignore the
     * weekday and anything after the time field (including the timezone).
     *
     * This routine is intended to be very fast; 10x faster than using sscanf.
     *
     * Originally from Andrew Daviel <andrew@vancouver-webpages.com>, 29 Jul 96
     * but many changes since then.
     *
     */
    struct tm ds;
    int mint, mon;
    const char *monstr, *timstr;
    const int months[12] = {
        ('J' << 16) | ( 'a' << 8) | 'n',  ('F' << 16) | ( 'e' << 8) | 'b',
        ('M' << 16) | ( 'a' << 8) | 'r',  ('A' << 16) | ( 'p' << 8) | 'r',
        ('M' << 16) | ( 'a' << 8) | 'y',  ('J' << 16) | ( 'u' << 8) | 'n',
        ('J' << 16) | ( 'u' << 8) | 'l',  ('A' << 16) | ( 'u' << 8) | 'g',
        ('S' << 16) | ( 'e' << 8) | 'p',  ('O' << 16) | ( 'c' << 8) | 't',
        ('N' << 16) | ( 'o' << 8) | 'v',  ('D' << 16) | ( 'e' << 8) | 'c'};

    if (!date)
        return BAD_DATE;

    while (*date && isspace(*date))      /* Find first non-whitespace char */
        ++date;

    if (*date == '\0')
        return BAD_DATE;

    if ((date = strchr(date,' ')) == NULL)   /* Find space after weekday */
        return BAD_DATE;

    ++date;    /* Now pointing to first char after space, which should be */
               /* start of the actual date information for all 3 formats. */

    if (checkmask(date, "## @$$ #### ##:##:## *")) {     /* RFC 1123 format */
        ds.tm_year = ((date[7] - '0') * 10 + (date[8] - '0') - 19) * 100;
        if (ds.tm_year < 0)
            return BAD_DATE;

        ds.tm_year += ((date[9] - '0') * 10) + (date[10] - '0');

        ds.tm_mday  = ((date[0] - '0') * 10) + (date[1] - '0');

        monstr = date + 3;
        timstr = date + 12;
    }
    else if (checkmask(date, "##-@$$-## ##:##:## *")) {  /* RFC 850 format  */
        ds.tm_year = ((date[7] - '0') * 10) + (date[8] - '0');
        if (ds.tm_year < 70)
            ds.tm_year += 100;

        ds.tm_mday = ((date[0] - '0') * 10) + (date[1] - '0');

        monstr = date + 3;
        timstr = date + 10;
    }
    else if (checkmask(date, "@$$ ~# ##:##:## ####*")) { /* asctime format  */
        ds.tm_year = ((date[16] - '0') * 10 + (date[17] - '0') - 19) * 100;
        if (ds.tm_year < 0)
            return BAD_DATE;

        ds.tm_year += ((date[18] - '0') * 10) + (date[19] - '0');

        if (date[4] == ' ')
            ds.tm_mday = 0;
        else
            ds.tm_mday = (date[4] - '0') * 10;

        ds.tm_mday += (date[5] - '0');

        monstr = date;
        timstr = date + 7;
    }
    else return BAD_DATE;

    if (ds.tm_mday <= 0 || ds.tm_mday > 31)
        return BAD_DATE;

    ds.tm_hour = ((timstr[0] - '0') * 10) + (timstr[1] - '0');
    ds.tm_min  = ((timstr[3] - '0') * 10) + (timstr[4] - '0');
    ds.tm_sec  = ((timstr[6] - '0') * 10) + (timstr[7] - '0');

    if ((ds.tm_hour > 23) || (ds.tm_min > 59) || (ds.tm_sec > 61))
        return BAD_DATE;

    mint = (monstr[0] << 16) | (monstr[1] << 8) | monstr[2];
    for (mon=0; mon < 12; mon++)
        if (mint == months[mon])
            break;
    if (mon == 12)
        return BAD_DATE;

    if ((ds.tm_mday == 31) && (mon == 3 || mon == 5 || mon == 8 || mon == 10))
        return BAD_DATE;

    /* February gets special check for leapyear */

    if ((mon == 1) && ((ds.tm_mday > 29) ||
         ((ds.tm_mday == 29) && ((ds.tm_year & 3) ||
           (((ds.tm_year % 100) == 0) && (((ds.tm_year % 400) != 100)))))))
        return BAD_DATE;

    ds.tm_mon = mon;

    return tm2Epoch(&ds);
}

// Set the system time and retain the previous settings for
// time zone offset and daylight savings rules
void OsDateTimeBase::setTime(struct timespec gmt)
{
   OsDateTime::setTime(gmt, sTzOffsetSecs, sDstRule);
}

// Set the time zone information
void OsDateTimeBase::setTimeZone(int tzOffsetSecs, DstRule dstRule)
{
   OsDateTime::setTimeZone(tzOffsetSecs, dstRule);
}

/* ============================ ACCESSORS ================================= */

void OsDateTimeBase::getDayOfWeek(int year, int  month, int dayOfMonth, int& dayOfWeek)
{
    // Note: this calculation was wonderfully documented on:
    // http://www.tondering.dk/claus/cal/node3.html#SECTION00350000000000000000
    // A copy of this page is also in:
    // \\Pingpdc\Software\Info\technology\calendar-math\node3.html

    // The calculation below assumes a 1-based month -- internally, we used a 0-based number
    // and the API docs ask for a 0-based month.
    month++;

//    osPrintf("getDayOfWeek (IN): year=%d, month=%d, dayofmonth=%d\n",year,month,dayOfMonth);

    int a = (14 - month)/12;
    int gy = year - a;

    int m = month + (12 * a) - 2;
    dayOfWeek = (dayOfMonth + gy + (gy/4) - (gy/100) + (gy/400) + (31 * m) / 12) % 7;

//    osPrintf("Month: %d day: %d, %d is on day of week: %d \n",
//        month, dayOfMonth, year, dayOfWeek);
}

void OsDateTimeBase::getDayOfWeek(int& dayOfWeek)
{
    getDayOfWeek(mYear, mMonth, mDay, dayOfWeek);
}

// Convert the OsDateTimeBase value to an OsTime value.
// The OsTime value is relative to midnight (0 hour) 01/01/70.
OsStatus OsDateTimeBase::cvtToTimeSinceEpoch(OsTime& rTime) const
{
   struct tm thisTime;
   time_t    thisTimeAsTimeT;

   // convert "this" OsDateTime to a time_t representation
   thisTime.tm_year  = mYear - 1900;
   thisTime.tm_mon   = mMonth;
   thisTime.tm_mday  = mDay;
   thisTime.tm_hour  = mHour;
   thisTime.tm_min   = mMinute;
   thisTime.tm_sec   = mSecond;
   thisTime.tm_wday  = 0;
   thisTime.tm_yday  = 0;
   thisTime.tm_isdst = 0;

   thisTimeAsTimeT  = tm2Epoch(&thisTime);
   assert(thisTimeAsTimeT >= 0);

   OsTime timeSinceEpoch(thisTimeAsTimeT, mMicrosecond);
   rTime = timeSinceEpoch;

   return OS_SUCCESS;
}

void OsDateTimeBase::getHttpTimeString(UtlString& dateString)
{
    char dateBuffer[200];

    // Need a day of the week function which takes year, month and day
    int dayOfTheWeek;
    getDayOfWeek(dayOfTheWeek);

    // Wed, 06 Mar 2002 05:51:44 GMT
    sprintf(dateBuffer, "%s, %.2d %s %d %.2d:%.2d:%.2d GMT",
            spDayStrings[dayOfTheWeek],
            mDay, spMonthStrings[mMonth], mYear,
            mHour, mMinute, mSecond);

    dateString = dateBuffer;
}

/// Set the dateString to the time as UTC time in a Postgres compatible format:
///   2002-08-26 19:21:32.000
void OsDateTimeBase::getSqlTimeStringZ(UtlString& dateString)
{
   dateString.resize(24);
   sprintf(const_cast<char*>(dateString.data()), "%4d-%02d-%02d %02d:%02d:%02d.%03d",
           mYear, mMonth+1, mDay,
           mHour, mMinute, mSecond, mMicrosecond/MICROSECS_PER_MILLISEC
           );
}

/// Set the dateString to the time as UTC time in the following format:
///   2002-08-26T19:21:32.000Z
void OsDateTimeBase::getIsoTimeStringZ(UtlString& dateString)
{
   dateString.resize(24);
   sprintf(const_cast<char*>(dateString.data()), "%4d-%02d-%02dT%02d:%02d:%02d.%03dZ",
           mYear, mMonth+1, mDay,
           mHour, mMinute, mSecond, mMicrosecond/MICROSECS_PER_MILLISEC
           );
}

/// Set the dateString to the time as UTC time in the following format:
///   2002-08-26T19:21:32.000000Z
void OsDateTimeBase::getIsoTimeStringZus(UtlString& dateString)
{
   dateString.resize(27);
   sprintf(const_cast<char*>(dateString.data()), "%4d-%02d-%02dT%02d:%02d:%02d.%06dZ",
           mYear, mMonth+1, mDay,
           mHour, mMinute, mSecond, mMicrosecond
           );
}

void OsDateTimeBase::getLocalTimeString(UtlString& dateString)
{
    char dateBuffer[200];
    char ampm[] = "AM";
    time_t ltime;
    struct tm *today;
#ifndef _VXWORKS

    /* Set time zone from TZ environment variable. If TZ is not set,
     * the operating system is queried to obtain the default value
     * for the variable.
     */
    tzset();
#endif /* _VXWORKS*/

    /* Convert to time structure and adjust for PM if necessary. */
    time( &ltime );
    today = localtime( &ltime );
    if( today->tm_hour >= 12 )
    {
      strcpy( ampm, "PM" );
      if (today->tm_hour > 12)
        today->tm_hour -= 12;
    }
    if( today->tm_hour == 0 )  /* Adjust if midnight hour. */
       today->tm_hour = 12;

    char tz[4] = {"   "};
#ifndef _VXWORKS
    UtlString timezone = tzname[0];

    if (today->tm_isdst == 1)
      timezone = tzname[1];

    size_t  len = timezone.length();

    if (len > 3)
    {
      ssize_t pos = timezone.index(" ");
      if (pos != UTL_NOT_FOUND)
      {
        tz[0] = timezone.data()[0];
        tz[1] = timezone.data()[pos + 1];
        if ((pos = timezone.index(" ", pos + 1)) != UTL_NOT_FOUND)
        {
          tz[2] = timezone.data()[pos + 1];
        }
      }
    }
    else if (len > 0)
    {
      memcpy(tz, timezone.data(), 3);
    }
#endif /* _VXWORKS */
    tz[3] = 0;

    // Mon, 25-Sep-2002 05:51:44 EST
    sprintf(dateBuffer, "%s, %d-%s-%d %.2d:%.2d:%.2d %s %s",
            spDayStrings[today->tm_wday],
            today->tm_mday, spMonthStrings[today->tm_mon], (today->tm_year + 1900),
            today->tm_hour, today->tm_min, today->tm_sec,
            ampm,
            tz);

    dateString = dateBuffer;
}

// Return the current time as an OsTime value
// Note that this method is overridden in both Linux and Windows with
// methods that return time with microsecond resolution.  So don't be
// deceived by this code that getCurTime only returns time with second
// resolution.
void OsDateTimeBase::getCurTime(OsTime& rTime)
{
   OsTime curTime(time(NULL), 0);

   rTime = curTime;
}

int OsDateTimeBase::usecs()
{
    return(mMicrosecond);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

int OsDateTimeBase::checkmask(const char *data, const char *mask)
{
    /*
     * Compare a string to a mask
     * Mask characters (arbitrary maximum is 256 characters, just in case):
     *   @ - uppercase letter
     *   $ - lowercase letter
     *   & - hex digit
     *   # - digit
     *   ~ - digit or space
     *   * - swallow remaining characters
     *  <x> - exact match for any other character
     */
    int i;
    char d;

    for (i = 0; i < 256; i++) {
        d  = data[i];
        switch (mask[i]) {
          case '\0': return (d == '\0');

          case '*':  return 1;

          case '@':  if (!isupper(d)) return 0;
                     break;
          case '$':  if (!islower(d)) return 0;
                     break;
          case '#':  if (!isdigit(d)) return 0;
                     break;
          case '&':  if (!isxdigit(d)) return 0;
                     break;
          case '~':  if ((d != ' ') && !isdigit(d)) return 0;
                     break;
          default:   if (mask[i] != d) return 0;
                     break;
        }
    }
    return 0;  /* We only get here if mask is corrupted (exceeds 256) */
}

/* ============================ FUNCTIONS ================================= */

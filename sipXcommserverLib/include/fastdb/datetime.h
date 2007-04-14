//-< DATEIME.H >-----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Date-time field type
//-------------------------------------------------------------------*--------*

#ifndef __DATETIME_H__
#define __DATETIME_H__

#include <time.h>
#include "stdtp.h"
#include "class.h"

#if !defined(NO_PTHREADS) && !defined(_WIN32)
#define USE_REENTRANT_LIBRARY
#endif

class FASTDB_DLL_ENTRY dbDateTime { 
    int4 stamp;
  public:
    bool operator == (dbDateTime const& dt) const { 
	return stamp == dt.stamp;
    }
    bool operator != (dbDateTime const& dt) const { 
	return stamp != dt.stamp;
    }
    bool operator > (dbDateTime const& dt) const { 
	return stamp > dt.stamp;
    }
    bool operator >= (dbDateTime const& dt) const { 
	return stamp >= dt.stamp;
    }
    bool operator < (dbDateTime const& dt) const { 
	return stamp < dt.stamp;
    }
    bool operator <= (dbDateTime const& dt) const { 
	return stamp <= dt.stamp;
    }
    int operator - (dbDateTime const& dt) { 
	return stamp - dt.stamp;
    }
     int operator + (dbDateTime const& dt) { 
	return stamp + dt.stamp;
    }
    static dbDateTime current() { 
	return dbDateTime(time(NULL));
    }
    dbDateTime(time_t tm) {
	stamp = tm;
    }
    dbDateTime() { 
	stamp = -1;
    } 
    bool isValid() const { 
	return stamp != -1;
    }

    time_t asTime_t() { return stamp; }

    void clear() { stamp = -1; }

    dbDateTime(int year, int month, int day, 
	       int hour=0, int min=0, int sec = 0) 
    { 
	struct tm t;
	t.tm_year = year > 1900 ? year - 1900 : year;
	t.tm_mon = month-1;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min = min;
	t.tm_sec = sec;
	t.tm_isdst = -1;
	stamp = mktime(&t);
    }
    dbDateTime(int hour, int min) { 
	stamp = (hour*60+min)*60;
    }

#ifdef USE_REENTRANT_LIBRARY
    int year() { 
	struct tm t;
	return localtime_r((time_t*)&stamp, &t)->tm_year + 1900;
    }
    int month() { // 1..12
	struct tm t;
	return localtime_r((time_t*)&stamp, &t)->tm_mon + 1;
    }
    int day() { // 1..31
	struct tm t;
	return localtime_r((time_t*)&stamp, &t)->tm_mday;
    }
    int dayOfYear() { // 1..366
	struct tm t;
	return localtime_r((time_t*)&stamp, &t)->tm_yday+1;
    }
    int dayOfWeek() { // 1..7
	struct tm t;
	return localtime_r((time_t*)&stamp, &t)->tm_wday+1;
    }
    int hour() { // 0..24
	struct tm t;
	return localtime_r((time_t*)&stamp, &t)->tm_hour;
    }
    int minute() { // 0..59
	struct tm t;
	return localtime_r((time_t*)&stamp, &t)->tm_min;
    }
    int second() { // 0..59
	struct tm t;
	return localtime_r((time_t*)&stamp, &t)->tm_sec;
    }
    char* asString(char* buf, int buf_size, char const* format = "%c") const { 
	struct tm t;
	strftime(buf, buf_size, format, localtime_r((time_t*)&stamp, &t));
	return buf;
    }
    static dbDateTime currentDate() { 
	struct tm t;
	time_t curr = time(NULL);
	localtime_r(&curr, &t);;
	t.tm_hour = 0;
	t.tm_min = 0;
	t.tm_sec = 0;
	return dbDateTime(mktime(&t));
    }
#else
    int year() { 
	return localtime((time_t*)&stamp)->tm_year + 1900;
    }
    int month() { // 1..12
	return localtime((time_t*)&stamp)->tm_mon + 1;
    }
    int day() { // 1..31
	return localtime((time_t*)&stamp)->tm_mday;
    }
    int dayOfYear() { // 1..366
	return localtime((time_t*)&stamp)->tm_yday+1;
    }
    int dayOfWeek() { // 1..7
	return localtime((time_t*)&stamp)->tm_wday+1;
    }
    int hour() { // 0..24
	return localtime((time_t*)&stamp)->tm_hour;
    }
    int minute() { // 0..59
	return localtime((time_t*)&stamp)->tm_min;
    }
    int second() { // 0..59
	return localtime((time_t*)&stamp)->tm_sec;
    }
    char* asString(char* buf, int buf_size, char const* format = "%c") const { 
	strftime(buf, buf_size, format, localtime((time_t*)&stamp));
	return buf;
    }
    static dbDateTime currentDate() { 
	time_t curr = time(NULL);
	struct tm* tp = localtime(&curr);;
	tp->tm_hour = 0;
	tp->tm_min = 0;
	tp->tm_sec = 0;
	return dbDateTime(mktime(tp));
    }
#endif    

    CLASS_DESCRIPTOR(dbDateTime, 
		     (KEY(stamp,INDEXED|HASHED), 
		      METHOD(year), METHOD(month), METHOD(day),
		      METHOD(dayOfYear), METHOD(dayOfWeek),
		      METHOD(hour), METHOD(minute), METHOD(second)));

    dbQueryExpression operator == (char const* field) const { 
	dbQueryExpression expr;
	expr = dbComponent(field,"stamp"),"=",stamp;
	return expr;
    }
    dbQueryExpression operator != (char const* field) const { 
	dbQueryExpression expr;
	expr = dbComponent(field,"stamp"),"<>",stamp;
	return expr;
    }
    dbQueryExpression operator < (char const* field) const { 
	dbQueryExpression expr;
	expr = dbComponent(field,"stamp"),">",stamp;
	return expr;
    }
    dbQueryExpression operator <= (char const* field) const { 
	dbQueryExpression expr;
	expr = dbComponent(field,"stamp"),">=",stamp;
	return expr;
    }
    dbQueryExpression operator > (char const* field) const { 
	dbQueryExpression expr;
	expr = dbComponent(field,"stamp"),"<",stamp;
	return expr;
    }
    dbQueryExpression operator >= (char const* field) const { 
	dbQueryExpression expr;
	expr = dbComponent(field,"stamp"),"<=",stamp;
	return expr;
    }
    friend dbQueryExpression between(char const* field, dbDateTime& from,
				     dbDateTime& till)
    { 
	dbQueryExpression expr;
	expr=dbComponent(field,"stamp"),"between",from.stamp,"and",till.stamp;
	return expr;
    }

    static dbQueryExpression ascent(char const* field) { 
	dbQueryExpression expr;
	expr=dbComponent(field,"stamp");
	return expr;
    }	
    static dbQueryExpression descent(char const* field) { 
	dbQueryExpression expr;
	expr=dbComponent(field,"stamp"),"desc";
	return expr;
    }	
};

#endif

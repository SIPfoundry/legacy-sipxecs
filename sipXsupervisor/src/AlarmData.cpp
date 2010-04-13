//
//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "AlarmData.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType cAlarmData::TYPE = "cAlarmData";

// STATIC VARIABLE INITIALIZATIONS

/* ============================ CREATORS ================================== */

cAlarmData::cAlarmData() :
   m_alarmId(""),
   m_code(""),
   m_component(""),
   m_shortTitle(""),
   m_description(""),
   m_resolution("")
{

}

cAlarmData::~cAlarmData()
{
}

UtlContainableType cAlarmData::getContainableType() const
{
      return cAlarmData::TYPE;
}

void cAlarmData::setAlarmId(const UtlString& newStr)
{
   m_alarmId = newStr;
}

void cAlarmData::setCode(const UtlString& newStr)
{
   m_code = newStr;
}

void cAlarmData::setComponent(const UtlString& newStr)
{
   m_component = newStr;
}

void  cAlarmData::setSeverity(const OsSysLogPriority newSev)
{
   m_severity = newSev;
}

void cAlarmData::setShortTitle(const UtlString& newStr)
{
   m_shortTitle = newStr;
}

void cAlarmData::setDescription(const UtlString& newStr)
{
   m_description = newStr;
}

void cAlarmData::setResolution(const UtlString& newStr)
{
   m_resolution = newStr;
}

void cAlarmData::setTime(OsTime now)
{
   // For alarm filtering; if "now" is newer than the threshold, reset counts
   if (now-lastTime > TIME_THRESH_OS)
   {
      resetCount();
   };
   lastTime=now;
}

/// Based on configured thresholds, determine whether alarm should be reported or not
UtlBoolean cAlarmData::applyThresholds()
{
   //:TODO: better filtering...
   return (count < min_threshold || count > max_report);
}



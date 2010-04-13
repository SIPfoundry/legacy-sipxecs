// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "alarm/Alarm.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// GLOBAL VARIABLES

// Send an alarm to the Alarm Server with no runtime parameter.  Non-blocking.
void Alarm::raiseAlarm(UtlString alarmId)
{
   UtlSList alarmParams;
   raiseAlarm(alarmId, alarmParams);
}

// Send an alarm to the Alarm Server with a single runtime parameter.  Non-blocking.
void Alarm::raiseAlarm(UtlString alarmId, UtlString alarmParam)
{
   UtlSList alarmParams;
   alarmParams.append(&alarmParam);
   raiseAlarm(alarmId, alarmParams);
}

// Send an alarm to the Alarm Server with a list of runtime parameters.  Non-blocking.
void Alarm::raiseAlarm(UtlString alarmId, UtlSList& alarmParams)
{
   OsSysLog::add(FAC_ALARM,PRI_DEBUG,"Alarm::raiseAlarm %s", alarmId.data());

   // send alarm to Alarm Server via the AlarmRequestTask
   AlarmRequestTask::getInstance()->raiseAlarm(alarmId, alarmParams);
}



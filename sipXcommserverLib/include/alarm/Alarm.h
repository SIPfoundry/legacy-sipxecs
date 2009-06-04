// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _Alarm_h_
#define _Alarm_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/UtlSList.h"
#include "alarm/AlarmRequestTask.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// GLOBAL VARIABLES

/// Provide static interface for application code to report alarm conditions.
class Alarm
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /// Send an alarm to the Alarm Server without any runtime parameter.  Non-blocking.
   /// The Alarm Server is configured in sipxsupervisor-config, so any process generating an
   /// alarm using this interface should declare sipxsupervisor-config as a resource.
   static void raiseAlarm(
         UtlString alarmId                 ///< internal unique alarm ID
         );
   /** Alarms are identified by a unique alarm ID, which is not visible to the customer.
    * Configuration data for new alarms are added by editing the alarm xml files
    * in sipXsupervisor/etc.
    */

   /// Send an alarm to the Alarm Server with a single runtime parameter.  Non-blocking.
   static void raiseAlarm(
         UtlString alarmId,                ///< internal unique alarm ID
         UtlString alarmParam              ///< string containing single runtime parameter
         );
   
   /// Send an alarm to the Alarm Server with a list of runtime parameters.  Non-blocking.
   static void raiseAlarm(
         UtlString alarmId,                ///< internal unique alarm ID
         UtlSList& alarmParams             ///< vector of strings containing runtime parameters
         );

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   
};

/* ============================ INLINE METHODS ============================ */

#endif  // _Alarm_h_

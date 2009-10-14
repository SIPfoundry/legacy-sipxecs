//
//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _AlarmData_h_
#define _AlarmData_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "os/OsTime.h"
#include "utl/UtlString.h"
#include "utl/UtlContainableAtomic.h"
#include "utl/UtlBool.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//:TODO: make this configurable.  Globally, or per-alarm? or different for min/max?
static const int TIME_THRESH_SEC = 20;
static const OsTime TIME_THRESH_OS = OsTime(TIME_THRESH_SEC, 0);
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class cAlarmServer;

///cAlarmData contains information about an alarm.
class cAlarmData : public UtlContainableAtomic
{
   friend class cAlarmServer;

public:
   cAlarmData();
   ~cAlarmData();

   virtual UtlContainableType getContainableType() const;
   static const UtlContainableType TYPE; ///< constant for class type comparison.

   /// One for each type of alarm Notifier
   enum eAlarmActions {
      eActionLog,       ///< Log Notifier
      eActionEmail,     ///< Email Notifier
      eActionSms,       ///< SMS Notifier
      eActionTrap,      ///< SNMP Trap Notifier (not currently implemented)
      eActionMax        ///< Insert new notifiers before this line
   };

   const UtlString& getId() const             { return m_alarmId; }
   const UtlString& getCode() const           { return m_code; }
   const UtlString& getComponent() const      { return m_component; }
   const UtlString& getShortTitle() const     { return m_shortTitle; }
   const UtlString& getDescription() const    { return m_description; }
   const OsSysLogPriority getSeverity() const { return m_severity; }
   const UtlString& getResolution() const     { return m_resolution; }
   const UtlBoolean getAction(const eAlarmActions act) { return actions[act]; }
   const UtlString& getGroupName() const      { return group_name; }
   const int        getCount() const          { return count; }
   const OsTime     getLastTime() const       { return lastTime; }

   void       setAlarmId(const UtlString& newStr);
   void       setCode(const UtlString& newStr);
   void       setComponent(const UtlString& newStr);
   void       setSeverity(const OsSysLogPriority newSev);
   void       setShortTitle(const UtlString& newStr);
   void       setDescription(const UtlString& newStr);
   void       setResolution(const UtlString& newStr);
   void       setTime(OsTime now);

   void       incrCount()      { count++; return; }
   void       resetCount()     { count=0; return; }

   /// Based on configured thresholds, determine whether alarm should be reported or not
   UtlBoolean applyThresholds();


private:
   // this section contains the elements which are read directly from the alarmDefinitions file
   UtlString m_alarmId;            ///< the unique internal ID.
   UtlString m_code;               ///< the unique external code.
   UtlString m_component;          ///< the component which raised the alarm.
   OsSysLogPriority m_severity;    ///< alarm severity

   // these are the strings which can be translated
   UtlString m_shortTitle;         ///< short title for alarm
   UtlString m_description;        ///< string including parameter placeholders
   UtlString m_resolution;         ///< suggested actions to resolve the problem

   // these are things that can be configured
   UtlBoolean actions[eActionMax]; ///< which notifiers should be invoked on this alarm
   UtlString  group_name;          ///< alarm group name to contact
   int        max_report;          ///< max number of times to report in a brief interval
   int        min_threshold;       ///< report only after this number of occurrences in a brief interval

   // these internal fields are used to filter events
   int        count;               ///< number of occurrences in a brief interval
   OsTime     lastTime;            ///< time of last occurrence, secs since last boot

   /// no copy constructor
   cAlarmData(const cAlarmData& nocopy);

};
#endif  // _AlarmData_h_

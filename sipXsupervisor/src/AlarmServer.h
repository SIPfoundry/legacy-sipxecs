//
//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _AlarmServer_h_
#define _AlarmServer_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsMutex.h"
#include "utl/UtlString.h"
#include "utl/UtlSList.h"
#include "utl/UtlContainableAtomic.h"
#include "utl/UtlBool.h"
#include "utl/UtlHashMap.h"
#include "xmlparser/tinyxml.h"
#include "AlarmData.h"
#include "NotifierBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MAX_NOTIFIERS 100
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/** cAlarmServer is a singleton class which generates alarm notifications.
  * It parses an alarm definitions XML file to build a searchable list of alarms,
  * each with configurable actions, and strings describing the event and steps to resolve it.
  * When an alarm is raised, it is passed to each notifier for processing.
*/
class cAlarmServer
{
public:
   ~cAlarmServer();

   /// returns the one and only alarm server.
   static cAlarmServer *getInstance();

   /// initialize the Alarm Server by loading alarms and strings
   bool init();

   /// clean up the Alarm Server by freeing all resources
   void cleanup();

   /// reload all alarm definitions and strings
   bool reloadAlarms();

   /// process the specified alarm, passing to all notifiers for handling
   bool handleAlarm(UtlString& callingHost,  ///< host which generated the alarm
                    UtlString& alarmId,      ///< internal alarm Id
                    UtlSList&  alarmParams   ///< list of string runtime parameters
                    );

   /// return number of alarms raised since last reboot
   int getAlarmCount();

   /// look up the specified alarm and return pointer to its configured data
   cAlarmData* lookupAlarm(const UtlString& id);

private:
   static cAlarmServer*   spAlarmServer;        ///< pointer to the one and only Alarm Server.
   static OsMutex         sLockMutex;           ///< Exclusive binary lock
   static const UtlString alarmIdKey;           ///< name of the hashmap key

   UtlHashMap mAlarmMap;                        ///< map from alarm ID to alarm data structure
   UtlString  mLanguage;                        ///< current language selection for notifications
   int        mAlarmCount;                      ///< number of alarms generated since last restart
   bool       gbActions[cAlarmData::eActionMax];///< global enable/disable for each notifier
   NotifierBase* mpNotifiers[MAX_NOTIFIERS];    ///< pointer to each notifier

   /// Constructor for singleton
   cAlarmServer();

   /// Load alarms and strings from the appropriate directories
   bool loadAlarms();

   /// Unload alarms and free all resources
   void unloadAlarms();

   bool loadAlarmConfig(const UtlString& alarmFile, const UtlString& groupFile);
   bool loadAlarmDefinitions(const UtlString& alarmFile);

   /// Load alarm strings from the specified file
   bool loadAlarmStrings(const UtlString& stringsFilename);
   bool loadAlarmStringsFile(const UtlString& stringsFile);

   /// Load alarm data from the XML element into the provided alarmData structure
   bool loadAlarmData(TiXmlElement *element, cAlarmData* data);

   /// No copy constructor
   cAlarmServer(const cAlarmServer& nocopy);

};
#endif  // _AlarmServer_h_

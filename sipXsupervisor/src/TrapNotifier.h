//
//
// Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _TrapNotifier_h_
#define _TrapNotifier_h_

// SYSTEM INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "utl/UtlHashMap.h"
#include "NotifierBase.h"

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Sends SNMPv2 traps containing alarm code, time, host, severity and description to configured trap receivers
class TrapNotifier : public NotifierBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   ///Default constructor
   TrapNotifier();

   ///Destructor
   virtual ~TrapNotifier();

   enum eAlarmAttributes {
      ALARM_CODE,
      ALARM_HOST,
      ALARM_TIME,
      ALARM_SEVERITY,
      ALARM_DESCR,
      MAX_ALARM_ATTRIBUTES
   };

/* ============================ MANIPULATORS ============================== */

   /// Format and send SNMPv2 trap notification to any configured trap receivers.
   virtual OsStatus handleAlarm(
         const OsTime alarmTime,           ///< time alarm was reported
         const UtlString& callingHost,     ///< host on which event occurred
         const cAlarmData* alarmData,      ///< pointer to alarmData structure
         const UtlString& alarmParameters  ///< formatted message with parameters
         );

   /// Initialize notifier (including loading parameters from the provided xml element).
   virtual OsStatus init(
         TiXmlElement* trapElement,       ///< pointer to xml config element for this notifier
         TiXmlElement* groupElement       ///< pointer to xml group config element for this notifier
         );

   /// Initialize the net-snmp session, load MIB files and libraries, create SNMP PDU and send trap.
   bool sendSnmpv2Trap(
         const UtlString alarmAttributes[], ///< Alarm attributes: Alarm code,time, host that generated the alarm,
                                            ///< severity and description
         const UtlString trap_receiver      ///< A string containing trap receiver information in the format:
                                            ///< trapReceiverAddress<unitSeparator>portNumber<unitSeparator>communityString
         );

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlHashMap mContacts;                   ///< list of contacts to send SNMPv2 trap notifications

   /// no copy constructor
   TrapNotifier(const TrapNotifier& nocopy);

   ///Assignment operator
   TrapNotifier& operator=(const TrapNotifier& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _TrapNotifier_h_


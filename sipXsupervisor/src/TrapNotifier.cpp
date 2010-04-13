//
//
// Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlTokenizer.h"
#include "AlarmUtils.h"
#include "TrapNotifier.h"
#include "sipXecsService/SipXecsService.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

const char* unitSeparator = "\x1F";  // ASCII for unit separator character  (US)
const char* groupSeparator = "\x1D"; // ASCII for group separator character (GS)
const char* snmpApplicationName = "snmpapp"; // SNMP Application name.

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
TrapNotifier::TrapNotifier() :
   mContacts()
{
}

// Destructor
TrapNotifier::~TrapNotifier()
{
   mContacts.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
TrapNotifier& TrapNotifier::operator=(const TrapNotifier& rhs)
{
   if (this == &rhs) // handle the assignment to self case
   return *this;

   return *this;
}

OsStatus TrapNotifier::handleAlarm(const OsTime alarmTime,
      const UtlString& callingHost,
      const cAlarmData* alarmData,
      const UtlString& alarmMsg)
{
   OsStatus retval = OS_SUCCESS;

   UtlString alarmAttributeValues[MAX_ALARM_ATTRIBUTES];

   alarmAttributeValues[ALARM_CODE] = alarmData->getCode();

   OsDateTime logTime(alarmTime);
   logTime.getIsoTimeStringZus(alarmAttributeValues[ALARM_TIME]);

   alarmAttributeValues[ALARM_HOST] = callingHost;

   alarmAttributeValues[ALARM_SEVERITY] = OsSysLog::priorityName(alarmData->getSeverity());

   alarmAttributeValues[ALARM_DESCR] = alarmMsg;

   UtlString groupKey(alarmData->getGroupName());

   OsSysLog::add(FAC_ALARM, PRI_DEBUG, "AlarmServer: SNMPv2 Trap parameters: AlarmCode = %s, AlarmTime = %s, "
           "AlarmHost = %s, AlarmSeverity = %s, AlarmDescription = %s, Alarm Group name = %s",
           alarmAttributeValues[ALARM_CODE].data(), alarmAttributeValues[ALARM_TIME].data(),
           alarmAttributeValues[ALARM_HOST].data(), alarmAttributeValues[ALARM_SEVERITY].data(),
           alarmAttributeValues[ALARM_DESCR].data(), groupKey.data());

   if (!groupKey.isNull())
   {
      UtlContainable* pContact = mContacts.findValue(&groupKey);
      if (pContact)
      {
         // Process the groupSeparator separated list of trap receiver contacts
         UtlString* contactList = dynamic_cast<UtlString*>(pContact);
         UtlTokenizer tokenList(*contactList);

         UtlString entry;
         while (tokenList.next(entry, groupSeparator))
         {
            if (!sendSnmpv2Trap(alarmAttributeValues, entry))
            {
               retval = OS_FAILED;
            }
         }
      }
   }
   return retval;
}

OsStatus TrapNotifier::init(TiXmlElement* trapElement, TiXmlElement* groupElement)
{
   OsSysLog::add(FAC_ALARM, PRI_DEBUG, "Created TrapNotifier");
   TiXmlElement* element;

   // Set the SNMP_PERSISTENT_FILE environment variable.
   // The SNMP application's persistent configuration file will be stored in this location.
   UtlString snmp_persistent_file_environ_variable("SNMP_PERSISTENT_FILE=");
   UtlString snmp_persistent_conf_file = SipXecsService::Path(SipXecsService::ConfigurationDirType);
   snmp_persistent_conf_file.append("/");
   snmp_persistent_conf_file.append(snmpApplicationName);
   snmp_persistent_conf_file.append(".conf");
   snmp_persistent_file_environ_variable.append(snmp_persistent_conf_file.data());
   putenv(strdup(snmp_persistent_file_environ_variable.data()));

   // Prepare the default directories to search for MIB files
   netsnmp_get_mib_directory();

   UtlString mib_file_path = SipXecsService::Path(SipXecsService::DataDirType);
   mib_file_path.append("/mibs");
   // Prepend "+" to indicate that it is an additional directory that needs to be searched for MIB files.
   mib_file_path.prepend("+");
   // Set the directory to search for additional MIB files (SIPXECS-ALARM-NOTIFICATION-MIB)
   netsnmp_set_mib_directory(mib_file_path.data());

   // Disable all logs and
   // remove all log handlers, then register a null handler.
   snmp_disable_log();
   // Get the pointer to the head node of the list of all log handlers
   netsnmp_log_handler* snmp_logh_head = get_logh_head();
   while (snmp_logh_head != NULL)
   {
      netsnmp_remove_loghandler(snmp_logh_head);
      snmp_logh_head = get_logh_head();
   }
   netsnmp_log_handler* log_h;
   int priority = 7; // net-snmp logging level LOG_DEBUG
   int pri_max = 0;  // net-snmp logging level LOG_EMERG
   log_h = netsnmp_register_loghandler(NETSNMP_LOGHANDLER_NONE, priority);
   if (log_h)
   {
      log_h->pri_max = pri_max;
   }

   // Extract the trap receiver addresses, port number and community string from the alarm groups configuration file
   element = groupElement->FirstChildElement("group");
   for (; element; element=element->NextSiblingElement("group") )
   {
      UtlString groupName = element->Attribute("id");
      UtlString contactList;

      if (!groupName.isNull())
      {
         OsSysLog::add(FAC_ALARM, PRI_DEBUG, "Processing alarm group name: %s", groupName.data());

         TiXmlElement* trapElement = element->FirstChildElement("trap");

         TiXmlElement* toElement = trapElement->FirstChildElement("trap-receiver-contact");
         for (; toElement; toElement=toElement->NextSiblingElement("trap-receiver-contact") )
         {
            // Store the trap receiver contact info in the below format
            // trapReceiverAddress<unitSeparator>portNumber<unitSeparator>communityString<groupSeparator> ...
            UtlString addressStr, portStr, communityStr;
            TiXmlElement* addressElement = toElement->FirstChildElement("trap-receiver-address");
            textContentShallow(addressStr, addressElement);
            if (addressStr)
            {
               contactList.append(addressStr);
               contactList.append(unitSeparator);
            }
            TiXmlElement* portElement = toElement->FirstChildElement("port-number");
            textContentShallow(portStr, portElement);
            if (portStr)
            {
               contactList.append(portStr);
               contactList.append(unitSeparator);
            }
            TiXmlElement* communityElement = toElement->FirstChildElement("community-string");
            textContentShallow(communityStr, communityElement);
            if (communityStr)
            {
               contactList.append(communityStr);
               contactList.append(groupSeparator);
            }
         }

         // Store the contact list as a groupSeparator separated string that UtlHashMap can handle.
         mContacts.insertKeyAndValue(new UtlString(groupName), new UtlString(contactList));
      }
   }
   return OS_SUCCESS;
}

bool TrapNotifier::sendSnmpv2Trap(
      const UtlString alarmAttributeValues[],
      const UtlString trap_receiver)
{
   // .iso.org.dod.internet.mgmt.mib-2.system.sysUpTime.0
   const oid objid_sysuptime[] = { 1, 3, 6, 1, 2, 1, 1, 3, 0 };
   // snmpTrapOID
   const oid objid_snmptrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };

   // Object Identifier of sipxecsAlarmNotification
   // .iso.org.dod.internet.private.enterprises.pingtel.sipxecs.alarms.alarmNotifications.sipxecsAlarmNotification
   const char* sipxecsAlarmNotification_oid = "SIPXECS-ALARM-NOTIFICATION-MIB::sipxecsAlarmNotification";

   // Object Identifiers of Alarm attributes in textual format
   UtlString alarm_attributes[MAX_ALARM_ATTRIBUTES] =
      {
        // .iso.org.dod.internet.private.enterprises.pingtel.sipxecs.alarms.alarmAttributes.sipxecsAlarmId.0
        "SIPXECS-ALARM-NOTIFICATION-MIB::sipxecsAlarmId.0",
        // .iso.org.dod.internet.private.enterprises.pingtel.sipxecs.alarms.alarmAttributes.sipxecsAlarmSource.0
        "SIPXECS-ALARM-NOTIFICATION-MIB::sipxecsAlarmSource.0",
        // .iso.org.dod.internet.private.enterprises.pingtel.sipxecs.alarms.alarmAttributes.sipxecsAlarmTime.0
        "SIPXECS-ALARM-NOTIFICATION-MIB::sipxecsAlarmTime.0",
        // .iso.org.dod.internet.private.enterprises.pingtel.sipxecs.alarms.alarmAttributes.sipxecsAlarmSeverity.0
        "SIPXECS-ALARM-NOTIFICATION-MIB::sipxecsAlarmSeverity.0",
        // .iso.org.dod.internet.private.enterprises.pingtel.sipxecs.alarms.alarmAttributes.sipxecsAlarmDescr.0
        "SIPXECS-ALARM-NOTIFICATION-MIB::sipxecsAlarmDescr.0"
      };

   // OID of alarm attributes in numeric format
   oid alarm_attribute_oid[MAX_ALARM_ATTRIBUTES][MAX_OID_LEN];
   // Length of alarm attribute OID
   size_t alarm_attribute_oid_len[MAX_ALARM_ATTRIBUTES] =
      { MAX_OID_LEN, MAX_OID_LEN, MAX_OID_LEN, MAX_OID_LEN, MAX_OID_LEN };

   netsnmp_session session;
   netsnmp_session* sessionHandle;
   netsnmp_pdu* pdu;
   char* sys_up_time = NULL;

   // The variable "trap_receiver" has information of one trap receiver in the below format
   // trapReceiverAddress<unitSeparator>portNumber<unitSeparator>communityString
   UtlTokenizer tokenL(trap_receiver);
   UtlString trapReceiverAddress, portNumber, communityString;
   // Retrieve the trap receiver address
   tokenL.next(trapReceiverAddress, unitSeparator);
   // Retrieve the port number
   tokenL.next(portNumber, unitSeparator);
   // Retrieve the community string
   tokenL.next(communityString, unitSeparator);
   // Append ":port number" to the trap receiver address
   trapReceiverAddress.append(":");
   trapReceiverAddress.append(portNumber);

   OsSysLog::add(FAC_ALARM, PRI_DEBUG,"Sending SNMPv2 trap to %s using %s as the community string", trapReceiverAddress.data(), communityString.data());

   // Initialize session to default values
   snmp_sess_init(&session);
   session.version = SNMP_VERSION_2c; // Set the SNMP version to v2c

   // Read in MIB database and initialize the snmp library
   init_snmp(snmpApplicationName);

   // Set the trap_receiver_address:port_number as SNMPv2 trap receiver for this SNMP session
   session.peername = (char *)trapReceiverAddress.data();
   // Set the community string
   session.community = (unsigned char *)communityString.data();
   session.community_len = communityString.length();

   session.callback = NULL;
   session.callback_magic = NULL;

   // Open the SNMP session
   sessionHandle = snmp_open(&session);
   if (sessionHandle == NULL)
   {
      OsSysLog::add(FAC_ALARM, PRI_ERR,"Failed to open a SNMP session. Trap receiver address = %s, Community String =%s", trapReceiverAddress.data(), communityString.data());
      return false;
   }

   // Create the SNMPv2 trap PDU
   pdu = snmp_pdu_create(SNMP_MSG_TRAP2);
   if (!pdu)
   {
      OsSysLog::add(FAC_ALARM, PRI_ERR,"Failed to create SNMPv2 Trap PDU");
      snmp_close(sessionHandle);
      return false;
   }
   long sysuptime;
   char csysuptime[20];
   sysuptime = get_uptime();
   sprintf(csysuptime, "%ld", sysuptime);
   sys_up_time = csysuptime;
   // Add system up time (sysUpTime.0) to the SNMP PDU
   if (snmp_add_var(pdu, objid_sysuptime, OID_LENGTH(objid_sysuptime), 't', sys_up_time) != 0)
   {
      OsSysLog::add(FAC_ALARM, PRI_ERR,"Failed to add sysUpTime.0 to the SNMP trap PDU");
      snmp_close(sessionHandle);
      return false;
   }
   if (snmp_add_var(pdu, objid_snmptrap, OID_LENGTH(objid_snmptrap), 'o', sipxecsAlarmNotification_oid) != 0 )
   {
      OsSysLog::add(FAC_ALARM, PRI_ERR,"Failed to add SIPXECS-ALARM-NOTIFICATION-MIB::sipxecsAlarmNotification oid to the SNMP trap PDU");
      snmp_close(sessionHandle);
      return false;
   }

   for (int k=0; k<MAX_ALARM_ATTRIBUTES; k++)
   {
      // Read the Alarm attribute's OID from the MIB
      if (!read_objid(alarm_attributes[k].data(), alarm_attribute_oid[k], &alarm_attribute_oid_len[k]))
      {
         OsSysLog::add(FAC_ALARM, PRI_ERR, "Failed to read the OID of %s from the MIB file", alarm_attributes[k].data());
         snmp_close(sessionHandle);
         return false;
      }

      // Add the Alarm attribute OID and the alarm attribute Value to the SNMPv2 trap PDU
      if (snmp_add_var(pdu, alarm_attribute_oid[k], alarm_attribute_oid_len[k],'s', alarmAttributeValues[k].data()) != 0)
      {
         OsSysLog::add(FAC_ALARM, PRI_ERR, "Failed to add %s attribute to the SNMPv2 trap PDU", alarm_attributes[k].data());
         snmp_close(sessionHandle);
         return false;
      }
   }

   // Send the SNMPv2 trap
   if (snmp_send(sessionHandle, pdu) == 0)
   {
      snmp_free_pdu(pdu);
      OsSysLog::add(FAC_ALARM, PRI_ERR,"Failed to send SNMPv2 trap to %s", trapReceiverAddress.data());
      snmp_close(sessionHandle);
      return false;
   }
   else
   {
      OsSysLog::add(FAC_ALARM, PRI_DEBUG,"SNMPv2 trap has been sent to %s", trapReceiverAddress.data());
   }
   snmp_close(sessionHandle);
   return true;
}
/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */


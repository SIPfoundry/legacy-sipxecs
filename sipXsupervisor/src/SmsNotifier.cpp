//
//
// Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "net/MailMessage.h"
#include "os/OsSysLog.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlTokenizer.h"
#include "AlarmUtils.h"
#include "SmsNotifier.h"
#include "EmailSendTask.h"
#include "sipXecsService/SipXecsService.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SmsNotifier::SmsNotifier() :
   mContacts(),
   mSmtpServer("localhost"),
   mReplyTo("postmaster@localhost"),
   mSmsStrFrom("default {0} Alarm Notification Service"),
   mSmsStrSubject("default Alarm {0}"),
   mSmsStrHost("default Reported on: {0}")
{
}

// Destructor
SmsNotifier::~SmsNotifier()
{
   mContacts.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SmsNotifier& SmsNotifier::operator=(const SmsNotifier& rhs)
{
   if (this == &rhs) // handle the assignment to self case
   return *this;

   return *this;
}

OsStatus SmsNotifier::handleAlarm(const OsTime alarmTime,
      const UtlString& callingHost,
      const cAlarmData* alarmData,
      const UtlString& alarmMsg)
{
   OsStatus retval = OS_FAILED;

   //execute the mail command for each user
   UtlString groupKey(alarmData->getGroupName());
   if (!groupKey.isNull())
   {
      UtlContainable* pContact = mContacts.findValue(&groupKey);
      if (pContact)
      {
         // Process the comma separated list of contacts
         UtlString* contactList = dynamic_cast<UtlString*> (pContact);
         if (!contactList->isNull())
         {
            MailMessage message(mSmsStrFrom, mReplyTo, mSmtpServer);
            UtlTokenizer tokenList(*contactList);

            UtlString entry;
            while (tokenList.next(entry, ","))
            {
               message.To(entry, entry);
            }

            UtlString body;
            UtlString tempStr;

            UtlString sevStr = OsSysLog::priorityName(alarmData->getSeverity());
            body.append(alarmMsg);

            OsSysLog::add(FAC_ALARM, PRI_DEBUG, "AlarmServer: sms body is %s", body.data());

            message.Body(body);

            UtlSList subjectParams;
            UtlString codeStr(alarmData->getCode());
            UtlString titleStr(sevStr);
            subjectParams.append(&codeStr);
            subjectParams.append(&titleStr);
            assembleMsg(mSmsStrSubject, subjectParams, tempStr);
            message.Subject(tempStr);

            // delegate send to separate task so as not to block
            EmailSendTask::getInstance()->sendMessage(message);
         }
      }
   }

   return retval;
}

OsStatus SmsNotifier::init(TiXmlElement* smsElement, TiXmlElement* groupElement)
{
   OsSysLog::add(FAC_ALARM, PRI_DEBUG, "Created SmsNotifier");
   TiXmlElement* element;

   // Extract the "From" contact from the alarm configuration file
   element = smsElement->FirstChildElement("email-notification-addr");
   textContentShallow(mReplyTo, element);

   // Extract the "To" contacts from the alarm groups configuration file
   element = groupElement->FirstChildElement("group");
   for (; element; element=element->NextSiblingElement("group") )
   {
      UtlString groupName = element->Attribute("id");
      UtlString contactList;

      if (!groupName.isNull())
      {
         OsSysLog::add(FAC_ALARM, PRI_DEBUG, "Processing alarm group name: %s", groupName.data());

         TiXmlElement* smsElement = element->FirstChildElement("sms");

         TiXmlElement* toElement = smsElement->FirstChildElement("contact");
         for (; toElement; toElement=toElement->NextSiblingElement("contact") )
         {
            UtlString contactStr;
            textContentShallow(contactStr, toElement);
            if (contactStr)
            {
               contactList.append(contactStr);
               contactList.append(",");
            }
         }

         // Store the contact list as a comma separated string that UtlHashMap can handle.
         mContacts.insertKeyAndValue(new UtlString(groupName), new UtlString(contactList));
      }
   }

   return OS_SUCCESS;
}


OsStatus SmsNotifier::initStrings(TiXmlElement* smsElement)
{
   OsSysLog::add(FAC_ALARM, PRI_DEBUG, "SmsNotifier initStrings");

   TiXmlElement* element = smsElement->FirstChildElement("email-intro");

   element = smsElement->FirstChildElement("email-subject");
   textContentShallow(mSmsStrSubject, element);

   element = smsElement->FirstChildElement("email-host");
   textContentShallow(mSmsStrHost, element);

   element = smsElement->FirstChildElement("email-from");
   textContentShallow(mSmsStrFrom, element);
   assembleMsg(mSmsStrFrom, SipXecsService::Name(), mSmsStrFrom);

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */


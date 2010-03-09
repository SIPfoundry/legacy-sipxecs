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
#include "net/MailMessage.h"
#include "os/OsSysLog.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlTokenizer.h"
#include "AlarmUtils.h"
#include "EmailNotifier.h"
#include "EmailSendTask.h"
#include "sipXecsService/SipXecsService.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
EmailNotifier::EmailNotifier() :
   mContacts(),
   mSmtpServer("localhost"),
   mReplyTo("postmaster@localhost"),
   mEmailStrFrom("default {0} Alarm Notification Service"),
   mEmailStrSubject("default Alarm {0}"),
   mEmailStrIntro("default Message from {0}"),
   mEmailStrAlarm("default Alarm: {0}"),
   mEmailStrTime("default Reported at: {0}"),
   mEmailStrHost("default Reported on: {0}"),
   mEmailStrSeverity("default"),
   mEmailStrDescription("default Alarm Text: {0}"),
   mEmailStrResolution("default Suggested Resolution: {0}")
{
}

// Destructor
EmailNotifier::~EmailNotifier()
{
   mContacts.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
EmailNotifier& EmailNotifier::operator=(const EmailNotifier& rhs)
{
   if (this == &rhs) // handle the assignment to self case
   return *this;

   return *this;
}

OsStatus EmailNotifier::handleAlarm(const OsTime alarmTime,
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
            MailMessage message(mEmailStrFrom, mReplyTo, mSmtpServer);
            UtlTokenizer tokenList(*contactList);

            UtlString entry;
            while (tokenList.next(entry, ","))
            {
               message.To(entry, entry);
            }

            UtlString body;
            UtlString tempStr;

            body = mEmailStrIntro;
            body.append("\n");

            assembleMsg(mEmailStrAlarm, alarmData->getCode(), tempStr);
            body.append(tempStr);
            body.append("\n");

            assembleMsg(mEmailStrHost, callingHost, tempStr);
            body.append(tempStr);
            body.append("\n");

            OsDateTime logTime(alarmTime);
            UtlString strTime;
            logTime.getIsoTimeStringZus(strTime);
            assembleMsg(mEmailStrTime, strTime, tempStr);
            body.append(tempStr);
            body.append("\n");

            UtlString sevStr = OsSysLog::priorityName(alarmData->getSeverity());
            assembleMsg(mEmailStrSeverity, sevStr, tempStr);
            body.append(tempStr);
            body.append("\n");
            assembleMsg(mEmailStrDescription, alarmMsg, tempStr);
            body.append(tempStr);
            body.append("\n");
            assembleMsg(mEmailStrResolution, alarmData->getResolution(), tempStr);
            body.append(tempStr);
            OsSysLog::add(FAC_ALARM, PRI_DEBUG, "AlarmServer: email body is %s", body.data());

            message.Body(body);

            UtlSList subjectParams;
            UtlString codeStr(alarmData->getCode());
            UtlString titleStr(alarmData->getShortTitle());
            subjectParams.append(&codeStr);
            subjectParams.append(&titleStr);
            assembleMsg(mEmailStrSubject, subjectParams, tempStr);
            message.Subject(tempStr);

            // delegate send to separate task so as not to block
            EmailSendTask::getInstance()->sendMessage(message);
         }
      }
   }

   return retval;
}

OsStatus EmailNotifier::init(TiXmlElement* emailElement, TiXmlElement* groupElement)
{
   OsSysLog::add(FAC_ALARM, PRI_DEBUG, "Created EmailNotifier");
   TiXmlElement* element;

   // Extract the "From" contact from the alarm configuration file
   element = emailElement->FirstChildElement("email-notification-addr");
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

         TiXmlElement* emailElement = element->FirstChildElement("email");

         TiXmlElement* toElement = emailElement->FirstChildElement("contact");
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

OsStatus EmailNotifier::initStrings(TiXmlElement* emailElement)
{
   TiXmlElement* element = emailElement->FirstChildElement("email-intro");
   textContentShallow(mEmailStrIntro, element);
   assembleMsg(mEmailStrIntro, SipXecsService::Name(), mEmailStrIntro);

   element = emailElement->FirstChildElement("email-subject");
   textContentShallow(mEmailStrSubject, element);

   element = emailElement->FirstChildElement("email-alarm");
   textContentShallow(mEmailStrAlarm, element);

   element = emailElement->FirstChildElement("email-time");
   textContentShallow(mEmailStrTime, element);

   element = emailElement->FirstChildElement("email-host");
   textContentShallow(mEmailStrHost, element);

   element = emailElement->FirstChildElement("email-severity");
   textContentShallow(mEmailStrSeverity, element);

   element = emailElement->FirstChildElement("email-description");
   textContentShallow(mEmailStrDescription, element);

   element = emailElement->FirstChildElement("email-resolution");
   textContentShallow(mEmailStrResolution, element);

   element = emailElement->FirstChildElement("email-from");
   textContentShallow(mEmailStrFrom, element);
   assembleMsg(mEmailStrFrom, SipXecsService::Name(), mEmailStrFrom);

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */


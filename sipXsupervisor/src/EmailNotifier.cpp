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
#include "AlarmUtils.h"
#include "EmailNotifier.h"
#include "EmailSendTask.h"

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
   mEmailStrFrom("default Alarm Notification Service"),
   mEmailStrSubject("default Alarm {0}"),
   mEmailStrIntro("default Message from sipXecs"),
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

   MailMessage message( mEmailStrFrom, mReplyTo, mSmtpServer );
   message.Body(body);
   
   UtlSList subjectParams;
   UtlString codeStr(alarmData->getCode());
   UtlString titleStr(alarmData->getShortTitle());
   subjectParams.append(&codeStr);
   subjectParams.append(&titleStr);
   assembleMsg(mEmailStrSubject, subjectParams, tempStr);
   message.Subject(tempStr);

   //execute the mail command for each user
   UtlSListIterator iterator(mContacts);
   UtlString* pObject;
   while ( (pObject = dynamic_cast<UtlString*>(iterator())))
   {
      message.To(*pObject, *pObject);
   }

   // delegate send to separate task so as not to block
   EmailSendTask::getInstance()->sendMessage(message);

   return retval;
}

OsStatus EmailNotifier::init(TiXmlElement* emailElement)
{
   OsSysLog::add(FAC_ALARM, PRI_DEBUG, "Created EmailNotifier");
   
   TiXmlElement* element = emailElement->FirstChildElement("email-notification-addr");
   textContentShallow(mReplyTo, element);

   element = emailElement->FirstChildElement("contact");
   for (; element; element=element->NextSiblingElement("contact") )
   {
      UtlString contactStr;
      textContentShallow(contactStr, element);
      if (contactStr)
      {
         mContacts.append(new UtlString(contactStr));
      }
   }
   return OS_SUCCESS;
}

OsStatus EmailNotifier::initStrings(TiXmlElement* emailElement)
{   
   TiXmlElement* element = emailElement->FirstChildElement("email-intro");
   textContentShallow(mEmailStrIntro, element);

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
   
   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */


//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include "net/SipNotifyStateTask.h"
#include "net/SipUserAgent.h"
#include "net/HttpBody.h"
#include "net/NameValueTokenizer.h"
#include "os/OsTimer.h"
#include "os/OsQueuedEvent.h"
#include "os/OsEventMsg.h"
#include "os/OsSysLog.h"

#ifdef _VXWORKS
#include "pingerjni/JXAPI.h"
#include "cmd/VersionCatalog.h"
#include "cmd/CommandSecurityPolicy.h"
#include "resparse/vxw/hd_string.h"
#endif

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define RUN_SCRIPT_BUSY_DELAY    60  // When the phone is busy, this is the
                                     // amount of time the phone will wait
                                     // before retrying.

// STATIC VARIABLE INITIALIZATIONS
// STRUCTURES
struct tagRunScriptInfo
{
   UtlString*              pContent ;
#ifdef _VXWORKS
   CommandSecurityPolicy* pPolicy ;
#endif
} ;   // Used to package data through the OsEvent/queue mechanism


/* //////////////////////////// PUBLIC //////////////////////////////////// */
void SipNotifyStateTask::defaultReboot()
{
    osPrintf("SipNotifyStateTask::defaultReboot request to reboot\n");
}

void SipNotifyStateTask::defaultBinaryMessageWaiting(const char* toUrl,
                                  UtlBoolean newMessages)
{
    osPrintf("SipNotifyStateTask::defaultBinaryMessageWaiting Message status for: %s\n%snew messages available\n",
        toUrl, newMessages ? "" : "NO ");
}

void SipNotifyStateTask::defaultDetailMessageWaiting(const char* toUrl,
                                  const char* messageMediaType,
                                  UtlBoolean absoluteValues,
                                  int totalNewMessages,
                                  int totalOldMessages,
                                  int totalUntouchedMessages,
                                  int urgentUntouchedMessages,
                                  int totalSkippedMessages,
                                  int urgentSkippedMessages,
                                  int totalFlaggedMessages,
                                  int urgentFlaggedMessages,
                                  int totalReadMessages,
                                  int urgentReadMessages,
                                  int totalAnsweredMessages,
                                  int urgentAnsweredMessages,
                                  int totalDeletedMessages,
                                  int urgentDeletedMessages)
{
    osPrintf("SipNotifyStateTask::defaultDetailMessageWaiting\n\
Messages status for URL: %s\n\
Message media type: %s\n\
%s\n\
%d new messages\n\
%d old messages\n\
Status\t\tTotal\tUrgent\n\
Untouched\t%d\t%d\n\
Skipped\t\t%d\t%d\n\
Flagged\t\t%d\t%d\n\
Read\t\t%d\t%d\n\
Answered\t%d\t%d\n\
Deleted\t\t%d\t%d\n",
               toUrl,
               messageMediaType,
               absoluteValues ? "Absolute counts:" : "Message deltas:",
               totalNewMessages,
               totalOldMessages,
               totalUntouchedMessages,
               urgentUntouchedMessages,
               totalSkippedMessages,
               urgentSkippedMessages,
               totalFlaggedMessages,
               urgentFlaggedMessages,
               totalReadMessages,
               urgentReadMessages,
               totalAnsweredMessages,
               urgentAnsweredMessages,
               totalDeletedMessages,
               urgentDeletedMessages);
}

/* ============================ CREATORS ================================== */

// Constructor
SipNotifyStateTask::SipNotifyStateTask(const UtlString& checkSyncPolicy,
                                       SipUserAgent* pSipUserAgent) :
   OsServerTask("SipNotifyStateTask-%d"),
   mCheckSyncPolicy(checkSyncPolicy)
{
   mpSipUserAgent = pSipUserAgent;
   mpRebootFunction = NULL;
   mpBinaryMessageWaitingFunction = NULL;
   mpDetailedMessageWaitingFunction = NULL;
   mpRunScriptTimer = NULL ;
   mpRunScriptEvent = NULL ;
}

// Destructor
SipNotifyStateTask::~SipNotifyStateTask()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipNotifyStateTask&
SipNotifyStateTask::operator=(const SipNotifyStateTask& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean SipNotifyStateTask::handleMessage(OsMsg& eventMessage)
{
        int msgType = eventMessage.getMsgType();
        int msgSubType = eventMessage.getMsgSubType();

    // SIP message
    if(msgType == OsMsg::PHONE_APP &&
       msgSubType == SipMessage::NET_SIP_MESSAGE)
    {
        const SipMessage* sipMessage = ((SipMessageEvent&)eventMessage).getMessage();

        // If this is a NOTIFY request
        UtlString method;
        if(sipMessage) sipMessage->getRequestMethod(&method);
        method.toUpper();
        if(sipMessage &&
            method.compareTo(SIP_NOTIFY_METHOD) == 0 &&
            !sipMessage->isResponse())
        {
            osPrintf("SipNotifyStateTask::handleMessage got NOTIFY message\n");
            // Loop through the event fields
            int eventIndex = 0;
            const char* eventFieldCharValue = NULL;
            UtlString eventField;
            if ((eventFieldCharValue = sipMessage->getHeaderValue(eventIndex, SIP_EVENT_FIELD)))
            {
                eventField = eventFieldCharValue;
                eventField.toLower();
                                // Need to get the body to if it is text base.
                // We are only going to support text format for now.
                UtlString contentType;
                sipMessage->getContentType(&contentType);
                contentType.toLower();
                const HttpBody* httpBody;


                // If this is a message waiting indication
                // Looking for header: "Event: message-summary"
                // This is for the draft-mahy-sip-message-waiting-00.txt version
                // of the draft.
                if(eventField.index(SIP_EVENT_MESSAGE_SUMMARY, 0, UtlString::ignoreCase) == 0 &&
                                        (contentType.index(CONTENT_TYPE_TEXT_PLAIN, 0, UtlString::ignoreCase) == 0 &&
                        (httpBody = sipMessage->getBody()) != NULL))
                {
                    osPrintf("SipNotifyStateTask::handleMessage found message-summary event ContentType plain/text\n");
                                        UtlDList bodyHeaderNameValues;
                    const char* bodyBytes;
                    ssize_t bodyLength;
                    httpBody->getBytes(&bodyBytes, &bodyLength);
                    HttpMessage::parseHeaders(bodyBytes, bodyLength,
                          bodyHeaderNameValues);

                    UtlString toField;
                    sipMessage->getToField(&toField);

                    // Loop through the body header nameValue pairs
                    UtlDListIterator iterator(bodyHeaderNameValues);
                        NameValuePair* nv;
                        while ((nv = (NameValuePair*) iterator()))
                        {
                        HttpMessage::cannonizeToken(*nv);

                        // if binary message waiting status
                        osPrintf("SipNotifyStateTask::handleMessage body field name: %s value: %s\n",
                            nv->data(), nv->getValue());
                        // Check for either a "Messages-Waiting" or a "Message-Waiting" field.
                        // "Messages-Waiting" is what's specified in the Internet draft;
                        // "Message-Waiting" is needed for backward compatibility.
                        if((strcmp(nv->data(), "Messages-Waiting") == 0) ||
                           (strcmp(nv->data(), "Message-Waiting") == 0))
                        {
                            if(mpBinaryMessageWaitingFunction)
                            {
                                UtlString binaryStatus = nv->getValue();
                                binaryStatus.toLower();
                                UtlBoolean newMessages = FALSE;
                                if(binaryStatus.compareTo("yes") == 0) newMessages = TRUE;

                                mpBinaryMessageWaitingFunction(toField.data(),
                                    newMessages);
                                                                                        binaryStatus.remove(0);

                            }
                        }

                        // if detailed, media specific message status
                        else if(mpDetailedMessageWaitingFunction)
                        {
                            UtlString status = nv->getValue();

                            // The name of the header is the message media type
                            // parse the message counts out
                            // The format is generically:
                            // <mediaType>: <newCount/oldCount> [<flag>: <totalCount>[/<urgentCount>]] ...
                            // Where there may be 0 or more flags

                            int newMessages = -1;
                            int oldMessages = -1;
                            int totalUntouched = -1;
                            int urgentUntouched = -1;
                            int totalSkipped = -1;
                            int urgentSkipped = -1;
                            int totalFlagged = -1;
                            int urgentFlagged = -1;
                            int totalRead = -1;
                            int urgentRead = -1;
                            int totalAnswered = -1;
                            int urgentAnswered = -1;
                            int totalDeleted = -1;
                            int urgentDeleted = -1;

                            // Get the number of new messages
                            UtlString numberString;
                            UtlBoolean absoluteValues = TRUE;
                            NameValueTokenizer::getSubField(status.data(), 0,
                                                       " \t/;", &numberString);

                            // If there is a + or - the numbers are in delta values
                            if(numberString.data()[0] == '+' ||
                                numberString.data()[0] == '-')
                            {
                                absoluteValues = FALSE;
                                oldMessages = 0;
                                totalUntouched = 0;
                                urgentUntouched = 0;
                                totalSkipped = 0;
                                urgentSkipped = 0;
                                totalFlagged = 0;
                                urgentFlagged = 0;
                                totalRead = 0;
                                urgentRead = 0;
                                totalAnswered = 0;
                                urgentAnswered = 0;
                                totalDeleted = 0;
                                urgentDeleted = 0;
                            }

                            if(!numberString.isNull())
                            {
                               newMessages = atoi(numberString.data());
                            }

                            // Get the number of old messages
                            NameValueTokenizer::getSubField(status.data(), 1,
                                                       " \t/;", &numberString);
                            if(!numberString.isNull())
                                oldMessages = atoi(numberString.data());

                            int parameterIndex = 2;
                            UtlString flag;
                            NameValueTokenizer::getSubField(status.data(), parameterIndex,
                                                       " \t/:;", &flag);
                            flag.toLower();

                            // Loop through the flags
                            do
                            {
                                osPrintf("SipNotifyStateTask::handleMessage flag=\'%s\'\n",
                                    flag.data());
                                switch(flag.data()[0])
                                {
                                case 'u':
                                    parameterIndex += getStatusTotalUrgent(status,
                                        absoluteValues,
                                        parameterIndex + 1,
                                        totalUntouched, urgentUntouched);
                                    break;

                                case 's':
                                    parameterIndex += getStatusTotalUrgent(status,
                                        absoluteValues,
                                        parameterIndex + 1,
                                        totalSkipped, urgentSkipped);
                                    break;

                                case 'f':
                                    parameterIndex += getStatusTotalUrgent(status,
                                        absoluteValues,
                                        parameterIndex + 1,
                                        totalFlagged, urgentFlagged);
                                    break;

                                case 'r':
                                    parameterIndex += getStatusTotalUrgent(status,
                                        absoluteValues,
                                        parameterIndex + 1,
                                        totalRead, urgentRead);
                                    break;

                                case 'a':
                                    parameterIndex += getStatusTotalUrgent(status,
                                        absoluteValues,
                                        parameterIndex + 1,
                                        totalAnswered, urgentAnswered);
                                    break;

                                case 'd':
                                    parameterIndex += getStatusTotalUrgent(status,
                                        absoluteValues,
                                        parameterIndex + 1,
                                        totalDeleted, urgentDeleted);
                                    break;

                                default:
                                    break;
                                }

                                parameterIndex+=2;
                                NameValueTokenizer::getSubField(status.data(), parameterIndex,
                                                       " \t/:;", &flag);
                            }
                            while(!flag.isNull());

                            // Make the MWI call back for the detailed status
                            mpDetailedMessageWaitingFunction(toField.data(),
                                nv->data(), // message media type
                                absoluteValues,
                                newMessages,
                                oldMessages,
                                totalUntouched,
                                urgentUntouched,
                                totalSkipped,
                                urgentSkipped,
                                totalFlagged,
                                urgentFlagged,
                                totalRead,
                                urgentRead,
                                totalAnswered,
                                urgentAnswered,
                                totalDeleted,
                                urgentDeleted);

                                                                status.remove(0);
                                                                numberString.remove(0);
                                                                flag.remove(0);
                                }
                                delete nv;
                                        }
                                        // Send a 200 OK response
                                        if(mpSipUserAgent)
                                        {
                                                SipMessage notifyOkResponse;
                                                notifyOkResponse.setOkResponseData(sipMessage);
                                                mpSipUserAgent->send(notifyOkResponse);
                                        }
                }

                // If this is a message waiting indication
                // Looking for header: "Event: simple-message-summary"
                // This is for the draft-mahy-sip-message-waiting-01.txt version of the draft.
                else if(((eventField.index(SIP_EVENT_SIMPLE_MESSAGE_SUMMARY, 0, UtlString::ignoreCase) == 0) ||
                                        (eventField.index(SIP_EVENT_MESSAGE_SUMMARY, 0, UtlString::ignoreCase) == 0)) &&
                                        (contentType.index(CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY, 0, UtlString::ignoreCase) == 0 &&
                        (httpBody = sipMessage->getBody()) != NULL))
                {
                    osPrintf("SipNotifyStateTask::handleMessage found simple-message-summary or message-summary event\n");
                    osPrintf("SipNotifyStateTask::handleMessage got application/simple-message-summary body\n");
                    UtlDList bodyHeaderNameValues;
                    const char* bodyBytes;
                    ssize_t bodyLength;
                    httpBody->getBytes(&bodyBytes, &bodyLength);
                    HttpMessage::parseHeaders(bodyBytes, bodyLength,
                          bodyHeaderNameValues);

                    UtlBoolean bBinaryNotification = true ;
                    UtlBoolean bNewMessages = false ;
                    UtlString  strMediaType ;
                    int iNewMessages = 0 ;
                    int iOldMessages = 0 ;

                    UtlString toField;
                    sipMessage->getToField(&toField);

                    // Loop through the body header nameValue pairs
                    UtlDListIterator iterator(bodyHeaderNameValues);
                        NameValuePair* nv;
                    while ((nv = (NameValuePair*) iterator()))
                        {
                        HttpMessage::cannonizeToken(*nv);

                        // if binary message waiting status
                        osPrintf("SipNotifyStateTask::handleMessage body field name: %s value: %s\n",
                            nv->data(), nv->getValue());
                        // Check for either a "Messages-Waiting" or a "Message-Waiting" field.
                        // "Messages-Waiting" is what's specified in the Internet draft;
                        // "Message-Waiting" is needed for backward compatibility.
                        if((strcmp(nv->data(), "Messages-Waiting") == 0) ||
                           (strcmp(nv->data(), "Message-Waiting") == 0))
                        {
                            UtlString binaryStatus = nv->getValue();
                            binaryStatus.toLower();
                            if (binaryStatus.compareTo("yes") == 0)
                                bNewMessages = TRUE ;
                            continue ;
                        }

                        if(strcmp(nv->data(), "Voice-Message") == 0)
                        {
                            UtlString numberString ;

                            UtlString status = nv->getValue() ;
                            status.toLower() ;

                            // Parse number of new messages
                            NameValueTokenizer::getSubField(status.data(), 0,
                                                       " \t/;()", &numberString);
                            iNewMessages = atoi(numberString.data());

                            // Parse number of old messages
                            NameValueTokenizer::getSubField(status.data(), 1,
                                                       " \t/;()", &numberString);
                            iOldMessages = atoi(numberString.data());

                            strMediaType = nv->data() ;
                            bBinaryNotification = false ;
                            continue ;
                        }
                    }

                    // If binary, notifiy the binary listener, otherwise
                    // notify either binary or detailed
                    if (bBinaryNotification) {
                        // Notify Binary Listener
                        if (mpBinaryMessageWaitingFunction != NULL) {
                            mpBinaryMessageWaitingFunction(toField.data(),
                                    bNewMessages) ;
                        }
                    } else {
                        if (mpDetailedMessageWaitingFunction != NULL) {
                            mpDetailedMessageWaitingFunction(toField.data(),
                                strMediaType.data(),
                                true,
                                iNewMessages,
                                iOldMessages,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0);
                        }
                    }

                    // Send a 200 OK response
                    if(mpSipUserAgent)
                    {
                        SipMessage notifyOkResponse;
                        notifyOkResponse.setOkResponseData(sipMessage);
                        mpSipUserAgent->send(notifyOkResponse);
                    }
                }

                // If this is a resync notification
                // Looking for header: "Event: check-sync"
                else if (eventField.index(SIP_EVENT_CHECK_SYNC) == 0)
                {
                   handleCheckSyncEvent(sipMessage) ;
                }
                else
                {
                   osPrintf("Unhandled NOTIFY event type: %s\n", eventField.data()) ;
                   syslog(FAC_SIP, PRI_WARNING,
                        "Unhandled NOTIFY event type: %s", eventField.data()) ;
                }

                eventIndex++;
                                    eventField.remove(0);
             }
        }
        method.remove(0);
    }
    else if (eventMessage.getMsgType() == OsMsg::OS_EVENT)
    {
        OsEventMsg* pEventMsg = (OsEventMsg*) &eventMessage;
        void* userData ;
        intptr_t eventData ;
        pEventMsg->getEventData(eventData);
        pEventMsg->getUserData(userData);

        if (eventData == (intptr_t)mpRunScriptTimer)
        {
            // Restart Event: handle the event and clean up the temp data
            struct tagRunScriptInfo* pInfo =
                    (struct tagRunScriptInfo*) userData ;
            if (pInfo != NULL)
            {
// Ideally remove this dependency on phone library
#ifdef _VXWORKS
                doRunScript(pInfo->pContent, pInfo->pPolicy) ;
#else
                assert("OPENDEV PORT: Unexpected code path for softphone");
#endif
                delete pInfo ;
            }
        }
    }

    return(TRUE);
}

/* ============================ ACCESSORS ================================= */

void SipNotifyStateTask::setRebootFunction(void (*rebootNotifyFunction)())
{
    mpRebootFunction = rebootNotifyFunction;
}

void SipNotifyStateTask::setBinaryMessageWaitingFunction(void (*binaryMessageWaitingFunc)(const char* toUrl,
                                        UtlBoolean newMessages))
{
    mpBinaryMessageWaitingFunction = binaryMessageWaitingFunc;
}

void SipNotifyStateTask::setDetailMessageWaitingFunction(void (*detailMessageWaitingFunction)(
                                      const char* toUrl,
                                      const char* messageMediaType,
                                      UtlBoolean absoluteValues,
                                      int totalNewMessages,
                                      int totalOldMessages,
                                      int totalUntouchedMessages,
                                      int urgentUntouchedMessages,
                                      int totalSkippedMessages,
                                      int urgentSkippedMessages,
                                      int totalFlaggedMessages,
                                      int urgentFlaggedMessages,
                                      int totalReadMessages,
                                      int urgentReadMessages,
                                      int totalAnsweredMessages,
                                      int urgentAnsweredMessages,
                                      int totalDeletedMessages,
                                      int urgentDeletedMessages))
{
    mpDetailedMessageWaitingFunction = detailMessageWaitingFunction;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

OsStatus SipNotifyStateTask::handleCheckSyncEvent(const SipMessage* source)
{
   OsStatus status = OS_SUCCESS ;   // Regardless of our success, we processed the event
   UtlString *pContent = NULL ;

   // First and foremost; send an ack back to the requestor
   if(mpSipUserAgent)
   {
      SipMessage notifyOkResponse;
      notifyOkResponse.setOkResponseData(source);
      mpSipUserAgent->send(notifyOkResponse);
   }

   // Figure out if a valid script file was included
   const HttpBody* body = source->getBody() ;
   if (body != NULL)
   {
      if (strcasecmp(body->getContentType(), CONTENT_TYPE_XPRESSA_SCRIPT) == 0)
      {
         pContent = new UtlString() ;

         ssize_t length = 0 ;
         body->getBytes(pContent, &length) ;
         if (pContent->isNull())
         {
            delete pContent ;
            pContent = NULL ;
         }
      }
   }


   // Execute according to policy
   if ((mCheckSyncPolicy.compareTo("SCRIPT", UtlString::ignoreCase) == 0) &&
         (pContent != NULL))
   {
#ifdef _VXWORKS
      //
      // Execute upgrade script
      //
      CommandSecurityPolicy *pPolicy = new CommandSecurityPolicy();

      pPolicy->setPolicy(CommandSecurityPolicy::CPT_RESTRICTIVE) ;
      pPolicy->permitCommand("reboot") ;
      pPolicy->permitCommand("confirm") ;
      pPolicy->permitCommand("factoryDefaults") ;
      pPolicy->permitCommand("rm", "/flash0/cache.ser") ;
      pPolicy->permitCommand("rm", "/flash0/app-config") ;
      pPolicy->permitCommand("rm", "/flash0/user-config") ;
      pPolicy->permitCommand("rm", "/flash0/pinger-config") ;
      pPolicy->permitCommand("rm", "/flash0/config-config") ;
      pPolicy->permitCommand("rm", "/flash0/local-config") ;
      pPolicy->permitCommand("rm", "/flash0/upgrade-log") ;
      pPolicy->permitCommand("rm", "/flash0/syslog.txt") ;

      // Schedule the script to run immediately.
      scheduleRunScript(pContent, pPolicy, 0) ;
      pContent = NULL ; // null out pContent so that we don't delete it now.
#endif
   }
   else if (  (mCheckSyncPolicy.compareTo("ENABLE", UtlString::ignoreCase) == 0) ||
         (mCheckSyncPolicy.compareTo("REBOOT", UtlString::ignoreCase) == 0) ||
         (mCheckSyncPolicy.compareTo("SCRIPT", UtlString::ignoreCase) == 0))
   {
      //
      // Perform a simple reboot
      //

      syslog(FAC_UPGRADE, PRI_NOTICE,
            "Rebooting in response to a check-sync event") ;

      if (mpRebootFunction != NULL)
      {
         // Call the reboot handler
         mpRebootFunction() ;
      }
   }
   else
   {
      syslog(FAC_UPGRADE, PRI_NOTICE,
            "Ignoring check-sync; Setting not enabled") ;
      //
      // Do nothing; check-sync is disabled.
      //
   }


   if (pContent != NULL)
   {
      delete pContent ;
      pContent = NULL ;
   }

   return status ;
}

// Ideally remove this dependency for hardphone too
#ifdef _VXWORKS

// Schedule a restart sometime in the future
UtlBoolean SipNotifyStateTask::scheduleRunScript(UtlString* pContent, CommandSecurityPolicy *pPolicy, int seconds)
{
   // Free the restart timer and event
   if (mpRunScriptTimer != NULL)
   {
      mpRunScriptTimer->stop() ;
      delete mpRunScriptTimer ;
      mpRunScriptTimer = NULL ;
   }
   if (mpRunScriptEvent != NULL)
   {
      delete mpRunScriptEvent ;
      mpRunScriptEvent = NULL ;
   }

   // Create a temp data structure to communicate with the handler
   struct tagRunScriptInfo* pInfo = new struct tagRunScriptInfo ;
   pInfo->pContent = pContent ;
   pInfo->pPolicy = pPolicy ;

   // Create the event/timer
   mpRunScriptEvent = new OsQueuedEvent(*getMessageQueue(), 0) ;
   mpRunScriptEvent->setUserData(pInfo) ;
   mpRunScriptTimer = new OsTimer(*mpRunScriptEvent) ;

   // Force a min of 1 seconds
   if (seconds <= 0)
      seconds = 1 ;

   // Finally, set the timer
   mpRunScriptTimer->oneshotAfter(OsTime(seconds, 0)) ;

   return OS_SUCCESS ;
}


OsStatus SipNotifyStateTask::doRunScript(UtlString* pContent, CommandSecurityPolicy *pPolicy)
{
   OsStatus status ;

   if (JXAPI_IsPhoneBusy())
   {
      syslog(FAC_UPGRADE, PRI_INFO,
            "Phone busy; rescheduling check-sync script execution in %s seconds", RUN_SCRIPT_BUSY_DELAY) ;

      // The phone is busy; reschedule the script until later
      scheduleRunScript(pContent, pPolicy, RUN_SCRIPT_BUSY_DELAY) ;
      status = OS_BUSY ;
   }
   else
   {
      syslog(FAC_UPGRADE, PRI_NOTICE,
            "Executing check-sync script under resticted security policy") ;

      // Perform the upgrade (this is blocking)
#ifdef _VXWORKS
      OsStatus status = VersionCatalog::installScript(*pContent, pPolicy) ;
#else
      assert("OPENDEV PORT:Unexpected code path");
#endif

      // Clean up
      delete pContent ;
      delete pPolicy ;
   }

   return status ;
}

#endif

/* //////////////////////////// PRIVATE /////////////////////////////////// */
UtlBoolean SipNotifyStateTask::getStatusTotalUrgent(const char* status,
                                                   UtlBoolean absoluteValues,
                                                 int parameterIndex,
                                                 int& total,
                                                 int& urgent)
{
    UtlString numberString;
    UtlBoolean urgentFound = FALSE;

    // Get the total for this type of status
    NameValueTokenizer::getSubField(status, parameterIndex,
                   " \t/;:", &numberString);
    if(!numberString.isNull())
    {
        total = atoi(numberString.data());

        // Get the total for this type of status
        NameValueTokenizer::getSubField(status, parameterIndex + 1,
                       " \t/;:", &numberString);
        if(!numberString.isNull() &&
            (isdigit(numberString.data()[0]) ||
                numberString.data()[0] == '+' ||
                numberString.data()[0] == '-'))
        {
            urgent = atoi(numberString.data());
            urgentFound = TRUE;
        }
        // It is not a digit so it must be a flag
        else if(absoluteValues)
            urgent = -1;
        else
            urgent = 0;
    }
    else if(absoluteValues)
    {
        total = -1;
        urgent = -1;
    }
    else
    {
        total = 0;
        urgent = 0;
    }

        numberString.remove(0);
    // true/false the urgent count was set
    return(urgentFound);
}


/* ============================ FUNCTIONS ================================= */

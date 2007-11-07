// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "ForkingProxyCseObserver.h"
#include <net/SipUserAgent.h>
#include <os/OsDateTime.h>
#include <os/OsQueuedEvent.h>
#include "os/OsEventMsg.h"
#include <os/OsSysLog.h>

//#define TEST_PRINT 1

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const int ForkingProxyCallStateFlushInterval = 20; /* seconds */

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ForkingProxyCseObserver::ForkingProxyCseObserver(SipUserAgent&         sipUserAgent,
                                                 const UtlString&      dnsName,
                                                 CallStateEventWriter* pWriter
                                                 ) :
   OsServerTask("ForkingProxyCseObserver-%d", NULL, 2000),
   mpSipUserAgent(&sipUserAgent),
   mpBuilder(NULL),
   mpWriter(pWriter),
   mSequenceNumber(0),
   mFlushTimer(getMessageQueue(), 0)
{
   OsTime timeNow;
   OsDateTime::getCurTime(timeNow);
   UtlString event;

   if (mpWriter)
   {
      switch (pWriter->getLogType())
      {
      case CallStateEventWriter::CseLogFile:
         mpBuilder = new CallStateEventBuilder_XML(dnsName);
         break;
      case CallStateEventWriter::CseLogDatabase:
         mpBuilder = new CallStateEventBuilder_DB(dnsName);
         break;
      }
      if (mpBuilder)
      {
         if (pWriter->openLog())
         {
            mpBuilder->observerEvent(mSequenceNumber, timeNow, CallStateEventBuilder::ObserverReset,
                                     "ForkingProxyCseObserver");      
            mpBuilder->finishElement(event);      

            if (!mpWriter->writeLog(event.data()))
            {      
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "ForkingProxyCseObserver initial event log write failed - disabling writer");
               mpWriter = NULL;
            }
            else
            {
               mpWriter->flush(); // try to ensure that at least the sequence restart gets to the file 
            }
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "ForkingProxyCseObserver initial event log write failed - disabling writer");
            mpWriter = NULL;
            
            // Set correct state even if nothing is written
            mpBuilder->observerEvent(mSequenceNumber, timeNow, CallStateEventBuilder::ObserverReset, "");                 
            mpBuilder->finishElement(event);                         
         }
      }
   }

   // Set up periodic timer to flush the log file.
   mFlushTimer.periodicEvery(OsTime(), OsTime(ForkingProxyCallStateFlushInterval, 0));

   // Register to get incoming requests
   sipUserAgent.addMessageObserver(*getMessageQueue(),
                                   SIP_INVITE_METHOD, // just INVITEs
                                   TRUE, // Requests,
                                   TRUE, //Responses,
                                   TRUE, //Incoming,
                                   TRUE, //OutGoing,
                                   "", //eventName,
                                   NULL, // any session
                                   NULL // no observerData
                                   );
}

// Destructor
ForkingProxyCseObserver::~ForkingProxyCseObserver()
{
   mFlushTimer.stop();

   if (mpBuilder)
   {
      delete mpBuilder;
      mpBuilder = NULL;
   }
   if (mpWriter)
   {
      mpWriter->flush();
      mpWriter = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean ForkingProxyCseObserver::handleMessage(OsMsg& eventMessage)
{
   int msgType = eventMessage.getMsgType();
   switch (msgType)
   {
   case OsMsg::OS_EVENT:
      switch (eventMessage.getMsgSubType())
      {
      case OsEventMsg::NOTIFY:
         if (mpWriter)
         {
            mpWriter->flush();
         }
         break;
      }
      break ;
      
   case OsMsg::PHONE_APP:
   {
      SipMessage* sipMsg;

      if(SipMessageEvent::TRANSPORT_ERROR == ((SipMessageEvent&)eventMessage).getMessageStatus())
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "ForkingProxyCseObserver::handleMessage transport error");
      }
      else if((sipMsg = (SipMessage*)((SipMessageEvent&)eventMessage).getMessage()))
      {
         UtlString method;
         int       rspStatus = 0;
         UtlString rspText;
         UtlString contact;
         UtlString toTag;

         enum
            {
               UnInteresting,
               aCallSetup,
               aCallFailure,
               aCallEnd,
               aCallTransfer
            } thisMsgIs = UnInteresting;

         if (!sipMsg->isResponse())
         {
            // sipMsg is a Request
            sipMsg->getRequestMethod(&method);
            if (0==method.compareTo(SIP_INVITE_METHOD, UtlString::ignoreCase))
            {
               Url toUrl;
               sipMsg->getToUrl(toUrl);
               // explicitly, an INVITE Request
               toUrl.getFieldParameter("tag", toTag);
               if (toTag.isNull())
               {
                  thisMsgIs = aCallSetup;
               }
            }
         }
         else
         {
            // sipMsg is a Response
            int seq;
            if (sipMsg->getCSeqField(&seq, &method)) // get the method out of cseq field
            if (0==method.compareTo(SIP_INVITE_METHOD, UtlString::ignoreCase))
            { 
               // explicitly, an INVITE Response
                rspStatus = sipMsg->getResponseStatusCode();
                if (rspStatus >= SIP_4XX_CLASS_CODE) // any failure
                {
                   // a final failure - this is a CallFailure
                   thisMsgIs = aCallFailure;
                   sipMsg->getResponseStatusText(&rspText);
                }
            }
         }

#if 1
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "ForkingProxyCseObserver message is %s",                       (  thisMsgIs == UnInteresting ? "UnInteresting"
                        : thisMsgIs == aCallEnd      ? "a Call End"
                        : thisMsgIs == aCallFailure  ? "a Call Failure"
                        : thisMsgIs == aCallSetup    ? "a Call Setup"
                        : thisMsgIs == aCallTransfer ? "a Call Transfer"
                        : "BROKEN"
                        ));
#endif

         if (thisMsgIs != UnInteresting)
         {
            // get the sequence data
            mSequenceNumber++;
            
            OsTime timeNow;
            OsDateTime::getCurTime(timeNow); 

            // get the dialog information
            UtlString contact;
            sipMsg->getContactEntry(0, &contact);

            UtlString callId;
            sipMsg->getCallIdField(&callId);

            // get the To and From header fields
            Url fromUrl;
            sipMsg->getFromUrl(fromUrl);
            UtlString fromTag;
            fromUrl.getFieldParameter("tag", fromTag);

            UtlString toField;
            sipMsg->getToField(&toField);
            
            UtlString fromField;
            sipMsg->getFromField(&fromField);
            
            UtlString responseMethod;
            int cseqNumber;
            sipMsg->getCSeqField(&cseqNumber, &responseMethod);

            // construct the event record
            if (mpBuilder)
            {
               switch (thisMsgIs)
               {
               case aCallSetup:
                  mpBuilder->callRequestEvent(mSequenceNumber, timeNow, contact);
                  break;
               case aCallFailure:
                  mpBuilder->callFailureEvent(mSequenceNumber, timeNow, rspStatus, rspText);
                  break;

               default:
                  // shouldn't be possible to get here
                  OsSysLog::add(FAC_SIP, PRI_ERR, "ForkingProxyCseObserver invalid thisMsgIs");
                  break;
               }

               mpBuilder->addCallData(cseqNumber, callId, fromTag, toTag, fromField, toField);

               UtlString via;
               for (int i=0; sipMsg->getViaField(&via, i); i++)
               {
                  mpBuilder->addEventVia(via);
               }
               mpBuilder->completeCallEvent();
            
               // get the complete event record
               UtlString event;
               mpBuilder->finishElement(event);
               
               // write it to the log
               if (mpWriter)
               {
                  mpWriter->writeLog(event.data());
               }               
            }
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "ForkingProxyCseObserver getMessage returned NULL");
      }
   }
   break;
   
   default:
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "ForkingProxyCseObserver invalid message type %d", msgType );
   }
   } // end switch (msgType)
   
   return(TRUE);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


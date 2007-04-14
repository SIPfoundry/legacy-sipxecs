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
#include "AuthProxyCseObserver.h"
#include <net/SipUserAgent.h>
#include <os/OsDateTime.h>
#include "os/OsEventMsg.h"
#include <os/OsSysLog.h>

//#define TEST_PRINT 1
#define LOG_DEBUG 1

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const int AuthProxyCallStateFlushInterval = 20; /* seconds */

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
AuthProxyCseObserver::AuthProxyCseObserver(SipUserAgent&         sipUserAgent,
                                           const UtlString&      dnsName,
                                           CallStateEventWriter* pWriter
                                           ) :
   OsServerTask("AuthProxyCseObserver-%d", NULL, 2000),
   mpSipUserAgent(&sipUserAgent),
   mpBuilder(NULL),
   mpWriter(pWriter),
   mSequenceNumber(0),
   mFlushTimer(mTimerEvent),
   mTimerEvent(*getMessageQueue(), 0)
{
   OsTime timeNow;
   OsDateTime::getCurTime(timeNow);
   
   UtlString event;
   // Define AuthProxyCseObserver as string constant
   
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
                                     "AuthProxyCseObserver");      
            mpBuilder->finishElement(event);      

            if (!mpWriter->writeLog(event.data()))
            {      
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "AuthProxyCseObserver initial event log write failed - disabling writer");
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
                          "AuthProxyCseObserver initial event log write failed - disabling writer");
            mpWriter = NULL;
            
            // Set correct state even if nothing is written
            mpBuilder->observerEvent(mSequenceNumber, timeNow, CallStateEventBuilder::ObserverReset, "");                 
            mpBuilder->finishElement(event);             
         }
      }
   }

   // set up periodic timer to flush log file
   mFlushTimer.periodicEvery(OsTime(), OsTime(AuthProxyCallStateFlushInterval, 0)) ;

  // Register to get incoming requests
   sipUserAgent.addMessageObserver(*getMessageQueue(),
                                   SIP_BYE_METHOD,
                                   TRUE, // Requests,
                                   FALSE, //Responses,
                                   TRUE, //Incoming,
                                   FALSE, //OutGoing,
                                   "", //eventName,
                                   NULL, // any session
                                   NULL // no observerData
                                   );
   sipUserAgent.addMessageObserver(*getMessageQueue(),
                                   SIP_INVITE_METHOD,
                                   TRUE, // Requests,
                                   TRUE, //Responses,
                                   TRUE, //Incoming,
                                   FALSE, //OutGoing,
                                   "", //eventName,
                                   NULL, // any session
                                   NULL // no observerData
                                   );
   sipUserAgent.addMessageObserver(*getMessageQueue(),
                                   SIP_REFER_METHOD,
                                   TRUE, // Requests,
                                   FALSE, //Responses,
                                   TRUE, //Incoming,
                                   FALSE, //OutGoing,
                                   "", //eventName,
                                   NULL, // any session
                                   NULL // no observerData
                                   );                                   
}

// Destructor
AuthProxyCseObserver::~AuthProxyCseObserver()
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

UtlBoolean AuthProxyCseObserver::handleMessage(OsMsg& eventMessage)
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
                       "AuthProxyCseObserver::handleMessage transport error");
      }
      else if((sipMsg = (SipMessage*)((SipMessageEvent&)eventMessage).getMessage()))
      {
         UtlString method;
         int       rspStatus = 0;
         UtlString rspText;
         UtlString contact;

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

            if (0==method.compareTo(SIP_REFER_METHOD, UtlString::ignoreCase))
            {
               thisMsgIs = aCallTransfer;
               sipMsg->getContactEntry(0, &contact);               
            }
            else if (0==method.compareTo(SIP_BYE_METHOD, UtlString::ignoreCase))
            {
               thisMsgIs = aCallEnd; // no additional information needed
            }
            else
            {
               // other request methods are not interesting
            }
         }
         else // this is a response
         {
            int seq;
            if (sipMsg->getCSeqField(&seq, &method)) // get the method out of cseq field
            {
               if (0==method.compareTo(SIP_INVITE_METHOD, UtlString::ignoreCase))
               {
                  // this is an INVITE response - might be either a Failure or a Setup

                  rspStatus = sipMsg->getResponseStatusCode();
                  if (   (rspStatus >= SIP_4XX_CLASS_CODE) // any failure
                      // except for these three
                      && ! (   (rspStatus == HTTP_UNAUTHORIZED_CODE)
                            || (rspStatus == HTTP_PROXY_UNAUTHORIZED_CODE)
                            || (rspStatus == SIP_REQUEST_TIMEOUT_CODE)
                            )
                      )
                  {
                     // a final failure - this is a CallFailure
                     thisMsgIs = aCallFailure;
                     sipMsg->getResponseStatusText(&rspText);
                  }
                  else if (   ( rspStatus >= SIP_2XX_CLASS_CODE )
                           && ( rspStatus <  SIP_3XX_CLASS_CODE )
                           )
                  {
                     thisMsgIs = aCallSetup;
                     sipMsg->getContactEntry(0, &contact);
                  }
               }
               else
               {
                  // responses to non-INVITES are not interesting
               }
            }
            else
            {
               OsSysLog::add(FAC_SIP, PRI_ERR, "AuthProxyCseObserver - no Cseq in response");
            }
         }

#        ifdef LOG_DEBUG
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "AuthProxyCseObserver message is %s",
                       (  thisMsgIs == UnInteresting ? "UnInteresting"
                        : thisMsgIs == aCallEnd      ? "a Call End"
                        : thisMsgIs == aCallFailure  ? "a Call Failure"
                        : thisMsgIs == aCallSetup    ? "a Call Setup"
                        : thisMsgIs == aCallTransfer ? "a Call Transfer"
                        : "BROKEN"
                        )); 
#        endif

         if (thisMsgIs != UnInteresting)
         {
            // collect the sequence data
            mSequenceNumber++;
            
            OsTime timeNow;
            OsDateTime::getCurTime(timeNow); 

            // collect the dialog information
            UtlString callId;
            sipMsg->getCallIdField(&callId);
         
            Url toUrl;
            sipMsg->getToUrl(toUrl);
            UtlString toTag;
            toUrl.getFieldParameter("tag", toTag);

            Url fromUrl;
            sipMsg->getFromUrl(fromUrl);
            UtlString fromTag;
            fromUrl.getFieldParameter("tag", fromTag);

            // collect the To and From
            UtlString toField;
            sipMsg->getToField(&toField);
            
            UtlString fromField;
            sipMsg->getFromField(&fromField);

            UtlString referTo;
            UtlString referredBy;
            UtlString requestUri;
            sipMsg->getReferToField(referTo);
            sipMsg->getReferredByField(referredBy);   
            sipMsg->getRequestUri(&requestUri);
            
            UtlString responseMethod;
            int cseqNumber;
            sipMsg->getCSeqField(&cseqNumber, &responseMethod);            

            // generate the call state event record
            if (mpBuilder)
            {
               switch (thisMsgIs)
               {
               case aCallSetup:
                  mpBuilder->callSetupEvent(mSequenceNumber, timeNow, contact);
                  break;
   
               case aCallFailure:
                  mpBuilder->callFailureEvent(mSequenceNumber, timeNow, rspStatus, rspText);
                  break;
                  
               case aCallEnd:
                  mpBuilder->callEndEvent(mSequenceNumber, timeNow);
                  break;
                  
               case aCallTransfer:
                  mpBuilder->callTransferEvent(mSequenceNumber, timeNow, 
                                               contact, referTo, referredBy, requestUri);
                  break;   
   
               default:
                  // shouldn't be possible to get here
                  OsSysLog::add(FAC_SIP, PRI_ERR, "AuthProxyCseObserver invalid thisMsgIs");
                  break;
               }
   
               mpBuilder->addCallData(cseqNumber, callId, fromTag, toTag, fromField, toField);
               UtlString via;
               for (int i=0; sipMsg->getViaField(&via, i); i++)
               {
                  mpBuilder->addEventVia(via);
               }
   
               mpBuilder->completeCallEvent();
                 
               // get the completed record
               UtlString event;
               mpBuilder->finishElement(event);
               
               if (mpWriter)
               {
                 mpWriter->writeLog(event.data());
               }
            }
            else
            {
               OsSysLog::add(FAC_SIP, PRI_ERR, "AuthProxyCseObserver - no CallStateEventBuilder!");               
            }
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "AuthProxyCseObserver getMessage returned NULL");
      }
   }
   break;
   
   default:
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "AuthProxyCseObserver invalid message type %d", msgType );
   }
   } // end switch (msgType)
   
   return(TRUE);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


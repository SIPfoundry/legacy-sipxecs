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
#include "SipXProxyCseObserver.h"
#include <net/SipUserAgent.h>
#include <net/SipXauthIdentity.h>
#include <os/OsDateTime.h>
#include "os/OsEventMsg.h"
#include "os/OsMutex.h"
#include <os/OsSysLog.h>
#include <utl/UtlHashMapIterator.h>

//#define TEST_PRINT 1
#define LOG_DEBUG 1
#define CSE_AGENT_OUTPUT_PROC_PRIO (105)

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const int SipXProxyCallStateFlushInterval = 20; /* seconds */
const int SipXProxyCallStateCleanupInterval = 60 * 30; /* 60 seconds * 30 = 30 minutes */


class BranchTimePair : public UtlString
{

public:
   BranchTimePair(const char* name, const unsigned long* time = NULL, const bool* paipresent = NULL) :
      UtlString(name)
   {
      if ( time ) {
         setValue(time);
      }      
      if ( paipresent ) {
         setPaiPresent(paipresent);
      }
   }

   virtual
   ~BranchTimePair()
   {
   }

   BranchTimePair& operator=(const BranchTimePair& rhs)
   {
      if (this == &rhs)            // handle the assignment to self case
         return *this;

      ((UtlString&) *this) = rhs.data();
      setValue(&rhs.timeInt);
      setPaiPresent(&rhs.paibool);

      return *this;

   }
      //:Copy constructor
   BranchTimePair(const BranchTimePair& rBranchTimePair):
   UtlString(rBranchTimePair),
   timeInt(0),
   paibool(false)
   {
      setValue(&rBranchTimePair.timeInt);
      setPaiPresent(&rBranchTimePair.paibool);
   }
      //:Copy constructor
      
   const unsigned long* getValue()
   {
      return &timeInt;
   }
   //: get value int
   //   //! returns: the integer containing the value <br>
   //      //! Note: this should not be freed as it is part of this object
   //
     
   const bool* getPaiPresent()
   {
      return &paibool;
   }
   //: get paipresent bool
   //   //! returns: the boolean containing the presence of a pai <br>
   //      //! Note: this should not be freed as it is part of this object
   //
   void setValue(const unsigned long* time)
   {
      if ( time ) 
      {
         timeInt = *time;
      }
   }

   void setPaiPresent(const bool* paipresent)
   {
      if ( paipresent ) 
      {
         paibool = *paipresent;
      }
   }

protected:

private:
   unsigned long timeInt;
   bool paibool;
   int count;

   BranchTimePair();
      //: Hide default constructor
};

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipXProxyCseObserver::SipXProxyCseObserver(SipUserAgent&         sipUserAgent,
                                           const UtlString&      dnsName,
                                           CallStateEventWriter* pWriter
                                           ) :
   OsServerTask("SipXProxyCseObserver-%d", NULL, 2000),
   mpSipUserAgent(&sipUserAgent),
   mpBuilder(NULL),
   mpWriter(pWriter),
   mSequenceNumber(0),
   mFlushTimer(getMessageQueue(), 0),
   mCallTransMutex(OsMutex::Q_FIFO),
   SipOutputProcessor( CSE_AGENT_OUTPUT_PROC_PRIO )
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
                                     "SipXProxyCseObserver");      
            mpBuilder->finishElement(event);      

            if (!mpWriter->writeLog(event.data()))
            {      
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "SipXProxyCseObserver initial event log write failed - disabling writer");
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
                          "SipXProxyCseObserver initial event log write failed - disabling writer");
            mpWriter = NULL;
            
            // Set correct state even if nothing is written
            mpBuilder->observerEvent(mSequenceNumber, timeNow, CallStateEventBuilder::ObserverReset, "");                 
            mpBuilder->finishElement(event);             
         }
      }
   }

   // set up periodic timer to flush log file
   mFlushTimer.periodicEvery(OsTime(), OsTime(SipXProxyCallStateFlushInterval, 0)) ;

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
                                   FALSE, //Responses,
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

   sipUserAgent.addSipOutputProcessor( this );

   // set up periodic timer to cleanup dead calls in the CallTransMap
   mpCleanupTimeoutCallback = new OsCallback((void*)this, CleanupTransMap);
   mpCleanupMapTimer = new OsTimer(*mpCleanupTimeoutCallback);
   mpCleanupMapTimer->periodicEvery(OsTime(), OsTime(SipXProxyCallStateCleanupInterval, 0)) ;
}

// Destructor
SipXProxyCseObserver::~SipXProxyCseObserver()
{
   mFlushTimer.stop();
   mpCleanupMapTimer->stop();

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

void SipXProxyCseObserver::handleOutputMessage( SipMessage& message,
                                                const char* address,
                                                int port )
{
   UtlString method;
   UtlString viaValue;
   UtlString branchId;
   UtlString callId;
   int       rspStatus = 0;
   
   
   if (message.isResponse()) {
      int seq;
      if (message.getCSeqField(&seq, &method)) {
         if (0==method.compareTo(SIP_INVITE_METHOD, UtlString::ignoreCase)) {
            rspStatus = message.getResponseStatusCode();
            if (rspStatus >= SIP_2XX_CLASS_CODE) {
                SipMessageEvent* finalTransResponse = new SipMessageEvent(new SipMessage(message), SipMessageEvent::APPLICATION);
                this->postMessage(*finalTransResponse);
                delete finalTransResponse;
            }          
         }          
      }          
   }          
}

void SipXProxyCseObserver::CleanupTransMap(void* userData, const intptr_t eventData)
{
    const int SECONDS_IN_AN_HOUR = 3600;
    const int MAX_CALL_LENGTH = SECONDS_IN_AN_HOUR * 8;

    unsigned long currentTime = OsDateTime::getSecsSinceEpoch();
    SipXProxyCseObserver* Observer = (SipXProxyCseObserver*) userData; 

    // Acquire the map mutex and then iterate over the entries seeing if any need to be deleted.
    Observer->mCallTransMutex.acquire();
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipXProxyCseObserver CleanupTransMap number of entries in map = %i", Observer->mCallTransMap.entries());
    UtlHashMapIterator callTransIter(Observer->mCallTransMap);
    UtlString* callId;
    while ((callId = dynamic_cast <UtlString*>(callTransIter())) != NULL)
    {
       BranchTimePair* callIdValue = dynamic_cast <BranchTimePair*> (callTransIter.value());
       const unsigned long* entryTime = callIdValue->getValue();
       if ( (currentTime - *entryTime) > MAX_CALL_LENGTH ) 
       {
          // Delete the entry.  It's way too old.
          Observer->mCallTransMap.destroy(callId);
       } 
    }
    Observer->mCallTransMutex.release();

}

UtlBoolean SipXProxyCseObserver::handleMessage(OsMsg& eventMessage)
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
                       "SipXProxyCseObserver::handleMessage transport error");
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
               aCallRequest,
               aCallSetup,
               aCallFailure,
               aCallEnd,
               aCallTransfer
            } thisMsgIs = UnInteresting;
         
         Url toUrl;

         sipMsg->getToUrl(toUrl);

         // explicitly, an INVITE Request
         toUrl.getFieldParameter("tag", toTag);

         if (!sipMsg->isResponse())
         {

            // sipMsg is a Request
            sipMsg->getRequestMethod(&method);

            if (0==method.compareTo(SIP_INVITE_METHOD, UtlString::ignoreCase))
            {
               if (toTag.isNull())
               {
                  sipMsg->getContactEntry(0, &contact);               
                  thisMsgIs = aCallRequest;
               }
            }
            else if (0==method.compareTo(SIP_REFER_METHOD, UtlString::ignoreCase))
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
                  // Responses to INVITES are handled differently based on whether
                  // or not the INVITE is dialog-forming.  If dialog-forming,
                  // any final response above 400 is considered a failure for CDR
                  // purposes.  If not dialog-forming, then any final response above 400
                  // except 401 Unauthorized, 407 Proxy Authentication Required and 
                  // 408 Request Timeout will terminate.  If we're in a dialog then
            	  // only 408 (Request Timeout) and 481 (Call/Transaction does not exist)
            	  // will terminate the dialog.

                  rspStatus = sipMsg->getResponseStatusCode();
                  if (rspStatus >= SIP_4XX_CLASS_CODE) // any failure
                  {
                     // a failure code - this is a potential CallFailure - Call Resolver will determine.
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
               OsSysLog::add(FAC_SIP, PRI_ERR, "SipXProxyCseObserver - no Cseq in response");
            }
         }

#        ifdef LOG_DEBUG
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipXProxyCseObserver message is %s",
                       (  thisMsgIs == UnInteresting ? "UnInteresting"
                        : thisMsgIs == aCallEnd      ? "a Call End"
                        : thisMsgIs == aCallFailure  ? "a Call Failure"
                        : thisMsgIs == aCallRequest  ? "a call Request"      
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


            // collect the branch Id (i.e. transaction id) and via count.
            UtlString viaValue;
            int viaCount;
            UtlString branchId;
            viaCount = sipMsg->getCountHeaderFields(SIP_VIA_FIELD);
            viaCount = viaCount + sipMsg->getCountHeaderFields(SIP_SHORT_VIA_FIELD);
            if ( sipMsg->getViaFieldSubField( &viaValue, 0 ) ) {
               sipMsg->getViaTag( viaValue, "branch", branchId );
            }
            UtlString referTo;
            UtlString referredBy;
            UtlString requestUri;
            UtlString references;
            UtlString replaces_callId;
            UtlString replaces_toTag;
            UtlString replaces_fromTag;
            UtlString matchingIdentityHeader;
            SipXauthIdentity sipxIdentity(*sipMsg, matchingIdentityHeader, true,SipXauthIdentity::allowUnbound);

            sipMsg->getReferToField(referTo);
            sipMsg->getReferredByField(referredBy);   
            sipMsg->getRequestUri(&requestUri);
            sipMsg->getReferencesField(&references);
            if (sipMsg->getReplacesData(replaces_callId, replaces_toTag, replaces_fromTag)) {
               if (references.length() != 0) {
                  references.append(",");
               }
               references.append(replaces_callId);
               references.append(";rel=xfer");
            }
            
            UtlString responseMethod;
            UtlString calleeRoute;

            int cseqNumber;
            sipMsg->getCSeqField(&cseqNumber, &responseMethod);            

            BranchTimePair* callIdBranchIdTime;
            // generate the call state event record
            if (mpBuilder)
            {
               UtlString identity;
               UtlString recordRoute;
               bool routeFound = false;
               bool paiPresent = false;

               switch (thisMsgIs)
               {
               case aCallRequest:
                 
                  if (sipxIdentity.getIdentity(identity)) {
                     paiPresent = true;
                  }

                  if ( branchId && branchId.data() ) {
                     mCallTransMutex.acquire();
                     unsigned long currentTime = OsDateTime::getSecsSinceEpoch();
                     if (NULL == mCallTransMap.insertKeyAndValue(new UtlString(callId), new BranchTimePair(branchId.data(), &currentTime, &paiPresent)) ) {
                        // Unable to add callId to map so it must already be present.  Check if the paiPresent value is set to true or not.
                        // If not set and we now have a PAI for this call, set it and generate another call request state event with this info. Otherwise
                        // skip over.
                        if ( paiPresent ) {
                           callIdBranchIdTime = (BranchTimePair*) mCallTransMap.findValue(&callId);
                           if ( callIdBranchIdTime && (*callIdBranchIdTime->getPaiPresent() == false) ) {
                              // need to generate another call request event in order to state originator is internal.
                              callIdBranchIdTime->setPaiPresent(&paiPresent);
                           }
                           else {
                              mCallTransMutex.release();
                              return(TRUE);
                           }
                        }
                        else {
                           mCallTransMutex.release();
                           return(TRUE);
                        }
                     }
                     mCallTransMutex.release();
                  }
                  mpBuilder->callRequestEvent(mSequenceNumber, timeNow, contact, references, branchId, viaCount, paiPresent);
                  break;
                  
               case aCallSetup:
                  // Clear out from the map only if rspStatus is higher than 200 as its possible to receive multiple 200 messages.
                  // If the response is 200, the call in the map will be cleared out when the call ends.
                  mCallTransMutex.acquire();
                  callIdBranchIdTime = (BranchTimePair*) mCallTransMap.findValue(&callId);
                  if ( callIdBranchIdTime && (0 == branchId.compareTo(callIdBranchIdTime)) ) {
                     if ( rspStatus > SIP_2XX_CLASS_CODE ) {
                           mCallTransMap.destroy(&callId);
                        }
                     mCallTransMutex.release();
                  }
                  else
                  {
                     // CallId/BranchId are either not found or doesn't match.  Not a final response.
                     mCallTransMutex.release();
                     return(TRUE);
                  }
                  for (int rrNum = 0;
                       (!routeFound && sipMsg->getRecordRouteUri(rrNum, &recordRoute));
                       rrNum++
                  )
                  {
                     Url recordRouteUrl(recordRoute);
                     if (mpSipUserAgent->isMyHostAlias(recordRouteUrl)) {
                        // This is a record route for our proxy, extract Call tags if they exist.
                        recordRouteUrl.getUrlParameter(SIP_SIPX_CALL_DEST_FIELD, calleeRoute, 0);
                        routeFound = true;
                     }
                  }
                  mpBuilder->callSetupEvent(mSequenceNumber, timeNow, contact, calleeRoute, branchId, viaCount);
                  break;
   
               case aCallFailure:
                  // Failure case means that the response code is > 400.  If the call is found
                  // in the map, then this is a final response.  Delete from the map and build an event.
                  mCallTransMutex.acquire();
                  callIdBranchIdTime = (BranchTimePair*) mCallTransMap.findValue(&callId);
                  if ( callIdBranchIdTime && (0 == branchId.compareTo(callIdBranchIdTime)) ) {
                     mCallTransMap.destroy(&callId);
                     mCallTransMutex.release();
                     if ( rspStatus != SIP_PROXY_AUTH_REQ_CODE ) {
                        mpBuilder->callFailureEvent(mSequenceNumber, timeNow, branchId, viaCount, rspStatus, rspText);
                     }
                     else {
                        // response was an authentication required.  Don't build a CSE for these as a new Invite will
                        // occur.
                        return(TRUE);
                     }
                  }
                  else
                  {
                     // Call was not found in the map so this is not a final response.  Ignore it.
                     mCallTransMutex.release();
                     return(TRUE);
                  }
                  break;
                  
               case aCallEnd:
                  mCallTransMutex.acquire();
                  mCallTransMap.destroy(&callId);
                  mCallTransMutex.release();
                  mpBuilder->callEndEvent(mSequenceNumber, timeNow);
                  break;
                  
               case aCallTransfer:
                  mpBuilder->callTransferEvent(mSequenceNumber, timeNow, 
                                               contact, referTo, referredBy, requestUri);
                  break;   
   
               default:
                  // shouldn't be possible to get here
                  OsSysLog::add(FAC_SIP, PRI_ERR, "SipXProxyCseObserver invalid thisMsgIs");
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
               OsSysLog::add(FAC_SIP, PRI_ERR, "SipXProxyCseObserver - no CallStateEventBuilder!");               
            }
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "SipXProxyCseObserver getMessage returned NULL");
      }
   }
   break;
   
   default:
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipXProxyCseObserver invalid message type %d", msgType );
   }
   } // end switch (msgType)
   
   return(TRUE);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


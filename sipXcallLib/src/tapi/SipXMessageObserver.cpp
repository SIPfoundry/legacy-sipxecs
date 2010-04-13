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
// APPLICATION INCLUDES
#include "tapi/SipXMessageObserver.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"
#include "tapi/SipXHandleMap.h"
#include "net/SipUserAgent.h"
#include "utl/UtlVoidPtr.h"
#include "os/OsEventMsg.h"
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern UtlSList*    g_pEventListeners;
extern OsMutex*        g_pEventListenerLock;
extern SipXHandleMap* gpInfoHandleMap ;   // sipXtapiInternal.cpp


// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */


SipXMessageObserver::SipXMessageObserver(const SIPX_INST hInst) :
    OsServerTask("SipXMessageObserver%d", NULL, 2000),
    mTestResponseCode(0),// if mTestResponseCode is set to a value other than 0,
                         // then this message observer can generate a test response.
                         // This feature is used by sipXtapiTest
    mhInst(hInst)
{
}

SipXMessageObserver::~SipXMessageObserver(void)
{
    waitUntilShutDown();
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean SipXMessageObserver::handleMessage(OsMsg& rMsg)
{
    UtlBoolean bRet = FALSE ;

    switch (rMsg.getMsgType())
    {
       case OsMsg::OS_EVENT:
       {
           OsEventMsg* pEventMsg = (OsEventMsg*) &rMsg ;
           void* eventType ;
           pEventMsg->getUserData(eventType) ;

           switch ((intptr_t)eventType)
           {
               case SIPXMO_NOTIFICATION_STUN:
                   handleStunOutcome(pEventMsg) ;
                   bRet = TRUE ;
                   break ;
           }
           break ;
        }
        case OsMsg::PHONE_APP:
        {
           SipMessage* pSipMessage = (SipMessage*) ((SipMessageEvent&)rMsg).getMessage() ;
           UtlString method;

           pSipMessage->getRequestMethod(&method);

           if (pSipMessage && pSipMessage->isResponse())
           {
               // ok, the phone has received a response to a sent INFO message.
               bRet = handleIncomingInfoStatus(pSipMessage);
           }
           else if (pSipMessage && !pSipMessage->isResponse())
           {
               if (method == SIP_INFO_METHOD)
               {
                   // ok, the phone has received an INFO message.
                   bRet = handleIncomingInfoMessage(pSipMessage);
               }
           }
           break ;
       }
       default:
          break ;
    }
    return bRet;
}

bool SipXMessageObserver::handleIncomingInfoMessage(SipMessage* pMessage)
{
    bool bRet = false;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) pMessage->getResponseListenerData();

    if (NULL != pInst && NULL != pMessage)
    {

        if (mTestResponseCode != 0)  // for unit testing purposes.
        {
            if (mTestResponseCode == 408)   // a timeout response is being tested
            {
                // simulate a timeout ....
                OsTask::delay(1000);
                // respond to whomever sent us the message
                SipMessage sipResponse;
                sipResponse.setOkResponseData(pMessage);
                sipResponse.setResponseFirstHeaderLine(SIP_PROTOCOL_VERSION, mTestResponseCode, "timed out");
                pInst->pSipUserAgent->send(sipResponse);
                return true ;
            }
        }
        else
        {
            // respond to whomever sent us the message
            SipMessage sipResponse;
            sipResponse.setOkResponseData(pMessage);
            pInst->pSipUserAgent->send(sipResponse);
        }

        // Find Line
        UtlString lineId;
        pMessage->getToUri(&lineId);
        UtlString requestUri;
        pMessage->getRequestUri(&requestUri);
        SIPX_LINE hLine = sipxLineLookupHandle(pInst, lineId.data(), requestUri);

        if (!pMessage->isResponse())
        {
            // find call
            UtlString callId;
            pMessage->getCallIdField(&callId);
            SIPX_CALL hCall = sipxCallLookupHandle(callId, pInst->pCallManager);

            if (0 == hCall)
            {
                // we are unaware of the call context
            }

            SIPX_INFO_DATA* pInfoData = new SIPX_INFO_DATA;

            memset((void*)pInfoData, 0, sizeof(SIPX_INFO_DATA));
            pInfoData->infoData.nSize = sizeof(SIPX_INFO_INFO);
            pInfoData->infoData.hCall = hCall;
            pInfoData->infoData.hLine = hLine;
            Url fromUrl;

            pInfoData->infoData.szFromURL = lineId.data();
            pInfoData->infoData.nContentLength = pMessage->getContentLength();

            // get and set the content type
            UtlString contentType;
            pMessage->getContentType(&contentType) ;
            pInfoData->infoData.szContentType = strdup(contentType.data());

            // get the user agent
            UtlString userAgent;
            pMessage->getUserAgentField(&userAgent);
            pInfoData->infoData.szUserAgent = strdup(userAgent.data());

            // get the content
            UtlString body;
            ssize_t dummyLength = pMessage->getContentLength();
            const HttpBody* pBody = pMessage->getBody();
            pBody->getBytes(&body, &dummyLength);
            pInfoData->infoData.pContent = body.data();

            // set the Instance
            pInfoData->pInst = pInst;

            // Create Mutex
            pInfoData->pMutex = new OsRWMutex(OsRWMutex::Q_FIFO);

            UtlVoidPtr* ptr = NULL;
            UtlSListIterator eventListenerItor(*g_pEventListeners);
            while ((ptr = (UtlVoidPtr*) eventListenerItor()) != NULL)
            {
                EVENT_LISTENER_DATA *pData = (EVENT_LISTENER_DATA*) ptr->getValue();
                if (pData->pInst == pInfoData->pInst)
                {
                    pData->pCallbackProc(EVENT_CATEGORY_INFO, &(pInfoData->infoData), pData->pUserData);
                }
            }

            bRet = true;
        } // if (0 != hLine)
    } // if (NULL != pInst && NULL != pMessage)
    return bRet;
}

bool SipXMessageObserver::handleIncomingInfoStatus(SipMessage* pSipMessage)
{
    if (NULL == pSipMessage)
    {
        // something went wrong
        return false;
    }

    SIPX_INFO hInfo = (SIPX_INFO)pSipMessage->getResponseListenerData();
    if (hInfo)
    {
        SIPX_INFOSTATUS_INFO infoStatus;

        memset((void*) &infoStatus, 0, sizeof(SIPX_INFOSTATUS_INFO));

        infoStatus.hInfo = hInfo;
        SIPX_INFO_DATA* pInfoData = sipxInfoLookup(hInfo, SIPX_LOCK_READ);
        infoStatus.nSize = sizeof(SIPX_INFOSTATUS_INFO);
        infoStatus.responseCode = pSipMessage->getResponseStatusCode();
        infoStatus.event = INFOSTATUS_RESPONSE;

        int statusCode = pSipMessage->getResponseStatusCode();
        if (statusCode < 400)
        {
        infoStatus.status = SIPX_MESSAGE_OK;
        }
        else if (statusCode < 500)
        {
            infoStatus.status = SIPX_MESSAGE_FAILURE;
        }
        else if (statusCode < 600)
        {
            infoStatus.status = SIPX_MESSAGE_SERVER_FAILURE;
        }
        else
        {
            infoStatus.status = SIPX_MESSAGE_GLOBAL_FAILURE;
        }

        UtlString sResponseText;
        pSipMessage->getResponseStatusText(&sResponseText);
        infoStatus.szResponseText = sResponseText.data();

        UtlVoidPtr* ptr = NULL;
        UtlSListIterator eventListenerItor(*g_pEventListeners);
        while ((ptr = (UtlVoidPtr*) eventListenerItor()) != NULL)
        {
            EVENT_LISTENER_DATA *pData = (EVENT_LISTENER_DATA*) ptr->getValue();
            if (pInfoData->pInst == pData->pInst)
            {
                pData->pCallbackProc(EVENT_CATEGORY_INFO_STATUS, &infoStatus, pData->pUserData);
            }
        }

        pInfoData->pInst->pSipUserAgent->removeMessageObserver(*(this->getMessageQueue()), (void*)hInfo);

        // release lock
        sipxInfoReleaseLock(pInfoData, SIPX_LOCK_READ);
        // info message has been handled, so go ahead and delete the object
        sipxInfoObjectFree(hInfo);
     }
     return true;
}


bool SipXMessageObserver::handleStunOutcome(OsEventMsg* pMsg)
{
    SIPX_CONTACT_ADDRESS sipxContact; // contact structure for notifying
                                       // sipxtapi event listeners
    ContactAddress* pContact = NULL;
    pMsg->getEventData((intptr_t&)pContact) ;

    SIPX_CONFIG_INFO eventInfo ;
    memset(&eventInfo, 0, sizeof(SIPX_CONFIG_INFO)) ;
    eventInfo.nSize = sizeof(SIPX_CONFIG_INFO) ;
    if (pContact)
    {
        // first, find the user-agent, and add the contact to
        // the user-agent's db
        SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) mhInst;
        pInst->pSipUserAgent->addContactAddress(*pContact);

        // ok, now generate an event for the sipXtapi application layer
        strcpy(sipxContact.cInterface, pContact->cInterface);
        strcpy(sipxContact.cIpAddress, pContact->cIpAddress);
        sipxContact.eContactType = CONTACT_NAT_MAPPED;
        sipxContact.id = pContact->id;
        sipxContact.iPort = pContact->iPort;

        eventInfo.pData = &sipxContact;
        eventInfo.event = CONFIG_STUN_SUCCESS ;

        delete pContact;
    }
    else
    {
        eventInfo.event = CONFIG_STUN_FAILURE ;
    }

    sipxFireEvent(this, EVENT_CATEGORY_CONFIG, &eventInfo) ;

    return true ;
}

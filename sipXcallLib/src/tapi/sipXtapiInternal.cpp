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

// APPLICATION INCLUDES
#include "net/SipUserAgent.h"
#include "cp/CallManager.h"
#include "net/SipLineMgr.h"
#include "net/SipRefreshMgr.h"
#include "net/SipSubscribeClient.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlString.h"
#include "utl/UtlInt.h"
#include "utl/UtlVoidPtr.h"
#include "utl/UtlString.h"
#include "utl/UtlDListIterator.h"
#include "os/OsLock.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"
#include "tapi/SipXHandleMap.h"
#include "net/Url.h"
#include "net/SipUserAgent.h"
#include "cp/CallManager.h"
#include "mi/CpMediaInterfaceFactory.h"

//#define DUMP_CALLS              1

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// GLOBAL VARIABLES
SipXHandleMap* gpCallHandleMap = new SipXHandleMap();  /**< Global Map of call handles */
SipXHandleMap* gpLineHandleMap = new SipXHandleMap() ;  /**< Global Map of line handles */
SipXHandleMap* gpConfHandleMap = new SipXHandleMap() ;  /**< Global Map of conf handles */
SipXHandleMap* gpInfoHandleMap = new SipXHandleMap() ;  /**< Global Map of info handles */
SipXHandleMap* gpPubHandleMap = new SipXHandleMap() ;  /**< Global Map of Published (subscription server) event data handles */
SipXHandleMap* gpSubHandleMap = new SipXHandleMap() ;  /**< Global Map of Subscribed (client) event data handles */


UtlDList*  gpSessionList  = new UtlDList() ;    /**< List of sipX sessions (to be replaced
                                                     by handle map in the future */
OsMutex*    gpSessionLock = new OsMutex(OsMutex::Q_FIFO);
static int      gSessions = 0;

/*
   A note about the locking in this code.

   Previous implementations had the following locking structure:
    1: lookup(handle, READ|WRITE)
    2:{
    3:   HandleMap->lock()
    4:   ptr = HandleMap(handle) ;
    5:   ptr->mutex->read|write lock ;
    6:   HandleMap->unlock()
    5:}

    6:free(handle)
    7:{
    8:  ptr = lookup(handle, WRITE)
    9:  HandleMap->remove(handle) ;
    9:  delete ptr ;
   10:}

   This was prone to deadlock:
   Thread A                             Thread B:
     6:free(handle)
     8:  ptr = lookup(handle, WRITE)
     (A holds WRITE lock on handle)
                                        1: lookup(handle, READ)
                                        3:   HandleMap->lock()
                                        4:   ptr = HandleMap(handle) ;
                                        5:   ptr->mutex->readlock ;
                                        (B holds lock on map,
                                          blocked on handle lock)
     9:  HandleMap->remove(handle) ;
     (A tries to access map, which, internally locks HandleMap->lock(), which
      blocks because B holds that lock at line 3.)

     Deadlock as A holds a resource B wants, and B holds a resource A wants.


   The current implementation has the following locking structure:
    1: lookup(handle, READ|WRITE)
    2:{
    3:   HandleMap->lock()
    4:   ptr = HandleMap(handle) ;
    5:   ptr->mutex->read|write lock ;
    6:   HandleMap->unlock()
    5:}

    6:free(handle)
    7:{
    8:  HandleMap->lock()
    9:  ptr = HandleMap->remove(handle) ;
   10:  HandleMap->unlock()
   11:  prt->mutex->writelock ;
   12:  delete ptr ;
   13:}

   This should be deadlock free, as lines 3 and 8 give exclusive access
   to the Map.  Then the handle is atomically removed from the map, so
   any subsequent calls to lookup() will not find the handle.

   Then line 11 waits for any previously existing lockers to release their
   locks, ensuring exclusive access to the handle, which is then safe to
   delete.

   But beware!  Any thread that holds a lock from lookup(), cannot try
   to lock the HandleMap again, or it may deadlock!  This is because it
   holds the ptr->mutex lock, and wants the HandleMap lock, and another
   thread may be holding the HandleMap lock, and want the ptr->mutex lock.

   The best solution I am aware of to this problem is to drop the ptr->mutex
   lock (fine grained locking), and simply hold the HandleMap lock for
   the duration of the transaction.

   --Woof!
*/

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

SIPX_CALL sipxCallLookupHandle(const UtlString& callID, const void* pSrc)
{
    SIPX_CALL hCall = 0 ;

    if (gpCallHandleMap->lock())
    {
        UtlHashMapIterator iter(*gpCallHandleMap);

        UtlInt* pIndex = NULL;
        UtlVoidPtr* pObj = NULL;

        while ((pIndex = dynamic_cast<UtlInt*>( iter() )) )
        {
            pObj = dynamic_cast<UtlVoidPtr*>(gpCallHandleMap->findValue(pIndex));
            SIPX_CALL_DATA* pData = NULL ;
            if (pObj)
            {
                pData = (SIPX_CALL_DATA*) pObj->getValue() ;
            }

            if (pData &&
                    (pData->callId->compareTo(callID) == 0 ||
                    (pData->sessionCallId && (pData->sessionCallId->compareTo(callID) == 0)) ||
                    (pData->transferCallId && (pData->transferCallId->compareTo(callID) == 0))) &&
                    pData->pInst->pCallManager == pSrc)
            {
                hCall = pIndex->getValue() ;
#ifdef DUMP_CALLS
                OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG, "***************** LookupHandle ***\nhCall %d\n****callId %s\n***sessionCallId %s\n",
                hCall,
                pData->callId ? pData->callId->data() : NULL,
                pData->sessionCallId ? pData->sessionCallId->data() : NULL);
#endif
                break ;
            }
        }

        gpCallHandleMap->unlock() ;
    }

    return hCall;
}


void sipxCallObjectFree(const SIPX_CALL hCall)
{
    // First remove it from the HandleMap so no one else can access it
    SIPX_CALL_DATA* pData =
       (SIPX_CALL_DATA *)gpCallHandleMap->removeHandle(hCall);

    if (pData)
    {
        // Then lock it so anyone who was previously using it is known
        // to have released it
        if (pData->pMutex->acquireWrite() == OS_SUCCESS)
        {
#ifdef DUMP_CALLS
            sipxDumpCalls();
#endif
            // Now it is safe to destroy
            destroyCallData(pData) ;
        }
    }
}


SIPX_CALL_DATA* sipxCallLookup(const SIPX_CALL hCall, SIPX_LOCK_TYPE type)
{
    SIPX_CALL_DATA* pRC = NULL ;

    if (gpCallHandleMap->lock())
    {
        pRC = (SIPX_CALL_DATA*) gpCallHandleMap->findHandle(hCall) ;
        if (validCallData(pRC))
        {
            switch (type)
            {
                case SIPX_LOCK_READ:
                    if (pRC->pMutex->acquireRead() != OS_SUCCESS)
                        pRC = NULL;
                    break ;
                case SIPX_LOCK_WRITE:
                    if (pRC->pMutex->acquireWrite() != OS_SUCCESS)
                        pRC = NULL;
                    break ;
                case SIPX_LOCK_NONE:
                    break ;
            }
        }
        else
        {
            pRC = NULL;
        }
#ifdef DUMP_CALLS
        // TEMP
        if (pRC)
        {
            OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
                          "***************** Lookup***\n"
                          "hCall %d\n****callId %s\n"
                          "***ghostCallId %s\n"
                          "***bRemoveInsteadOfDrop %d\n",
                          hCall,
                          pRC->callId ? pRC->callId->data() : NULL,
                          pRC->ghostCallId ? pRC->ghostCallId->data() : NULL,
                          pRC->bRemoveInsteadOfDrop);
        }
#endif
        gpCallHandleMap->unlock() ;
    }

    return pRC ;
}


UtlBoolean validCallData(SIPX_CALL_DATA* pData)
{
    return (pData
            && pData->callId
            && pData->lineURI
            && pData->pInst
            && pData->pInst->pCallManager
            && pData->pInst->pRefreshManager
            && pData->pInst->pLineManager
            && pData->pMutex) ;
}


void sipxCallReleaseLock(SIPX_CALL_DATA* pData, SIPX_LOCK_TYPE type)
{
    if ((type != SIPX_LOCK_NONE) && validCallData(pData))
    {
        switch (type)
        {
            case SIPX_LOCK_READ:
                pData->pMutex->releaseRead() ;
                break ;
            case SIPX_LOCK_WRITE:
                pData->pMutex->releaseWrite() ;
                break ;
            case SIPX_LOCK_NONE:
                break ;
        }
    }
}

UtlBoolean sipxCallGetCommonData(SIPX_CALL hCall,
                                 SIPX_INSTANCE_DATA** pInst,
                                 UtlString* pStrCallId,
                                 UtlString* pStrRemoteAddress,
                                 UtlString* pLineId,
                                 UtlString* pGhostCallId)
{
    UtlBoolean bSuccess = FALSE ;
    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ);
    if (pData)
    {
        if (pInst)
        {
            *pInst = pData->pInst ;
        }

        if (pStrCallId)
        {
            if (pData->sessionCallId)
            {
                *pStrCallId = *pData->sessionCallId ;
            }
            else
            {
                *pStrCallId = *pData->callId ;
            }
        }

        if (pStrRemoteAddress)
        {
            if (pData->remoteAddress)
            {
                *pStrRemoteAddress = *pData->remoteAddress ;
            }
            else
            {
                pStrRemoteAddress->remove(0) ;
            }
        }

        if (pLineId)
        {
            *pLineId = *pData->lineURI ;
        }

        if (pGhostCallId)
        {
            if (pData->ghostCallId)
            {
                *pGhostCallId = *pData->ghostCallId;
            }
        }

        bSuccess = TRUE ;

        sipxCallReleaseLock(pData, SIPX_LOCK_READ) ;
    }

    return bSuccess ;
}


SIPX_CONF sipxCallGetConf(SIPX_CALL hCall)
{
    SIPX_CONF hConf = 0 ;

    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ);
    if (pData)
    {
        hConf = pData->hConf ;
        sipxCallReleaseLock(pData, SIPX_LOCK_READ) ;
    }

    return hConf ;
}


UtlBoolean sipxCallGetState(SIPX_CALL hCall,
                            SIPX_CALLSTATE_EVENT& lastEvent,
                            SIPX_CALLSTATE_CAUSE& lastCause,
                            SIPX_INTERNAL_CALLSTATE& state)
{
    UtlBoolean bSuccess = false ;

    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ);
    if (pData)
    {
        lastEvent = pData->lastCallstateEvent ;
        lastCause = pData->lastCallstateCause ;
        state = pData->state ;
        sipxCallReleaseLock(pData, SIPX_LOCK_READ) ;
    }

    return bSuccess ;

}


UtlBoolean sipxCallSetState(SIPX_CALL hCall,
                            SIPX_CALLSTATE_EVENT event,
                            SIPX_CALLSTATE_CAUSE cause)
{
    UtlBoolean bSuccess = false ;

    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE);
    if (pData)
    {
        // Store state
        pData->lastCallstateEvent = event ;
        pData->lastCallstateCause = cause ;

        // Calculate internal state
        switch (event)
        {
            case CALLSTATE_NEWCALL:
                break ;
            case CALLSTATE_DIALTONE:
            case CALLSTATE_REMOTE_OFFERING:
            case CALLSTATE_REMOTE_ALERTING:
                pData->state = SIPX_INTERNAL_CALLSTATE_OUTBOUND_ATTEMPT ;
                break ;
            case CALLSTATE_CONNECTED:
                switch (cause)
                {
                    case CALLSTATE_CONNECTED_ACTIVE:
                        pData->state = SIPX_INTERNAL_CALLSTATE_CONNECTED ;
                        pData->bInFocus = true ;
                        break ;
                    case CALLSTATE_CONNECTED_REQUEST_NOT_ACCEPTED:
                        pData->state = SIPX_INTERNAL_CALLSTATE_CONNECTED ;
                        break ;
                    case CALLSTATE_CONNECTED_ACTIVE_HELD:
                        pData->state = SIPX_INTERNAL_CALLSTATE_BRIDGED ;
                        pData->bInFocus = false ;
                    case CALLSTATE_CONNECTED_INACTIVE:
                        pData->state = SIPX_INTERNAL_CALLSTATE_HELD ;
                        pData->bInFocus = false ;
                        break ;
                    default:
                        break ;
                }
                break ;
            case CALLSTATE_DISCONNECTED:
                pData->state = SIPX_INTERNAL_CALLSTATE_DISCONNECTED ;
                pData->bInFocus = false ;
                break ;
            case CALLSTATE_OFFERING:
            case CALLSTATE_ALERTING:
                pData->state = SIPX_INTERNAL_CALLSTATE_OUTBOUND_ATTEMPT ;
                break ;
            case CALLSTATE_DESTROYED:
                pData->bInFocus = false ;
                break ;
            case CALLSTATE_AUDIO_EVENT:
                break ;
            case CALLSTATE_TRANSFER:
                break ;
            case CALLSTATE_UNKNOWN:
                break ;
        }

        sipxCallReleaseLock(pData, SIPX_LOCK_WRITE) ;
    }

    return bSuccess ;

}



SIPX_CONTACT_TYPE sipxCallGetLineContactType(SIPX_CALL hCall)
{
    SIPX_CONTACT_TYPE contactType = CONTACT_AUTO ;

    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ);
    if (pData)
    {
        SIPX_LINE_DATA* pLineData = sipxLineLookup(pData->hLine, SIPX_LOCK_READ) ;
        if (pLineData)
        {
            contactType = pLineData->contactType ;
            sipxLineReleaseLock(pLineData, SIPX_LOCK_READ) ;
        }
        sipxCallReleaseLock(pData, SIPX_LOCK_READ) ;
    }

    return contactType ;
}



SIPX_LINE_DATA* sipxLineLookup(const SIPX_LINE hLine, SIPX_LOCK_TYPE type)
{
    SIPX_LINE_DATA* pRC = NULL ;

    if (gpLineHandleMap->lock())
    {
        pRC = (SIPX_LINE_DATA*) gpLineHandleMap->findHandle(hLine) ;
        if (validLineData(pRC))
        {
            switch (type)
            {
                case SIPX_LOCK_READ:
                    if (pRC->pMutex->acquireRead() != OS_SUCCESS)
                        pRC = NULL ;
                    break ;
                case SIPX_LOCK_WRITE:
                    if (pRC->pMutex->acquireWrite() != OS_SUCCESS)
                        pRC = NULL ;
                    break ;
                case SIPX_LOCK_NONE:
                    break ;
            }
        }
        else
        {
            pRC = NULL ;
        }
        gpLineHandleMap->unlock() ;
    }

    return pRC ;
}

SIPX_INFO_DATA* sipxInfoLookup(const SIPX_INFO hInfo, SIPX_LOCK_TYPE type)
{
    SIPX_INFO_DATA* pRC = NULL ;

    if (gpInfoHandleMap->lock())
    {
        pRC = (SIPX_INFO_DATA*) gpInfoHandleMap->findHandle(hInfo) ;
        switch (type)
        {
            case SIPX_LOCK_READ:
                if (pRC->pMutex->acquireRead() != OS_SUCCESS)
                    pRC = NULL ;
                break ;
            case SIPX_LOCK_WRITE:
                if (pRC->pMutex->acquireWrite() != OS_SUCCESS)
                    pRC = NULL ;
                break ;
            case SIPX_LOCK_NONE:
                break ;
        }
        gpInfoHandleMap->unlock() ;
    }

    return pRC ;
}

void sipxSubscribeClientSubCallback(SipSubscribeClient::SubscriptionState newState,
                                   const char* earlyDialogHandle,
                                   const char* dialogHandle,
                                   void* applicationData,
                                   int responseCode,
                                   const char* responseText,
                                   long expiration,
                                   const SipMessage* subscribeResponse)
{
    SIPX_SUB subscriptionHandle = (SIPX_SUB)applicationData;
    SIPX_SUBSCRIPTION_DATA* subscriptionData =
        (SIPX_SUBSCRIPTION_DATA*) gpSubHandleMap->findHandle(subscriptionHandle);

    if(subscriptionData && subscriptionData->pInst)
    {
        SIPX_SUBSTATUS_INFO pInfo;
        pInfo.nSize = sizeof(SIPX_SUBSTATUS_INFO);
        UtlString userAgent;
        if(subscribeResponse)
        {
            subscribeResponse->getUserAgentField(&userAgent);
        }
        pInfo.szSubServerUserAgent = userAgent;
        pInfo.hSub = subscriptionHandle;
        // TODO: Should probably set some cause codes based upon
        // the response code from the sip message
        pInfo.cause = SUBSCRIPTION_CAUSE_NORMAL;
        UtlString errorState;

        switch(newState)
        {
        case SipSubscribeClient::SUBSCRIPTION_INITIATED: // Early dialog
            pInfo.state = SIPX_SUBSCRIPTION_PENDING;
            break;

        case SipSubscribeClient::SUBSCRIPTION_SETUP:     // Established dialog
            pInfo.state = SIPX_SUBSCRIPTION_ACTIVE;
            break;

        case SipSubscribeClient::SUBSCRIPTION_TERMINATED:
            pInfo.state = SIPX_SUBSCRIPTION_EXPIRED;
            break;

        default:
            {
                pInfo.state = SIPX_SUBSCRIPTION_FAILED;
                errorState = "unknown: ";
                char numBuf[20];
                sprintf(numBuf, "%d", newState);
                errorState.append(numBuf);
            }
            break;
        }

        // If the dialog changed from and early dialog to an
        // established dialog, update the dialog handle in the
        // subcription data structure
        if(earlyDialogHandle && dialogHandle &&
            SipDialog::isEarlyDialog(*subscriptionData->pDialogHandle))
        {
            *(subscriptionData->pDialogHandle) = dialogHandle;
        }

        // Fire the event if it is a supported state change
        if(errorState.isNull())
        {
            sipxFireEvent(subscriptionData->pInst->pCallManager,
                       EVENT_CATEGORY_SUB_STATUS,
                       &pInfo);
        }
        else
        {
            OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                "sipxSubscribeClientSubCallback: invalid SubscriptionState: %s",
                errorState.data());
        }
    }

    // Cannot find subsription data for this handle
    else
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
            "sipxSubscribeClientSubCallback: cannot find subscription data for handle: %p",
            applicationData);
    }
}


bool sipxSubscribeClientNotifyCallback(const char* earlyDialogHandle,
                                     const char* dialogHandle,
                                     void* applicationData,
                                     const SipMessage* notifyRequest)
{
    SIPX_SUB subscriptionHandle = (SIPX_SUB)applicationData;
    SIPX_SUBSCRIPTION_DATA* subscriptionData =
        (SIPX_SUBSCRIPTION_DATA*) gpSubHandleMap->findHandle(subscriptionHandle);

    if(subscriptionData && subscriptionData->pInst)
    {
        SIPX_NOTIFY_INFO pInfo;
        UtlString userAgent;
        UtlString contentType;
        const HttpBody* contentBody = NULL;
        ssize_t bodyLength = 0;
        const char* bodyBytes = NULL;

        // If the dialog changed from and early dialog to an
        // established dialog, update the dialog handle in the
        // subcription data structure
        if(earlyDialogHandle && dialogHandle &&
            SipDialog::isEarlyDialog(*subscriptionData->pDialogHandle))
        {
            *(subscriptionData->pDialogHandle) = dialogHandle;
        }

        if(notifyRequest)
        {
            notifyRequest->getUserAgentField(&userAgent);
            notifyRequest->getContentType(&contentType);
            contentBody = notifyRequest->getBody();

            if(contentBody)
            {
                contentBody->getBytes(&bodyBytes, &bodyLength);
            }
        }

        pInfo.nSize = sizeof(SIPX_NOTIFY_INFO);
        pInfo.hSub = (SIPX_SUB) applicationData;
        pInfo.szNotiferUserAgent = userAgent;
        pInfo.nContentLength = bodyLength;
        pInfo.pContent = bodyBytes;
        pInfo.szContentType = contentType;

        sipxFireEvent(subscriptionData->pInst->pCallManager,
           EVENT_CATEGORY_NOTIFY,
           &pInfo);

    }

    // No data for the subscription handle
    else
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
            "sipxSubscribeClientNotifyCallback: cannot find subscription data for handle: %p",
            applicationData);
    }
    return true;
}

void sipxLineReleaseLock(SIPX_LINE_DATA* pData, SIPX_LOCK_TYPE type)
{
    if (validLineData(pData))
    {
        switch (type)
        {
            case SIPX_LOCK_READ:
                pData->pMutex->releaseRead() ;
                break ;
            case SIPX_LOCK_WRITE:
                pData->pMutex->releaseWrite() ;
                break ;
            case SIPX_LOCK_NONE:
                break ;
        }
    }
}


void sipxInfoReleaseLock(SIPX_INFO_DATA* pData, SIPX_LOCK_TYPE type)
{
    switch (type)
    {
        case SIPX_LOCK_READ:
            pData->pMutex->releaseRead() ;
            break ;
        case SIPX_LOCK_WRITE:
            pData->pMutex->releaseWrite() ;
            break ;
        case SIPX_LOCK_NONE:
            break ;
    }
}

UtlBoolean validLineData(const SIPX_LINE_DATA* pData)
{
    UtlBoolean bValid = FALSE ;

    if (pData && pData->lineURI && pData->pInst &&
            pData->pInst->pCallManager && pData->pMutex)
    {
        bValid = TRUE ;
    }

    return bValid ;
}


void sipxLineObjectFree(const SIPX_LINE hLine)
{
    // First remove it from the HandleMap so no one else an find it
    SIPX_LINE_DATA* pData =
       (SIPX_LINE_DATA *)gpLineHandleMap->removeHandle(hLine) ;

    if (pData)
    {
        // Then lock it so anyone who was previously using it is known
        // to have released it
        if (pData->pMutex->acquireWrite() == OS_SUCCESS)
        {
            // Now it is safe to delete
            pData->pInst->pLock->acquire() ;
            pData->pInst->nLines-- ;
            assert(pData->pInst->nLines >= 0) ;
            pData->pInst->pLock->release() ;

            if (pData->lineURI)
            {
                delete pData->lineURI ;
            }

            if (pData->pMutex)
            {
                delete pData->pMutex ;
            }

            if (pData->pLineAliases)
            {
                UtlVoidPtr* pValue ;
                while (pValue = (UtlVoidPtr*) pData->pLineAliases->get())
                {
                    Url* pUri = (Url*) pValue->getValue() ;
                    if (pUri)
                    {
                        delete pUri ;
                    }
                    delete pValue ;
                }
            }

            delete pData ;
        }
    }
}



void sipxInfoObjectFree(SIPX_INFO hInfo)
{
    // First remove it from the HandleMap so no one else an find it
    SIPX_INFO_DATA* pData =
       (SIPX_INFO_DATA *)gpInfoHandleMap->removeHandle(hInfo) ;

    if (pData)
    {
        // Then lock it so anyone who was previously using it is known
        // to have released it
        if (pData->pMutex->acquireWrite() == OS_SUCCESS)
        {
            // Now it is safe to delete
            sipxInfoFree(pData) ;
        }
    }
}

void sipxInfoFree(SIPX_INFO_DATA* pData)
{
    if (pData)
    {
        if (pData->pMutex)
        {
            delete pData->pMutex ;
        }
        free((void*)pData->infoData.pContent);
        free((void*)pData->infoData.szContentType);
        free((void*)pData->infoData.szFromURL);
        free((void*)pData->infoData.szUserAgent);

        delete pData;
    }
}

SIPX_LINE sipxLineLookupHandle(SIPX_INSTANCE_DATA* pInst,
                               const char* szLineURI,
                               const char* szRequestUri)
{
    SIPX_LINE hLine = 0;

    hLine = sipxLineLookupHandleByURI(pInst, szLineURI);
    if (!hLine)
    {
        hLine = sipxLineLookupHandleByURI(pInst, szRequestUri);
    }

    return hLine;
}

SIPX_LINE sipxLineLookupHandleByURI(SIPX_INSTANCE_DATA* pInst,
                                    const char* szURI)
{
    SIPX_LINE hLine = 0 ;
    Url urlLine(szURI) ;

    // Use the line manager to find identity if available
    if (pInst && pInst->pLineManager)
    {
        SipLine* pLine = pInst->pLineManager->findLineByURL(urlLine, "unknown") ;
        if (pLine)
            urlLine = pLine->getIdentity() ;
    }

    if (gpLineHandleMap->lock())
    {
        UtlHashMapIterator iter(*gpLineHandleMap);

        UtlInt* pIndex = NULL;
        UtlVoidPtr* pObj = NULL;

        while (pIndex = dynamic_cast<UtlInt*>( iter() ) )
        {
            pObj = dynamic_cast<UtlVoidPtr*>(gpLineHandleMap->findValue(pIndex));
            SIPX_LINE_DATA* pData = NULL ;
            if (pObj)
            {
                pData = (SIPX_LINE_DATA*) pObj->getValue() ;
                if (pData)
                {
                    // Check main line definition
                    if (urlLine.isUserHostPortEqual(*pData->lineURI))
                    {
                        hLine = pIndex->getValue() ;
                        break ;
                    }

                    // Check for line aliases
                    if (pData->pLineAliases)
                    {
                        UtlVoidPtr* pValue ;
                        Url* pUrl ;
                        UtlSListIterator iterator(*pData->pLineAliases) ;

                        while (pValue = (UtlVoidPtr*) iterator())
                        {
                            pUrl = (Url*) pValue->getValue() ;

                            if (urlLine.isUserHostPortEqual(*pUrl))
                            {
                                hLine = pIndex->getValue() ;
                                break ;
                            }
                        }
                    }
                }
            }
        }

        gpLineHandleMap->unlock() ;
    }

    return hLine;
}


UtlBoolean sipxRemoveCallHandleFromConf(SIPX_CONF_DATA *pConfData,
                                        const SIPX_CALL hCall)
{
    //
    // WARNING: This relies on outside locking
    //


    OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG, "sipxRemoveCallHandleFromConf pConfData=%p, hCall=%u",
                  pConfData, hCall);

    UtlBoolean bFound = false ;
    size_t idx ;

    // First find the handle
    for (idx=0; idx < pConfData->nCalls; idx++)
    {
        if (pConfData->hCalls[idx] == hCall)
        {
            bFound = true ;
            break ;
        }
    }

    if (bFound)
    {
        // Next step on it.
        pConfData->nCalls-- ;

        for (; idx < pConfData->nCalls; idx++)
        {
            pConfData->hCalls[idx] = pConfData->hCalls[idx+1] ;
        }

        pConfData->hCalls[pConfData->nCalls] = SIPX_CALL_NULL ;
    }

    return bFound ;
}


UtlBoolean validConfData(const SIPX_CONF_DATA* pData)
{
    UtlBoolean bValid = FALSE ;

    if (pData && pData->pMutex)
    {
        bValid = TRUE ;
    }

    return bValid ;
}


#if 1
/*
   Course grained locking is the way to go!
*/
SIPX_CONF_DATA* sipxConfLookup(const SIPX_CONF hConf, SIPX_LOCK_TYPE type)
{
    SIPX_CONF_DATA* pRC = NULL;
    if (gpConfHandleMap->lock())
    {
        pRC = (SIPX_CONF_DATA*) gpConfHandleMap->findHandle(hConf) ;
        if (pRC == NULL) {
           gpConfHandleMap->unlock() ;
        }
    }
    return pRC ;
}

void sipxConfReleaseLock(SIPX_CONF_DATA* pData, SIPX_LOCK_TYPE type)
{
   if (pData != NULL) {
      gpConfHandleMap->unlock() ;
   }
}


#else
SIPX_CONF_DATA* sipxConfLookup(const SIPX_CONF hConf, SIPX_LOCK_TYPE type)
{
    SIPX_CONF_DATA* pRC = NULL ;

    if (gpConfHandleMap->lock())
    {
        pRC = (SIPX_CONF_DATA*) gpConfHandleMap->findHandle(hConf) ;
        if (validConfData(pRC))
        {
            switch (type)
            {
                case SIPX_LOCK_READ:
                    if (pRC->pMutex->acquireRead() != OS_SUCCESS)
                       pRC = NULL ;
                    break ;
                case SIPX_LOCK_WRITE:
                    if (pRC->pMutex->acquireWrite() != OS_SUCCESS)
                        pRC = NULL ;
                    break ;
            }

        }
        else
        {
            pRC = NULL ;
        }
        gpConfHandleMap->unlock() ;
    }

    return pRC ;
}


void sipxConfReleaseLock(SIPX_CONF_DATA* pData, SIPX_LOCK_TYPE type)
{
    if (validConfData(pData))
    {
        switch (type)
        {
            case SIPX_LOCK_READ:
                pData->pMutex->releaseRead() ;
                break ;
            case SIPX_LOCK_WRITE:
                pData->pMutex->releaseWrite() ;
                break ;
            case SIPX_LOCK_NONE:
                break ;
        }
    }
}
#endif

void sipxConfFree(const SIPX_CONF hConf)
{
    // First remove it from the HandleMap so no one else an find it
    SIPX_CONF_DATA* pData =
       (SIPX_CONF_DATA *)gpConfHandleMap->removeHandle(hConf);

    if (pData)
    {
        // Then lock it so anyone who was previously using it is known
        // to have released it
        if (pData->pMutex->acquireWrite() == OS_SUCCESS)
        {
            // Now it is safe to delete
            UtlString callId ;
            SIPX_INSTANCE_DATA* pInst = NULL ;

            pData->pInst->pLock->acquire() ;
            pData->pInst->nConferences-- ;
            assert(pData->pInst->nConferences >= 0) ;
            pData->pInst->pLock->release() ;

            callId = *pData->strCallId ;
            pInst = pData->pInst ;

            delete pData->pMutex ;
            delete pData->strCallId;
            delete pData ;

            if (pInst && !callId.isNull())
            {
                pInst->pCallManager->drop(callId) ;
            }
        }
    }
}

SIPX_INSTANCE_DATA* findSessionByCallManager(const void* pCallManager)
{
    SIPX_INSTANCE_DATA *pInst = NULL ;

    UtlDListIterator iter(*gpSessionList);

    UtlVoidPtr* pObj = NULL;

    while ((pObj = dynamic_cast<UtlVoidPtr*>(iter())))
    {
        SIPX_INSTANCE_DATA* pTest = (SIPX_INSTANCE_DATA*) pObj->getValue() ;
        if ((pTest) && (pTest->pCallManager == pCallManager))
        {
            pInst = pTest ;
            break ;
        }
    }

    return pInst ;
}

// Get the external host and port given the contact preference
void sipxGetContactHostPort(SIPX_INSTANCE_DATA* pData,
                            SIPX_CONTACT_TYPE contactType,
                            Url& uri)
{
    UtlBoolean bSet = FALSE  ;
    UtlString useIp ;
    int       usePort ;

    if (contactType == CONTACT_RELAY)
    {
        // Relay is not supported yet -- default to AUTO for now.
        contactType = CONTACT_AUTO  ;
    }

    // Use configured address first
    if ((contactType == CONTACT_AUTO) || (contactType == CONTACT_CONFIG))
    {
        if (pData->pSipUserAgent->getConfiguredPublicAddress(&useIp, &usePort))
        {
            uri.setHostAddress(useIp) ;
            uri.setHostPort(usePort) ;
            bSet = TRUE ;
        }
    }

    // Use NAT_MAPPED next
    if (!bSet && ((contactType == CONTACT_AUTO) || (contactType == CONTACT_NAT_MAPPED)))
    {
        if (pData->pSipUserAgent->getNatMappedAddress(&useIp, &usePort))
        {
            uri.setHostAddress(useIp) ;
            uri.setHostPort(usePort) ;
            bSet = TRUE ;
        }
    }

    // Lastly, use local
    if (!bSet)
    {
        if (pData->pSipUserAgent->getLocalAddress(&useIp, &usePort))
        {
            uri.setHostAddress(useIp) ;
            uri.setHostPort(usePort) ;
            bSet = TRUE ;
        }
    }
}


SIPXTAPI_API void sipxLogEntryAdd(OsSysLogPriority priority,
                     const char *format,
                     ...)
{
    va_list ap;
    va_start(ap, format);

    pthread_t threadId;
    OsTask::getCurrentTaskId(threadId) ;
    OsSysLog::vadd("sipXtapi", threadId, FAC_SIPXTAPI, priority, format, ap);

    va_end(ap);
}

SIPXTAPI_API SIPX_RESULT sipxConfigAllowMethod(const SIPX_INST hInst, const char* method, const bool bAllow)
{
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;

    if (pInst)
    {
        pInst->pSipUserAgent->allowMethod(method, bAllow);
    }
    return SIPX_RESULT_SUCCESS;
}

void sipxIncSessionCount()
{
    OsLock lock(*gpSessionLock);

    ++gSessions;
}

void sipxDecSessionCount()
{
    OsLock lock(*gpSessionLock);
    --gSessions;
}

int sipxGetSessionCount()
{
    return gSessions;
}


UtlBoolean sipxIsCallInFocus()
{
    UtlBoolean inFocus = false ;
    if (gpCallHandleMap->lock())
    {
        UtlHashMapIterator iter(*gpCallHandleMap);

        UtlInt* pIndex = NULL;
        UtlVoidPtr* pObj = NULL;
        SIPX_CALL hCall = 0 ;

        while (pIndex = dynamic_cast<UtlInt*>( iter() ) )
        {
            pObj = dynamic_cast<UtlVoidPtr*>(gpCallHandleMap->findValue(pIndex));
            SIPX_CALL_DATA* pData = NULL ;
            if (pObj)
            {
                pData = (SIPX_CALL_DATA*) pObj->getValue() ;
                if (pData->bInFocus)
                {
                    inFocus = true ;
                    break ;
                }
            }
        }

        gpCallHandleMap->unlock() ;
    }

    return inFocus ;
}


SIPX_RESULT sipxGetActiveCallIds(SIPX_INST hInst, int maxCalls, int& actualCalls, UtlString callIds[])
{
    SIPX_RESULT rc = SIPX_RESULT_FAILURE ;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;
    if (pInst)
    {
        OsStatus status = pInst->pCallManager->getCalls(maxCalls, actualCalls, callIds) ;
        if (status == OS_SUCCESS)
        {
            rc = SIPX_RESULT_SUCCESS ;
        }
    }
    else
    {
        rc = SIPX_RESULT_INVALID_ARGS ;
    }

    return rc ;
}


SIPX_RESULT sipxCheckForHandleLeaks()
{
    SIPX_RESULT rc = SIPX_RESULT_SUCCESS ;

    if (gpCallHandleMap->entries() != 0)
    {
        printf("\ngpCallHandleMap Leaks (%zu):\n", gpCallHandleMap->entries()) ;
        gpCallHandleMap->dump() ;
        rc = SIPX_RESULT_FAILURE ;
    }

    if (gpLineHandleMap->entries() != 0)
    {
        printf("\ngpLineHandleMap Leaks (%zu):\n", gpLineHandleMap->entries()) ;
        gpLineHandleMap->dump() ;
        rc = SIPX_RESULT_FAILURE ;
    }

    if (gpConfHandleMap->entries() != 0)
    {
        printf("\ngpConfHandleMap Leaks (%zu):\n", gpConfHandleMap->entries()) ;
        gpConfHandleMap->dump() ;
        rc = SIPX_RESULT_FAILURE ;
    }

    if (gpInfoHandleMap->entries() != 0)
    {
        printf("\ngpInfoHandleMap Leaks (%zu):\n", gpInfoHandleMap->entries()) ;
        gpInfoHandleMap->dump() ;
        rc = SIPX_RESULT_FAILURE ;
    }

    if (gpSessionList->entries() != 0)
    {
        printf("\ngSessionList leaks (%zu)\n", gpSessionList->entries()) ;
        rc = SIPX_RESULT_FAILURE ;
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxCallGetConnectionMediaInterface(const SIPX_CALL hCall,
                                                             void** ppInstData)
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE;
    int connectionId = -1;
    UtlString callId ;
    UtlString remoteAddress ;

    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ);

    assert(pData != 0);
    assert(pData->pInst != 0);

    if (pData && pData->callId && pData->remoteAddress)
    {
        callId = *pData->callId ;
        remoteAddress = *pData->remoteAddress ;
    }

    if (pData)
    {
        sipxCallReleaseLock(pData, SIPX_LOCK_READ) ;
    }

    if (!callId.isNull() && !remoteAddress.isNull())
    {
        connectionId = pData->pInst->pCallManager->getMediaConnectionId(callId, remoteAddress, ppInstData);
        if (-1 != connectionId)
        {
            sr = SIPX_RESULT_SUCCESS;
        }
    }

    return sr;
}

#ifdef VOICE_ENGINE
#include "include\VoiceEngineMediaInterface.h"
SIPXTAPI_API GipsVoiceEngineLib* sipxCallGetVoiceEnginePtr(const SIPX_CALL hCall)
{
    VoiceEngineMediaInterface* pMediaInterface = NULL;
    GipsVoiceEngineLib* pLib = NULL;

    if (hCall)
    {
        sipxCallGetConnectionMediaInterface(hCall, (void**)&pMediaInterface);

        if (pMediaInterface)
        {
            pLib = pMediaInterface->getVoiceEnginePtr();
        }
    }
    return pLib;
}

SIPXTAPI_API GipsVoiceEngineLib* sipxConfigGetVoiceEnginePtr(const SIPX_INST hInst)
{
    GipsVoiceEngineLib* ptr = NULL;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (pInst)
    {
        VoiceEngineFactoryImpl* pInterface =
                static_cast<VoiceEngineFactoryImpl*>(pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation());
        if (pInterface)
        {
            ptr = pInterface->getVoiceEnginePointer();
        }
    }

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigGetVoiceEnginePtr hInst=%x, ptr=%08X",
        hInst, ptr);
    return ptr;
}

SIPXTAPI_API GIPSAECTuningWizard* sipxConfigGetVoiceEngineAudioWizard()
{
    GIPSAECTuningWizard& wizard = GetGIPSAECTuningWizard();

    return &wizard;
}

#ifdef VIDEO
SIPXTAPI_API GipsVideoEngineWindows* sipxConfigGetVideoEnginePtr(const SIPX_INST hInst)
{
    GipsVideoEngineWindows* ptr = NULL;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (pInst)
    {
        VoiceEngineFactoryImpl* pImpl = (VoiceEngineFactoryImpl *) pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();
        if (pImpl)
        {
            ptr = pImpl->getVideoEnginePointer();
        }
    }

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigGetVideoEnginePtr hInst=%x, ptr=%08X",
        hInst, ptr);
    return ptr;
}
#endif

void GIPSVETraceCallback(char *szMsg, int iNum)
{
    OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
            "%s (%d)",
            szMsg,
            iNum);
}


SIPXTAPI_API SIPX_RESULT sipxEnableAudioLogging(const SIPX_INST hInst, bool bEnable)
{
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    GipsVoiceEngineLib* ptr = sipxConfigGetVoiceEnginePtr(hInst);

    if (NULL != ptr)
    {
        if (bEnable)
        {
            int irc = ptr->GIPSVE_SetTraceCallback(GIPSVETraceCallback);
        }
        else
        {
            int irc = ptr->GIPSVE_SetTraceCallback(NULL);
        }
        rc = SIPX_RESULT_SUCCESS;
    }

    return rc;
}

#endif

SIPXTAPI_API SIPX_RESULT sipxTranslateToneId(const TONE_ID toneId,
                                             TONE_ID& xlateId)
{
    SIPX_RESULT sr = SIPX_RESULT_SUCCESS;
    xlateId = (TONE_ID)0;
#ifdef VOICE_ENGINE
    if (toneId >= '0' && toneId <= '9')
    {
        xlateId = (TONE_ID)(toneId - '0');
    }
    else if (toneId == ID_DTMF_STAR)
    {
        xlateId = (TONE_ID)10;
    }
    else if (toneId == ID_DTMF_POUND)
    {
        xlateId = (TONE_ID)11;
    }
    else if (toneId == ID_DTMF_FLASH)
    {
        xlateId = (TONE_ID)16;
    }
    else
    {
        sr = SIPX_RESULT_FAILURE;
    }
#else
    if (toneId != ID_DTMF_FLASH)
    {
        xlateId = toneId;
    }
    else
    {
        sr = SIPX_RESULT_FAILURE;
    }
#endif /* VOICE_ENGINE */

    return sr;
}

SIPXTAPI_API SIPX_RESULT sipxConfigVoicemailSubscribe(const SIPX_INST hInst,
                                                      const char* szSubscribeURL)
{
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigVoicemailSubscribe hInst=%p URL=%s",
        hInst, szSubscribeURL);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;

    assert(pInst);
    if (pInst)
    {
        assert(pInst->pRefreshManager);
        assert(pInst->pLineManager);
        assert(pInst->pSipUserAgent);
        if (pInst->pRefreshManager && pInst->pLineManager && pInst->pSipUserAgent)
        {
            if (szSubscribeURL != NULL)
            {
                UtlString userId;
                UtlString contactUri;
                UtlString outboundLine;
                Url subscribeUrl(szSubscribeURL);
                subscribeUrl.getUserId(userId);

                pInst->pLineManager->getDefaultOutboundLine(outboundLine);

                // If the PHONESET_MSG_WAITING_SUBSCRIBE setting does
                // not have a userid field specified, get the one
                // from the default outbound line and use that
                if( userId.isNull() )
                {
                    // get default outbound line from the line manager
                    Url outbound(outboundLine);
                    outbound.getUserId(userId);
                    subscribeUrl.setUserId(userId);
                }

                // make sure we associate a user with the line
                pInst->pSipUserAgent->getContactUri( &contactUri );
                Url contactForLine ( contactUri );
                contactForLine.setUserId( userId );

                UtlString contactforLineStr = contactForLine.toString();

                SipMessage* mwiSubscribeRequest = new SipMessage();

                int sipSubscribePeriodSeconds;

                // get the Subscribe Period from the RefreshManager
                sipSubscribePeriodSeconds = pInst->pRefreshManager->getSubscribeTimeout();

                mwiSubscribeRequest->setVoicemailData (
                    outboundLine.data(),           // fromField
                    outboundLine.data(),           // toField
                    subscribeUrl.toString().data(),// uri
                    contactforLineStr.data(),      // contactUrl
                    NULL,                          // callId, do not set here
                    1,                             // cseq
                    sipSubscribePeriodSeconds);    // expiresInSeconds

                // send the subscribe request to the refresh manager
                pInst->pRefreshManager->newSubscribeMsg( *mwiSubscribeRequest );
                rc = SIPX_RESULT_SUCCESS;
            }
        }
    }

    return rc;
}

UtlBoolean sipxCallSetRemoveInsteadofDrop(SIPX_CALL hCall)
{
    UtlBoolean bSuccess = FALSE ;

    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE);
    if (pData)
    {
        pData->bRemoveInsteadOfDrop = TRUE ;
        bSuccess = TRUE ;

        sipxCallReleaseLock(pData, SIPX_LOCK_WRITE) ;
    }

    return bSuccess ;
}


UtlBoolean sipxCallIsRemoveInsteadOfDropSet(SIPX_CALL hCall)
{
    UtlBoolean bShouldRemove = FALSE ;

    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ);
    if (pData)
    {
        bShouldRemove = pData->bRemoveInsteadOfDrop ;

        sipxCallReleaseLock(pData, SIPX_LOCK_READ) ;
    }

    return bShouldRemove ;
}

UtlBoolean sipxAddCallHandleToConf(const SIPX_CALL hCall,
                                   const SIPX_CONF hConf)
{
    UtlBoolean bRC = false ;

    SIPX_CONF_DATA* pConfData = sipxConfLookup(hConf, SIPX_LOCK_WRITE) ;
    if (pConfData)
    {
        SIPX_CALL_DATA * pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE) ;
        if (pCallData)
        {
            pConfData->hCalls[pConfData->nCalls++] = hCall ;
            pCallData->hConf = hConf ;
            bRC = true ;

            sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE) ;
        }
        sipxConfReleaseLock(pConfData, SIPX_LOCK_WRITE) ;
    }

    return bRC ;
}

void sipxDumpCalls()
{
    if (gpCallHandleMap->lock())
    {
        UtlHashMapIterator iter(*gpCallHandleMap);

        UtlInt* pIndex = NULL;
        UtlVoidPtr* pObj = NULL;
        SIPX_CALL hCall = 0 ;

        while (pIndex = dynamic_cast<UtlInt*>( iter() ) )
        {
            pObj = dynamic_cast<UtlVoidPtr*>(gpCallHandleMap->findValue(pIndex));
            SIPX_CALL_DATA* pData = NULL ;
            if (pObj)
            {
                pData = (SIPX_CALL_DATA*) pObj->getValue() ;
            }

            if (pData)
            {
                hCall = pIndex->getValue() ;
                OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
                              "***************** CallDump***\n"
                              "hCall %d\n"
                              "****callId %s\n"
                              "****ghostCallId %s\n"
                              "***bRemoveInsteadOfDrop %d\n"
                              "****lineUri %s\n",
                    hCall,
                    pData->callId ? pData->callId->data() : NULL,
                    pData->ghostCallId ? pData->ghostCallId->data() : NULL,
                    pData->bRemoveInsteadOfDrop,
                    pData->lineURI ? pData->lineURI->data() : NULL);

            }
        }
        gpCallHandleMap->unlock() ;
    }
}

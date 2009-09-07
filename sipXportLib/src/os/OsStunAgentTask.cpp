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
#include "os/OsStunAgentTask.h"
#include "os/OsStunDatagramSocket.h"
#include "os/OsStunQueryAgent.h"
#include "os/OsMutex.h"
#include "os/OsLock.h"
#include "os/OsEvent.h"
#include "utl/UtlVoidPtr.h"
#include "utl/UtlHashMapIterator.h"

#ifndef _WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
OsMutex OsStunAgentTask::sLock(OsMutex::Q_FIFO) ;
OsStunAgentTask* OsStunAgentTask::spInstance = NULL ;

typedef struct CONNECTIVITY_INFO
{
    OsStunDatagramSocket* pSocket ;
    UtlString             address ;
    int                   iPort ;
    OsDateTime            lastSent ;
} CONNECTIVITY_INFO ;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

OsStunAgentTask::OsStunAgentTask()
    : OsServerTask("OsStunAgentTask-%d")
    , mReleased(false)
    , mMapsLock(OsMutex::Q_FIFO)
{
}

OsStunAgentTask::~OsStunAgentTask()
{
    // Wait for the thread to shutdown
    waitUntilShutDown() ;

    // discard any timers from the pool
    mTimerPool.destroyAll();

    {
       // this scope ensures that the following iterator is deleted before the mResponseMap
       //   I would have thought that it was without this, but it doesn't seem to work that way.
       UtlHashMapIterator iterator(mResponseMap);
       OsStunDatagramSocket* pKey;

       while ((pKey = (OsStunDatagramSocket*)iterator()))
       {
          OsTimer* pTimer = dynamic_cast<OsTimer*>(iterator.value());
          if (pTimer)
          {
             delete pTimer;
          }
          mResponseMap.removeReference(pKey);
       }
    }
}


OsStunAgentTask* OsStunAgentTask::getInstance(OsStunDatagramSocket* socket)
{
   {
      OsLock lock(sLock) ;

      if (spInstance == NULL)
      {
         spInstance = new OsStunAgentTask() ;
         spInstance->start() ;
      }
   }
   spInstance->addSocket(socket);

   return spInstance ;
}


void OsStunAgentTask::releaseInstance()
{
   OsLock lock(sLock) ;
   if (spInstance)
   {
      spInstance->mReleased = true;
      spInstance->releaseIfNotReferenced();
   }
}

void OsStunAgentTask::addSocket(OsStunDatagramSocket* socket)
{
   OsLock lock(mMapsLock);

   mSocketReferences.insert(socket);
}

/* Internal release routine that prevents premature destruction */
void OsStunAgentTask::releaseIfNotReferenced()
{
   if ( mReleased && mSocketReferences.isEmpty() && mResponseMap.isEmpty() )
   {
      delete spInstance ;
      spInstance = NULL ;
   }
}



/* ============================ MANIPULATORS ============================== */

UtlBoolean OsStunAgentTask::handleMessage(OsMsg& rMsg)
{
    UtlBoolean bHandled = false ;

    switch (rMsg.getMsgType())
    {
        case STUN_MSG_TYPE:
            bHandled = handleStunMessage((StunMsg&) rMsg) ;
            break ;
        case SYNC_MSG_TYPE:
            bHandled = handleSynchronize((OsRpcMsg&) rMsg) ;
            break ;
        case OsMsg::OS_EVENT:
            bHandled = handleStunTimerEvent((OsEventMsg&) rMsg) ;
            break ;
    }

    return bHandled ;
}


UtlBoolean OsStunAgentTask::handleStunMessage(StunMsg& rMsg)
{
    StunMessage msg;
    size_t nBuffer = rMsg.getLength() ;
    char* pBuffer = rMsg.getBuffer() ;
    OsStunDatagramSocket* pSocket = rMsg.getSocket() ;
    unsigned int mappedAddress ;

    if ((nBuffer > 0) && pBuffer && pSocket)
    {
        memset(&msg, 0, sizeof(StunMessage)) ;
        msg.parseMessage(pBuffer, nBuffer) ;

        switch (msg.msgHdr.msgType)
        {
            case BindRequestMsg:
                {
                    StunMessage respMsg ;
                    memset(&respMsg, 0, sizeof(StunMessage));

                    // Copy over ID
                    respMsg.msgHdr.msgType = BindResponseMsg;
                    for (int i=0; i<16; i++)
                    {
                        respMsg.msgHdr.id.octet[i] = msg.msgHdr.id.octet[i];
                    }

                    // TODO Send Error if changed port/ip requested

                    respMsg.hasMappedAddress = true;
                    respMsg.mappedAddress.ipv4.port = htons(rMsg.getReceivedPort()) ;
                    respMsg.mappedAddress.ipv4.addr = htonl(inet_addr(rMsg.getReceivedIp().data())) ;

                    UtlString hostIp ;
                    hostIp = pSocket->getLocalIp() ;
                    respMsg.hasSourceAddress = true;
                    respMsg.sourceAddress.ipv4.port = htons(pSocket->getLocalHostPort()) ;
                    respMsg.sourceAddress.ipv4.addr = htonl(inet_addr(hostIp.data())) ;

                    if (msg.hasResponseAddress)
                    {
                        // add reflected from
                        respMsg.hasReflectedFrom = true ;
                        respMsg.reflectedFrom.ipv4.port =respMsg.mappedAddress.ipv4.port ;
                        respMsg.reflectedFrom.ipv4.addr = respMsg.mappedAddress.ipv4.addr ;
                    }

                    char buf[STUN_MAX_MESSAGE_SIZE];
                    int len = STUN_MAX_MESSAGE_SIZE;

                    len = respMsg.encodeMessage(buf, len);
                    if (msg.hasResponseAddress)
                    {
                        UINT addr=htonl(msg.responseAddress.ipv4.addr) ;
                        pSocket->write(buf, len, inet_ntoa((*((in_addr*)&addr))), msg.responseAddress.ipv4.port) ;
                    }
                    else
                    {
                        pSocket->write(buf, len, rMsg.getReceivedIp(), rMsg.getReceivedPort()) ;
                    }
                }
                break ;
            case BindResponseMsg:
                {
                    UtlString address ;
                    int iPort ;

                    if (msg.msgHdr.id.octet[0] == 0x00)
                    {
                        // Discovery Response
                        mappedAddress=htonl (msg.mappedAddress.ipv4.addr) ;
                        address = inet_ntoa (*((in_addr*)&mappedAddress)) ;
                        iPort = msg.mappedAddress.ipv4.port ;

                        pSocket->setStunAddress(address, iPort) ;
                        signalStunOutcome(pSocket, true) ;
                    }
                    else
                    {
                        // Connectivity Response
                        OsLock lock(mMapsLock) ;

                        UtlString key ;
                        char cTemp[3] ;
                        for (int i=0; i<16; i++)
                        {
                            sprintf(cTemp, "%2X", msg.msgHdr.id.octet[i]) ;
                            key.append(cTemp) ;
                        }

                        UtlVoidPtr* pValue = (UtlVoidPtr*) mConnectivityMap.findValue(&key) ;
                        if (pValue)
                        {
                            CONNECTIVITY_INFO* pInfo = (CONNECTIVITY_INFO*) pValue->getValue() ;
                            pSocket->setDestinationAddress(pInfo->address, pInfo->iPort, msg.msgHdr.id.octet[0]) ;
                            mConnectivityMap.destroy(&key) ;
                            delete pInfo ;
                        }
                    }
                }
                break ;
            case BindErrorResponseMsg:
                {
                    UtlString empty ;
                    pSocket->setStunAddress(empty, -1) ;
                    signalStunOutcome(pSocket, false) ;
                }
                break ;
            case SharedSecretRequestMsg:
                    // TODO Send Error
                    break ;
            case SharedSecretResponseMsg:
            case SharedSecretErrorResponseMsg:
                break ;
        }
    }

    if (pBuffer)
    {
       free(pBuffer);
    }

    return true ;
}


UtlBoolean OsStunAgentTask::handleSynchronize(OsRpcMsg& rMsg)
{
    OsEvent* pEvent = rMsg.getEvent() ;
    pEvent->signal(0) ;

    return true ;
}


UtlBoolean OsStunAgentTask::handleStunTimerEvent(OsEventMsg& rMsg)
{
    OsLock lock(mMapsLock) ;
    OsStunDatagramSocket* pSocket ;
    void *pSocketVoid ;
    OsStatus rc ;

    // Pull out socket
    rc = rMsg.getUserData(pSocketVoid) ;
    assert(rc == OS_SUCCESS) ;
    pSocket = (OsStunDatagramSocket*) pSocketVoid ;

    // Refresh the socket
    if ((rc == OS_SUCCESS) && pSocket)
    {
        UtlVoidPtr key(pSocket) ;
        if (mResponseMap.contains(&key))
        {
            // We were waiting for a response -- timeout failure
            signalStunOutcome(pSocket, false) ;
        }
        else
        {
            // Refresh attempt
            pSocket->refreshStunBinding() ;
        }
    }

    return true ;
}


UtlBoolean OsStunAgentTask::sendStunDiscoveryRequest(OsStunDatagramSocket* pSocket,
                                                     const UtlString& stunServer,
                                                     const int stunPort,
                                                     const int stunOptions)
{
    OsLock lock(mMapsLock) ;

    assert(pSocket) ;
    assert(portIsValid(stunPort)) ;
    assert(stunServer.length() > 0) ;

    if (pSocket && portIsValid(stunPort) && (stunServer.length() > 0))
    {
        // Send the STUN request -- the OsStunQueryAgent will handle the
        // response
        UtlString serverAddress ;
        if (OsSocket::getHostIpByName(stunServer, &serverAddress) &&
                OsSocket::isIp4Address(serverAddress))
        {
            StunMessage reqMsg ;
            memset(&reqMsg, 0, sizeof(StunMessage)) ;

            // Set Msg Type
            reqMsg.msgHdr.msgType = BindRequestMsg;

            // Add randomness to transaction id
            for ( int i=0; i<16; i=i+4 )
            {
                int r = (rand()<<16) + rand() ;
                reqMsg.msgHdr.id.octet[i+0]= r>>0;
                reqMsg.msgHdr.id.octet[i+1]= r>>8;
                reqMsg.msgHdr.id.octet[i+2]= r>>16;
                reqMsg.msgHdr.id.octet[i+3]= r>>24;
            }

            // The first byte is used to communicate status back to the agent.
            // A value of zero indicates that this is a normal stun discovery
            // request.
            reqMsg.msgHdr.id.octet[0] = 0x00 ;

            if ((stunOptions & STUN_OPTION_CHANGE_PORT) || (stunOptions & STUN_OPTION_CHANGE_ADDRESS))
            {
                reqMsg.hasChangeRequest = 1 ;
                reqMsg.changeRequest.value = 0 ;
                if (stunOptions & STUN_OPTION_CHANGE_PORT)
                {
                    reqMsg.changeRequest.value |= 2 ;
                }

                if (stunOptions & STUN_OPTION_CHANGE_ADDRESS)
                {
                    reqMsg.changeRequest.value |= 4 ;
                }
            }

            char buf[STUN_MAX_MESSAGE_SIZE];
            int len = STUN_MAX_MESSAGE_SIZE;
            len = reqMsg.encodeMessage(buf, len);

            if (pSocket->write(buf, len, serverAddress, stunPort) <= 0)
            {
                signalStunOutcome(pSocket, false) ;
            }
            else
            {
                OsTime timeout(0, STUN_TIMEOUT_RESPONSE_MS * OsTime::USECS_PER_MSEC);
                OsQueuedEvent* pQueuedEvent ;
                OsTimer* pTimer = dynamic_cast<OsTimer*>(mTimerPool.last()) ;
                if (pTimer)
                {
                    pQueuedEvent = dynamic_cast<OsQueuedEvent*>(pTimer->getNotifier());
                    if (pQueuedEvent)
                    {
                        pQueuedEvent->setUserData(pSocket) ;
                    }
                }
                else
                {
                    pTimer = new OsTimer(getMessageQueue(), pSocket) ;
                    pQueuedEvent = (OsQueuedEvent*)pTimer->getNotifier();
                }

                OsTime reportFailureAfter(OsTime(0, 500)) ;
                pTimer->oneshotAfter(timeout) ;
                mResponseMap.insertKeyAndValue(pSocket, pTimer) ;
            }
        }
        else
        {
            signalStunOutcome(pSocket, false) ;
        }
    }

    return false ;
}


UtlBoolean OsStunAgentTask::sendStunConnectivityRequest(OsStunDatagramSocket* pSocket,
                                                        const UtlString& stunServer,
                                                        int iStunPort,
                                                        unsigned char cPriority)
{
    UtlString serverAddress ;
    int i ;

    if (OsSocket::getHostIpByName(stunServer, &serverAddress) &&
            OsSocket::isIp4Address(serverAddress))
    {
        StunMessage reqMsg ;
        memset(&reqMsg, 0, sizeof(StunMessage)) ;

        // Set Msg Type
        reqMsg.msgHdr.msgType = BindRequestMsg;

        // Add randomness to transaction id
        for (i=0; i<16; i=i+4)
        {
            int r = (rand()<<16) + rand() ;
            reqMsg.msgHdr.id.octet[i+0]= r>>0;
            reqMsg.msgHdr.id.octet[i+1]= r>>8;
            reqMsg.msgHdr.id.octet[i+2]= r>>16;
            reqMsg.msgHdr.id.octet[i+3]= r>>24;
        }

        // The first byte is used to communicate status back to the agent.
        // A value of zero indicates that this is a normal stun discovery
        // request.  Anything else suggest a connectivity check.
        reqMsg.msgHdr.id.octet[0] = cPriority ;

        char buf[STUN_MAX_MESSAGE_SIZE];
        int len = STUN_MAX_MESSAGE_SIZE;
        len = reqMsg.encodeMessage(buf, len);

        // Store info on the connectivity request
        UtlString key ;
        char cTemp[3] ;
        for (i=0; i<16; i++)
        {
            sprintf(cTemp, "%2X", reqMsg.msgHdr.id.octet[i]) ;
            key.append(cTemp) ;
        }

        CONNECTIVITY_INFO* pInfo = new CONNECTIVITY_INFO ;
        pInfo->address = stunServer ;
        pInfo->iPort = iStunPort ;
        pInfo->pSocket = pSocket ;
        OsDateTime::getCurTime(pInfo->lastSent) ;

        mMapsLock.acquire() ;
        mConnectivityMap.insertKeyAndValue(new UtlString(key), new UtlVoidPtr(pInfo)) ;
        mMapsLock.release() ;

        pSocket->write(buf, len, serverAddress, iStunPort) ;
    }

    return TRUE ;
}


void OsStunAgentTask::synchronize()
{
    OsLock lock(sLock) ;

    if (isStarted() && (getCurrentTask() != this))
    {
        // Send an event to ourself and wait for that message to be processed.
        OsEvent event ;
        OsRpcMsg msg(SYNC_MSG_TYPE, 0, event) ;
        if (postMessage(msg) == OS_SUCCESS)
        {
            event.wait() ;
        }
    }
}


void OsStunAgentTask::removeSocket(OsStunDatagramSocket* pSocket)
{
   {
      OsLock lock(mMapsLock);

      // Remove contents response map and null out the timer
      OsTimer* pTimer ;
      pTimer = dynamic_cast<OsTimer*>(mResponseMap.findValue(pSocket));
      if (pTimer)
      {
         pTimer->stop() ;
         OsQueuedEvent* pEvent = (OsQueuedEvent*) pTimer->getNotifier() ;
         if (pEvent)
         {
            pEvent->setUserData(0) ;
            if (!mTimerPool.find(pTimer))
            {
               mTimerPool.insert(pTimer);
            }
         }
         mResponseMap.removeReference(pSocket) ;
      }

      // Remove contents from Connectivity Map
      UtlHashMapIterator itor(mConnectivityMap) ;
      UtlString* pKey ;
      while ((pKey = (UtlString*) itor()))
      {
         UtlVoidPtr* pValue = (UtlVoidPtr*) mConnectivityMap.findValue(pKey) ;
         CONNECTIVITY_INFO* pInfo = (CONNECTIVITY_INFO*) pValue->getValue() ;

         if (pInfo->pSocket == pSocket)
         {
            mConnectivityMap.destroy(pKey) ;
            delete pInfo ;
         }
      }
      mSocketReferences.remove(pSocket);
   }

   OsLock lock(sLock);
   releaseIfNotReferenced();
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


void OsStunAgentTask::signalStunOutcome(OsStunDatagramSocket* pSocket,
                                        UtlBoolean bSuccess)
{
    OsLock lock(mMapsLock) ;

    // Remove from waiting response map
    OsTimer* pTimer = dynamic_cast<OsTimer*>(mResponseMap.findValue(pSocket));
    if (pTimer)
    {
       pTimer->stop() ;
       OsQueuedEvent* pEvent = (OsQueuedEvent*) pTimer->getNotifier() ;
       if (pEvent)
       {
          pEvent->setUserData(0) ;
          if (!mTimerPool.contains(pTimer))
          {
             mTimerPool.insert(pTimer) ;
          }
       }
    }
    mResponseMap.remove(pSocket) ;

    // Signal Socket
    if (bSuccess)
    {
        pSocket->markStunSuccess() ;
    }
    else
    {
        pSocket->markStunFailure() ;
    }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

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
#include <net/SipProtocolServerBase.h>
#include <net/SipUserAgent.h>
#include <utl/UtlHashMapIterator.h>
#include <os/OsDateTime.h>
#include <os/OsEvent.h>
#include <os/OsLock.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//#define TEST_PRINT
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipProtocolServerBase::SipProtocolServerBase(SipUserAgent* userAgent,
                                             const char* protocolString,
                                             const char* taskName) :
     OsTask(taskName),
     mClientLock(OsMutex::Q_FIFO)
{
   mSipUserAgent = userAgent;
   mProtocolString = protocolString;
   mDefaultPort = SIP_PORT;
}

// Copy constructor
SipProtocolServerBase::SipProtocolServerBase(const SipProtocolServerBase& rSipProtocolServerBase) :
    mClientLock(OsMutex::Q_FIFO)
{
}

// Destructor
SipProtocolServerBase::~SipProtocolServerBase()
{
    mDataGuard.acquire();
    mClientLock.acquireWrite();

    waitUntilShutDown();
    
    int iteratorHandle = mClientList.getIteratorHandle();
    SipClient* client = NULL;
    while ((client = (SipClient*)mClientList.next(iteratorHandle)))
    {
        mClientList.remove(iteratorHandle);
        delete client;
    }
    mClientList.releaseIteratorHandle(iteratorHandle);
    mClientLock.releaseWrite();
    mDataGuard.release();
}

/* ============================ MANIPULATORS ============================== */



UtlBoolean SipProtocolServerBase::send(SipMessage* message,
                            const char* hostAddress,
                            int hostPort)
{
    UtlBoolean sendOk = FALSE;


    UtlString localIp(message->getLocalIp());
    
    if (localIp.length() < 1)
    {
        localIp = mDefaultIp;
    }

    SipClient* client = createClient(hostAddress, hostPort, localIp);
    if(client)
    {
        int isBusy = client->isInUseForWrite();
        UtlString clientNames;

        client->getClientNames(clientNames);
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServerBase::send %p isInUseForWrite %d, client info\n %s",
                mProtocolString.data(), client, isBusy, clientNames.data());

        sendOk = client->sendTo(*message, hostAddress, hostPort);
        if(!sendOk)
        {
            OsTask* pCallingTask = OsTask::getCurrentTask();
            int callingTaskId = -1;
            int clientTaskId = -1;

            if ( pCallingTask )
            {
               pCallingTask->id(callingTaskId);
            }
            client->id(clientTaskId);

            if (clientTaskId != callingTaskId)
            {
               // Do not need to clientLock.acquireWrite();
               // as deleteClient uses the locking list lock
               // which is all that is needed as the client is
               // already marked as busy when we called
               // createClient above.
               deleteClient(client);
               client = NULL;
            }
        }
    }

        if(client)
        {
            releaseClient(client);
        }

    return(sendOk);
}

void SipProtocolServerBase::releaseClient(SipClient* client)
{
    mClientLock.acquireWrite();

    if(client &&
                clientExists(client))
    {
        if(client->isInUseForWrite())
        {
            client->markAvailbleForWrite();
        }
        else
        {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipProtocolServerBase::releaseClient releasing %s client not locked: %p",
                          mProtocolString.data(), client);
        }
    }
    else
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipProtocolServerBase::releaseClient releasing %s client not in list: %p",
                      mProtocolString.data(), client);
    }

    mClientLock.releaseWrite();
}

UtlBoolean SipProtocolServerBase::startListener()
{
    UtlHashMapIterator iter(mServerSocketMap);
    UtlVoidPtr* pSocketContainer = NULL;
    UtlString* pKey;
    while ((pKey =(UtlString*)iter()))
    {
        OsSocket* pSocket = NULL;
        SipClient* pServer = NULL;
        UtlVoidPtr* pServerContainer = NULL;

        UtlString localIp = *pKey;
        pSocketContainer = (UtlVoidPtr*)iter.value();
         
        if (pSocketContainer)
        {    
            pSocket = (OsSocket*)pSocketContainer->getValue();
        }
        
        pServerContainer = (UtlVoidPtr*)mServers.findValue(&localIp);
        if (!pServerContainer)
        {
            pServer = new SipClient(pSocket);
            this->mServers.insertKeyAndValue(new UtlString(localIp), new UtlVoidPtr((void*)pServer));
            pServer->start();
        }
        else
        {
            pServer = (SipClient*) pServerContainer->getValue();
        }
        if(mSipUserAgent)
        {
            if (pServer)
            {
                pServer->setUserAgent(mSipUserAgent);
            }
        }
    }
    return(TRUE);
}

SipClient* SipProtocolServerBase::createClient(const char* hostAddress,
                                               int hostPort,
                                               const char* localIp)
{
    UtlString remoteHostAddr;
    UtlBoolean clientStarted = FALSE;

    mClientLock.acquireWrite();

    SipClient* client = getClient(hostAddress, hostPort, localIp);

    if(! client)
    {
#       if TEST_CLIENT_CREATION
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipProtocolServerBase::createClient( %s, %d )",
                      hostAddress, hostPort);
#       endif

        if(!portIsValid(hostPort))
        {
            hostPort = mDefaultPort;
#           if TEST_CLIENT_CREATION
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipProtocolServerBase::createClient port defaulting to %d",
                          hostPort);
#           endif
        }

        OsTime time;
        OsDateTime::getCurTimeSinceBoot(time);
        long beforeSecs = time.seconds();

        OsSocket* clientSocket = buildClientSocket(hostPort, hostAddress, localIp);

        OsDateTime::getCurTimeSinceBoot(time);
        long afterSecs = time.seconds();
        if(afterSecs - beforeSecs > 1)
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING, "SIP %s socket create for %s:%d took %d seconds",
                mProtocolString.data(), hostAddress, hostPort,
                (int)(afterSecs - beforeSecs));
        }

        UtlBoolean isOk = clientSocket->isOk();
        if(!isOk)
        {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "SIP %s socket %s:%d not OK",
                          mProtocolString.data(), hostAddress, hostPort);
        }
        int writeWait = 3000; // mSec
        UtlBoolean isReadyToWrite = clientSocket->isReadyToWrite(writeWait);
        if(!isReadyToWrite)
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SIP %s socket %s:%d not ready for writing after %d seconds",
                          mProtocolString.data(), hostAddress, hostPort, (int) (writeWait/1000));
        }

        if(isOk &&
           isReadyToWrite)
        {
#ifdef TEST
            osPrintf("Socket OK, creating client\n");
#endif
            client = new SipClient(clientSocket) ;
            if (client && mSipUserAgent->getUseRport() &&
                    clientSocket->getIpProtocol() == OsSocket::UDP)
            {
                client->setSharedSocket(TRUE) ;
            }

#ifdef TEST
            osPrintf("Created client\n");
#endif
            if(mSipUserAgent)
            {
                client->setUserAgent(mSipUserAgent);
            }

            if (clientSocket->getIpProtocol() != OsSocket::UDP)
            {
                //osPrintf("starting client\n");
                clientStarted = client->start();
                if(!clientStarted)
                {
                    osPrintf("SIP %s client failed to start\n",
                        mProtocolString.data());
                }
            }

            OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServer(SipProtocolServerBase)::createClient client: %p %s:%d",
                mProtocolString.data(), client, hostAddress, hostPort);

            mClientList.push(client);
        }

        // The socket failed to be connected
        else
        {
            if(clientSocket)
            {
                if (!mSipUserAgent->getUseRport() ||
                        (clientSocket->getIpProtocol() == OsSocket::TCP))
                {
                    delete clientSocket;
                }
                clientSocket = NULL;
            }
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "Sip%sServer(SipProtocolServerBase)::createClient client %p Failed to create socket %s:%d",
                          mProtocolString.data(), this, hostAddress, hostPort);
        }
    }

    int isBusy = FALSE;
    if(client)
    {
        isBusy = client->isInUseForWrite();

        if(!isBusy)
            client->markInUseForWrite();
    }

    mClientLock.releaseWrite();

    if(client && isBusy)
    {
        if(!waitForClientToWrite(client)) client = NULL;
    }

    return(client);
}

int SipProtocolServerBase::isOk()
{
    UtlBoolean bRet = true;
    
    SipClient* pServer = NULL;
    UtlHashMapIterator iterator(mServers);
    UtlVoidPtr* pServerContainer = NULL;
    UtlString* pKey = NULL;
    
    while ((pKey = (UtlString*)iterator()))
    {
        pServerContainer = (UtlVoidPtr*)iterator.value();
        if (pServerContainer)
        {
            pServer = (SipClient*)pServerContainer->getValue();
        }
        
        if (pServer)
        {
            bRet = bRet && pServer->isOk();
        }
    }
    return bRet;
}

UtlBoolean SipProtocolServerBase::waitForClientToWrite(SipClient* client)
{
    UtlBoolean exists;
    UtlBoolean busy = FALSE;
    int numTries = 0;

    do
    {
        numTries++;

        mClientLock.acquireWrite();
        exists = clientExists(client);

        if(exists)
        {
            busy =  client->isInUseForWrite();
            if(!busy)
            {
                client->markInUseForWrite();
                mClientLock.releaseWrite();
                if(numTries > 1)
                {
                   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                 "Sip%sServerBase::waitForClientToWrite %p locked after %d tries",
                                 mProtocolString.data(), client, numTries);
                }
            }
            else
            {
                // We set an event to be signaled when a
                // transaction is released.
                OsEvent* waitEvent = new OsEvent();
                client->notifyWhenAvailableForWrite(*waitEvent);

                // Must unlock while we wait or there is a dead lock
                mClientLock.releaseWrite();
#ifdef TEST_PRINT
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                              "Sip%sServerBase::waitForClientToWrite %p "
                              "waiting on: %p after %d tries",
                              mProtocolString.data(), client, waitEvent, numTries);
#endif

                                // Do not block forever
                                OsTime maxWaitTime(0, 500000);

                                // If the other side signalled
            if(waitEvent->wait(maxWaitTime)  == OS_SUCCESS)
                                {
                                        // The other side is no longer referencing
                                        // the event.  This side must clean it up
                                        delete waitEvent;
                                        waitEvent = NULL;
                                }
                                // A timeout occured and the otherside did not signal yet
                                else
                                {
                                        // Signal the other side to indicate we are done
                                        // with the event.  If already signaled, we lost
                                        // a race and the other side was done first.
                                        if(waitEvent->signal(0) == OS_ALREADY_SIGNALED)
                                        {
                                                delete waitEvent;
                                                waitEvent = NULL;
                                        }
                                }

#ifdef TEST_PRINT
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "Sip%sServerBase::waitForClientToWrite %p done waiting after %d tries",
                          mProtocolString.data(), client, numTries);
#endif
            }
        }
        else
        {
            mClientLock.releaseWrite();

            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "Sip%sServerBase::waitForClientToWrite %p gone after %d tries",
                          mProtocolString.data(), client, numTries);
        }
    }
    while(exists && busy);

    return(exists && !busy);
}

SipClient* SipProtocolServerBase::getClient(const char* hostAddress,
                                  int hostPort, const char* localIp)
{
    UtlBoolean isSameHost = FALSE;
    UtlString hostAddressString(hostAddress ? hostAddress : "");
    int iteratorHandle = mClientList.getIteratorHandle();
    SipClient* client = NULL;

#   if TEST_CLIENT_CREATION
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipProtocolServerBase::getClient( %s, %d )",
                  hostAddress, hostPort);
#   endif

    while ((client = (SipClient*)mClientList.next(iteratorHandle)))
    {
        // Are these the same host?

        isSameHost = client->isConnectedTo(hostAddressString, hostPort);

        if(isSameHost && client->isOk() &&
           0 == strcmp(client->getLocalIp(), localIp))
        {
            break;
        }
        else if(isSameHost)
        {
            if(!client->isOk())
            {
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "%s Client matches but is not OK",
                    mProtocolString.data());
            }
        }
    }
    mClientList.releaseIteratorHandle(iteratorHandle);

#   ifdef TEST_CLIENT_CREATION
    if (!client)
    {
       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                     "SipProtocolServerBase::getClient( %s, %d ) NOT FOUND",
                     hostAddress, hostPort);
    }
#   endif

    return(client);
}

void SipProtocolServerBase::deleteClient(SipClient* sipClient)
{
    // Find the client in the list of clients and shut it down
    int iteratorHandle = mClientList.getIteratorHandle();
    SipClient* client = NULL;

#ifdef TEST_PRINT

    OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServer::deleteClient(%p)",
        mProtocolString.data(), sipClient);
#endif
    while ((client = (SipClient*)mClientList.next(iteratorHandle)))
    {
        // Remove this bad client
                // This used to be a little over zealous and delete any
                // SipClient that was not ok.  It was not checking if
                // the SipClient was busy or not so bad things could
                // happen.  This is now on the conservative side and
                // deleting only the thing it is supposed to.
        if(client == sipClient)
        {
#ifdef TEST_PRINT
            UtlString clientNames;
            client->getClientNames(clientNames);
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "Removing %s client %p names:\n%s",
                mProtocolString.data(), this, clientNames.data());
#endif
            mClientList.remove(iteratorHandle);

            break;
        }
    }
    mClientList.releaseIteratorHandle(iteratorHandle);

    // Delete the client outside the lock on the list as
    // it can create a deadlock.  If the client is doing
    // an operation that requires the locking list, the
    // client gets blocked from shutting down.  We then
    // block here trying to delete the client forever.
    if(client)
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServer::deleteClient(%p) done",
                      mProtocolString.data(), sipClient);
        delete client;
        client = NULL;
    }

#ifdef TEST_PRINT
    OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServer::deleteClient(%p) done",
        mProtocolString.data(), sipClient);
#endif
}

void SipProtocolServerBase::removeOldClients(long oldTime)
{
    mClientLock.acquireWrite();
    // Find the old clients in the list  and shut them down
    int iteratorHandle = mClientList.getIteratorHandle();
    SipClient* client;
    int numClients = mClientList.getCount();
    int numDelete = 0;
    int numBusy = 0;
    SipClient** deleteClientArray = NULL;


    UtlString clientNames;
    while ((client = (SipClient*)mClientList.next(iteratorHandle)))
    {
        if(client->isInUseForWrite()) numBusy++;

        // Remove any client with a bad socket
        // With TCP clients let them stay around if they are still
        // good as the may stay open for the session
        // The clients opened from this side for sending requests
        // get closed by the server (i.e. other side).  The clients
        // opened as servers for requests from the remote side are
        // explicitly closed on this side when the final response is
        // sent.
        if(   ! client->isInUseForWrite() // can't remove it if writing to it...
           && (   ! client->isOk() // socket is bad
               || client->getLastTouchedTime() < oldTime // idle for long enough
               )
           )
        {
           client->getClientNames(clientNames);
#ifdef TEST_PRINT
            osPrintf("Removing %s client names:\n%s\r\n",
                mProtocolString.data(), clientNames.data());
#endif
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServer::Removing old client %p:\n%s\r",
                          mProtocolString.data(), client, clientNames.data());

            mClientList.remove(iteratorHandle);
            // Delete the clients after releasing the lock
            if(!deleteClientArray) deleteClientArray =
                new SipClient*[numClients];

            deleteClientArray[numDelete] = client;
            numDelete++;

            client = NULL;
        }
        else
        {
#           ifdef TEST_PRINT
            UtlString names;
            client->getClientNames(names);
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServer::removeOldClients leaving client:\n%s",
                mProtocolString.data(), names.data());
#           endif
        }
    }
    mClientList.releaseIteratorHandle(iteratorHandle);
    mClientLock.releaseWrite();

    if ( numDelete || numBusy ) // get rid of lots of 'doing nothing when nothing to do' messages in the log
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "Sip%sServer::removeOldClients deleting %d of %d SipClients (%d busy)",
                      mProtocolString.data(), numDelete, numClients, numBusy);
    }
    // These have been removed from the list so delete them
    // after releasing the locks
    for(int clientIndex = 0; clientIndex < numDelete; clientIndex++)
    {
        delete deleteClientArray[clientIndex];
    }

    if(deleteClientArray)
    {
        delete[] deleteClientArray;
        deleteClientArray = NULL;
    }
}

// Assignment operator
SipProtocolServerBase&
SipProtocolServerBase::operator=(const SipProtocolServerBase& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void SipProtocolServerBase::startClients()
{
        int iteratorHandle = mClientList.getIteratorHandle();
    SipClient* client = NULL;
    while ((client = (SipClient*)mClientList.next(iteratorHandle)))
    {
        client->start();
    }
    mClientList.releaseIteratorHandle(iteratorHandle);
}

void SipProtocolServerBase::shutdownClients()
{
        // For each client request shutdown
    int iteratorHandle = mClientList.getIteratorHandle();
    SipClient* client = NULL;
    while ((client = (SipClient*)mClientList.next(iteratorHandle)))
    {
        client->requestShutdown();
    }
    mClientList.releaseIteratorHandle(iteratorHandle);
}

/* ============================ ACCESSORS ================================= */
int SipProtocolServerBase::getClientCount()
{
    return(mClientList.getCount());
}

void SipProtocolServerBase::addClient(SipClient* client)
{
    if(client)
    {
        mClientList.push(client);
    }
}

UtlBoolean SipProtocolServerBase::clientExists(SipClient* client)
{
    SipClient* listClient;
    UtlBoolean found = FALSE;

    int iteratorHandle = mClientList.getIteratorHandle();
    while ((listClient = (SipClient*)mClientList.next(iteratorHandle)))
    {
        if(client == listClient)
        {
            found = TRUE;
            break;
        }
    }

    mClientList.releaseIteratorHandle(iteratorHandle);

    return(found);
}

void SipProtocolServerBase::printStatus()
{
    int numClients = mClientList.getCount();
    int iteratorHandle = mClientList.getIteratorHandle();

    OsTime time;
    OsDateTime::getCurTimeSinceBoot(time);
    long currentTime = time.seconds();

    //long currentTime = OsDateTime::getSecsSinceEpoch();
    SipClient* client;
    UtlString clientNames;
    long clientTouchedTime;
    UtlBoolean clientOk;

    osPrintf("%s %d clients in list at: %ld\n",
        mProtocolString.data(), numClients, currentTime);

    while ((client = (SipClient*)mClientList.next(iteratorHandle)))
    {
        // Remove this or any other bad client
        clientTouchedTime = client->getLastTouchedTime();
        clientOk = client->isOk();
        client->getClientNames(clientNames);

        osPrintf("%s client %p last used: %ld ok: %d names:\n%s\n",
            mProtocolString.data(), this, clientTouchedTime,
            clientOk, clientNames.data());
    }
    mClientList.releaseIteratorHandle(iteratorHandle);
}

/* ============================ INQUIRY =================================== */

#ifdef LOOKING_FOR_T220_COMPILER_BUG /* [ */
int SipProtocolServerBase::dumpLayout(void *Ths)
{
   SipProtocolServerBase* THIS = (SipProtocolServerBase*) Ths;
   printf("SipProtocolServerBase: size = %d bytes\n", sizeof(*THIS));
   printf("  offset(startOfSipProtocolServerBase) = %d\n",
      (((int) &(THIS->startOfSipProtocolServerBase)) - ((int) THIS)));
   printf("  offset(mProtocolString) = %d\n",
      (((int) &(THIS->mProtocolString)) - ((int) THIS)));
   printf("  offset(mDefaultPort) = %d\n",
      (((int) &(THIS->mDefaultPort)) - ((int) THIS)));
   printf("  offset(mSipUserAgent) = %d\n",
      (((int) &(THIS->mSipUserAgent)) - ((int) THIS)));
   printf("  offset(mClientLock) = %d\n",
      (((int) &(THIS->mClientLock)) - ((int) THIS)));
   printf("  offset(mClientList) = %d\n",
      (((int) &(THIS->mClientList)) - ((int) THIS)));
   printf("  offset(endOfSipProtocolServerBase) = %d\n",
      (((int) &(THIS->endOfSipProtocolServerBase)) - ((int) THIS)));
   OsLockingList::dumpLayout(Ths);
   return sizeof(*THIS);
}
#endif /* LOOKING_FOR_T220_COMPILER_BUG ] */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

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
#include <net/SipClient.h>
#include <net/SipClientUdp.h>
#include <net/SipClientTcp.h>
#include <net/SipClientTls.h>
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
   OsServerTask(taskName),
   mProtocolString(protocolString),
   mDefaultPort(SIP_PORT),
   mSipUserAgent(userAgent),
   mClientLock(OsBSem::Q_FIFO, OsBSem::FULL)
{
}

// Destructor
SipProtocolServerBase::~SipProtocolServerBase()
{
   // Shut down the thread.
   waitUntilShutDown();

   /* We do not seize mClientLock because the caller of a destructor has
    * to ensure single-threaded access anyway. */
   mClientList.destroyAll();

   // Delete all the server SipClient's.
   mServers.destroyAll();
   mServerPortMap.destroyAll();    
   mServerSocketMap.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Handles an incoming message (from the message queue).
UtlBoolean SipProtocolServerBase::handleMessage(OsMsg& eventMessage)
{
   UtlBoolean messageProcessed = FALSE;

   int msgType = eventMessage.getMsgType();
   int msgSubType = eventMessage.getMsgSubType();

   if(msgType == OsMsg::OS_EVENT &&
      msgSubType == SipProtocolServerBase::SIP_SERVER_GC)
   {
      // A client signals that it is closing down and so clients should
      // be garbage-collected.
      // Remove only clients that have shut down.
      removeOldClients(0);
      messageProcessed = TRUE;
   }

   return (messageProcessed);
}

UtlBoolean SipProtocolServerBase::send(SipMessage* message,
                                       const char* hostAddress,
                                       int hostPort)
{
    UtlBoolean sendOk = FALSE;

    UtlString localIp(message->getLocalIp());
    
    if (localIp.isNull())
    {
        localIp = mDefaultIp;
    }

    // Take the client list lock, because we have to make sure that
    // the client isn't deleted before its sendTo() returns.
    OsLock lock(mClientLock);

    SipClient* client = createClient(hostAddress, hostPort, localIp);
    if (client)
    {
       if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
       {
          UtlString clientNames;
          client->getClientNames(clientNames);
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipProcolServerBase[%s]::send %p, clientNames '%s'",
                        getName().data(), client, clientNames.data());
       }
       sendOk = client->sendTo(*message, hostAddress, hostPort);
    }

    return (sendOk);
}

UtlBoolean SipProtocolServerBase::startListener()
{
    UtlHashMapIterator iter(mServerSocketMap);
    UtlVoidPtr* pSocketContainer = NULL;
    UtlString* pKey;
    while ((pKey = dynamic_cast <UtlString*> (iter())))
    {
        OsSocket* pSocket = NULL;

        UtlString localIp = *pKey;
        pSocketContainer = (UtlVoidPtr*) iter.value();
         
        if (pSocketContainer)
        {    
            pSocket = (OsSocket*)pSocketContainer->getValue();
        }
        
        // Look up the SipClient for localIp.
        SipClient* pServer =
           dynamic_cast <SipClient*> (mServers.findValue(&localIp));
        if (!pServer)
        {
           // No SipClient exists in mServers, so create one and insert it.
           pServer = 
              strcmp(mProtocolString, SIP_TRANSPORT_UDP) == 0 ?
              static_cast <SipClient*> (new SipClientUdp(pSocket, this, mSipUserAgent)) :
              strcmp(mProtocolString, SIP_TRANSPORT_TCP) == 0 ?
              static_cast <SipClient*> (new SipClientTcp(pSocket, this, mSipUserAgent)) :
              strcmp(mProtocolString, SIP_TRANSPORT_TLS) == 0 ?
              static_cast <SipClient*> (new SipClientTls(pSocket, this, mSipUserAgent)) :
              NULL;
           this->mServers.insertKeyAndValue(new UtlString(localIp), pServer);
           pServer->start();
        }
    }
    return(TRUE);
}

SipClient* SipProtocolServerBase::createClient(const char* hostAddress,
                                               int hostPort,
                                               const char* localIp)
{
   UtlString remoteHostAddr;
   SipClient* client;
   UtlBoolean clientStarted = FALSE;

   {
      client = getClient(hostAddress, hostPort, localIp);
      if(!client)
      {
#       ifdef TEST_CLIENT_CREATION
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipProtocolServerBase[%s]::createClient('%s', %d)",
                       getName().data(), hostAddress, hostPort);
#       endif

         if(!portIsValid(hostPort))
         {
            hostPort = mDefaultPort;
#           ifdef TEST_CLIENT_CREATION
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipProtocolServerBase[%s]::createClient port defaulting to %d",
                          getName().data(), hostPort);
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

#ifdef TEST
         osPrintf("Socket OK, creating client\n");
#endif
         client = 
            strcmp(mProtocolString, SIP_TRANSPORT_UDP) == 0 ?
            static_cast <SipClient*> (new SipClientUdp(clientSocket, this, mSipUserAgent)) :
            strcmp(mProtocolString, SIP_TRANSPORT_TCP) == 0 ?
            static_cast <SipClient*> (new SipClientTcp(clientSocket, this, mSipUserAgent)) :
            strcmp(mProtocolString, SIP_TRANSPORT_TLS) == 0 ?
            static_cast <SipClient*> (new SipClientTls(clientSocket, this, mSipUserAgent)) :
            NULL;
         if (client && mSipUserAgent->getUseRport() &&
             clientSocket->getIpProtocol() == OsSocket::UDP)
         {
            client->setSharedSocket(TRUE);
         }

#ifdef TEST
         osPrintf("Created client\n");
#endif

         // Start the client's thread.
         clientStarted = client->start();
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipProtocolServerBase[%s]::createClient client: %p '%s':%d",
                       getName().data(), client, hostAddress, hostPort);
         if (!clientStarted)
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "SipProtocolServerBase[%s]::createClient start() failed",
                          getName().data());
         }

         mClientList.append(client);
      }
   }

   return (client);
}

int SipProtocolServerBase::isOk()
{
    UtlBoolean bRet = true;
    
    UtlHashMapIterator iterator(mServers);
    UtlString* pKey;
    
    // Iterate through all the SipClient's.
    while ((pKey = dynamic_cast <UtlString*> (iterator())))
    {
       // 'and' its status into the overall status.
       SipClient* pServer = dynamic_cast <SipClient*> (iterator.value());
       bRet = bRet && pServer->isOk();
    }
    return bRet;
}

SipClient* SipProtocolServerBase::getClient(const char* hostAddress,
                                            int hostPort,
                                            const char* localIp)
{
   UtlBoolean isSameHost = FALSE;
   UtlString hostAddressString(hostAddress ? hostAddress : "");
   SipClient* client = NULL;

#   ifdef TEST_CLIENT_CREATION
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipProtocolServerBase[%s]::getClient('%s', %d)",
                 getName().data(), hostAddress, hostPort);
#   endif

   UtlSListIterator iter(mClientList);
   while ((client = dynamic_cast <SipClient*> (iter())))
   {
      // Are these the same host?
      isSameHost = client->isConnectedTo(hostAddressString, hostPort);
      if (isSameHost)
      {
         // Is the client OK?
         if (client->isOk())
         {
            // But only accept it if the local IP is correct, too.
            if (0 == strcmp(client->getLocalIp(), localIp))
            {
               break;
            }
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipProtocolServerBase[%s]::getClient('%s', %d)"
                          " Client matches host/port but is not OK",
                          getName().data(), mProtocolString.data(), hostPort);
         }
      }
   }

#   ifdef TEST_CLIENT_CREATION
   if (!client)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipProtocolServerBase[%s]::getClient('%s', %d) NOT FOUND",
                    getName().data(), hostAddress, hostPort);
   }
#   endif

   return (client);
}

void SipProtocolServerBase::deleteClient(SipClient* sipClient)
{
#ifdef TEST_PRINT

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipProtocolServerBase[%s]::deleteClient(%p)",
                 getName().data(), sipClient);
#endif
   
   // Remove sipClient from mClientList (if it is on the list).
   UtlContainable* ret = mClientList.removeReference(sipClient);

   if (ret != NULL)
   {
#ifdef TEST_PRINT
      UtlString clientNames;
      client->getClientNames(clientNames);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipProtocolServerBase[%s]::deleteClient Removing %s client %s %p names: %s",
                    getName().data(),
                    sipClient->mProtocolString.data(),
                    sipClient->getName().data(), sipClient,
                    clientNames.data());
#endif
      delete sipClient;
   }

#ifdef TEST_PRINT
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipProtocolServerBase[%s]::deleteClient(%p) done",
                 getName().data(), sipClient);
#endif
}

void SipProtocolServerBase::removeOldClients(long oldTime)
{
   // Find the old clients in the list and shut them down
   int numClients;
   int numDelete = 0;
   SipClient** deleteClientArray = NULL;

   {
      OsLock lock(mClientLock);

      numClients = mClientList.entries();

      UtlSListIterator iter(mClientList);
      SipClient* client;

      while ((client = dynamic_cast <SipClient*> (iter())))
      {
         // Remove any client with a bad socket
         // With TCP clients, let them stay around if they are still
         // good as the may stay open for the session
         // The clients opened from this side for sending requests
         // get closed by the server (i.e. other side).  The clients
         // opened as servers for requests from the remote side are
         // explicitly closed on this side when the final response is
         // sent.
         if (   !client->isOk() // socket is bad or task has stopped
             || client->getLastTouchedTime() < oldTime // idle for long enough
            )
         {
            UtlString clientNames;
            client->getClientNames(clientNames);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipProtocolServerBase[%s]::removeOldClients Removing old client %p: %s",
                          getName().data(), client, clientNames.data());

            mClientList.removeReference(client);
            // Record the client in the array so it can be deleted
            // after we release the lock.
            if (!deleteClientArray)
            {
               deleteClientArray = new SipClient*[numClients];
            }
            deleteClientArray[numDelete] = client;
            numDelete++;
         }
         else
         {
#           ifdef TEST_PRINT
            UtlString names;
            client->getClientNames(names);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipProtocolServerBase[%s]::removeOldClients leaving client:\n%s",
                          getName().data(), names.data());
#           endif
         }
      }
   }

   if (numDelete > 0) // get rid of lots of 'doing nothing when nothing to do' messages in the log
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipProtocolServerBase[%s]::removeOldClients deleting %d of %d SipClients",
                    getName().data(), numDelete, numClients);
   }

   // If there are any clients that have been removed from the list, delete them
   // now that we have released the lock.
   if (deleteClientArray)
   {
      for (int clientIndex = 0; clientIndex < numDelete; clientIndex++)
      {
         delete deleteClientArray[clientIndex];
      }
      delete[] deleteClientArray;
   }
}

void SipProtocolServerBase::startClients()
{
   UtlSListIterator iter(mClientList);
   SipClient* client;
   while ((client = dynamic_cast <SipClient*> (iter())))
   {
      client->start();
   }
}

void SipProtocolServerBase::shutdownClients()
{
   // For each client, request shutdown.
   UtlSListIterator iter(mClientList);
   SipClient* client;
   while ((client = dynamic_cast <SipClient*> (iter())))
   {
      client->requestShutdown();
   }
}

/* ============================ ACCESSORS ================================= */
int SipProtocolServerBase::getClientCount()
{
   return (mClientList.entries());
}

void SipProtocolServerBase::addClient(SipClient* client)
{
   if (client)
   {
      mClientList.append(client);
   }
}

UtlBoolean SipProtocolServerBase::clientExists(SipClient* client)
{
   return mClientList.contains(client);
}

void SipProtocolServerBase::printStatus()
{
   int numClients = mClientList.entries();

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipProtocolServerBase[%s]::printStatus %d clients in list at time %ld",
                 getName().data(), numClients, OsDateTime::getSecsSinceEpoch());

   UtlSListIterator iter(mClientList);
   SipClient* client;
   while ((client = dynamic_cast <SipClient*> (iter())))
   {
      long clientTouchedTime = client->getLastTouchedTime();
      bool clientOk = client->isOk();
      UtlString clientNames;
      client->getClientNames(clientNames);

      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipProtocolServerBase[%s]::printStatus client %s %p last used: %ld, ok: %d, names: %s",
                    getName().data(),
                    client->getName().data(), client,
                    clientTouchedTime, clientOk, clientNames.data());
   }
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

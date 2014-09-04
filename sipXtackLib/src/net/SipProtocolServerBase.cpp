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
#include <boost/lexical_cast.hpp>

#include "net/HttpMessage.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

//#define TEST_CLIENT_CREATION

// STATIC VARIABLE INITIALIZATIONS
static const int MAX_CONCURRENT_THREADS = 10;
static const bool ENFORCE_MAX_CONCURRENT_THREADS = true;
static const bool ENABLE_THREAD_POOL = true;

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
  mIsSecureTransport(false),
  _threadPoolSem(MAX_CONCURRENT_THREADS)
{
}

// Destructor
SipProtocolServerBase::~SipProtocolServerBase()
{
   // Shut down the thread.
   waitUntilShutDown();

   /* We do not seize mClientLock because the caller of a destructor has
    * to ensure single-threaded access anyway. */
   _clientList.clear();

   // mServerSocketMap entries are removed rather than destroyed because
   // the keys are owned by mServerPortMap and the values are OsSocket's
   // owned by the SipServerBroker's in mServers.
   mServerSocketMap.removeAll();
   // Delete all the server SipClient's.
   mServers.destroyAll();
   mServerPortMap.destroyAll();
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

void SipProtocolServerBase::handleSend(SipMessage* pMsg)
{
  std::string hostAddress;
  std::string hostPortString;
  int hostPort = SIP_PORT;
  
  pMsg->getProperty("send-address", hostAddress);
  if (pMsg->getProperty("send-port", hostPortString))
  {
    try
    {
      hostPort = boost::lexical_cast<int>(hostPortString);
    }
    catch(...)
    {
      hostPort = SIP_PORT;
    }
  }
  
  UtlString localIp(pMsg->getInterfaceIp());

  if (localIp.isNull())
  {
      localIp = mDefaultIp;
  }

  SipClient::Ptr client = getClientForDestination(hostAddress.c_str(), hostPort, localIp);
  if (client)
  {
     if (Os::Logger::instance().willLog(FAC_SIP, PRI_DEBUG))
     {
        UtlString clientNames;
        client->getClientNames(clientNames);
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipProtocolServerBase[%s]::send %s (%p), %s",
                      getName().data(), client->getName().data(), client.get(), clientNames.data());
     }

     pMsg->setProperty("disable-outbound-queue", "true");
     client->sendTo(*pMsg, hostAddress.c_str(), hostPort);
  }

  delete pMsg;
  
  if (ENABLE_THREAD_POOL && ENFORCE_MAX_CONCURRENT_THREADS)
    _threadPoolSem.set();
}

UtlBoolean SipProtocolServerBase::send(SipMessage* message,
                                       const char* hostAddress,
                                       int hostPort)
{
  //
  // clone the message.  we do not own the message pointer
  //
  if (!hostPort)
    hostPort = SIP_PORT;
  
  SipMessage* pMsg = new SipMessage(*message);
  std::string hostPortString = boost::lexical_cast<std::string>(hostPort);
  

  pMsg->setProperty("send-address", hostAddress);
  pMsg->setProperty("send-port", hostPortString);


  if (ENABLE_THREAD_POOL && ENFORCE_MAX_CONCURRENT_THREADS)
    _threadPoolSem.wait();
  
  if (ENABLE_THREAD_POOL)
  {
    return _threadPool.schedule(boost::bind(&SipProtocolServerBase::handleSend, this, _1), pMsg);
  }
  else
  {
    handleSend(pMsg);
    return TRUE;
  }
}

UtlBoolean SipProtocolServerBase::startListener()
{
    UtlHashMapIterator iter(mServerSocketMap);
    UtlString* pKey;
    while ((pKey = dynamic_cast <UtlString*> (iter())))
    {
        // Get the socket for this local address.
        OsSocket* pSocket = dynamic_cast <OsSocket*> (iter.value());

        // Look up the SipClient for pKey.
        SipClient* pServer =
           dynamic_cast <SipClient*> (mServers.findValue(pKey));
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

           if (pServer->isOk())
           {
              mServers.insertKeyAndValue(new UtlString(*pKey), pServer);
              pServer->start();
              UtlString localName;
              pSocket->getLocalHostIp(&localName);
              Os::Logger::instance().log(FAC_SIP, PRI_INFO,
                            "SipProtocolServerBase[%s]::startListener "
                            "started server %s for address %s protocol %s",
                            getName().data(), pServer->getName().data(),
                            localName.data(), mProtocolString.data());
           }
           else
           {
              UtlString localName;
              pSocket->getLocalHostIp(&localName);
              Os::Logger::instance().log(FAC_SIP, PRI_CRIT,
                            "SipProtocolServerBase[%s]::startListener "
                            "unable to start server for address %s protocol %s",
                            getName().data(),
                            localName.data(), mProtocolString.data());
           }
        }
    }
    return(TRUE);
}

int SipProtocolServerBase::isOk()
{
    UtlBoolean bRet = true;

    UtlHashMapIterator iterator(mServers);
    UtlString* pKey;
    int count = 0;

    // Iterate through all the SipClient's.
    while ((pKey = dynamic_cast <UtlString*> (iterator())))
    {
       // 'and' its status into the overall status.
       SipClient* pServer = dynamic_cast <SipClient*> (iterator.value());
       bRet = bRet && pServer->isOk();
       count++ ;
    }

    // We are not OK if any of the SipClients report problems or we don't
    // have a client (e.g. unable to bind on port)
    return bRet && (count > 0);
}

void SipProtocolServerBase::deleteClient(SipClient* sipClient)
{
  mutex_lock lock(_clientMutex);
  for (SipClientList::iterator iter = _clientList.begin(); iter != _clientList.end(); iter++)
  {
    SipClient::Ptr pClient = *iter;
    if (pClient.get() == sipClient)
    {
      _clientList.erase(iter);
    }
  }
}

void SipProtocolServerBase::removeOldClients(long oldTime)
{
  mutex_lock lock(_clientMutex);
  for (SipClientList::iterator iter = _clientList.begin(); iter != _clientList.end();)
  {
    SipClient::Ptr pClient = *iter;
    
    if (!pClient->isOk() // socket is bad or task has stopped
             || pClient->getLastTouchedTime() < oldTime // idle for long enough
       )
    {
      UtlString clientNames;
      pClient->getClientNames(clientNames);
      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                    "SipProtocolServerBase[%s]::removeOldClients Removing old client %s(%p): %s",
                    getName().data(), pClient->getName().data(),
                    pClient.get(), clientNames.data());
      
      _clientList.erase(iter++);
    }
    else
    {
      ++iter;
    }
  }
}

void SipProtocolServerBase::startClients()
{
  mutex_lock lock(_clientMutex);
  for (SipClientList::iterator iter = _clientList.begin(); iter != _clientList.end(); iter++)
  {
    (*iter)->start();
  }
}

void SipProtocolServerBase::shutdownClients()
{
  mutex_lock lock(_clientMutex);
  // For each client, request shutdown.
  for (SipClientList::iterator iter = _clientList.begin(); iter != _clientList.end(); iter++)
  {
    (*iter)->requestShutdown();
  }
}

/* ============================ ACCESSORS ================================= */
int SipProtocolServerBase::getClientCount()
{
  mutex_lock lock(_clientMutex);
  return _clientList.size();
}

void SipProtocolServerBase::addClient(const SipClient::Ptr& pClient)
{
  mutex_lock lock(_clientMutex);
  if (pClient)
  {
    _clientList.push_back(pClient);
  }
}

void SipProtocolServerBase::printStatus()
{
   mutex_lock lock(_clientMutex);
   int numClients = _clientList.size();

   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipProtocolServerBase[%s]::printStatus %d clients in list at time %ld",
                 getName().data(), numClients, OsDateTime::getSecsSinceEpoch());


   for (SipClientList::iterator iter = _clientList.begin(); iter != _clientList.end(); iter++)
   {
      SipClient::Ptr pClient = *iter;
      long clientTouchedTime = pClient->getLastTouchedTime();
      bool clientOk = pClient->isOk();
      UtlString clientNames;
      pClient->getClientNames(clientNames);

      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                    "SipProtocolServerBase[%s]::printStatus client %s %p last used: %ld, ok: %d, names: %s",
                    getName().data(),
                    pClient->getName().data(), pClient.get(),
                    clientTouchedTime, clientOk, clientNames.data());
   }
}

SipClient::Ptr SipProtocolServerBase::findExistingClientForDestination(const UtlString& hostAddress, int hostPort, const UtlString& localIp)
{
  mutex_lock lock(_clientMutex);
  SipClient::Ptr pClient;
  for (SipClientList::iterator iter = _clientList.begin(); iter != _clientList.end(); iter++)
  {
    pClient = *iter;
    if (pClient->isAcceptableForDestination(hostAddress, hostPort, localIp))
      break;
  }   
  return pClient;
}

SipClient::Ptr SipProtocolServerBase::getClientForDestination(const UtlString& hostAddress, int hostPort, const UtlString& localIp)
{

  UtlString remoteHostAddr;
  SipClient::Ptr pClient;
  UtlBoolean clientStarted = FALSE;

  pClient = findExistingClientForDestination(hostAddress, hostPort, localIp);
  
  if (!pClient)
  {
    if(!portIsValid(hostPort))
    {
       hostPort = mDefaultPort;
    }
    
    OsTime time;
    OsDateTime::getCurTimeSinceBoot(time);
    long beforeSecs = time.seconds();

    bool isClientSocketReused;
    OsSocket* clientSocket = buildClientSocket(hostPort, hostAddress, localIp, isClientSocketReused);
    
    if (clientSocket && clientSocket->isOk())
    {
      OsDateTime::getCurTimeSinceBoot(time);
      long afterSecs = time.seconds();
      if(afterSecs - beforeSecs > 1)
      {
         Os::Logger::instance().log(FAC_SIP, PRI_WARNING, "SIP %s socket create for '%s':%d local '%s' took %d seconds",
                       mProtocolString.data(), hostAddress.data(), hostPort, localIp.data(),
                       (int)(afterSecs - beforeSecs));
      }

      pClient =
         strcmp(mProtocolString, SIP_TRANSPORT_UDP) == 0 ?
         SipClient::Ptr(new SipClientUdp(clientSocket, this, mSipUserAgent, isClientSocketReused)) :
         strcmp(mProtocolString, SIP_TRANSPORT_TCP) == 0 ?
         SipClient::Ptr(new SipClientTcp(clientSocket, this, mSipUserAgent, isClientSocketReused)):
         strcmp(mProtocolString, SIP_TRANSPORT_TLS) == 0 ?
         SipClient::Ptr(new SipClientTls(clientSocket, this, mSipUserAgent, isClientSocketReused)):
         SipClient::Ptr();
       
       // Check to see we could create the SipClient, specifically, that
      // creating the pipe succeeded.
      if (pClient && pClient->isOk())
      {
         // Start the client's thread.
         clientStarted = pClient->start();
         Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                       "SipProtocolServerBase[%s]::getClientForDestination client: %s(%p) '%s':%d local '%s'",
                       getName().data(), pClient->getName().data(),
                       pClient.get(), hostAddress.data(), hostPort, localIp.data());
         if (!clientStarted)
         {
            Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                          "SipProtocolServerBase[%s]::getClientForDestination start() failed",
                          getName().data());
            return SipClient::Ptr();
         }
      }
      else
      {
        //
        // If we havent created the client, socket will leak
        //
        if (!pClient && clientSocket)
        {
          delete clientSocket;
          clientSocket = 0;
        }
        
        Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                      "SipProtocolServerBase[%s]::getClientForDestination creating %s client failed",
                      getName().data(), mProtocolString.data());
        
        return SipClient::Ptr();
      }
    }
    else
    {
      Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                    "Unable to create %s socket for '%s':%d local '%s'",
                    mProtocolString.data(), hostAddress.data(), hostPort, localIp.data());
      return SipClient::Ptr();
    }
    
    if (pClient)
    {
      addClient(pClient);
    }
  }
  
  return pClient;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

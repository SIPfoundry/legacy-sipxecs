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
#include <net/SipClientTcp.h>
#include <net/SipTcpServer.h>
#include <net/SipUserAgent.h>
#include <os/OsDateTime.h>
#include <os/HostAdapterAddress.h>
#include <utl/UtlHashMapIterator.h>
#include <net/SipServerBroker.h>
#include <os/OsPtrMsg.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//#define TEST_PRINT
// Number of connection requests to allow in queue.
#define ACCEPT_QUEUE_SIZE 64

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipTcpServer::SipTcpServer(int port,
                           SipUserAgent* userAgent,
                           const char* protocolString, 
                           const char* taskName,
                           UtlBoolean bUseNextAvailablePort,
                           const char* szBindAddr) :
    SipProtocolServerBase(userAgent,
                          protocolString,
                          taskName)
{   
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipTcpServer[%s]::_  '%s' %s port %d szBindAddr = '%s'",
                 getName().data(), mName.data(),
                 bUseNextAvailablePort ? "use next available" : "specific",
                 port, szBindAddr);

   mServerPort = port;
   mpServerBrokerListener = new SipServerBrokerListener(this);

   if (szBindAddr && 0 != strcmp(szBindAddr, "0.0.0.0"))
   {
      mDefaultIp = szBindAddr;
      createServerSocket(szBindAddr, mServerPort, bUseNextAvailablePort);
   }
   else
   {
      int numAddresses = 0;
      const HostAdapterAddress* adapterAddresses[MAX_IP_ADDRESSES];
      getAllLocalHostIps(adapterAddresses, numAddresses);

      for (int i = 0; i < numAddresses; i++)
      {
         createServerSocket(adapterAddresses[i]->mAddress.data(),
                            mServerPort,
                            bUseNextAvailablePort);
         if (0 == i)
         {
            // use the first IP address in the array
            // for the 'default ip'
            mDefaultIp = adapterAddresses[i]->mAddress.data();
         }
         delete adapterAddresses[i];
      }
   }
}

UtlBoolean SipTcpServer::startListener()
{
    UtlBoolean bRet(FALSE);
#   ifdef TEST_PRINT
    osPrintf("SIP Server binding to port %d\n", mServerPort);
#   endif

    // iterate over the SipServerBroker map and call start
    UtlHashMapIterator iterator(mServerBrokers);
    UtlVoidPtr* pBrokerContainer = NULL;
    SipServerBroker* pBroker = NULL;
    UtlString* pKey = NULL;
    
    while((pKey = (UtlString*)iterator()))
    {
        pBrokerContainer = (UtlVoidPtr*) iterator.value();
        if (pBrokerContainer)
        {
            pBroker = (SipServerBroker*)pBrokerContainer->getValue();
            if (pBroker)
            {
                pBroker->start();
                bRet = TRUE;
            }
        }
    }
    return bRet;
}

void SipTcpServer::createServerSocket(const char* szBindAddr,
                                      int& port,
                                      const UtlBoolean& bUseNextAvailablePort)
{
   // port == PORT_NONE can be used to suppress creating sockets.
   // This is a hack, but is needed to support SipTlsServer.
   // :TODO: Clean this up.
   if (port != PORT_NONE)
   {
      // Create the socket.
      OsServerSocket* pSocket =
         new OsServerSocket(ACCEPT_QUEUE_SIZE, port, szBindAddr);

      // If the socket is busy or unbindable and the user requested using the
      // next available port, try the next SIP_MAX_PORT_RANGE ports.
      if (bUseNextAvailablePort)
      {
         for (int i=1; !pSocket->isOk() && i<=SIP_MAX_PORT_RANGE; i++)
         {
            delete pSocket;
            pSocket = new OsServerSocket(ACCEPT_QUEUE_SIZE, port+i, szBindAddr);
         }
      }

      // If we opened the socket.
      if (pSocket->isOk())
      {
         // Inform the SipUserAgent of the contact address.
         if (mSipUserAgent)
         {
            port = pSocket->getLocalHostPort();
            CONTACT_ADDRESS contact;
            strcpy(contact.cIpAddress, szBindAddr);
            contact.iPort = port;
            contact.eContactType = LOCAL;
            char szAdapterName[16];
            memset((void*)szAdapterName, 0, sizeof(szAdapterName)); // null out the string

            getContactAdapterName(szAdapterName, contact.cIpAddress);

            strcpy(contact.cInterface, szAdapterName);
            mSipUserAgent->addContactAddress(contact);
         }

         // add address and port to the maps
         mServerSocketMap.insertKeyAndValue(new UtlString(szBindAddr),
                                            new UtlVoidPtr((void*)pSocket));
         mServerPortMap.insertKeyAndValue(new UtlString(szBindAddr),
                                          new UtlInt(pSocket->getLocalHostPort()));
         mServerBrokers.insertKeyAndValue(new UtlString(szBindAddr),
                                          new UtlVoidPtr(new SipServerBroker((OsServerTask*)mpServerBrokerListener,
                                                                             pSocket)));                                                   
      }
   }
}

// Destructor
SipTcpServer::~SipTcpServer()
{
    if (mpServerBrokerListener)
    {
        mpServerBrokerListener->requestShutdown();
        delete mpServerBrokerListener;
    }
    waitUntilShutDown();
    {
        SipServerBroker* pBroker = NULL;
        UtlHashMapIterator iterator(mServerBrokers);
        UtlVoidPtr* pBrokerContainer = NULL;
        UtlString* pKey = NULL;
        
        while ((pKey = (UtlString*)iterator()))
        {
            pBrokerContainer = (UtlVoidPtr*)iterator.value();
            if (pBrokerContainer)
            {
                pBroker = (SipServerBroker*)pBrokerContainer->getValue();
                if (pBroker)
                {
                    delete pBroker;
                }
            }
        }
        mServerBrokers.destroyAll();
    }
}

/* ============================ MANIPULATORS ============================== */

int SipTcpServer::run(void* runArgument)
{
    while (!isShuttingDown())
    {
        OsTask::delay(500); // this method really shouldn't do anything
    }

    return (0);
}

void SipTcpServer::shutdownListener()
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipTcpServer[%s]::shutdownListener - before requestShutDown",
                 mName.data());

    requestShutdown();

    {
       OsLock lock(mClientLock);

       shutdownClients();
    }
}


OsSocket* SipTcpServer::buildClientSocket(int hostPort,
                                          const char* hostAddress,
                                          const char* localIp)
{
    // Create a socket in non-blocking mode while connecting
    OsConnectionSocket* socket =
       new OsConnectionSocket(hostPort, hostAddress, FALSE, localIp);
    socket->makeBlocking();
    return(socket);
}

/* ============================ ACCESSORS ================================= */

// The local server port for this server
int SipTcpServer::getServerPort() const 
{
    return mServerPort;
}

UtlBoolean SipTcpServer::SipServerBrokerListener::handleMessage(OsMsg& eventMessage)
{
    UtlBoolean bRet(FALSE);
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();
    OsPtrMsg *pPtrMsg = NULL;
    
    
    if (msgType == OsMsg::OS_EVENT)
    {
        // if we are receiving this message, an accept has occurred,
        // and the socket created by the accept is being sent to us in
        // this message
        if (msgSubType == SIP_SERVER_BROKER_NOTIFY)
        {
            // unpackage the client socket
            pPtrMsg = dynamic_cast<OsPtrMsg*>(&eventMessage);
            assert(pPtrMsg);
            
            OsConnectionSocket* clientSocket = reinterpret_cast<OsConnectionSocket*>(pPtrMsg->getPtr());
            assert(clientSocket);
            
            SipClient* client = 
               new SipClientTcp(clientSocket, mpOwner, mpOwner->mSipUserAgent);

            UtlString hostAddress;
            int hostPort;
            clientSocket->getRemoteHostIp(&hostAddress, &hostPort);

            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipTcpServer[%s]::run client: %p %s:%d",
                          getName().data(),
                          client, hostAddress.data(), hostPort);

            UtlBoolean clientStarted = client->start();
            if(!clientStarted)
            {
                OsSysLog::add(FAC_SIP, PRI_ERR,
                              "SIP %s Client failed to start",
                              mpOwner->mProtocolString.data());
            }
            {
               OsLock lock(mpOwner->mClientLock);
               mpOwner->addClient(client);
            }
            bRet = TRUE;
        }
        else
        {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "SIP %s Client received spurious message",
                          mpOwner->mProtocolString.data());
        }
    }
    
    return bRet;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

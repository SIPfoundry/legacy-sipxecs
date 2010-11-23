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
#include <net/SipClientTls.h>
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
                           const char* taskName,
                           UtlBoolean bUseNextAvailablePort,
                           const char* szBindAddr,
                           const char* protocolString) :
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

   // port == PORT_NONE can be used to suppress creating sockets.
   // This is a hack, but is needed to support SipTlsServer.
   // :TODO: Clean this up.
   if (port != PORT_NONE)
   {
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
}

UtlBoolean SipTcpServer::startListener()
{
    UtlBoolean bRet(FALSE);
#   ifdef TEST_PRINT
    osPrintf("SIP Server binding to port %d\n", mServerPort);
#   endif

    // Iterate over the SipServerBroker map and call start on each element.
    UtlHashMapIterator iterator(mServerBrokers);
    UtlString* pKey;

    while ((pKey = dynamic_cast <UtlString*> (iterator())))
    {
       (dynamic_cast <SipServerBroker*> (iterator.value()))->start();
       bRet = TRUE;
    }
    return bRet;
}

UtlBoolean SipTcpServer::createServerSocket(const char* szBindAddr,
                                            int& port,
                                            const UtlBoolean& bUseNextAvailablePort)
{
   UtlBoolean bSuccess = TRUE;

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
         ContactAddress contact;
         strcpy(contact.cIpAddress, szBindAddr);
         contact.iPort = port;
         contact.eContactType = ContactAddress::LOCAL;
         char szAdapterName[16];
         memset((void*)szAdapterName, 0, sizeof(szAdapterName)); // null out the string

         getContactAdapterName(szAdapterName, contact.cIpAddress);

         strcpy(contact.cInterface, szAdapterName);
         mSipUserAgent->addContactAddress(contact);
      }

      // Add address and port to the maps.
      UtlString* address = new UtlString(szBindAddr);
      mServerSocketMap.insertKeyAndValue(address, pSocket);
      mServerPortMap.insertKeyAndValue(address,
                                       new UtlInt(pSocket->getLocalHostPort()));
      // pSocket is owned by the SipServerBroker.
      mServerBrokers.insertKeyAndValue(new UtlString(szBindAddr),
                                       new SipServerBroker(mpServerBrokerListener,
                                                           pSocket));
   }
   else
   {
      bSuccess = false;
   }

   return bSuccess;
}

// Destructor
SipTcpServer::~SipTcpServer()
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipTcpServer[%s]::~ called",
                  mName.data());

    if (mpServerBrokerListener)
    {
        mpServerBrokerListener->requestShutdown();
        delete mpServerBrokerListener;
    }
    waitUntilShutDown();

    // This deletes the SipServerBrokers, as they are now the values
    // of mServerBrokers.
    mServerBrokers.destroyAll();
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
                                          const char* localIp,
                                          bool& existingSocketReused)
{
    // Create a socket in non-blocking mode while connecting
    OsConnectionSocket* socket =
       new OsConnectionSocket(hostPort, hostAddress, FALSE, localIp, 0);
    socket->makeBlocking();
    existingSocketReused = false;
    return(socket);
}

/* ============================ ACCESSORS ================================= */

// The local server port for this server
int SipTcpServer::getServerPort() const
{
    return mServerPort;
}

int SipTcpServer::isOk()
{
    UtlBoolean bRet = true;
    int count = 0 ;

    UtlHashMapIterator iterator(mServerBrokers);
    UtlString* pKey;

    while ((pKey = dynamic_cast <UtlString*> (iterator())))
    {
       count++;
       SipServerBroker* pBroker = dynamic_cast<SipServerBroker*> (iterator.value());
       if (pBroker)
       {
          bRet = bRet && pBroker->isOk() ;
       }
       else
       {
          assert(false); // List ALWAYS contains brokers
          bRet = false;
       }
    }

    // We are not OK if any of the brokers report problems or we don't
    // have any brokers (e.g. unable to bind on port)
    return bRet && (count > 0);
}

UtlBoolean SipTcpServer::SipServerBrokerListener::handleMessage(OsMsg& eventMessage)
{
    UtlBoolean bRet(FALSE);
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();
    OsPtrMsg *pPtrMsg = NULL;

    if (msgType == OsMsg::OS_EVENT)
    {
        // If we are receiving this message, an accept has occurred,
        // and the socket created by the accept (that is, a pointer to
        // the OsConnectionSocket) is being sent to us in this
        // message.
        if (msgSubType == SIP_SERVER_BROKER_NOTIFY)
        {
            // unpackage the client socket
            pPtrMsg = dynamic_cast<OsPtrMsg*>(&eventMessage);
            assert(pPtrMsg);

            OsConnectionSocket* clientSocket = reinterpret_cast<OsConnectionSocket*>(pPtrMsg->getPtr());
            assert(clientSocket);

            SipClient* client = 0;

            if (!mpOwner->mIsSecureTransport)
                client = new SipClientTcp(clientSocket, mpOwner, mpOwner->mSipUserAgent);
            else
                client = new SipClientTls(clientSocket, mpOwner, mpOwner->mSipUserAgent);

            UtlString hostAddress;
            int hostPort;
            clientSocket->getRemoteHostIp(&hostAddress, &hostPort);

            if (client->isOk())
            {
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipTcpServer[%s]::run client created for incoming connection: %s (%p) %s:%d",
                             getName().data(),
                             client->getName().data(), client,
                             hostAddress.data(), hostPort);

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
            }
            else
            {
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "SipTcpServer[%s]::run failed to create client for incoming connection: %s:%d",
                             getName().data(),
                             hostAddress.data(), hostPort);
               // Note that destroying 'client' also destroys 'clientSocket'
               // because clientSocket is unshared and so client owns
               // clientSocket.
               delete client;
            }

            // Tell our caller that we have handled this message.
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

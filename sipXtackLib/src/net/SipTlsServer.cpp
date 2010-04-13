//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Copyright (C) 2009 Nortel Networks, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// Skip all this code if SIP_TLS (TLS support for SIP) is not defined.
// Properly, we should delete this file from the Makefile, but that would
// take more investigation than I am willing to put in right now.
#ifdef SIP_TLS

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <net/SipServerBroker.h>
#include <net/SipTlsServer.h>
#include <net/SipUserAgent.h>
#include <os/OsDateTime.h>
#include <os/HostAdapterAddress.h>
#include <os/OsSSLServerSocket.h>
#include <os/OsSSLConnectionSocket.h>

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
SipTlsServer::SipTlsServer(int port,
                           SipUserAgent* userAgent,
                           UtlBoolean bUseNextAvailablePort,
                           const char* szBindAddr) :
   // Use PORT_NONE here to prevent SipTcpServer from opening a socket.
   SipTcpServer(PORT_NONE,
                userAgent,
                "SipTlsServer-%d",
                FALSE,
                NULL,
                SIP_TRANSPORT_TLS)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipTlsServer[%s]::_  '%s' %s port %d szBindAddr = '%s'",
                 getName().data(), mName.data(),
                 bUseNextAvailablePort ? "use next available" : "specific",
                 port, szBindAddr);

   // mServerPort is set to PORT_NONE by SipTcpServer::SipTcpServer.
   mServerPort = port;
   mpServerBrokerListener = new SipServerBrokerListener(this);

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipTlsServer[%s]::_ port %d",
                 getName().data(), mServerPort);

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

   // Correct mDefaultPort, which is set to SIP_PORT in SipProtocolServerBase.
   mDefaultPort = SIP_TLS_PORT;
}

UtlBoolean SipTlsServer::createServerSocket(const char* szBindAddr,
                                            int& port,
                                            const UtlBoolean& bUseNextAvailablePort)
{
   UtlBoolean bSuccess = TRUE;

   OsServerSocket* pSocket =
      new OsSSLServerSocket(ACCEPT_QUEUE_SIZE, port, szBindAddr);

   // If the socket is busy or unbindable and the user requested using the
   // next available port, try the next SIP_MAX_PORT_RANGE ports.
   if (bUseNextAvailablePort)
   {
      for (int i=1; !pSocket->isOk() && i<=SIP_MAX_PORT_RANGE; i++)
      {
         delete pSocket;
         pSocket = new OsSSLServerSocket(ACCEPT_QUEUE_SIZE, port+i, szBindAddr);
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
SipTlsServer::~SipTlsServer()
{
   // The thread will be shut down by the base class destructor.
}

/* ============================ MANIPULATORS ============================== */

OsSocket* SipTlsServer::buildClientSocket(int hostPort,
                                          const char* hostAddress,
                                          const char* localIp,
                                          bool& existingSocketReused)
{
   OsSocket* socket;
   socket = new OsSSLConnectionSocket(hostPort, hostAddress);

   socket->makeBlocking();
   existingSocketReused = false;
   return(socket);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

#endif // SIP_TLS

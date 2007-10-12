//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
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
#include <net/SipTlsServer.h>
#include <net/SipUserAgent.h>
#include <os/OsDateTime.h>
#include <os/OsSSLServerSocket.h>
#include <os/OsSSLConnectionSocket.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//#define TEST_PRINT
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipTlsServer::SipTlsServer(int port,
                           SipUserAgent* userAgent,
                           UtlBoolean bUseNextAvailablePort) :
   SipTcpServer(PORT_NONE,
                userAgent,
                SIP_TRANSPORT_TLS,
                "SipTlsServer-%d")
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipTlsServer[%s]::_ port = %d, bUseNextAvailablePort = %d",
                 getName().data(), port, bUseNextAvailablePort);

   mServerPort = PORT_NONE;
   
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipTlsServer[%s]::_ port %d",
                 getName().data(), port);

   if (portIsValid(port))
   {
      OsServerSocket* pServerSocket = new OsSSLServerSocket(64, port);

      // If the socket is busy or unbindable and the user requested using the
      // next available port, try the next SIP_MAX_PORT_RANGE ports.
      if (bUseNextAvailablePort && !pServerSocket->isOk())
      {
         for (int i=1; i<=SIP_MAX_PORT_RANGE; i++)
         {
            delete pServerSocket ;
            pServerSocket = new OsSSLServerSocket(64, port+i);
            if (pServerSocket->isOk())
            {
               break ;
            }
         }
      }

      if (pServerSocket->isOk())
      {
         mServerPort = pServerSocket->getLocalHostPort() ;
      }
   }

   mDefaultPort = SIP_TLS_PORT;
}

// Destructor
SipTlsServer::~SipTlsServer()
{
   // The thread will be shut down by the base class destructor.
}

/* ============================ MANIPULATORS ============================== */

OsSocket* SipTlsServer::buildClientSocket(int hostPort, const char* hostAddress)
{
   OsSocket* socket;
   socket = new OsSSLConnectionSocket(hostPort, hostAddress);

   socket->makeBlocking();
   return(socket);
}

/* ============================ ACCESSORS ================================= */

// The local server port for this server
int SipTlsServer::getServerPort() const 
{
    return mServerPort;
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

#endif // SIP_TLS

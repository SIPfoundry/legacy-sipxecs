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
#include <net/SipTlsServer.h>
#include <net/SipUserAgent.h>
#include <os/OsDateTime.h>
#ifdef SIP_TLS
#    include <os/OsSSLServerSocket.h>
#    include <os/OsSSLConnectionSocket.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//#define TEST_PRINT
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipTlsServer::SipTlsServer(int port, SipUserAgent* userAgent, UtlBoolean bUseNextAvailablePort) :
 SipTcpServer(PORT_NONE, userAgent, SIP_TRANSPORT_TLS, "SipTlsServer-%d")
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipTlsServer::_ port = %d, bUseNextAvailablePort = %d",
                  port, bUseNextAvailablePort);

    mServerPort = PORT_NONE ;
   
    OsSysLog::add(FAC_SIP,PRI_DEBUG,"SipTlsServer::~ port %d", port);

    if(portIsValid(port))
    {
#ifdef SIP_TLS
        OsServerSocket* pServerSocket = new OsSSLServerSocket(64, port);
#else
        OsServerSocket* pServerSocket = new OsServerSocket(64, port);
#endif

        // If the socket is busy or unbindable and the user requested using the
        // next available port, try the next SIP_MAX_PORT_RANGE ports.
        if (bUseNextAvailablePort && !pServerSocket->isOk())
        {
            for (int i=1; i<=SIP_MAX_PORT_RANGE; i++)
            {
                delete pServerSocket ;
#ifdef SIP_TLS
                pServerSocket = new OsSSLServerSocket(64, port+i);
#else
                pServerSocket = new OsServerSocket(64, port+i);
#endif                
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
    waitUntilShutDown();

}

/* ============================ MANIPULATORS ============================== */

OsSocket* SipTlsServer::buildClientSocket(int hostPort, const char* hostAddress)
{
    OsSocket* socket;
#ifdef SIP_TLS
    socket = new OsSSLConnectionSocket(hostPort, hostAddress);
#else
    // Create the socket in non-blocking mode so it does not block
    // while conecting
    socket = new OsConnectionSocket(hostPort, hostAddress, FALSE);
#endif

   socket->makeBlocking();
   return(socket);
}

/* ============================ ACCESSORS ================================= */

// The the local server port for this server
int SipTlsServer::getServerPort() const 
{
    return mServerPort ;
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

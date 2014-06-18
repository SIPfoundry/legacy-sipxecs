//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdio.h>

#define OsSS_CONST
#if defined(_WIN32)
#   include <winsock.h>
#undef OsSS_CONST
#define OsSS_CONST const
#elif defined(_VXWORKS)
#   include <inetLib.h>
#   include <sockLib.h>
#   include <unistd.h>
#elif defined(__pingtel_on_posix__)
#   include <netinet/in.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netdb.h>
#   include <unistd.h>
#   include <resolv.h>
#else
#error Unsupported target platform.
#endif

// APPLICATION INCLUDES
#include "os/OsSSL.h"
#include "os/OsSSLServerSocket.h"
#include "os/OsDefs.h"
#include "os/OsLogger.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsSSLServerSocket::OsSSLServerSocket(int connectionQueueSize, int serverPort,
      const char* szBindAddr)
   : OsServerSocket(connectionQueueSize,serverPort,szBindAddr),
      mVerifyPeer(true)
{
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsSSLServerSocket::_ %p", this );
}

// Destructor
OsSSLServerSocket::~OsSSLServerSocket()
{
  close();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsSSLServerSocket&
OsSSLServerSocket::operator=(const OsSSLServerSocket& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

OsConnectionSocket* OsSSLServerSocket::accept()
{
   OsSSLConnectionSocket* newSocket = NULL;

   if (socketDescriptor == OS_INVALID_SOCKET_DESCRIPTOR)
   {
      Os::Logger::instance().log(FAC_KERNEL, PRI_ERR
                    , "OsSSLServerSocket: accept exiting because socketDescriptor is %d"
                    ,socketDescriptor);
   }
   else
   {
      /* Block while waiting for a client to connect. */
      struct sockaddr_in clientSocketAddr;
      socklen_t clientAddrLength = sizeof clientSocketAddr;
      int clientSocket = ::accept(socketDescriptor,
                                  (struct sockaddr*) &clientSocketAddr,
                                  &clientAddrLength);

      if (clientSocket < 0)
      {
         int error = OsSocketGetERRNO();
         if (0 != error)
         {
            Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                          "OsSSLServerSocket: accept call failed with error: %d=%x",
                          error, error);
            socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
         }
      }
      else
      {
         Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                       "OsSSLServerSocket::accept socket accepted: %d",
                       clientSocket);

         // TODO: allow this to be a non-shared context...
         SSL* pSSL = OsSharedSSL::get()->getServerConnection();
         if (pSSL)
         {
            SSL_set_fd (pSSL, clientSocket);

            newSocket = new OsSSLConnectionSocket(pSSL, clientSocket, mLocalIp);
            if (newSocket)
            {
               int result = SSL_accept(pSSL);
               if (1 == result)
               {
                  OsSSL::logConnectParams(FAC_KERNEL, PRI_DEBUG
                                          ,"OsSSLServerSocket::accept"
                                          ,pSSL);

                  Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                                "OsSSLServerSocket::accept connection %p",
                                this
                                );
                  // test and cache the peer identity
                  if (mVerifyPeer)
                    newSocket->peerIdentity(NULL, NULL);
               }
               else
               {
                  OsSSL::logError(FAC_KERNEL, PRI_ERR,
                                  (  result == 0
                                   ? "OsSSLServerSocket SSL_accept - incompatible client?"
                                   : "OsSSLServerSocket SSL_accept SSL handshake error"
                                   ),
                                  SSL_get_error(pSSL, result));

                  // SSL failed, so clear this out.
                  delete newSocket;
                  newSocket = NULL;
               }
            }
            else
            {
               Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                             "OsSSLServerSocket::accept - new OsSSLConnectionSocket failed"
                             );
            }
         }
         else
         {
            ::close (clientSocket);
            Os::Logger::instance().log(FAC_KERNEL, PRI_ERR
                          , "OsSSLConnectionSocket::accept - Error creating new SSL connection.");
         }
      }
   }

   return(newSocket);
}

void OsSSLServerSocket::close()
{

   if(socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR)
   {
      OsServerSocket::close();
   }
}

/* ============================ ACCESSORS ================================= */

int OsSSLServerSocket::getLocalHostPort() const
{
   return(localHostPort);
}

/* ============================ INQUIRY =================================== */

OsSocket::IpProtocolSocketType OsSSLServerSocket::getIpProtocol() const
{
    return(OsSocket::SSL_SOCKET);
}

UtlBoolean OsSSLServerSocket::isOk() const
{
    return(socketDescriptor != OS_INVALID_SOCKET_DESCRIPTOR);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

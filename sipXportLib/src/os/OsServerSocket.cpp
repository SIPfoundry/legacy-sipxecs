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
#include <stdio.h>
#include <poll.h>

#if defined(_WIN32)
#   include <winsock.h>
#undef OsSS_CONST
#define OsSS_CONST const
#elif defined(_VXWORKS)
#   include <inetLib.h>
#   include <sockLib.h>
#   include <unistd.h>
#elif defined(__pingtel_on_posix__)
#undef OsSS_CONST
#define OsSS_CONST const
#   include <netinet/in.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netdb.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <unistd.h>
#   include <resolv.h>
#else
#error Unsupported target platform.
#endif

// APPLICATION INCLUDES
#include <os/OsServerSocket.h>
#include "os/OsLogger.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

#ifdef __pingtel_on_posix__
typedef socklen_t SocketLenType;
#else
typedef int SocketLenType;
#endif

// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType OsServerSocket::TYPE = "OsServerSocket";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsServerSocket::OsServerSocket(int connectionQueueSize,
    int serverPort,
    const char* szBindAddr)
{
   const int one = 1;
   int error = 0;
   socketDescriptor = 0;
   struct sockaddr_in localAddr;
   SocketLenType addrSize;

   // Windows specific startup
   if(!OsSocket::socketInit())
   {
      goto EXIT;
   }

   localHostPort = serverPort;

   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                 "OsServerSocket::_ queue=%d port=%d bindaddr=%s",
                 connectionQueueSize, serverPort, szBindAddr
                 );

   // Create the socket
   socketDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(socketDescriptor == OS_INVALID_SOCKET_DESCRIPTOR)
   {
      error = OsSocketGetERRNO();
      Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                    "OsServerSocket: socket call failed with error: %d=0x%x",
                    error, error);
      socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
      goto EXIT;
   }

#ifndef WIN32
   if(setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)))
      Os::Logger::instance().log(FAC_KERNEL, PRI_ERR, "OsServerSocket: setsockopt(SO_REUSEADDR) failed!");
#endif
/*
    Don't know why we don't want to route...we do support subnets, do we not?
    setsockopt(socketDescriptor, SOL_SOCKET, SO_DONTROUTE, (char *)&one, sizeof(one)) ;
*/

#  if defined(__MACH__)
   // Under OS X, we use SO_NOSIGPIPE here because MSG_NOSIGNAL
   // is not supported for the write() call.
   if(setsockopt(socketDescriptor, SOL_SOCKET, SO_NOSIGPIPE, (char *)&one, sizeof(one)))
   {
      error = OsSocketGetERRNO();
      close();
      Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                    "setsockopt call failed with error: 0x%x in OsServerSocket::OsServerSocket",
                    error);
      goto EXIT;
   }
#       endif



   localAddr.sin_family = AF_INET;

   // Bind to a specific server port if given, or let the system pick
   // any available port number if PORT_DEFAULT.
   localAddr.sin_port = htons((PORT_DEFAULT == serverPort) ? 0 : serverPort);

   // Allow IP in on any of this host's addresses or NICs.
   if (szBindAddr)
   {
      localAddr.sin_addr.s_addr = inet_addr (szBindAddr);
      mLocalIp = szBindAddr;
   }
   else
   {
      localAddr.sin_addr.s_addr=OsSocket::getDefaultBindAddress();
      mLocalIp = inet_ntoa(localAddr.sin_addr);
   }
//   localAddr.sin_addr.s_addr=htonl(INADDR_ANY); // Allow IP in on

   error = bind(socketDescriptor,
                (OsSS_CONST struct sockaddr*) &localAddr,
                sizeof(localAddr));
   if (error == OS_INVALID_SOCKET_DESCRIPTOR)
   {
      error = OsSocketGetERRNO();
      Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                    "OsServerSocket:  bind to port %s:%d failed with error: %d = 0x%x",
                    inet_ntoa(localAddr.sin_addr),
                    ((PORT_DEFAULT == serverPort) ? 0 : serverPort), error, error);
      socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
      goto EXIT;
   }

   addrSize = sizeof (struct sockaddr_in);
   error = getsockname(socketDescriptor,
                       (struct sockaddr*) &localAddr, &addrSize);
   if (error) {
      error = OsSocketGetERRNO();
      Os::Logger::instance().log(FAC_KERNEL, PRI_ERR, "OsServerSocket: getsockname call failed with error: %d=0x%x",
         error, error);
   } else {
      localHostPort = htons(localAddr.sin_port);
   }

   // Setup the queue for connection requests
   error = listen(socketDescriptor,  connectionQueueSize);
   if (error)
   {
      error = OsSocketGetERRNO();
      Os::Logger::instance().log(FAC_KERNEL, PRI_ERR, "OsServerSocket: listen call failed with error: %d=0x%x",
         error, error);
      socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
   }

EXIT:
   ;

}

// Destructor
OsServerSocket::~OsServerSocket()
{
   close();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsServerSocket&
OsServerSocket::operator=(const OsServerSocket& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

OsConnectionSocket* OsServerSocket::accept()
{
   OsConnectionSocket* connectSock = NULL;

   /* Block while waiting for a client to connect. */
   struct sockaddr_in clientSocketAddr;
   SocketLenType clientAddrLength = sizeof (clientSocketAddr);
   // Remember the value of socketDescriptor, which may be changed by other
   // methods (particularly in a race with ::close()).
   int s = socketDescriptor;
   int clientSocket = ::accept(s,
                               (struct sockaddr*) &clientSocketAddr,
                               &clientAddrLength);
   if (clientSocket < 0)
   {
      int error = OsSocketGetERRNO();
      Os::Logger::instance().log(FAC_KERNEL, PRI_ERR, "OsServerSocket: accept(%d) error: %d=%s",
                    s, error, strerror(error));
      // Flag the socket as invalid.
      socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
      // Close the socket, since we are losing record that we have it open.
      ::close(s);
   }
   else
   {
      connectSock = new OsConnectionSocket(mLocalIp,clientSocket);
   }

   return(connectSock);
}

OsConnectionSocket* OsServerSocket::accept(long waitMilliseconds)
{
    OsConnectionSocket* pRC = NULL ;
    int error=0;
    struct pollfd pset[1];

    // Wait for some data to be available
    pset[0].fd      = socketDescriptor;
    pset[0].events  = POLLIN;
    pset[0].revents = 0;
    int pollResult  = poll(pset, 1, waitMilliseconds); // returns # of events
    if (1 == pollResult)
    {
        if (pset[0].revents & POLLERR)
        {
            // Socket is perhaps dead?
            error = OsSocketGetERRNO();
        }
        else if (pset[0].revents & POLLIN)
        {
            // Data is available; invoke accept
            error = 0;
            pRC = accept();
            if (pRC == NULL)
                error = OsSocketGetERRNO() ;
        }
    }
    else if (0 == pollResult)
    {
        // Timed out waiting for connection
        error = ETIMEDOUT;
    }
    else
    {
        // Some other error -- socket dead?
        error = OsSocketGetERRNO();
    }

    if (pRC == NULL)
    {
        Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsServerSocket: accept(%d, %ld ms) error: %d=%s",
                        socketDescriptor, waitMilliseconds, error, strerror(error));
    }

    return pRC ;
}



void OsServerSocket::close()
{
   if (socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR)
   {
      // Save socketDescriptor because other methods on this object
      // (especially ::accept()) may change it before we are done.
      int s = socketDescriptor;

#if defined(_WIN32)
      closesocket(s);
#elif defined(_VXWORKS) || defined(__pingtel_on_posix__)
      // Call shutdown first to unblock blocking calls on Linux
      ::shutdown(s, 2);
      ::close(s);
#else
#error Unsupported target platform.
#endif
       socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
   }
}

/* ============================ ACCESSORS ================================= */

int OsServerSocket::getLocalHostPort() const
{
   return(localHostPort);
}

/* ============================ INQUIRY =================================== */

UtlBoolean OsServerSocket::isOk() const
{
    return(socketDescriptor != OS_INVALID_SOCKET_DESCRIPTOR);
}

UtlContainableType OsServerSocket::getContainableType() const
{
   return OsServerSocket::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

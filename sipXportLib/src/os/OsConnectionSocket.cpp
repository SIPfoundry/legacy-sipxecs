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

#if defined(_WIN32)
#   include <winsock.h>
#elif defined(_VXWORKS)
#   include <inetLib.h>
#   include <netdb.h>
#   include <resolvLib.h>
#   include <sockLib.h>
#elif defined(__pingtel_on_posix__)
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netdb.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#else
#error Unsupported target platform.
#endif

// APPLICATION INCLUDES
#include <os/OsConnectionSocket.h>
#include "os/OsUtil.h"
#include "utl/UtlSList.h"
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsConnectionSocket::OsConnectionSocket(int connectedSocketDescriptor)
{
        socketDescriptor = connectedSocketDescriptor;
}

OsConnectionSocket::OsConnectionSocket(const char* szLocalIp, int connectedSocketDescriptor)
{
        socketDescriptor = connectedSocketDescriptor;
        mLocalIp = szLocalIp;
}

// Constructor
OsConnectionSocket::OsConnectionSocket(int serverPort,
                                       const char* serverName,
                                       UtlBoolean blockingConnect,
                                       const char* localIp)
{
   int error = 0;
   UtlBoolean isIp;
   struct in_addr* serverAddr;
   struct hostent* server = NULL;
   struct sockaddr_in serverSockAddr;
   UtlString temp_output_address;

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "OsConnectionSocket::_ attempt %s:%d %s"
                 ,serverName, serverPort, blockingConnect ? "BLOCKING" : "NON-BLOCKING" );

   socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;

   remoteHostPort = serverPort;

   // Connect to a remote host if given
   if(! serverName || strlen(serverName) == 0)
   {
#if defined(_VXWORKS)
      serverName = "127.0.0.1";
#elif defined(__pingtel_on_posix__)
    unsigned long address_val = OsSocket::getDefaultBindAddress();
      
    if (!localIp)
    {
        if (address_val == htonl(INADDR_ANY))
            serverName = "localhost";
        else
        {
            struct in_addr in;
            in.s_addr = address_val;

            serverName = inet_ntoa(in);
        }
    }
    else
    {
        mLocalIp = localIp;
        serverName = localIp;
    }

#elif defined(WIN32)
      unsigned long address_val = OsSocket::getDefaultBindAddress();
      
    if (!localIp)
    {
        if (address_val == htonl(INADDR_ANY))
            serverName = "localhost";
        else
        {
            struct in_addr in;
            in.S_un.S_addr = address_val;

            serverName = inet_ntoa(in);
        }
    }
    else
    {
        mLocalIp = localIp;
        serverName = localIp;
    }
      
#else
#error Unsupported target platform.
#endif

   }
   if(serverName)
   {
      remoteHostName.append(serverName);
   }

   if (localIp)
   {
        mLocalIp = localIp;
   }

   if(!socketInit())
   {
      goto EXIT;
   }

#       if defined(_VXWORKS)
   char hostentBuf[512];
#       endif

   // Create the socket
   socketDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(socketDescriptor == OS_INVALID_SOCKET_DESCRIPTOR)
   {
      error = OsSocketGetERRNO();
      socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
      OsSysLog::add(FAC_SIP, PRI_ERR, "OsConnectionSocket::_ 'socket' failed: %x", error);
      goto EXIT;
   }

   if(!blockingConnect)
   {
      makeNonblocking();
   }

   isIp = isIp4Address(serverName);
   if(!isIp)
   {
#       if defined(_WIN32) || defined(__pingtel_on_posix__)
      server = gethostbyname(serverName);
#       elif defined(_VXWORKS)
      server = resolvGetHostByName((char*) serverName,
                                   hostentBuf, sizeof(hostentBuf));
#       else
#       error Unsupported target platform.
#       endif //_VXWORKS
   }

   if(!isIp && !server)
   {
      close();
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "DNS failed to look up host: '%s'\n",
                    serverName);
      goto EXIT;
   }

   if (!isIp)
   {
      inet_ntoa_pt(*((in_addr*) (server->h_addr)),temp_output_address);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "OsConnectionSocket::_: connecting to host at: %s:%d\n",
                    temp_output_address.data(), serverPort);
      serverAddr = (in_addr*) (server->h_addr);
      serverSockAddr.sin_family = server->h_addrtype;
      serverSockAddr.sin_port = htons(serverPort);
      serverSockAddr.sin_addr.s_addr = (serverAddr->s_addr);
   }
   else
   {
      serverSockAddr.sin_family = AF_INET;
      serverSockAddr.sin_port = htons(serverPort);
      serverSockAddr.sin_addr.s_addr = inet_addr(serverName);
   }

   // Set the default destination address for the socket
   int connectReturn;
#       if defined(_WIN32) || defined(__pingtel_on_posix__)
   connectReturn = connect(socketDescriptor,
                           (const struct sockaddr*) &serverSockAddr,
                           sizeof(serverSockAddr));
#       elif defined(_VXWORKS)
   connectReturn = connect(socketDescriptor,
                           (struct sockaddr*) &serverSockAddr,
                           sizeof(serverSockAddr));
#       else
#       error Unsupported target platform.
#       endif

   error = OsSocketGetERRNO();

#if defined(_WIN32)
   if(error == WSAEWOULDBLOCK &&
      !blockingConnect)
   {
      error = 0;
      connectReturn = 0;
   }
#elif defined(_VXWORKS)
   if(error == EWOULDBLOCK &&
      !blockingConnect)
   {
      error = 0;
      connectReturn = 0;
   }
#elif defined(__pingtel_on_posix__)
   if(error == EINPROGRESS &&
      !blockingConnect)
   {
      error = 0;
      connectReturn = 0;
   }
#endif

   if(connectReturn && error)
   {
      char* msgBuf;
      close();

      msgBuf = strerror(error);
      OsSysLog::add(FAC_SIP, PRI_INFO, "OsConnectionSocket(%s:%d): call to connect() failed: %s\n"
                    "connect call failed with error: %d %d\n",
                    serverName, serverPort, msgBuf, error, connectReturn);
#if 0
      // Report exactly what the arguments to connect() were
      // to diagnose difficult errors.
      char buffer[100];
      int i;
      unsigned char* p;
      for (i = 0, p = (unsigned char*) &serverSockAddr;
           i < sizeof(serverSockAddr); i++)
      {
         sprintf(buffer + 2 * i, "%02X", *p++);
      }
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "OsConnectionSocket socketDescriptor = %d, "
                    "serverSockAddr = %s",
                    socketDescriptor, buffer);
#endif
   }
   else
   {
      mIsConnected = TRUE;
   }
        
  EXIT:
   return;
}

/// Is this connection encrypted using TLS/SSL?
bool OsConnectionSocket::isEncrypted() const
{
   return false;
}

   
/// Get any authenticated peer host names.
bool OsConnectionSocket::peerIdentity( UtlSList* altNames
                                      ,UtlString* commonName
                                      ) const
{
   /*
    * @returns
    * - true if the connection is TLS/SSL and the peer has presented
    *        a certificate signed by a trusted certificate authority
    * - false if not
    */
   if (altNames)
   {
      altNames->destroyAll();
   }
   if (commonName)
   {
      commonName->remove(0);
   }
   return false; // an OsSSLServerSocket might return true...
}


// Destructor
OsConnectionSocket::~OsConnectionSocket()
{
        remoteHostName = OsUtil::NULL_OS_STRING;
        close();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsConnectionSocket&
OsConnectionSocket::operator=(const OsConnectionSocket& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean OsConnectionSocket::reconnect()
{
        OsSysLog::add(FAC_SIP, PRI_WARNING, " reconnect NOT implemented!\n");
        return(FALSE);
}

// Because we have overided one read method, we
// must implement them all in OsConnectionSocket or
// we end up hiding some of the methods.
int OsConnectionSocket::read(char* buffer, int bufferLength)
{
    // Use base class implementation
    int bytesRead = OsSocket::read(buffer, bufferLength);
    return(bytesRead);
}

int OsConnectionSocket::read(char* buffer,
                             int bufferLength,
                             UtlString* ipAddress,
                             int* port)
{
    // Overide base class version as recvfrom does not
    // seem to return host info correctly for TCP
    // Use base class version without the remote host info
    int bytesRead = OsSocket::read(buffer, bufferLength);

    // Explicitly get the remote host info.
    getRemoteHostIp(ipAddress, port);

    return(bytesRead);
}

// Because we have overided one read method, we
// must implement them all in OsConnectionSocket or
// we end up hiding some of the methods.
int OsConnectionSocket::read(char* buffer,
                            int bufferLength,
                            long waitMilliseconds)
{
    // Use base class implementation
    int bytesRead = OsSocket::read(buffer, bufferLength, waitMilliseconds);
    return(bytesRead);
}

/* ============================ ACCESSORS ================================= */
OsSocket::IpProtocolSocketType OsConnectionSocket::getIpProtocol() const
{
        return(TCP);
}
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

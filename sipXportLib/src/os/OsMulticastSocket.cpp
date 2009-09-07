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
#include <stdio.h>

#if defined(_WIN32)
#   include <winsock.h>
#elif defined(_VXWORKS)
#   include <inetLib.h>
#   include <netdb.h>
#   include <resolvLib.h>
#   include <sockLib.h>
#elif defined(__pingtel_on_posix__)
#   include <netinet/in.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netdb.h>
#   include <resolv.h>
#else
#error Unsupported target platform.
#endif


// APPLICATION INCLUDES
#include <os/OsMulticastSocket.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

const UtlContainableType OsMulticastSocket::TYPE = "OsMulticastSocket";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsMulticastSocket::OsMulticastSocket(int multicastPortNum, const char* multicastHost,
                        int localHostPortNum, const char* localHost)
{
        int error = 0;
        struct sockaddr_in localAddr;
        struct hostent* server = NULL;
        int iTmp = TRUE;

        socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
        localHostPort = localHostPortNum;
        if(localHost)
        {
                localHostName.append(localHost);
        }
        remoteHostPort = multicastPortNum;
        if(multicastHost)
        {
                remoteHostName.append(multicastHost);
        }


        if(!socketInit())
        {
                goto EXIT;
        }

#       ifdef _VXWORKS
        char hostentBuf[512];
#       endif


        // Create the socket
        socketDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(socketDescriptor == OS_INVALID_SOCKET_DESCRIPTOR)
        {
                error = OsSocketGetERRNO();
                close();
                perror("call to socket failed in OsMulticastSocket::OsMulticastSocket\n");
                osPrintf("socket call failed with error in OsMulticastSocket::OsMulticastSocket: 0x%x\n", error);
                goto EXIT;
        }

         /* avoid EADDRINUSE error on bind() */
        iTmp = TRUE;
        if(setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, (char *)&iTmp,
     sizeof(iTmp)))
        {
                error = OsSocketGetERRNO();
                close();
                perror("call to setsockopt failed\n");
                osPrintf("setsockopt SO_REUSEADDR call failed with error: %d\n", error);
                goto EXIT;
        }


        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons(multicastPortNum);
        if(localHost == NULL)
        {
                localAddr.sin_addr.s_addr=OsSocket::getDefaultBindAddress();
//              localAddr.sin_addr.s_addr=htonl(INADDR_ANY); // Allow IP in on
                // any of this hosts addresses or NICs.
        }
        else
        {
                // Should use host address specified, for now use any
                localAddr.sin_addr.s_addr=OsSocket::getDefaultBindAddress();
//              localAddr.sin_addr.s_addr=htonl(INADDR_ANY); // Allow IP in on
                // any of this hosts addresses or NICs.
        }

#       if defined(_WIN32)
        error = bind( socketDescriptor, (const struct sockaddr*) &localAddr,
                        sizeof(localAddr));
#       elif defined(_VXWORKS) || defined(__pingtel_on_posix__)

        error = bind( socketDescriptor, (struct sockaddr*) &localAddr,
                        sizeof(localAddr));
#       else
#       error Unsupported target platform.
#       endif

        if(error == OS_INVALID_SOCKET_DESCRIPTOR)
        {
                // error = OsSocketGetERRNO();
                close();
                // perror("bind to socket failed\n");
                goto EXIT;
        }


        // Setup multicast options
#       if defined(_WIN32) || defined(__pingtel_on_posix__)
        server = gethostbyname(multicastHost);

#       elif defined(_VXWORKS)
        server = resolvGetHostByName((char*) multicastHost,
                                hostentBuf, sizeof(hostentBuf));
#       else
#       error Unsupported target platform.
#       endif //_VXWORKS

        if(server == NULL)
        {
                error = OsSocketGetERRNO();
                close();
                perror("call to gethostbyname failed\n");
                osPrintf("gethostbyname(%s) call failed with error: %d\n",multicastHost,
                                error);
                goto EXIT;
        }
        struct ip_mreq mreq;
        mreq.imr_multiaddr = *((in_addr*) (server->h_addr));

        mreq.imr_interface.s_addr = OsSocket::getDefaultBindAddress();
//      mreq.imr_interface.s_addr = htonl(INADDR_ANY);

   if(setsockopt(socketDescriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq)))
        {
                error = OsSocketGetERRNO();
                close();
                perror("call to setsockopt failed\n");
                osPrintf("setsockopt call failed with error: %d\n", error);
                goto EXIT;
        }

   joinMulticast(multicastPortNum, multicastHost);

EXIT:
   ;
}

// Copy constructor
OsMulticastSocket::OsMulticastSocket(const OsMulticastSocket& rOsMulticastSocket)
{
}

// Destructor
OsMulticastSocket::~OsMulticastSocket()
{
        close();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsMulticastSocket&
OsMulticastSocket::operator=(const OsMulticastSocket& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void OsMulticastSocket::joinMulticast(int multicastPort, const char* multicastHostName)
{
}

UtlBoolean OsMulticastSocket::reconnect()
{
        osPrintf("WARNING: reconnect NOT implemented!\n");
        return(FALSE);
}

int OsMulticastSocket::read(char* buffer, int bufferLength)
{
        int error;
        struct sockaddr_in serverSockAddr;
        int fromStructLength = sizeof(serverSockAddr);
        //int bytesRead = recv(socketDescriptor, buffer, bufferLength, 0);
        int bytesRead = recvfrom(socketDescriptor, buffer, bufferLength,
                0,
                (struct sockaddr*) &serverSockAddr,
#ifdef __pingtel_on_posix__
                        (socklen_t *)
#endif
                        &fromStructLength);

        if(bytesRead == -1)
        {
                error = OsSocketGetERRNO();
                // WIN32: 10038 WSAENOTSOCK not a valid socket descriptor
                if(error)
                {
                        close();
                        perror("OsSocket::read call to recv failed\n");
                }
        }

        return(bytesRead);
}

/* ============================ ACCESSORS ================================= */
OsSocket::IpProtocolSocketType OsMulticastSocket::getIpProtocol() const
{
        return(MULTICAST);
}
/* ============================ INQUIRY =================================== */

UtlContainableType OsMulticastSocket::getContainableType() const
{
   return OsMulticastSocket::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

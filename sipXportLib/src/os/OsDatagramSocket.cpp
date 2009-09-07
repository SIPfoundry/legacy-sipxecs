//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>
#include <stdio.h>

#define OPTIONAL_CONST const
#define OPTIONAL_VXW_CHARSTAR_CAST
#define resolvGetHostByName(a, b, c) gethostbyname(a)
#if defined(_WIN32)
#   include <winsock.h>
#   include <time.h>
#elif defined(_VXWORKS)
#   undef resolvGetHostByName
#   include <inetLib.h>
#   include <netdb.h>
#   include <resolvLib.h>
#   include <sockLib.h>
#   undef OPTIONAL_CONST
#   define OPTIONAL_CONST
#   undef OPTIONAL_VXW_CHARSTAR_CAST
#   define OPTIONAL_VXW_CHARSTAR_CAST (char*)
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
#include "os/OsDatagramSocket.h"
#include "os/OsSysLog.h"

// DEFINES
#define SOCKET_LEN_TYPE
#ifdef __pingtel_on_posix__
#undef SOCKET_LEN_TYPE
#define SOCKET_LEN_TYPE (socklen_t *)
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

#define MIN_REPORT_SECONDS 10

// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType OsDatagramSocket::TYPE = "OsDatagramSocket";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsDatagramSocket::OsDatagramSocket(int remoteHostPortNum,
                                   const char* remoteHost,
                                   int localHostPortNum,
                                   const char* localHost) :
   mNumTotalWriteErrors(0),
   mNumRecentWriteErrors(0),
   mSimulatedConnect(FALSE)     // Simulated connection is off until
                                // activated in doConnect.
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG, "OsDatagramSocket::_ attempt %s:%d"
                  ,remoteHost != NULL ? remoteHost : "[null]", remoteHostPortNum);

    int                error = 0;
#ifdef IP_MTU_DISCOVER
    int                pmtuDiscover;
#endif
    struct sockaddr_in localAddr;

    // Verify socket layer is initialized.
    if(!socketInit())
    {
        goto EXIT;
    }

    // Obtain time for throttling logging of write errors
    time(&mLastWriteErrorTime);

    // Initialize state settings
    mToSockaddrValid = FALSE;
    mpToSockaddr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
    assert(NULL != mpToSockaddr);
    memset(mpToSockaddr, 0, sizeof(struct sockaddr_in));
    socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
    localHostPort = localHostPortNum;
    if(localHost)
    {
        localHostName = localHost ;
    }

    // Create the socket
    socketDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socketDescriptor == OS_INVALID_SOCKET_DESCRIPTOR)
    {
        error = OsSocketGetERRNO();
        close();

        OsSysLog::add(FAC_KERNEL, PRI_WARNING,
                      "OsDatagramSocket::_ socket(%d, %d, %d) failed w/ errno %d '%s')",
                      AF_INET, SOCK_DGRAM, IPPROTO_UDP,
                      error, strerror(error));

        goto EXIT;
    }

    // Set Socket Options: IP_MTU_DISCOVER to DONT: Do not set dontfrag bit in
    // IP header
#ifdef IP_MTU_DISCOVER
    pmtuDiscover = IP_PMTUDISC_DONT ;
    error = setsockopt(socketDescriptor, IPPROTO_IP, IP_MTU_DISCOVER, &pmtuDiscover, sizeof(pmtuDiscover)) ;
    if (error != 0)
    {
        static bool bReported = false ;
        // Failure to set the socket option isn't a show-stopper, but should
        // be noted (and not spammed).
        if (!bReported)
        {
            bReported = true ;
            OsSysLog::add(FAC_KERNEL, PRI_WARNING,
                    "OsDatagramSocket::_ socket %d failed to set IP_MTU_DISCOVER (rc=%d, errno=%d)",
                    socketDescriptor, error,  OsSocketGetERRNO());
        }
    }
#endif

    // Bind to the socket
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port =
       htons(localHostPort == PORT_DEFAULT ? 0 : localHostPort);

    // Should use host address (if specified in localHost) but for now use any
    if (!localHost)
    {
        localAddr.sin_addr.s_addr=OsSocket::getDefaultBindAddress(); // $$$
        mLocalIp = inet_ntoa(localAddr.sin_addr);
    }
    else
    {
        struct in_addr  ipAddr;
        ipAddr.s_addr = inet_addr (localHost);
        localAddr.sin_addr.s_addr= ipAddr.s_addr;
        mLocalIp = localHost;
    }


#   if defined(_WIN32)
    error = bind( socketDescriptor, (const struct sockaddr*) &localAddr,
            sizeof(localAddr));
#   elif defined(__pingtel_on_posix__)

    error = bind( socketDescriptor, (struct sockaddr*) &localAddr,
            sizeof(localAddr));
#   endif
    if(error == OS_INVALID_SOCKET_DESCRIPTOR)
    {
        error = OsSocketGetERRNO();
        close();

        // Extract the address and port we were trying to bind() to.
        const char *addr = inet_ntoa(localAddr.sin_addr);
        int port = ntohs(localAddr.sin_port);
        OsSysLog::add(FAC_KERNEL, PRI_WARNING,
                      "OsDatagramSocket::_ %d (%s:%d %s:%d) bind(%d, %s:%d) failed w/ errno %d '%s')",
                      socketDescriptor,
                      remoteHost, remoteHostPortNum, localHost, localHostPortNum,
                      socketDescriptor, addr, port,
                      error, strerror(error));

        goto EXIT;
    }
    else
    {
        sockaddr_in addr ;
        int addrSize = sizeof(struct sockaddr_in);
        error = getsockname(socketDescriptor, (struct sockaddr*) &addr, SOCKET_LEN_TYPE& addrSize);
        localHostPort = htons(addr.sin_port);
    }

    OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                  "OsDatagramSocket::_ %d (%s:%d %s:%d) socket and bind succeeded",
                  socketDescriptor,
                  remoteHost, remoteHostPortNum, localHost, localHostPortNum);

    // Kick off connection
    mSimulatedConnect = FALSE;
    doConnect(remoteHostPortNum, remoteHost, mSimulatedConnect) ;

EXIT:
    return;
}


// Destructor
OsDatagramSocket::~OsDatagramSocket()
{
    close();
    free(mpToSockaddr);
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsDatagramSocket&
OsDatagramSocket::operator=(const OsDatagramSocket& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean OsDatagramSocket::reconnect()
{
    osPrintf("WARNING: reconnect NOT implemented!\n");
    return(FALSE);
}

void OsDatagramSocket::doConnect(int remoteHostPortNum,
                                 const char* remoteHost,
                                 UtlBoolean simulateConnect)
{
    struct hostent* server;

    mToSockaddrValid = FALSE;
    memset(mpToSockaddr, 0, sizeof(struct sockaddr_in));
    remoteHostPort = remoteHostPortNum;

    // Store host name
    if(remoteHost)
    {
        remoteHostName = remoteHost ;
        getHostIpByName(remoteHostName, &mRemoteIpAddress);
    }
    else
    {
        remoteHostName.remove(0) ;
    }

    // Connect to a remote host if given
    if(portIsValid(remoteHostPort) && remoteHost && !simulateConnect)
    {
        server = gethostbyname(remoteHost);
        if (server)
        {
            struct in_addr* serverAddr = (in_addr*) (server->h_addr);
            struct sockaddr_in serverSockAddr;
            serverSockAddr.sin_family = server->h_addrtype;
            serverSockAddr.sin_port = htons(remoteHostPort);
            serverSockAddr.sin_addr.s_addr = (serverAddr->s_addr);

            // Set the default destination address for the socket
            if(connect(socketDescriptor, (const struct sockaddr*)
                    &serverSockAddr, sizeof(serverSockAddr)))
            {
                int error = OsSocketGetERRNO();
                close();
                OsSysLog::add(FAC_KERNEL, PRI_WARNING,
                              "OsDatagramSocket::doConnect %d (%s:%d %s:%d) failed w/ errno %d '%s')",
                              socketDescriptor,
                              remoteHost, remoteHostPortNum,
                              localHostName.data(), localHostPort,
                              error, strerror(error));
            }
            else
            {
                mIsConnected = TRUE;
                OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                              "OsDatagramSocket::doConnect %d (%s:%d %s:%d) succeeded)",
                              socketDescriptor,
                              remoteHost, remoteHostPortNum,
                              localHostName.data(), localHostPort);
            }
        }
        else
        {
            close();
            OsSysLog::add(FAC_KERNEL, PRI_WARNING,
                    "OsDatagramSocket::doConnect( %s:%d ) failed host lookup)",
                    remoteHost, remoteHostPortNum);

            goto EXIT;
        }
    }
    else if(portIsValid(remoteHostPort) && remoteHost && simulateConnect)
    {
        mIsConnected = TRUE;
        mSimulatedConnect = TRUE;
        OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                      "OsDatagramSocket::doConnect %d (%s:%d %s:%d) simulated connect)",
                      socketDescriptor,
                      remoteHost, remoteHostPortNum,
                      localHostName.data(), localHostPort);
    }
EXIT:
   ;
}

int OsDatagramSocket::write(const char* buffer, int bufferLength)
{
    int returnCode;

    if(mSimulatedConnect)
    {
        returnCode = writeTo(buffer, bufferLength);
    }
    else
    {
        returnCode = OsSocket::write(buffer, bufferLength);
    }

    return(returnCode);
}

int OsDatagramSocket::write(const char* buffer, int bufferLength,
                            const char* ipAddress, int port)
{
    int bytesSent = 0;

    struct sockaddr_in toSockAddress;
    toSockAddress.sin_family = AF_INET;
    toSockAddress.sin_port = htons(port);

    if(ipAddress == NULL || !strcmp(ipAddress, "0.0.0.0") ||
        strlen(ipAddress) == 0 ||
        (toSockAddress.sin_addr.s_addr = inet_addr(ipAddress)) ==
            OS_INVALID_INET_ADDRESS)
    {
        osPrintf("OsDatagramSocket::write invalid IP address: \"%s\"\n",
            ipAddress);
    }
    else
    {
        // Why isn't this abstracted into OsSocket, as is done in ::write(2)?
        bytesSent = sendto(socketDescriptor,
#ifdef _VXWORKS
            (char*)
#endif
            buffer, bufferLength,
            0,
            (struct sockaddr*) &toSockAddress, sizeof(struct sockaddr_in));

        if(bytesSent != bufferLength)
        {
           OsSysLog::add(FAC_SIP, PRI_ERR,
                         "OsDatagramSocket::write(4) %d ipAddress = '%s', "
                         "port = %d, bytesSent = %d, "
                         "bufferLength = %d, errno = %d '%s'",
                         socketDescriptor,
                         ipAddress ? ipAddress : "[null]", port,
                         bytesSent, bufferLength, errno, strerror(errno));
            time_t rightNow;

            (void) time(&rightNow);

            mNumRecentWriteErrors++;

            if (MIN_REPORT_SECONDS <= (rightNow - mLastWriteErrorTime)) {

                mNumTotalWriteErrors += mNumRecentWriteErrors;
                if (0 == mNumTotalWriteErrors) {
                    mLastWriteErrorTime = rightNow;
                }
                osPrintf("OsDataGramSocket::write:\n"
                    "     In last %ld seconds: %d errors; total %d errors;"
                    " last errno=%d\n",
                    (rightNow - mLastWriteErrorTime), mNumRecentWriteErrors,
                    mNumTotalWriteErrors, OsSocketGetERRNO());

                mLastWriteErrorTime = rightNow;
                mNumRecentWriteErrors = 0;
            }
        }
    }
    return(bytesSent);
}

UtlBoolean OsDatagramSocket::getToSockaddr()
{
    const char* ipAddress = mRemoteIpAddress.data();

    if (!mToSockaddrValid)
    {
        mpToSockaddr->sin_family = AF_INET;
        mpToSockaddr->sin_port = htons(remoteHostPort);

        if (ipAddress == NULL || !strcmp(ipAddress, "0.0.0.0") ||
            strlen(ipAddress) == 0 ||
            (mpToSockaddr->sin_addr.s_addr = inet_addr(ipAddress)) ==
                OS_INVALID_INET_ADDRESS)
        {
/*
            osPrintf(
               "OsDatagramSocket::getToSockaddr: invalid IP address: \"%s\"\n",
                ipAddress);
*/
        } else {
            mToSockaddrValid = TRUE;
        }
    }
    return mToSockaddrValid;
}

int OsDatagramSocket::writeTo(const char* buffer, int bufferLength)
{
    int bytesSent = 0;

    if (getToSockaddr()) {
        bytesSent = sendto(socketDescriptor,
#ifdef _VXWORKS
            (char*)
#endif
            buffer, bufferLength,
            0,
            (struct sockaddr*) mpToSockaddr, sizeof(struct sockaddr_in));

        if(bytesSent != bufferLength)
        {
            time_t rightNow;

            (void) time(&rightNow);

            mNumRecentWriteErrors++;

            if (MIN_REPORT_SECONDS <= (rightNow - mLastWriteErrorTime)) {

                mNumTotalWriteErrors += mNumRecentWriteErrors;
                if (0 == mNumTotalWriteErrors) {
                    mLastWriteErrorTime = rightNow;
                }
                osPrintf("OsDataGramSocket::write:\n"
                    "     In last %ld seconds: %d errors; total %d errors;"
                    " last errno=%d\n",
                    (rightNow - mLastWriteErrorTime), mNumRecentWriteErrors,
                    mNumTotalWriteErrors, OsSocketGetERRNO());

                mLastWriteErrorTime = rightNow;
                mNumRecentWriteErrors = 0;
            }
        }
    }
    return(bytesSent);
}

#ifdef WANT_GET_FROM_INFO /* [ */
#ifdef _VXWORKS /* [ */
static int getFromInfo = 0;
extern "C" { extern int setFromInfo(int x);};
int setFromInfo(int getIt) {
   int save = getFromInfo;
   getFromInfo = getIt ? 1 : 0;
   return save;
}
#define GETFROMINFO getFromInfo
#endif /* _VXWORKS ] */
#endif /* WANT_GET_FROM_INFO ] */

int OsDatagramSocket::read(char* buffer, int bufferLength)
{
    int bytesRead;

    // If the remote end is not "connected" we cannot use recv
    if(mSimulatedConnect || !portIsValid(remoteHostPort) || remoteHostName.isNull())
    {
#ifdef GETFROMINFO /* [ */
        if (GETFROMINFO) {
          int fromPort;
          UtlString fromAddress;
          bytesRead = OsSocket::read(buffer, bufferLength,
             &fromAddress, &fromPort);
          fromAddress.remove(0);
        } else
#endif /* GETFROMINFO ] */
        {
           bytesRead = OsSocket::read(buffer, bufferLength,
              (struct in_addr*) NULL, NULL);
        }
    }
    else
    {
        bytesRead = OsSocket::read(buffer, bufferLength);
    }
    return(bytesRead);
}

/* ============================ ACCESSORS ================================= */
OsSocket::IpProtocolSocketType OsDatagramSocket::getIpProtocol() const
{
    return(UDP);
}

void OsDatagramSocket::getRemoteHostIp(struct in_addr* remoteHostAddress,
                               int* remotePort)
{
#ifdef __pingtel_on_posix__
    socklen_t len;
#else
    int len;
#endif
    struct sockaddr_in remoteAddr;
    const struct sockaddr_in* pAddr;

    if (mSimulatedConnect) {
        getToSockaddr();
        pAddr = mpToSockaddr;
    } else {
        pAddr = &remoteAddr;

        len = sizeof(struct sockaddr_in);

        if (getpeername(socketDescriptor, (struct sockaddr *)pAddr, &len) != 0)
        {
            memset(&remoteAddr, 0, sizeof(struct sockaddr_in));
        }
    }
    memcpy(remoteHostAddress, &(pAddr->sin_addr), sizeof(struct in_addr));

#ifdef TEST_PRINT
    {
        int p = ntohs(pAddr->sin_port);
        UtlString o;
        inet_ntoa_pt(*remoteHostAddress, o);
        osPrintf("getRemoteHostIP: Remote name: %s:%d\n"
           "   (pAddr->sin_addr) = 0x%X, sizeof(struct in_addr) = %d\n",
           o.data(), p, (pAddr->sin_addr), sizeof(struct in_addr));
    }
#endif

    if (NULL != remotePort)
    {
        *remotePort = ntohs(pAddr->sin_port);
    }
}

// Return the external IP address for this socket.
UtlBoolean OsDatagramSocket::getExternalIp(UtlString* ip, int* port)
{
    return FALSE ;
}


/* ============================ INQUIRY =================================== */

UtlContainableType OsDatagramSocket::getContainableType() const
{
   return OsDatagramSocket::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

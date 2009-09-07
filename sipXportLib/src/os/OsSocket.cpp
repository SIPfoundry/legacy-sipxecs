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
#include <fcntl.h>
#include <poll.h>

//uncomment and recompile to make the socket layer fail after 20 calls.
//after that, every 10 calls
//#define FORCE_SOCKET_ERRORS

#include "utl/UtlRscTrace.h"

#if defined(_WIN32)
#   include <winsock2.h>
#elif defined(_VXWORKS)
#   include <hostLib.h>
#   include <inetLib.h>
#   include <netdb.h>
#   include <resolvLib.h>
#   include <sockLib.h>
#   include <unistd.h>
#elif defined(__pingtel_on_posix__)
#   include <sys/types.h>
#   include <sys/time.h>
#   include <sys/socket.h>
#   include <sys/poll.h>
#   include <netdb.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <pthread.h>
#   include <unistd.h>
#   include <errno.h>
    /* for some reason this isn't defined on Solaris... */
#   ifndef INADDR_NONE
#      define INADDR_NONE ((unsigned long) -1)
#   endif
#else
#error Unsupported target platform.
#endif

#ifdef __linux__
#include "os/linux/host_address.h"
#endif


#define DOMAIN_NAME_LENGTH 512
#define HOST_NAME_LENGTH 512


// APPLICATION INCLUDES
#include <os/OsSocket.h>
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
#ifdef _VXWORKS
extern "C" int enetIsLinkActive(void);
#endif

#ifdef __hpux
// getdomainname prototype is not defined in hp-ux header files.
extern "C"
{
   int getdomainname(char *name, int namelen);
}
#endif

// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType OsSocket::TYPE = "OsSocket";

// STATIC VARIABLE INITIALIZATIONS
UtlBoolean OsSocket::socketInitialized = FALSE;
UtlString  OsSocket::m_DomainName = "";
// this should be htonl(INADDR_ANY) but count on it being 0...
// seems that g++ won't compile it with optimization enabled for some reason.
unsigned long OsSocket::m_DefaultBindAddress = INADDR_ANY;

OsBSem OsSocket::mInitializeSem(OsBSem::Q_PRIORITY, OsBSem::FULL);


/* //////////////////////////// PUBLIC //////////////////////////////////// */

//static method for accessing the bind address from C source
unsigned long osSocketGetDefaultBindAddress()
{
        return OsSocket::getDefaultBindAddress();
}

/* ============================ CREATORS ================================== */

// Constructor
OsSocket::OsSocket()
   : socketDescriptor(OS_INVALID_SOCKET_DESCRIPTOR)
   , localHostPort(OS_INVALID_SOCKET_DESCRIPTOR)
   , remoteHostPort(OS_INVALID_SOCKET_DESCRIPTOR)
   , mIsConnected(FALSE)
   , mActual_socketDescriptor(OS_INVALID_SOCKET_DESCRIPTOR)
{
}

// Destructor
OsSocket::~OsSocket()
{
   close();
   if(mActual_socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR)
   {
      // Now it is safe to give this descriptor back to the OS
      ::close(mActual_socketDescriptor);
      OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSocket::~ close socket %d",
                    mActual_socketDescriptor);
   }
}

/* ============================ MANIPULATORS ============================== */


int OsSocket::write(const char* buffer, int bufferLength)
{

#ifdef FORCE_SOCKET_ERRORS

   static int numForFailure = 30;
   static int count = 0;
   count++;

   if (count > numForFailure)
   {
      count = 0;
      numForFailure = 10;
#ifdef _WIN32
      closesocket(socketDescriptor);
#else
      ::close(socketDescriptor);
      OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSocket::write close socket %d",
                    socketDescriptor);
#endif
      return 0;
   }
#endif // FORCE_SOCKET_ERRORS

   ssize_t bytesSent;

   int flags = 0;

#if defined(__linux__) || defined(sun) || defined(__FreeBSD__)
   // We do not want send to throw signals if there is a
   // problem with the socket as this results in the process
   // getting aborted. We just want it to return an error.
   // (Under OS X, we use SO_NOSIGPIPE because this is not
   // supported... this is done in the constructors for
   // stream socket types as it is a one-time-only thing.)
   flags = MSG_NOSIGNAL;
#endif

#       if defined(_WIN32) || defined(__pingtel_on_posix__)
   bytesSent = send(socketDescriptor, buffer, bufferLength, flags);
   if (bytesSent != bufferLength)
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR,
                    "OsSocket::write %d (%s:%d %s:%d) send returned %zd, errno=%d '%s'",
                    socketDescriptor,
                    remoteHostName.data(), remoteHostPort,
                    localHostName.data(), localHostPort,
                    bytesSent, errno, strerror(errno));
   }

   // 10038 WSAENOTSOCK not a valid socket descriptor

#       elif defined(_VXWORKS)
   bytesSent = send(socketDescriptor, (char*)buffer, bufferLength, 0);
#       else
#       error Unsupported target platform.
#       endif

   return(bytesSent);
}

int OsSocket::write(const char* buffer, int bufferLength, long waitMilliseconds)
{
        int numBytes = 0;
    if(isReadyToWrite(waitMilliseconds))
    {
        numBytes = write(buffer, bufferLength);
    }
        return(numBytes);
}

int OsSocket::read(char* buffer, int bufferLength)
{

#ifdef FORCE_SOCKET_ERRORS

   static int numForFailure = 30;
   static int count = 0;
   count++;

   if (count > numForFailure)
   {
      count = 0;
      numForFailure = 10;
#ifdef _WIN32
      closesocket(socketDescriptor);
#else
      ::close(socketDescriptor);
      OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSocket::read[2] close socket %d",
                    socketDescriptor);
#endif
      return 0;
   }
#endif //FORCE_SOCKET_ERRORS

   int flags = 0;

#if defined(__linux__) || defined(sun) || defined(__FreeBSD__)
   // We do not want send to throw signals if there is a
   // problem with the socket as this results in the process
   // getting aborted. We just want it to return an error.
   // (Under OS X, we use SO_NOSIGPIPE because this is not
   // supported... this is done in the constructors for
   // stream socket types as it is a one-time-only thing.)
   flags = MSG_NOSIGNAL;
#endif

   int error;
   ssize_t bytesRead = recv(socketDescriptor, buffer, bufferLength, flags);
   if (bytesRead < 0)
   {
      error = OsSocketGetERRNO();
      // WIN32: 10038 WSAENOTSOCK not a valid socket descriptor
   }

   return bytesRead;
}

int OsSocket::read(char* buffer, int bufferLength,
                   struct in_addr* fromAddress, int* fromPort)
{

#ifdef FORCE_SOCKET_ERRORS

   static int numForFailure = 30;
   static int count = 0;
   count++;

   if (count > numForFailure)
   {
      count = 0;
      numForFailure = 10;
#ifdef _WIN32
      closesocket(socketDescriptor);
#else
      ::close(socketDescriptor);
      OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSocket::read[4in_addr] close socket %d",
                    socketDescriptor);
#endif
      return 0;
   }

#endif //FORCE_SOCKET_ERRORS

   int error;
   struct sockaddr_in fromSockAddress;
   size_t fromLength = sizeof(fromSockAddress);

   if (NULL != fromPort) *fromPort = PORT_NONE;
   if (NULL != fromAddress) fromAddress->s_addr = 0;

   int flags = 0;

#if defined(__linux__) || defined(sun) || defined(__FreeBSD__)
   // We do not want send to throw signals if there is a
   // problem with the socket as this results in the process
   // getting aborted. We just want it to return an error.
   // (Under OS X, we use SO_NOSIGPIPE because this is not
   // supported... this is done in the constructors for
   // stream socket types as it is a one-time-only thing.)
   flags = MSG_NOSIGNAL;
#endif

   ssize_t bytesRead = recvfrom(socketDescriptor, buffer, bufferLength,
                            flags,
                            (struct sockaddr*) &fromSockAddress,
#ifdef __pingtel_on_posix__
                            (socklen_t *)
#endif
                            &fromLength);
   if(bytesRead == -1)
   {
      error = OsSocketGetERRNO();
      // 10038 WSAENOTSOCK not a valid socket descriptor
      switch(error)
      {
         // These should be ignored
         case EAGAIN:
         case EINTR:
            break ;

         // Others should not.
         default:
            OsSysLog::add(FAC_KERNEL, PRI_WARNING,
                          "OsSocket::read %d (%s:%d %s:%d) recvfrom returned %d '%s'",
                          socketDescriptor,
                          remoteHostName.data(), remoteHostPort,
                          localHostName.data(), localHostPort,
                          error, strerror(error)
               );
            close();
      }

   }
   else
   {
      if (NULL != fromPort)
      {
         *fromPort = ntohs(fromSockAddress.sin_port);
      }
      if (NULL != fromAddress)
      {
         *fromAddress = fromSockAddress.sin_addr;
      }
   }

   return (bytesRead);
}

int OsSocket::read(char* buffer, int bufferLength,
                UtlString* fromAddress, int* fromPort)
{

#ifdef FORCE_SOCKET_ERRORS

   static int numForFailure = 30;
   static int count = 0;
   count++;

   if (count > numForFailure)
   {
      count = 0;
      numForFailure = 10;
#ifdef _WIN32
                closesocket(socketDescriptor);
#else
                ::close(socketDescriptor);
                OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSocket::read[4UtlString] close socket %d",
                              socketDescriptor);
#endif
      return 0;
   }
#endif //FORCE_SOCKET_ERRORS

   int bytesRead;
   struct in_addr fromSockAddress;

   if (fromAddress)
   {
      fromAddress->remove(0);
   }

   bytesRead = read(buffer, bufferLength, &fromSockAddress, fromPort);
   if(bytesRead != -1)
   {
      if (NULL != fromAddress)
      {
         inet_ntoa_pt(fromSockAddress, *fromAddress);
      }
   }

   return(bytesRead);
}

int OsSocket::read(char* buffer, int bufferLength, long waitMilliseconds)
{
    int numBytes = 0;
    if(isReadyToRead(waitMilliseconds))
    {
        numBytes = read(buffer, bufferLength);
    }

    return(numBytes);
}

UtlBoolean OsSocket::isReadyToReadEx(long waitMilliseconds,UtlBoolean &rSocketError) const
{
   int tempSocketDescr = OS_INVALID_SOCKET_DESCRIPTOR;
   rSocketError = FALSE;

#ifdef FORCE_SOCKET_ERRORS
   static int numForFailure = 30;
   static int count = 0;
   count++;

   if (count > numForFailure)
   {
      count = 0;
      numForFailure = 10;

#ifdef _WIN32
                closesocket(socketDescriptor);
#else
                ::close(socketDescriptor);
                OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSocket::isReadyToReadEx close socket %d",
                              socketDescriptor);
#endif
      rSocketError = TRUE;
      return FALSE;
   }
#endif //FORCE_SOCKET_ERRORS

    int numReady = 0;

    //making a temp copy of the descriptor because it may change from another
    //thread.  And we wouldn't want to use a bad descriptor
    //in this next piece of code.
    tempSocketDescr = socketDescriptor;

    if(tempSocketDescr > OS_INVALID_SOCKET_DESCRIPTOR)
    {
#ifdef __pingtel_on_posix__ /* [ */
#define POLLSET(XX) ((pollState.revents & XX) != 0)

        int resCode;
        struct pollfd pollState;
        pollState.fd      = tempSocketDescr;
        pollState.events  = POLLIN;
        pollState.revents = 0;

        // In a POSIX system the system call might be interrupted...
        do
        {
            resCode = poll(&pollState, 1, waitMilliseconds);
        } while (   resCode == -1 && errno == EINTR // system call was interrupted
                 && socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR // socket has not been closed
                );

        if(resCode < 0)
        {
            rSocketError = TRUE;
            numReady = -1;
            OsSysLog::add(FAC_KERNEL, PRI_WARNING,
                          "OsSocket::isReadyToRead %d (%s:%d %s:%d) poll returned %d (errno=%d '%s') in socket: %x %p",
                          socketDescriptor,
                          remoteHostName.data(), remoteHostPort,
                          localHostName.data(), localHostPort,
                          resCode, errno, strerror(errno), tempSocketDescr, this);
        }
        else if(resCode > 0 &&  pollState.revents != 0 && !POLLSET(POLLIN))
        {
            numReady = -1;
            OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                          "OsSocket::isReadyToRead polllState.revents = %d: "
                          "POLLIN: %d POLLPRI: %d POLLERR: %d POLLHUP: %d POLLNVAL: %d",
                          pollState.revents,
                          POLLSET(POLLIN), POLLSET(POLLPRI), POLLSET(POLLERR), POLLSET(POLLHUP), POLLSET(POLLNVAL));
        }
        else if(resCode > 0 && POLLSET(POLLIN))
        {
#          if 0 // turn on to debug socket polling problems
           OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                         "OsSocket::isReadyToRead socket: %x READY POLLIN", tempSocketDescr);
#          endif
           numReady = 1;
        }
        else if(resCode == 0)
        {
            numReady = 0;
            if(waitMilliseconds < 0)
            {
#              if 0 // turn on to debug socket polling problems
               OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                             "OsSocket::isReadyToRead poll returned %d in socket: %d %p",
                             resCode, tempSocketDescr, this);
#              endif
            }
        }
        else
        {
           OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                         "OsSocket::isReadyToRead socket: %x READY %d", tempSocketDescr, resCode);
            numReady = resCode;
        }

#else /* __pingtel_on_posix__ ] [ */

        //making a temp copy of the descriptor because it may change from another
        //thread.  And we wouldn't want to use a bad descriptor
        //in this next piece of code.
        tempSocketDescr = socketDescriptor;
        if (tempSocketDescr > OS_INVALID_SOCKET_DESCRIPTOR)
        {
           struct pollfd pset[1];
           pset[0].fd = tempSocketDescr;
           pset[0].events = POLLIN;
           pset[0].revents = 0;

           // if wait time is less than zero block indefinitely
           int resCode = poll(pset, 1, waitMilliseconds);

           // if poll returns an error, then numReady is set to -1
           // if there has been an exception on the socket, then numReady is set to -1
           // otherwise, numReady is set to the value returned by poll()

           //test socketDescriptor to be sure it didn't become invalid
           //while in select
           if (resCode == -1 || socketDescriptor <= OS_INVALID_SOCKET_DESCRIPTOR)
           {
              numReady = -1;
              rSocketError = TRUE;
           }
           else if (resCode > 0 && (pset[0].revents & POLLERR) != 0)
              numReady = -1;
           else
              numReady = resCode;

           if(numReady < 0 || numReady > 1 || (numReady == 0 && waitMilliseconds < 0))
           {
               // perror("OsSocket::isReadyToRead()");
               OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSocket::isReadyToRead poll returned %d in socket: %d %p",
                   resCode, tempSocketDescr, this);
           }
        } //if tempSocketDescr if ok
#endif /* __pingtel_on_posix__ ] */

    }

    return(numReady == 1);

}

UtlBoolean OsSocket::isReadyToRead(long waitMilliseconds) const
{
   UtlBoolean bSocketError = FALSE;

   return isReadyToReadEx(waitMilliseconds,bSocketError);
}

UtlBoolean OsSocket::isReadyToWrite(long waitMilliseconds) const
{
   int tempSocketDescr = OS_INVALID_SOCKET_DESCRIPTOR;

#ifdef FORCE_SOCKET_ERRORS

   static int numForFailure = 30;
   static int count = 0;
   count++;

   if (count > numForFailure)
   {
      count = 0;
      numForFailure = 10;

#ifdef _WIN32
                closesocket(socketDescriptor);
#else
                ::close(socketDescriptor);
                OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSocket::isReadyToWrite close socket %d",
                              socketDescriptor);
#endif
      return FALSE;
   }
#endif //FORCE_SOCKET_ERRORS

    int numReady = 0;
    if(socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR)
    {
       int    resCode = 0;
       struct pollfd pset[1];

// In a POSIX system the system call might be interrupted...
#ifdef __pingtel_on_posix__
       do
       {
#endif
          //making a temp copy of the descriptor because it may change from another
          //thread.  And we wouldn't want to use a bad descriptor
          //in this next piece of code.
          tempSocketDescr = socketDescriptor;
          if (tempSocketDescr > OS_INVALID_SOCKET_DESCRIPTOR)
          {
             pset[0].fd = tempSocketDescr;
             pset[0].events = POLLOUT;
             pset[0].revents = 0;

             // if wait time is less than zero block indefinitely
             resCode = poll(pset, 1, waitMilliseconds);
          }

#ifdef __pingtel_on_posix__
       } while (resCode == -1 && errno == EINTR);
#endif

       if(resCode < 0)
       {
          OsSysLog::add(FAC_KERNEL, PRI_WARNING,
                        "OsSocket::isReadyToWrite %d (%s:%d %s:%d) poll returned %d (errno=%d '%s') in socket: %d %p",
                        socketDescriptor,
                        remoteHostName.data(), remoteHostPort,
                        localHostName.data(), localHostPort,
                        resCode, errno, strerror(errno), tempSocketDescr, this);
       }

       // if select returns an error, then numReady is set to -1
       // if there has been an exception on the socket, then numReady is set to -1
       // otherwise, numReady is set to the value returned by select()

       //test socketDescriptor to be sure it didn't become invalid
       //while in select
       if (resCode == -1 || socketDescriptor <= OS_INVALID_SOCKET_DESCRIPTOR)
          numReady = -1;
       else if (resCode > 0 && (pset[0].revents & POLLERR) != 0)
          numReady = -1;
       else
          numReady = resCode;

       if(numReady < 0 || numReady > 1 || (numReady == 0 && waitMilliseconds < 0))
       {
          // perror("OsSocket::isReadyToWrite()");
          OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSocket::isReadyToWrite poll returned %d in socket: %d %p",
                        resCode, tempSocketDescr, this);
       }

    }     //socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR

    return(numReady == 1);
}

void OsSocket::close()
{
   if(socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR)
   {
#     if defined(_WIN32)
      closesocket(socketDescriptor);
#     elif defined(_VXWORKS) || defined(__pingtel_on_posix__)

#     if defined(__pingtel_on_posix__)
      // This forces any selects which are blocked on
      // this socket to return
      shutdown(socketDescriptor, SHUT_RDWR);
#     endif

      // Don't really close the actual descriptor until the destructor.
      mActual_socketDescriptor = socketDescriptor;

#     else
#       error Unsupported target platform.
#     endif
      socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
   }
}

const char* socketType_UNKNOWN = "UNKNOWN";
const char* socketType_TCP = "TCP";
const char* socketType_UDP = "UDP";
const char* socketType_MULTICAST = "MULTICAST";
const char* socketType_SSL = "TLS";
const char* socketType_invalid = "INVALID";

const char* OsSocket::ipProtocolString(OsSocket::IpProtocolSocketType type)
{
   switch (type)
   {
   case OsSocket::UNKNOWN:
      return socketType_UNKNOWN;
      break;
   case OsSocket::TCP:
      return socketType_TCP;
      break;
   case OsSocket::UDP:
      return socketType_UDP;
      break;
   case OsSocket::MULTICAST:
      return socketType_MULTICAST;
      break;
   case OsSocket::SSL_SOCKET:
      return socketType_SSL;
      break;
   default:
      return socketType_invalid;
   }
}


void OsSocket::makeNonblocking()
{
   OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSocket::makeNonblocking %d", socketDescriptor);
#if defined(_WIN32)
    unsigned long c_ONDELAY=1;
    ioctlsocket(socketDescriptor, FIONBIO, &c_ONDELAY);
#elif defined(_VXWORKS)
    #error Unsupported target platform.
#elif defined(__pingtel_on_posix__)
    int flags = fcntl(socketDescriptor, F_GETFL);
    fcntl(socketDescriptor, F_SETFL,  flags | O_NDELAY);
#else
    #error Unsupported target platform.
#endif
}

void OsSocket::makeBlocking()
{
   OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSocket::makeBlocking %d", socketDescriptor);
#if defined(_WIN32)
    unsigned long c_ONDELAY=0;
    ioctlsocket(socketDescriptor, FIONBIO, &c_ONDELAY);
#elif defined(_VXWORKS)
    #error Unsupported target platform.
#elif defined(__pingtel_on_posix__)
    int flags = fcntl(socketDescriptor, F_GETFL);
    fcntl(socketDescriptor, F_SETFL, flags & ~O_NDELAY);
#else
    #error Unsupported target platform.
#endif
}

UtlBoolean OsSocket::socketInit()
{
        UtlBoolean returnCode = TRUE;

        mInitializeSem.acquire();
        if(!socketInitialized)
        {
            socketInitialized = TRUE;
            mInitializeSem.release();
           // Windows specific startup
#               ifdef _WIN32
                WORD wVersionRequested = MAKEWORD( 1, 1 );
                WSADATA wsaData;
                int error = WSAStartup(wVersionRequested, &wsaData);
                if(error)
                {
                        osPrintf("WSAStartup call failed with error: %d in OsServerSocket::OsServerSocket\n", error);
                        osPrintf("winsock version: %d.%d supported\n", LOBYTE( wsaData.wVersion ),
                                HIBYTE( wsaData.wVersion ));
                        returnCode = FALSE;
                }
#               endif


        }
        else
        {
            mInitializeSem.release();
        }
        return(returnCode);
}


/////////////////////////////
//
// returns the corredct ipaddress in network byte order
//
//
/////////////////////////////////

unsigned long OsSocket::initDefaultAdapterID(UtlString &interface_id)
{
    mInitializeSem.acquire();
    UtlString address = "";
    unsigned long retip = htonl(INADDR_ANY);

#ifdef WIN32
    // Under windows it is possible for many network devices to be present.
    // in this case we will either return an empty string
    // or if the configuration parameter  PHONESET_BIND_MAC_ADDRESS is defined
    // we will look up the mac address against the windows adapters
    // and then return the correct ip address for that adapter
    int numAdapters = getAdaptersInfo();  //fills in the structure with the adapter info
    if (numAdapters < 2)
    {
        retip = htonl(INADDR_ANY);
    }
    else
    {
        char ipaddress[20];
        char adapter_id[30];

        *ipaddress = '\0';
        *adapter_id = '\0';

        strcpy(adapter_id, interface_id.data());

        //if this fails, then we need to choose any address
        if (strlen(adapter_id) == 0 ||
                lookupIpAddressByMacAddress(adapter_id, ipaddress) == -1)
        {
            retip = htonl(INADDR_ANY);
        }
        else
        {
            address = ipaddress;
        }
    }

    //now convert if it has a string ip address
    if (address != "")
    {
        struct in_addr  ipAddr;
        ipAddr.s_addr = inet_addr (address.data());
        retip = ipAddr.s_addr;
    }

#endif

    mInitializeSem.release();
    return retip;
}

/* ============================ ACCESSORS ================================= */

// Returns the socket descriptor
// Warning: use of this method risks the creation of platform
// dependent code.
int OsSocket::getSocketDescriptor() const
{
        return(socketDescriptor);
}

void OsSocket::setDefaultBindAddress(const unsigned long bind_address)
{
    mInitializeSem.acquire();
    m_DefaultBindAddress = bind_address;
    mInitializeSem.release();
}

unsigned long OsSocket::getDefaultBindAddress()
{
    return(m_DefaultBindAddress);
}


//gets static member m_DomainName
void OsSocket::getDomainName(UtlString &domain_name)
{
    if (m_DomainName == NULL)
    {
#ifdef __pingtel_on_posix__
        char nameBuffer[DOMAIN_NAME_LENGTH];
        if (0 == getdomainname(nameBuffer, DOMAIN_NAME_LENGTH - 1))
        {
           m_DomainName = nameBuffer;
        }
#endif  //__pingtel_on_posix__

#ifdef _VXWORKS
        RESOLV_PARAMS_S resolvParams;

        resolvParamsGet(&resolvParams);
        m_DomainName = resolvParams.domainName;
#endif  //VXWORKS

#ifdef WIN32
        char domain[DOMAIN_NAME_LENGTH];
        getWindowsDomainName (domain);
        m_DomainName = domain;
#endif  //WIN32

    }

    domain_name = m_DomainName;
}


void OsSocket::getHostName(UtlString* hostName)
{
        socketInit();
        char nameBuffer[HOST_NAME_LENGTH];
        gethostname(nameBuffer, HOST_NAME_LENGTH - 1);
        hostName->remove(0);
        hostName->append(nameBuffer);
}

void OsSocket::getHostIp(UtlString* hostAddress)
{
      socketInit();
      UtlString thisHost;

#ifdef _VXWORKS /* [ */

      char ipAddr[100];
      ipAddr[0] = '\0';
      hostAddress->remove(0);
      if(!ifAddrGet("csp0", ipAddr))
         hostAddress->append(ipAddr);

#elif defined(__pingtel_on_posix__) /* ] [ */

#ifdef __linux__
      char ipAddr[100];
      unsigned int ip_int = ntohl(getExternalHostAddressLinux());
      sprintf(ipAddr, "%d.%d.%d.%d", ip_int >> 24, (ip_int >> 16) & 255, (ip_int >> 8) & 255, ip_int & 255);
      hostAddress->remove(0);
      hostAddress->append(ipAddr);
#else
      getHostName(&thisHost);
      getHostIpByName(thisHost.data(), hostAddress);
#endif /* __linux__ */

#elif defined(_WIN32) /* ] [ */

      unsigned long address_val = OsSocket::getDefaultBindAddress();

      if (address_val == htonl(INADDR_ANY))
      {
         getHostName(&thisHost);
         getHostIpByName(thisHost.data(), hostAddress);
      }
      else
      {
         struct in_addr in;
         char tmp[50];
         in.S_un.S_addr = address_val;
         strcpy(tmp,inet_ntoa(in));
         *hostAddress = tmp;
      }

#else /* ] [ */
#error Unsupported target platform.
#endif /* ] */

      thisHost.remove(0);
}

void OsSocket::getRemoteHostIp(struct in_addr* remoteHostAddress,
                               int* remotePort)
{
    struct sockaddr_in remoteAddr;
#ifdef __pingtel_on_posix__
    socklen_t len;
#else
    int len;
#endif

    len = sizeof(struct sockaddr_in);

    int ret = getpeername(socketDescriptor, (struct sockaddr *)&remoteAddr, &len);
    if (ret != 0)
    {
        memset(&remoteAddr, 0, len);
    }

    // *remoteHostAddress = remoteAddr.sin_addr;
    memcpy(remoteHostAddress, &(remoteAddr.sin_addr), sizeof(struct in_addr));

#ifdef TEST_PRINT
    {
        UtlString output_address;
        inet_ntoa_pt(remoteAddr.sin_addr, output_address);
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "OsSocket::getRemoteHostIp Remote name '%s'",
                      output_address.data());
    }
#endif

    if (NULL != remotePort)
    {
        *remotePort = ntohs(remoteAddr.sin_port);
    }
}

void OsSocket::getRemoteHostIp(UtlString* remoteHostAddress, int* remotePort)
{
    struct in_addr remoteAddr;

    getRemoteHostIp(&remoteAddr, remotePort);
    remoteHostAddress->remove(0);
    inet_ntoa_pt(remoteAddr, *remoteHostAddress);

    // If querying the network layer did not obtain an address (probably
    // because the socket is still in the process of connecting), return
    // the values stored from the connect attempt.
    if (remoteHostAddress->compareTo("0.0.0.0") == 0)
    {
       remoteHostAddress->remove(0);
       remoteHostAddress->append(mRemoteIpAddress);
       if (remotePort)
       {
          *remotePort = remoteHostPort;
       }
    }
}

void OsSocket::getRemoteHostName(UtlString* remoteHostNameString) const
{
   remoteHostNameString->remove(0);
   remoteHostNameString->append(remoteHostName);
}

int OsSocket::getRemoteHostPort() const
{
   return remoteHostPort;
}

UtlBoolean OsSocket::getHostIpByName(const char* hostName, UtlString* hostAddress)
{
    UtlBoolean bSuccess = FALSE;

    struct hostent* server;
    socketInit();

   // If this is aleady an IP address
   if (isIp4Address(hostName))
   {
      *hostAddress = hostName;
      bSuccess = TRUE ;
   }
   else if (strcmp(hostName, "localhost") == 0)
   {
      *hostAddress = "127.0.0.1";
      bSuccess = TRUE ;
   }
   // if no default domain name and host name not fully qualified
   else if (!hasDefaultDnsDomain() && (strchr(hostName, '.') == NULL))
   {
      *hostAddress = "0.0.0.0";
   }
   else
   {
#if defined(_WIN32) || defined(__pingtel_on_posix__)
        server = gethostbyname(hostName);
#else
#error Unsupported target platform.
#endif
        if(server)
        {
            inet_ntoa_pt(*((in_addr*) (server->h_addr)), *hostAddress);
            bSuccess = TRUE ;
        }
        else
        {
            *hostAddress = "0.0.0.0";
        }
    }

   return bSuccess ;
}

void OsSocket::getLocalHostName(UtlString* name) const
{
   *name = localHostName;
}

void OsSocket::getLocalHostIp(UtlString* localHostAddress) const
{
    OsSocket::getHostIp(localHostAddress) ;
}

int OsSocket::getLocalHostPort() const
{
        return(localHostPort);
}

/* ============================ INQUIRY =================================== */

UtlBoolean OsSocket::isOk() const
{
   return(socketDescriptor != OS_INVALID_SOCKET_DESCRIPTOR);
}

UtlBoolean OsSocket::isConnected() const
{
    return(mIsConnected);
}

UtlBoolean OsSocket::isLocalHost(const char* hostAddress)
{
        UtlBoolean local;
        UtlString thisHost;
        UtlString thisHostAddr;
        getHostName(&thisHost);
        getHostIp(&thisHostAddr);

        if(strcmp(hostAddress, "127.0.0.1") == 0 ||
                strcmp(hostAddress, "localhost") == 0 ||
                strcmp(hostAddress, thisHost.data()) == 0 ||
                strcmp(hostAddress, thisHostAddr.data()) == 0)
        {
                local = TRUE;
        }
        else
        {
                local = FALSE;
        }
        thisHost.remove(0);
        thisHostAddr.remove(0);
        return(local);
}

UtlBoolean OsSocket::isIp4Address(const char* address)
{
    // nnn.nn.n.nnn
    //         ^==== dot3
    //       ^====== dot2
    //    ^========= dot1

    const char* dot1 = strchr(address, '.');
    UtlBoolean isIp4 = FALSE;
    if(dot1)
    {
        const char* dot2 = strchr(dot1 + 1, '.');
        if((dot2) && (dot2-dot1 > 1))
        {
            const char* dot3 = strchr(dot2 + 1, '.');
            if((dot3) && (dot3-dot2 > 1))
            {
                const char* dot4 = strchr(dot3 + 1, '.');
                if((dot4 == NULL) && strlen(dot3) > 1)
                {
                    if(INADDR_NONE != inet_addr(address))
                    {
                        isIp4 = TRUE;
                    }
                }
            }
        }
    }

    return(isIp4);
}


UtlBoolean OsSocket::isSameHost(const char* host1, const char* host2)
{
        UtlBoolean same;
        UtlBoolean isSubDomain = FALSE;

        //osPrintf("OsSocket::isSameHost host1: %s host2: %s\n", host1, host2);
        if (!isIp4Address(host1) && !isIp4Address(host2))
        {
                if( (strstr(host1, host2) == host1 || // && need to compare w/  local domain name
                        strstr(host2, host1) == host2 ))
                {
                        // && need to compare w/  local domain name
                        isSubDomain = TRUE;
                }
        }

        if(strcmp(host1, host2) == 0 ||
                (isLocalHost(host1) && isLocalHost(host2)) ||
                isSubDomain )
        {
                same = TRUE;
        }
        else
        {
                // Avoid using gethostbyname if possible
                UtlString host1Addr;
                UtlString host2Addr;
                getHostIpByName(host1, &host1Addr);
                getHostIpByName(host2, &host2Addr);
                if(host1Addr.compareTo(host2Addr) == 0)
                {
                        same = TRUE;
                }
                else
                {
                        same = FALSE;
                }
                host1Addr.remove(0);
                host2Addr.remove(0);
        }
        return(same);
}

const UtlString& OsSocket::getLocalIp() const
{
    return mLocalIp;
}

// change the ip address into the dot ip address
void OsSocket::inet_ntoa_pt(struct in_addr input_address,
                            UtlString& output_address)
{
    output_address.remove(0);
#ifdef _VXWORKS
                char temp_addr[30];
                inet_ntoa_b(input_address, temp_addr);
                output_address.append(temp_addr);
#elif defined(_WIN32) || defined(__pingtel_on_posix__)
                output_address.append(inet_ntoa(input_address));
#else
#error Unsupported target platform.
#endif
}

//:Returns TRUE if the given IpProtocolSocketType is a framed message protocol
// (that is, every read returns exactly one message), and so the Content-Length
// header may be omitted.
UtlBoolean OsSocket::isFramed(IpProtocolSocketType type)
{
   UtlBoolean r;

   switch (type)
   {
   case TCP:
   case SSL_SOCKET:
      // UNKNOWN and all other values return FALSE for safety.
   case UNKNOWN:
   default:
      r = FALSE;
      break;

   case UDP:
   case MULTICAST:
      r = TRUE;
      break;
   }

   return r;
}

//:Returns TRUE if the given IpProtocolSocketType is a relaible message protocol
// (that is, the transport mechanism will ensure delivery), so that "100 Trying"
// responses and re-sends are not needed.
UtlBoolean OsSocket::isReliable(IpProtocolSocketType type)
{
   UtlBoolean r;

   switch (type)
   {
   case TCP:
   case SSL_SOCKET:
      r = TRUE;
      break;

   case UDP:
   case MULTICAST:
      // UNKNOWN and all other values return FALSE for safety.
   case UNKNOWN:
   default:
      r = FALSE;
      break;
   }

   return r;
}

UtlContainableType OsSocket::getContainableType() const
{
   return OsSocket::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Returns TRUE if this host has a default DNS domain
UtlBoolean OsSocket::hasDefaultDnsDomain()
{
#ifdef _VXWORKS
    RESOLV_PARAMS_S resolvParams;

    resolvParamsGet(&resolvParams);
    return (strlen(resolvParams.domainName) != 0);
#else
    // For now, on non-vxWorks platforms, assume that the host has a
    // default domain name.
    return TRUE;
#endif
}

/* ============================ FUNCTIONS ================================= */

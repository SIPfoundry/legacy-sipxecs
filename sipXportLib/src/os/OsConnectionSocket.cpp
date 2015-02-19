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
#include <os/OsLogger.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

const unsigned int OsConnectionSocket::DefaultConnectionTimeoutMs = 4000;

// STATIC VARIABLE INITIALIZATIONS

const UtlContainableType OsConnectionSocket::TYPE = "OsConnectionSocket";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsConnectionSocket::OsConnectionSocket(int connectedSocketDescriptor)
  : OsSocket(OsSocket::SAFE_WRITE)
{
   Os::Logger::instance().log(FAC_KERNEL, PRI_INFO,
                 "OsConnectionSocket::_[1] (%d)",
                 connectedSocketDescriptor
                 );
   socketDescriptor = connectedSocketDescriptor;
}

OsConnectionSocket::OsConnectionSocket(const char* szLocalIp, int connectedSocketDescriptor)
  : OsSocket(OsSocket::SAFE_WRITE)
{
   Os::Logger::instance().log(FAC_KERNEL, PRI_INFO,
                 "OsConnectionSocket::_[2] (%s, %d)",
                 szLocalIp, connectedSocketDescriptor
                 );

   socketDescriptor = connectedSocketDescriptor;
   mLocalIp = szLocalIp;
}

// Determine the errno value for a connect() that is non-blocking.
#if defined(_WIN32)
   #define NONBLOCK WSAEWOULDBLOCK
#elif defined(_VXWORKS)
   #define NONBLOCK EWOULDBLOCK
#elif defined(__pingtel_on_posix__)
   #define NONBLOCK EINPROGRESS
#endif

// Constructor
OsConnectionSocket::OsConnectionSocket(int serverPort,
                                       const char* serverName,
                                       UtlBoolean blockingConnect,
                                       const char* localIp,
                                       unsigned int timeoutInMilliseconds
                                       )
   : OsSocket(OsSocket::SAFE_WRITE)
{
   int error = 0;
   UtlBoolean isIp;
   struct in_addr* serverAddr;
   struct hostent* server = NULL;
   struct sockaddr_in serverSockAddr;
   UtlString temp_output_address;

   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsConnectionSocket::_ attempt %s:%d %s timeout %d"
                 ,serverName, serverPort,
                 blockingConnect ? "BLOCKING" : "NON-BLOCKING", timeoutInMilliseconds );

   socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;

   remoteHostPort = serverPort;

   // Connect to a remote host if given
   if(! serverName || strlen(serverName) == 0)
   {
#     if defined(_VXWORKS)
      serverName = "127.0.0.1";
#     elif defined(__pingtel_on_posix__)
      unsigned long address_val = OsSocket::getDefaultBindAddress();

      if (!localIp)
      {
         if (address_val == htonl(INADDR_ANY))
         {
            serverName = "localhost";
         }
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

#     elif defined(WIN32)
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

#     else
#        error Unsupported target platform.
#     endif

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

#  if defined(_VXWORKS)
   char hostentBuf[512];
#  endif

   // Create the socket
   socketDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(socketDescriptor == OS_INVALID_SOCKET_DESCRIPTOR)
   {
      error = OsSocketGetERRNO();
      socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
      Os::Logger::instance().log(FAC_KERNEL, PRI_ERR, "OsConnectionSocket::_ 'socket' failed: %x", error);
      goto EXIT;
   }

   // connect() on a blocking socket will block forever; there is no
   // way to specify a timeout.  So if we need to block, but not forever,
   // we must make the socket non-blocking and then do a poll() to
   // block properly.
   // For a non-blocking connect, of course we need to make the socket
   // non-blocking.
   // But for a connect that is to block forever, we leave the socket
   // in blocking mode.
   if(   !blockingConnect
      || (blockingConnect && timeoutInMilliseconds > 0)
      )
   {
      makeNonblocking();
   }

   isIp = isIp4Address(serverName);
   if(!isIp)
   {
#     if defined(_WIN32) || defined(__pingtel_on_posix__)
      server = gethostbyname(serverName);
#     elif defined(_VXWORKS)
      server = resolvGetHostByName((char*) serverName,
                                   hostentBuf, sizeof(hostentBuf));
#     else
#       error Unsupported target platform.
#     endif //_VXWORKS

      if (!server)
      {
         close();
         Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                       "DNS failed to look up host: '%s'",
                       serverName);
         goto EXIT;
      }
   }

   if (!isIp)
   {
      inet_ntoa_pt(*((in_addr*) (server->h_addr)),temp_output_address);
      Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                    "OsConnectionSocket::_: connecting to host at: %s:%d",
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
   // Set remoteIpAddress.
   inet_ntoa_pt(serverSockAddr.sin_addr, mRemoteIpAddress);

   // Ask the TCP layer to connect
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsConnectionSocket::_ connect %d",
                 socketDescriptor);

   int connectReturn;
#  if defined(_WIN32) || defined(__pingtel_on_posix__)
   // If we should block forever, this connect() will do so.  For all
   // other cases, the socket has been set to non-blocking and so
   // connect() will not block. It will likely return NONBLOCK (the
   // actual value is OS-dependent, and NONBLOCK is #define'd above).
   connectReturn = connect(socketDescriptor,
                           (const struct sockaddr*) &serverSockAddr,
                           sizeof(serverSockAddr));
   // If connect() returned success, then return success to our caller.
   // Otherwise, get errno and analyze it.
   if (connectReturn != 0)
   {
      error = OsSocketGetERRNO();

      // A NONBLOCK from a non-blocking connect() requires further analysis.
      // All other errors are returned to our caller.
      if (NONBLOCK == error)
      {
         // If the connect is to block, but for a limited time, we must
         // do a poll() to block for the right amount of time.
         if (timeoutInMilliseconds > 0 )
         {
            Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsConnectionSocket::_ poll %d timeout %d msec",
                          socketDescriptor, timeoutInMilliseconds);

            struct pollfd pset[1];
            pset[0].fd      = socketDescriptor;
            pset[0].events  = POLLOUT;
            pset[0].revents = 0;
            int pollResult  = poll(pset, 1, timeoutInMilliseconds); // returns # of events
            if (1 == pollResult)
            {
               if (pset[0].revents & POLLERR)
               {
                  // some error on the socket
                  error = OsSocketGetERRNO();
               }
               else if (pset[0].revents & POLLOUT)
               {
                  // the connect has completed
                  error = 0;
               }
               else
               {
                  Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                                "OsConnectionSocket::_ unexpected return from poll(): pollResult = %d, pset[0].revents = %d",
                                pollResult, pset[0].revents);
                  // Need to set error to some non-0 value.
                  error = ETIMEDOUT;
               }
            }
            else if (0 == pollResult)
            {
               // timeout
               error = ETIMEDOUT;
            }
            else
            {
               // pollResult < 0, some other error
               error = OsSocketGetERRNO();
            }

            Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                          "OsConnectionSocket::_ after poll(), error = %d",
                          error);
         }
         else
         {
            // The connect request is non-blocking.  NONBLOCK is not
            // an error, and we should return success to our caller.
            error = 0;
         }
      }

      // In any case, all further socket operations should be blocking.
      makeBlocking();
   }
   else // connectReturn == 0
   {
      error = 0;
   }

#  elif defined(_VXWORKS)
   // This code probably doesn't handle timeouts correctly.
   connectReturn = connect(socketDescriptor,
                           (struct sockaddr*) &serverSockAddr,
                           sizeof(serverSockAddr));
   error = OsSocketGetERRNO();
   if (error == NONBLOCK &&
       !blockingConnect)
   {
      error = 0;
   }

#  else
#    error Unsupported target platform.
#  endif

   if (error != 0)
   {
      char* msgBuf;
      close();

      msgBuf = strerror(error);
      Os::Logger::instance().log(FAC_KERNEL, PRI_ERR, "OsConnectionSocket(%s:%d): call to connect() failed "
                    "with error: %d '%s'",
                    serverName, serverPort, error, msgBuf);
      // mIsConnected (set in OsSocket::_) remains FALSE.
   }
   else
   {
      Os::Logger::instance().log(FAC_KERNEL, PRI_INFO,
                    "OsConnectionSocket::_[5] connected %d to %s:%d",
                    socketDescriptor, serverName, serverPort
                    );
      mIsConnected = TRUE;
   }

  EXIT:
   // mIsConnected signals success/failure to our caller.
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
   Os::Logger::instance().log(FAC_KERNEL, PRI_INFO, "OsConnectionSocket::~");
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
   Os::Logger::instance().log(FAC_KERNEL, PRI_WARNING, "reconnect not implemented");
   return FALSE;
}

// Because we have overridden one read method, we
// must implement them all in OsConnectionSocket or
// we end up hiding some of the methods.
int OsConnectionSocket::read(char* buffer, int bufferLength)
{
    // Use base class implementation
    int bytesRead = OsSocket::read(buffer, bufferLength);
    if (!bytesRead)
    {
       int error = OsSocketGetERRNO();
       if (error != EINTR && error != EAGAIN)
       {
          char* msgBuf;
          msgBuf = strerror(error);

          Os::Logger::instance().log(FAC_KERNEL, PRI_INFO,
                        "OsConnectionSocket::read[2] error or EOF on fd %d, "
                        "errno = %d %s",
                        socketDescriptor, error, msgBuf
                        );
          close();
       }
    }

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
    if (bytesRead)
    {
       // Explicitly get the remote host info.
       getRemoteHostIp(ipAddress, port);
    }
    else
    {
       int error = OsSocketGetERRNO();
       if (error != EINTR && error != EAGAIN)
       {
          char* msgBuf;
          msgBuf = strerror(error);

          Os::Logger::instance().log(FAC_KERNEL, PRI_INFO,
                        "OsConnectionSocket::read[4] error or EOF on fd %d, "
                        "errno = %d %s",
                        socketDescriptor, error, msgBuf
                        );
          close();
       }
    }

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
    if (!bytesRead)
    {
       int error = OsSocketGetERRNO();
       if (error != EINTR && error != EAGAIN)
       {
          char* msgBuf;
          msgBuf = strerror(error);

          Os::Logger::instance().log(FAC_KERNEL, PRI_INFO,
                        "OsConnectionSocket::read[3] error or EOF on fd %d, "
                        "errno = %d %s",
                        socketDescriptor, error, msgBuf
                        );
          close();
       }
    }

    return(bytesRead);
}

/* ============================ ACCESSORS ================================= */
OsSocket::IpProtocolSocketType OsConnectionSocket::getIpProtocol() const
{
        return(TCP);
}
/* ============================ INQUIRY =================================== */

UtlContainableType OsConnectionSocket::getContainableType() const
{
   return OsConnectionSocket::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

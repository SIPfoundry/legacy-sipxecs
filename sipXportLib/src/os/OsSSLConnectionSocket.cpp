//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

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
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netdb.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#else
#error Unsupported target platform.
#endif

// APPLICATION INCLUDES

//SSL related includes
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "utl/UtlSListIterator.h"
#include "os/OsSSL.h"
#include "os/OsSSLConnectionSocket.h"
#include "os/OsUtil.h"
#include "os/OsLogger.h"
#include "os/OsTask.h"

#ifdef VALGRIND_MAKE_READABLE
#include "valgrind/memcheck.h"
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsSSLConnectionSocket::OsSSLConnectionSocket(int connectedSocketDescriptor,
                                             long timeoutInSecs) :
   OsConnectionSocket(NULL, connectedSocketDescriptor),
   mSSL(NULL),
   mPeerIdentity(NOT_IDENTIFIED)
{
   mbExternalSSLSocket = FALSE;
   
   if (mIsConnected)
   {
      Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                    "OsSSLConnectionSocket::_(socket %d , timeout %ld)",
                    connectedSocketDescriptor, timeoutInSecs
                    );
      SSLInitSocket(connectedSocketDescriptor, timeoutInSecs);
   }
   else
   {
      Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                    "OsSSLConnectionSocket::_ underlying OsConnectionSocket not connected!!");
   }
}

OsSSLConnectionSocket::OsSSLConnectionSocket(SSL *s, int connectedSocketDescriptor, const char* localIp) :
   OsConnectionSocket(localIp, connectedSocketDescriptor),
   mSSL(s),
   mPeerIdentity(NOT_IDENTIFIED)
{
    mbExternalSSLSocket = TRUE;
    socketDescriptor = connectedSocketDescriptor;
    Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                  "OsSSLConnectionSocket::_(SSL %p, socket %d)", s, connectedSocketDescriptor);
}

// Constructor
OsSSLConnectionSocket::OsSSLConnectionSocket(int serverPort, const char* serverName,
                                             long timeoutInSecs) :
   OsConnectionSocket(serverPort,serverName,TRUE /* BLOCKING */,
                      NULL /* local address == INADDR_ANY */,
                      timeoutInSecs * OsTime::MSECS_PER_SEC ),
   mSSL(NULL),
   mPeerIdentity(NOT_IDENTIFIED)
{
    mbExternalSSLSocket = FALSE;
    if (mIsConnected)
    {
       Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                     "OsSSLConnectionSocket::_(port %d, name '%s', timeout %ld)",
                     serverPort, serverName, timeoutInSecs
                     );
       SSLInitSocket(socketDescriptor, timeoutInSecs);
    }
}


// Destructor
OsSSLConnectionSocket::~OsSSLConnectionSocket()
{
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                 "OsSSLConnectionSocket::~"
                 );
   remoteHostName = OsUtil::NULL_OS_STRING;
   mAltNames.destroyAll();
   close();
}

/* ============================ MANIPULATORS ============================== */


UtlBoolean OsSSLConnectionSocket::reconnect()
{
    return FALSE; //TODO
}

void OsSSLConnectionSocket::close()
{
   Os::Logger::instance().log(FAC_KERNEL, PRI_INFO, "OsSSLConnectionSocket::close");

    if (mSSL)
    {
       if (mIsConnected)
          SSL_shutdown(mSSL);
       else
          SSL_clear(mSSL);
    }

   /* Clean up. */
    OsConnectionSocket::close();

   if (mSSL)
   {
      SSL_free(mSSL);
      ERR_remove_state(0);
      mSSL = NULL;
   }
}

int OsSSLConnectionSocket::write(const char* buffer, int bufferLength)
{
    int bytesWritten = -1;

    bytesWritten = SSL_write (mSSL, buffer, bufferLength);

    return(bytesWritten);
}

int OsSSLConnectionSocket::write(const char* buffer, int bufferLength, long waitMilliseconds)
{
    int bytesWritten = -1;
    if(isReadyToWrite(waitMilliseconds))
    {
        bytesWritten = SSL_write (mSSL, buffer, bufferLength);
    }

    return(bytesWritten);
}

int OsSSLConnectionSocket::read(char* buffer, int bufferLength)
{
    // Use base class implementation
    int bytesRead = -1;

    bytesRead = SSL_read (mSSL, buffer, bufferLength);

#ifdef VALGRIND_MAKE_READABLE
    // If we are using Valgrind, we have to compensate for the fact that
    // Valgrind thinks all the output of SSL_read is undefined.
    // (This probably comes from SSL's use of uninitialized memory as part
    // of its key-generation algorithm.)
    VALGRIND_DISCARD(VALGRIND_MAKE_READABLE(&bytesRead, sizeof (bytesRead)));
    if (bytesRead > 0)
    {
       VALGRIND_DISCARD(VALGRIND_MAKE_READABLE(buffer, bytesRead));
    }
#endif /* VALGRIND_MAKE_READABLE */

    return(bytesRead);
}

int OsSSLConnectionSocket::read(char* buffer,
                             int bufferLength,
                             UtlString* ipAddress,
                             int* port)
{
    // Overide base class version as recvfrom does not
    // seem to return host info correctly for TCP
    // Use base class version without the remote host info
    int bytesRead = -1;

    bytesRead = SSL_read (mSSL, buffer, bufferLength);

#ifdef VALGRIND_MAKE_READABLE
    // If we are using Valgrind, we have to compensate for the fact that
    // Valgrind thinks all the output of SSL_read is undefined.
    // (This probably comes from SSL's use of uninitialized memory as part
    // of its key-generation algorithm.)
    VALGRIND_DISCARD(VALGRIND_MAKE_READABLE(&bytesRead, sizeof (bytesRead)));
    if (bytesRead > 0)
    {
       VALGRIND_DISCARD(VALGRIND_MAKE_READABLE(buffer, bytesRead));
    }
#endif /* VALGRIND_MAKE_READABLE */
    // Explicitly get the remote host info.
    getRemoteHostIp(ipAddress);
    *port = getRemoteHostPort();
    return(bytesRead);
}

// Because we have overided one read method, we
// must implement them all in OsSSLConnectionSocket or
// we end up hiding some of the methods.
int OsSSLConnectionSocket::read(char* buffer,
                            int bufferLength,
                            long waitMilliseconds)
{

    int bytesRead = -1;
    if(isReadyToRead(waitMilliseconds))
    {
        bytesRead = SSL_read (mSSL, buffer, bufferLength);
    }
#ifdef VALGRIND_MAKE_READABLE
    // If we are using Valgrind, we have to compensate for the fact that
    // Valgrind thinks all the output of SSL_read is undefined.
    // (This probably comes from SSL's use of uninitialized memory as part
    // of its key-generation algorithm.)
    VALGRIND_DISCARD(VALGRIND_MAKE_READABLE(&bytesRead, sizeof (bytesRead)));
    if (bytesRead > 0)
    {
       VALGRIND_DISCARD(VALGRIND_MAKE_READABLE(buffer, bytesRead));
    }
#endif /* VALGRIND_MAKE_READABLE */
    return(bytesRead);
}

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
OsSocket::IpProtocolSocketType OsSSLConnectionSocket::getIpProtocol() const
{
    return(OsSocket::SSL_SOCKET);
}

/// Is this connection encrypted using TLS/SSL?
bool OsSSLConnectionSocket::isEncrypted() const
{
   return true;
}


/// Get any authenticated peer host names.
bool OsSSLConnectionSocket::peerIdentity( UtlSList* altNames
                                         ,UtlString* commonName
                                         ) const
{
   if (NOT_IDENTIFIED == mPeerIdentity)
   {
      // examine cert to get identity and cache the results
      mPeerIdentity = OsSSL::peerIdentity( mSSL, &mAltNames, &mCommonName )
         ? TRUSTED : UNTRUSTED;
      if ( TRUSTED == mPeerIdentity )
      {
         Os::Logger::instance().log(FAC_SIP, PRI_INFO,
                       "OsSSLConnectionSocket::peerIdentity %p OsSSL returned trusted",
                       this);
      }
      else
      {
         Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                       "OsSSLConnectionSocket::peerIdentity %p OsSSL returned NOT trusted",
                       this);
      }
   }

   /*
    * - true if the connection is TLS/SSL and the peer has presented
    *        a certificate signed by a trusted certificate authority
    * - false if not
    */
   if (commonName)
   {
      commonName->remove(0);
   }
   if (altNames)
   {
      altNames->destroyAll();
   }

   if (TRUSTED == mPeerIdentity)
   {
      if (commonName)
      {
         *commonName = mCommonName;
      }
      if (altNames)
      {
         UtlString* altName;
         UtlSListIterator names(mAltNames);
         while ( (altName = dynamic_cast<UtlString*>(names())) )
         {
            altNames->insert(new UtlString(*altName));
         }
      }
   }

   return TRUSTED == mPeerIdentity;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


void OsSSLConnectionSocket::SSLInitSocket(int socket, long timeoutInSecs)
{
    if (mIsConnected)
    {
       int err = -1;

       // TODO: eventually this should allow for other SSL contexts...
       mSSL = OsSharedSSL::get()->getServerConnection();

       if (mSSL && (socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR))
       {
          SSL_set_fd (mSSL, socketDescriptor);

          err = SSL_connect(mSSL);
          if (err > 0)
          {
             OsSSL::logConnectParams(FAC_KERNEL, PRI_DEBUG,
                                     "OsSSLConnectionSocket::SSLInitSocket",
                                     mSSL);
          }
          else
          {
             OsSSL::logError(FAC_KERNEL, PRI_ERR,
                             "OsSSLConnectionSocket SSL_connect failed: ",
                             SSL_get_error(mSSL, err));
             mIsConnected = FALSE;
             OsConnectionSocket::close();
             socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
          }
       }
       else
       {
          Os::Logger::instance().log(FAC_KERNEL, PRI_CRIT
                        , "OsSSLConnectionSocket bad parameters mSSL=%p,%d, closing socket..."
                        , mSSL, socketDescriptor);
          mIsConnected = FALSE;
          OsConnectionSocket::close();
          socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
       }
    }
    else
    {
       Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                     "OsSSLConnectionSocket::SSLInitSocket called on unconnected socket"
                     );
    }
}
/* ============================ FUNCTIONS ================================= */

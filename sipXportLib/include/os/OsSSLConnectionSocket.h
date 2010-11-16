//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsSSLConnectionSocket_h_
#define _OsSSLConnectionSocket_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlSList.h>
#include <utl/UtlString.h>
#include <os/OsConnectionSocket.h>

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Implements TLS version of OsSocket
class OsSSLConnectionSocket : public OsConnectionSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsSSLConnectionSocket(int         remoteHostPort,
                         const char* remoteHostName,
                         long        timeoutInSecs = 0
                         );

   OsSSLConnectionSocket(int connectedSocketDescriptor,
                         long timeoutInSecs = 0);

   OsSSLConnectionSocket(SSL *s, int connectedSocketDescriptor, const char* localIp);

  virtual
   ~OsSSLConnectionSocket();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean reconnect();
   //: Sets up the connection again, assuming the connection failed

   virtual int write(const char* buffer, int bufferLength);
   //:Blocking write to the socket
   // Write the characters in the given buffer to the socket.
   // This method will block until all of the bytes are written.
   //!param: buffer - The bytes to be written to the socket.
   //!param: bufferLength - The number of bytes contained in buffer.
   //!returns: The number of bytes actually written to the socket.
   //!returns: <br>Note: This does not necessarily mean that the bytes were
   //!returns: actually received on the other end.

   virtual int write(const char* buffer, int bufferLength, long waitMilliseconds);
   //:Non-blocking or limited blocking write to socket
   // Same as blocking version except that this write will block
   // for no more than the specified length of time.
   //!param: waitMilliseconds - The maximum number of milliseconds to block. This may be set to zero, in which case it does not block.

   virtual int read(char* buffer, int bufferLength);
   //:Blocking read from the socket
   // Read bytes into the buffer from the socket up to a maximum of
   // bufferLength bytes.  This method will block until there is
   // something to read from the socket.
   //!param: buffer - Place to put bytes read from the socket.
   //!param: bufferLength - The maximum number of bytes buffer will hold.
   //!returns: The number of bytes actually read.

   virtual int read(char* buffer, int bufferLength,
                    UtlString* ipAddress, int* port);
   //:Blocking read from the socket
   // Read bytes into the buffer from the socket up to a maximum of
   // bufferLength bytes.  This method will block until there is
   // something to read from the socket.
   //!param: buffer - Place to put bytes read from the socket.
   //!param: bufferLength - The maximum number of bytes buffer will hold.
   //!param: ipAddress - The address of the socket that sent the bytes read.
   //!param: port - The port of the socket that sent the bytes read.
   //!returns: The number of bytes actually read.

   virtual int read(char* buffer, int bufferLength, long waitMilliseconds);
   //: Non-blocking or limited blocking read from socket
   // Same as blocking version except that this read will block
   // for no more than the specified length of time.
   //!param: waitMilliseconds - The maximum number of milliseconds to block. This may be set to zero in which case it does not block.

/* ============================ ACCESSORS ================================= */

   virtual void close();
   //: Closes the SSL socket

/* ============================ INQUIRY =================================== */

   virtual OsSocket::IpProtocolSocketType getIpProtocol() const;
   //: Returns the protocol type of this socket

   /// Is this connection encrypted using TLS/SSL?
   virtual bool isEncrypted() const;

   /// Get any authenticated peer host names.
   virtual bool peerIdentity( UtlSList* altNames /**< UtlStrings for verfied subjectAltNames
                                                  *   are added to this - caller must free them.
                                                  */
                             ,UtlString* commonName ///< the Subject name is returned here
                             ) const;
   /**<
    * Usually, the names in the altNames will be easier to parse and use than commonName
    * Either or both of altNames or commonName may be NULL, in which case no names are returned;
    * the return value still indicates the trust relationship with the peer certificate.
    * @returns
    * - true if the connection is TLS/SSL and the peer has presented
    *        a certificate signed by a trusted certificate authority
    * - false if not
    */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SSL*     mSSL;

   // cached copies of peer information - parsing the cert is expensive
   mutable enum
   {
      NOT_IDENTIFIED,
      TRUSTED,
      UNTRUSTED
   }         mPeerIdentity;
   mutable UtlSList  mAltNames;
   mutable UtlString mCommonName;

   UtlBoolean mbExternalSSLSocket;
     //:Should this object clean up SSL when shutdown.
     //:It shouldn't if SSL is managed by a parent class
   void SSLInitSocket(int socket, long timeoutInSecs);

   OsSSLConnectionSocket(const OsSSLConnectionSocket& rOsSSLConnectionSocket);
     //:Disable copy constructor

   OsSSLConnectionSocket();
     //:Disable default constructor

   OsSSLConnectionSocket& operator=(const OsSSLConnectionSocket& rhs);
     //:Disable Assignment operator
};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsSSLConnectionSocket_h_

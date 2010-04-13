//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsSSLServerSocket_h_
#define _OsSSLServerSocket_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsConnectionSocket.h>
#include <os/OsServerSocket.h>
#include <os/OsSSLConnectionSocket.h>
#include <os/OsFS.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Implements Server for accepting TLS/SSL connections
class OsSSLServerSocket : public OsServerSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /// Constructor to set up TCP socket server
   OsSSLServerSocket(int connectionQueueSize, /**< The maximum number of outstanding connection
                                               *   requests which are allowed before subsequent
                                               *   requests are turned away.*/
                     int serverPort=PORT_DEFAULT, /**< The port on which the server will listen to
                                                   *   accept connection requests.
                                                   *   PORT_DEFAULT means let OS pick port. */
                     const char* szBindAddr=NULL /**< Default bind-to address */
                     );
   /**
    * Sets the socket connection queue and starts listening on the
    * port for requests to connect.
    */

   /// Assignment operator
   OsSSLServerSocket& operator=(const OsSSLServerSocket& rhs);

  virtual
   ~OsSSLServerSocket();
     //:Destructor


/* ============================ MANIPULATORS ============================== */

   virtual OsConnectionSocket* accept();
   //:Blocking accept of next connection
   // Blocks and waits for the next TCP connection request.
   //!returns: Returns a socket connected to the client requesting the
   //!returns: connection.  If an error occurs returns NULL.


   void close();
   //: Close down the server

/* ============================ ACCESSORS ================================= */

   virtual int getLocalHostPort() const;
   //:Return the local port number
   // Returns the port to which this socket is bound on this host.

/* ============================ INQUIRY =================================== */

   virtual OsSocket::IpProtocolSocketType getIpProtocol() const;
   //: Returns the protocol type of this socket

   virtual UtlBoolean isOk() const;
   //: Server socket is in ready to accept incoming conection requests.

   int isConnectionReady();
   //: Poll to see if connections are waiting
   //!returns: 1 if one or call to accept() will not block <br>
   //!returns: 0 if no connections are ready (i.e. accept() will block).

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   static int pemPasswdCallbackFunc(char *buf, int size, int rwflag, void *userdata);
    //:Callback used to set the private key password for decrypting the key
    //:buf is the buffer to fill with a password
    //:size is the maximum size of the buffer

   OsSSLServerSocket(const OsSSLServerSocket& rOsSSLServerSocket);
     //:Disable copy constructor

   OsSSLServerSocket();
     //:Disable default constructor

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsSSLServerSocket_h_

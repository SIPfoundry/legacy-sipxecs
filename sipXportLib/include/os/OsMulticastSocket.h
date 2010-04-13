//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsMulticastSocket_h_
#define _OsMulticastSocket_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsSocket.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Implements UDP version of OsSocket
// This class provides the implementation of the UDP datagram
// based socket class which may be instantiated.

class OsMulticastSocket : public OsSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// Determine whether or not the values in a containable are comparable.
   virtual UtlContainableType getContainableType() const;
   /**<
    * This returns a unique type for this class.
    */

/* ============================ CREATORS ================================== */

   OsMulticastSocket(int multicastPort = PORT_DEFAULT,
                     const char* multicastHostName = NULL,
                     int localHostPort = PORT_DEFAULT,
                     const char* localHostName = NULL);

  virtual
   ~OsMulticastSocket();
     //:Destructor


/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean reconnect();
   //: Sets up the connection again, assuming the connection failed

   void joinMulticast(int multicastPort, const char* multicastHostName);

   virtual int read(char* buffer, int bufferLength);
   //: Blocking read from the socket
   // Read bytes into the buffer from the socket up to a maximum of
   // bufferLength bytes.  This method will block until there is
   // something to read from the socket.
   //! param: buffer - Place to put bytes read from the socket.
   //! param: bufferLength - The maximum number of bytes buffer will hold.
   //! returns: The number of bytes actually read.

/* ============================ ACCESSORS ================================= */
   virtual OsSocket::IpProtocolSocketType getIpProtocol() const;
   //: Returns the protocol type of this socket


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   static const UtlContainableType TYPE;    ///< Class type used for runtime checking

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsMulticastSocket(const OsMulticastSocket& rOsMulticastSocket);
     //:Disable copy constructor

   OsMulticastSocket& operator=(const OsMulticastSocket& rhs);
     //:Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsMulticastSocket_h_

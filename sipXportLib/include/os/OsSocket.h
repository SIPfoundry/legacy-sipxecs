//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsSocket_h_
#define _OsSocket_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "utl/UtlContainableAtomic.h"
#include "utl/UtlString.h"
#include "os/OsBSem.h"


// DEFINES
#define MAX_IP_ADDRESSES 32

//: constant indentifier that cannot be a valid socket descriptor
#define OS_INVALID_SOCKET_DESCRIPTOR (-1)

#if defined(_WIN32)
#   include <os/wnt/getWindowsDNSServers.h>
#   include "os/wnt/WindowsAdapterInfo.h"
#  define OsSocketGetERRNO() (WSAGetLastError())
#  define OS_INVALID_INET_ADDRESS INADDR_NONE // 0xffffffff
#elif defined(_VXWORKS)
#  define OsSocketGetERRNO() (errno)
#  define OS_INVALID_INET_ADDRESS 0xffffffff
#elif defined(__pingtel_on_posix__)
#  include "os/linux/AdapterInfo.h"
#  define OsSocketGetERRNO() (errno)
#  define OS_INVALID_INET_ADDRESS 0xffffffff
#else
#  error Unsupported target platform.
#endif

// MACROS
// EXTERNAL FUNCTIONS

//used to access BindAddress from "C" code
extern "C" unsigned long osSocketGetDefaultBindAddress();

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

/** Type for storing Contact Record identifiers */
typedef int ContactId;

/**
 * The ContactAddress class includes contact information (ip and port),
 * address source type, and interface.
 */
class ContactAddress
{
  public:

    /**
     * ContactType is an enumeration of possible address types for use with
     * SIP contacts and SDP connection information.
     */
    enum ContactType_e
    {
        LOCAL,      /**< Local address for a particular interface */
        NAT_MAPPED, /**< NAT mapped address (e.g. STUN)           */
        RELAY,      /**< Relay address (e.g. TURN)                */
        CONFIG,     /**< Manually configured address              */

        AUTO = -1,  /**< Automatic contact selection; used for API
                         parameters */
        ALL  = -2   /**< Filter value for the SipContactDb, for looking
                         up records of all types. */
    };

    ContactAddress()
    {
        memset((void*)cInterface, 0, sizeof(cInterface));
        memset((void*)cIpAddress, 0, sizeof(cIpAddress));
        eContactType = AUTO;
        id = 0;
        iPort = -1;
    }

    // copy contstructor
    ContactAddress(const ContactAddress& ref)
    {
        strcpy(this->cInterface, ref.cInterface);
        strcpy(this->cIpAddress, ref.cIpAddress);
        this->eContactType = ref.eContactType;
        this->id = ref.id;
        this->iPort = ref.iPort;
    }

    // assignment operator
    ContactAddress& operator=(const ContactAddress& ref)
    {
        // check for assignment to self
        if (this == &ref) return *this;

        strcpy(this->cInterface, ref.cInterface);
        strcpy(this->cIpAddress, ref.cIpAddress);
        this->eContactType = ref.eContactType;
        this->id = ref.id;
        this->iPort = ref.iPort;

        return *this;
    }

    ContactId          id;              /**< Contact record Id */
    enum ContactType_e eContactType;    /**< Address type/source */
    char               cInterface[32];  /**< Source interface    */
    char               cIpAddress[32];  /**< IP Address          */
    int                iPort;           /**< Port                */
};

/**
 * ContactType is an enumeration of possible address types for use with
 * SIP contacts and SDP connection information.
 */
typedef enum ContactAddress::ContactType_e ContactType;


// TYPEDEFS

// FORWARD DECLARATIONS

//: Abstract Socket class
// This class encapsulates the Berkley socket in an object oriented,
// platform independent way.  The intention is also to be independent of
// the protocol over IP as much as is possible.  This should enable
// generic message transport with minimal knowledge of the underlying
// protocol.

class OsSocket : public UtlContainableAtomic
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static UtlBoolean socketInitialized;

   /// Determine whether or not the values in a containable are comparable.
   virtual UtlContainableType getContainableType() const;
   /**<
    * This returns a unique type for this class.
    */

   typedef enum
   {
      UNKNOWN = -1,
      TCP = 0,
      UDP = 1,
      MULTICAST = 2,
      SSL_SOCKET = 3
   } IpProtocolSocketType;
   //: Protocol Types
   //  Note: If you add a value to this enum, add a case in OsSocket::isFramed
   //  and OsSocket::isReliable.

/* ============================ CREATORS ================================== */
   OsSocket();
     //:Default constructor

   virtual
   ~OsSocket();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   static UtlBoolean socketInit();
   static unsigned long initDefaultAdapterID(UtlString &adapter_id);

   virtual int write(const char* buffer, int bufferLength);
   //:Blocking write to the socket
   // Write the characters in the given buffer to the socket.
   // This method will block until all of the bytes are written.
   //!param: buffer - The bytes to be written to the socket.
   //!param: bufferLength - The number of bytes contained in buffer.
   //!returns: The number of bytes actually written to the socket.
   //!returns: <br>Note: This does not necessarily mean that the bytes were
   //!returns: actually received on the other end.
   //!returns: -1 is returned to indicate error, and errno is set.

   virtual int write(const char* buffer, int bufferLength, long waitMilliseconds);
   //:Non-blocking or limited blocking write to socket
   // Same as blocking version except that this write will block
   // for no more than the specified length of time.
   //!param: waitMilliseconds - The maximum number of milliseconds to block.  This may be set to zero, in which case it does not block.

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

   virtual int read(char* buffer, int bufferLength,
       struct in_addr* ipAddress, int* port);
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

   virtual void close();
   //: Closes the socket

   virtual void makeNonblocking();
   virtual void makeBlocking();
   //: Make the connect and all subsequent operations blocking
   // By default the sockets are blocking.

   static void setDefaultBindAddress(const unsigned long bind_address);
   //set the default ipaddress the phone should bind to

/* ============================ ACCESSORS ================================= */

   virtual OsSocket::IpProtocolSocketType getIpProtocol() const = 0;
   //:Return the protocol type of this socket

   /// return the string representation of the SocketProtocolType
   static const char* ipProtocolString(OsSocket::IpProtocolSocketType);

   virtual UtlBoolean reconnect() = 0;
   //:Set up the connection again, assuming the connection failed

   int getSocketDescriptor() const;
   //:Return the socket descriptor
   // Warning: Use of this method risks the creation of platform-dependent
   // code.


   static void getDomainName(UtlString &domain_name);
        //gets static member m_DomainName

   static unsigned long getDefaultBindAddress();
   // get default ip address in network byte order

   static void getHostName(UtlString* hostName);
   //:Get this host's name
   // Gets the host name independent of a socket.

   static void getHostIp(UtlString* hostAddress);
   //:Get this host's IP address

   void getLocalHostName(UtlString* localHostName) const;
   //:Return this host's name
   // Returns a string containing the name of the host on which this socket
   // resides.  This may be the local name, a fully qualified domain name or
   // anything in between. This name may vary on the same host if it is
   // multi-homed, depending upon which NIC the Socket is associated with.
   // Not fully implemented.

   void getLocalHostIp(UtlString* localHostAddress) const;
   //:Return this host's ip address
   // Returns the ip address for this host on which this socket is communicating
   // On multi-homed machines, this is the address to the NIC over which the
   // socket communicates. The format is of the form: xx.x.xxx.x

   const UtlString& getLocalIp() const;
   //:Return this socket's Local Ip Address

   void setLocalIp(const UtlString& localIp) { mLocalIp = localIp; }

   virtual int getLocalHostPort() const;
   //:Return the local port number
   // Returns the port to which this socket is bound on this host.

   virtual void getRemoteHostName(UtlString* remoteHostName) const;
   //:Return remote host name
   // Returns a string containing the name of the host on which the socket
   // on the other end of this socket is bound. This may be the local
   // name, a fully qualified domain name or anything in between.


   virtual void getRemoteHostIp(struct in_addr* remoteHostAddress,
                                int* remotePort = NULL);
   //:Return remote host ip address
   // Returns the ip address for the host on which the socket on the
   // other end of this socket is bound.
   // If the socket is in the process of connecting, may return
   // "0.0.0.0" and 0.

   virtual void getRemoteHostIp(UtlString* remoteHostAddress,
                                int* remotePort = NULL);
   //:Return remote host ip address
   // Returns the ip address for the host on which the socket on the
   // other end of this socket is bound.
   // If the socket is in the process of connecting, will return the
   // address and port recorded from the connect attempt.

   virtual int getRemoteHostPort() const;
   //:Return the remote port number
   // Returns the port to which the socket on the other end of this socket
   // is bound.

/* ============================ INQUIRY =================================== */

   virtual UtlBoolean isOk() const;
   //:Returns TRUE if this socket's descriptor is not the invalid descriptor

   virtual UtlBoolean isConnected() const;
   //:Returns TRUE if this socket is connected

   virtual UtlBoolean isReadyToReadEx(long waitMilliseconds, UtlBoolean &rSocketError) const;
   //:Poll if there are bytes to read
   // Returns TRUE if socket is read to read.
   // Returns FALSE if wait expires or socket error.
   // rSocketError returns TRUE is socket error occurred.

   virtual UtlBoolean isReadyToRead(long waitMilliseconds = 0) const;
   //:Poll if there are bytes to read
   // Returns TRUE if socket is ready to read.
   // Returns FALSE if wait expires or socket error.

   virtual UtlBoolean isReadyToWrite(long waitMilliseconds = 0) const;
   //:Poll if socket is able to write without blocking

   static UtlBoolean isIp4Address(const char* address);
   //:Is the address a dotted IP4 address
   // (i.e., nnn.nnn.nnn.nnn where 0 <= nnn <= 255)

   static UtlBoolean isLocalHost(const char* hostAddress);
   //:Is the given host name this host

   static UtlBoolean isSameHost(const char* host1, const char* host2);
   //:Are these two host names/addresses equivalent

   static UtlBoolean getHostIpByName(const char* hostName, UtlString* hostAddress);
   //:Look up IP address for host name

   static void inet_ntoa_pt(struct in_addr input_address, UtlString& output_address);
   //:Convert in_addr input_address to dot ip address to avoid memory leak

   static UtlBoolean isFramed(IpProtocolSocketType type);
   //:Returns TRUE if the given IpProtocolSocketType is a framed message protocol
   // (that is, every read returns exactly one message), and so the Content-Length
   // header may be omitted.

   static UtlBoolean isReliable(IpProtocolSocketType type);
   //:Returns TRUE if the given IpProtocolSocketType is a relaible message protocol
   // (that is, the transport mechanism will ensure delivery), so that "100 Trying"
   // responses and re-sends are not needed.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   static const UtlContainableType TYPE;    ///< Class type used for runtime checking
   static OsBSem mInitializeSem;
   int socketDescriptor;

   /// Local port number.
   int localHostPort;
   /// Remote port number.
   int remoteHostPort;
   /// The local IP address used by this socket.
   UtlString mLocalIp;
   /// The name of the local end, if it was set by the constructor.
   // Not fully implemented.
   UtlString localHostName;
   /// The name of the remote end.  Null if this is an un-connected socket.
   UtlString remoteHostName;
   /// The IP address of the remote end.  Not set for un-connected sockets.
   UtlString mRemoteIpAddress;
   /// TRUE if the socket is connected to a particular remote end.
   UtlBoolean mIsConnected;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static unsigned long m_DefaultBindAddress;
        //default ip address the phone should bind to. May be IPADDR_ANY

        static UtlString m_DomainName;
        //domain name for host machine

   OsSocket& operator=(const OsSocket& rhs);
     //:Disable assignment operator

   OsSocket(const OsSocket& rOsSocket);
     //:Disable copy constructor

   static UtlBoolean hasDefaultDnsDomain();
     //:Returns TRUE if this host has a default DNS domain

   int mActual_socketDescriptor;
     //:Holds the descriptor from the OS to be sent to ::close() ONLY during
     //destruction.  This prevents the OS from re-using the old descriptor
     //number before the lifetime of this object is complete.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsSocket_h_

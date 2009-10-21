//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipClient_h_
#define _SipClient_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsSocket.h>
#include <os/OsTask.h>
#include <os/OsServerTaskWaitable.h>
#include <os/OsBSem.h>
#include <net/SipMessage.h>
#include <utl/UtlContainableAtomic.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipProtocolServerBase;
class SipUserAgentBase;
class OsEvent;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipClient : public OsServerTaskWaitable, public UtlContainableAtomic
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipClient(OsSocket* socket,
             SipProtocolServerBase* pSipServer,
             SipUserAgentBase* sipUA,
             const char* taskNameString,
             UtlBoolean bIsSharedSocket);
     //:Default constructor

   virtual ~SipClient();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   /// Handles an incoming message (from the message queue).
   virtual UtlBoolean handleMessage(OsMsg& rMsg);

   // Queue a message to be sent to the specified address and port.
   // Returns true if message was successfully queued.
   UtlBoolean sendTo(SipMessage& message,
                     const char* address,
                     int port);

   /// Send a message.  Executed by the thread.
   // Will only be called when mWriteQueued is false.
   virtual void sendMessage(const SipMessage& message,
                            const char* address,
                            int port
                            ///< port to send to; must not be PORT_NONE
      ) = 0;

   // Continue sending stored message content (because the socket
   // is now writable).
   virtual void writeMore(void);

   // Remove and report(if requested) all stored message content (because the socket
   // is not usable).
   virtual void emptyBuffer(bool reportError);

   UtlBoolean isSharedSocket( void ) const;

   void touch();
   //: Set the time when this client was last used
   // This is potentially used for garbage collection

/* ============================ ACCESSORS ================================= */

    void getClientNames(UtlString& clientNames) const;

    long getLastTouchedTime(void) const;

/* ============================ INQUIRY =================================== */

    UtlBoolean isOk(void);

   // Used to identify if the socket associated with SIP Client is bound to the specified
   // local IP and suitable for sending to the supplied destination host and port.
    UtlBoolean isAcceptableForDestination( const UtlString& hostName, int hostPort, const UtlString& localIp );

    const UtlString& getLocalIp(void);

    // Return the default port for the protocol of this SipClient.
    virtual int defaultPort(void) const = 0;

    virtual UtlContainableType getContainableType(void) const
    {
       return SipClient::TYPE;
    };

    /** Class type used for runtime checking */
    static const UtlContainableType TYPE;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /// The entry point for the task.
    virtual int run(void* pArg);

    /** Do preliminary processing of a message read from the socket:
     *  log it, clean up its data, and extract any needed source address.
     */
    void preprocessMessage(SipMessage& msg,
                           const UtlString& msgText,
                           int msgLength);

    /// Test whether the socket is ready to read.  (Does not block.)
    UtlBoolean isReadyToRead();
    /// Wait until the socket is ready to read (or has an error).
    UtlBoolean waitForReadyToRead();

    /** Called by the thread to shut the SipClient down and signal its
     *  owning server that it has done so.
     */
    void clientStopSelf();

    OsSocket* mClientSocket;
    OsSocket::IpProtocolSocketType mSocketType;
    SipUserAgentBase* mpSipUserAgent;
    SipProtocolServerBase* mpSipServer;

    /** The name of the remote end of mClientSocket, as obtained from
     *  mClientSocket.  Null if mClientSocket is un-connected.
     */
    UtlString mRemoteHostName;
    /** The address/port of the remote end of mClientSocket, as obtained
     *  from mClientSocket, which obtains it from the network layer.
     */
    UtlString mRemoteSocketAddress;
    int mRemoteHostPort;

    /** The address/port of the remote end of mClientSocket, as obtained
     *  from the Via of an incoming message.  Note that this cannot
     *  be trusted unless the connection is authenticated.
     */
    UtlString mRemoteViaAddress;
    int mRemoteViaPort;

    /** The "send address" extracted from an incoming message.
     *  (Which obtained it from the network layer regarding the socket the
     *  message was read from.)
     */
    UtlString mReceivedAddress;
    int mRemoteReceivedPort;

    long touchedTime;
    OsBSem mSocketLock;
    int mFirstResendTimeoutMs;

    /** Is this a shared socket?
     *  If true, do not delete or close it.
     *  If false, close it when SipClient terminates.
     */
    UtlBoolean mbSharedSocket;

    /** Is there buffered outgoing data waiting for the socket to be writable?
     *  If true, wait on socket writable.
     *  If false, wait on outgoing SIP message in queue.
     */
    UtlBoolean mWriteQueued;

    /**
     *  On non-blocking connect failures, we need to get the first
     *  send message in order to successfully trigger the protocol
     *  fallback mechanism.
     *  TRUE from startup until first successful write on a TCP
     *  socket, then false forever.
     */
    UtlBoolean mbTcpOnErrWaitForSend;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    SipClient(const SipClient& rSipClient);
     //:disable Copy constructor

    SipClient& operator=(const SipClient& rhs);
     //:disable Assignment operator

};

// Special message type for use by SipClient.
// Contains a SipMessage, a destination address string, and a port number.
// Owns the SipMessage and address string.

class SipClientSendMsg : public OsMsg
{
public:

   enum EventSubTypes
   {
       SIP_CLIENT_SEND = 1,
       SIP_CLIENT_SEND_KEEP_ALIVE = 2
   };

   SipClientSendMsg(const unsigned char msgType, const unsigned char msgSubType,
                    const SipMessage& message, const char* address, int port);
     //:Constructor
     // Copies 'message'.

   SipClientSendMsg(const unsigned char msgType, const unsigned char msgSubType,
                    const char* address, int port);
     //:Constructor for Keep Alive with no actual message

   SipClientSendMsg(const SipClientSendMsg& rOsMsg);
     //:Copy constructor

   ~SipClientSendMsg();
     //:Destructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   SipClientSendMsg& operator=(const SipClientSendMsg& rhs);
     //:Assignment operator

   /// Return the SipMessage component, and NULL the SipMessage component,
   /// so the SipClientSendMsg no longer owns it.
   SipMessage* detachMessage();

   // Component accessors.
   const SipMessage* getMessage() const;
   const char* getAddress() const;
   int getPort() const;

protected:
   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */
   SipMessage* mpMessage;
   char* mAddress;
   int mPort;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipClient_h_

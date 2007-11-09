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
             const char* taskNameString);
     //:Default constructor

   virtual ~SipClient();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   /// Handles an incoming message (from the message queue).
   virtual UtlBoolean handleMessage(OsMsg& rMsg);

   // Queue a message to be sent to the specified address and port.
   // Returns true if message was successfully queued.
   UtlBoolean sendTo(const SipMessage& message,
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

   /** Set the indicator whether the socket is shared (and thus should
    *  not be deleted).
    */
   // Initial value is FALSE, i.e., socket should be deleted.
   // :TODO:  This value should be made into an argument on the constructor,
   // since setting it correctly is mandatory and the value should never
   // change over the life of the client.  Also, if shared is TRUE, the
   // client should not read on the socket, since a server is already
   // reading on the socket.
   void setSharedSocket(UtlBoolean bShared);

   UtlBoolean sendInvite(char* toAddress, char* callId, int rtpPort,
                         int numCodecs, int rtpCodecs[],
                         int sequenceNumber = 1);

   void touch();
   //: Set the time when this client was last used
   // This is potentially used for garbage collection

/* ============================ ACCESSORS ================================= */

    void getClientNames(UtlString& clientNames) const;

    long getLastTouchedTime(void) const;

/* ============================ INQUIRY =================================== */

    UtlBoolean isOk(void);

    UtlBoolean isConnectedTo(UtlString& hostName, int hostPort);

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

    // Do preliminary processing of message to log it,
    // clean up its data, and extract any needed source address.
    void preprocessMessage(SipMessage& msg,
                           const UtlString& msgText,
                           int msgLength);

    // Test whether the socket is ready to read.  (Does not block.)
    UtlBoolean isReadyToRead();
    // Wait until the socket is ready to read (or has an error).
    UtlBoolean waitForReadyToRead();

    // Called by the thread to shut the SipClient down and signal its
    // owning server that it has done so.
    void clientStopSelf();

    OsSocket* clientSocket;
    OsSocket::IpProtocolSocketType mSocketType;
    SipUserAgentBase* mpSipUserAgent;
    SipProtocolServerBase* mpSipServer;
    UtlString mRemoteHostName;
    UtlString mRemoteViaAddress;
    UtlString mRemoteSocketAddress;
    UtlString mReceivedAddress;
    int mRemoteViaPort;
    int mRemoteHostPort;
    int mRemoteReceivedPort;
    long touchedTime;
    OsBSem mSocketLock;
    int mFirstResendTimeoutMs;

    // Is this a shared socket?
    // If true, do not delete or close it.
    // If false, close it when SipClient terminates.
    UtlBoolean mbSharedSocket;

    // Is there buffered outgoing data waiting for the socket to be writable?
    // If true, wait on socket writable.
    // If fase, wait on outgoing SIP message in queue.
    UtlBoolean mWriteQueued;

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
      SIP_CLIENT_SEND = 1
   };

   SipClientSendMsg(const unsigned char msgType, const unsigned char msgSubType,
                    const SipMessage& message, const char* address, int port);
     //:Constructor
     // Copies 'message'.

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

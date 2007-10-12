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
#include <os/OsServerTask.h>
#include <os/OsBSem.h>
#include <net/SipMessage.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgentBase;
class OsEvent;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipClient : public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipClient(OsSocket* socket = NULL);
     //:Default constructor


   virtual
   ~SipClient();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
        UtlBoolean send(SipMessage* message);

    UtlBoolean sendTo(const SipMessage& message,
                     const char* address,
                     int port);

        void setUserAgent(SipUserAgentBase* sipUA);
        //void addEventConsumer(OsServerTask* messageEventListener);
        //void removeEventConsumer(OsServerTask* messageEventListener);


        virtual int run(void* pArg);

        UtlBoolean sendInvite(char* toAddress, char* callId, int rtpPort,
                                                                int numCodecs, int rtpCodecs[],
                                                                int sequenceNumber = 1);

    void touch();
    //: Set the time when this client was last used
    // This is potentially used for garbage collection

    void notifyWhenAvailableForWrite(OsEvent& availableEvent);
    //: The given event is signaled when this SipClient is not busy

    void signalNextAvailableForWrite();

    void signalAllAvailableForWrite();

    void setSharedSocket(UtlBoolean bShared) ;

/* ============================ ACCESSORS ================================= */

    void getClientNames(UtlString& clientNames) const;
    long getLastTouchedTime() const;
    const UtlString& getLocalIp();

    void markInUseForWrite();
    void markAvailbleForWrite();


/* ============================ INQUIRY =================================== */

    UtlBoolean isOk();

    UtlBoolean isConnectedTo(UtlString& hostName, int hostPort);

    int isInUseForWrite();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    // Test whether the socket is ready to read.  (Does not block.)
    UtlBoolean isReadyToRead();
    // Wait until the socket is ready to read (or has an error).
    UtlBoolean waitForReadyToRead();

    OsSocket* clientSocket;
    OsSocket::IpProtocolSocketType mSocketType;
    SipUserAgentBase* sipUserAgent;
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
    int mInUseForWrite;
    UtlSList* mWaitingList;  // Events waiting until this is available
    UtlBoolean mbSharedSocket; // Shared socket-- do not delete or close (UDP / rport)

    SipClient(const SipClient& rSipClient);
     //:disable Copy constructor

    SipClient& operator=(const SipClient& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipClient_h_

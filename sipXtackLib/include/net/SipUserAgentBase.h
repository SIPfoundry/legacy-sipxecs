//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipUserAgentBase_h_
#define _SipUserAgentBase_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlHashBag.h>

#include <os/OsServerTask.h>
#include <os/OsRWMutex.h>
#include <net/SipMessage.h>
#include <net/SipMessageEvent.h>
#include <net/SipContactDb.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsConfigDb;

//:Base class for SipUserAgent
// Class detailed description which may extend to multiple lines
class SipUserAgentBase : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   SipUserAgentBase(int sipTcpPort = SIP_PORT,
                    int sipUdpPort = SIP_PORT,
                    int sipTlsPort = SIP_TLS_PORT,
                    int queueSize = OsServerTask::DEF_MAX_MSGS);
     //:Default constructor


   virtual
   ~SipUserAgentBase();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

    virtual UtlBoolean handleMessage(OsMsg& eventMessage) = 0;

    virtual void addMessageConsumer(OsServerTask* messageConsumer) = 0;
    //: Add a SIP message recipient


    //: Send a SIP message over the net
    // This method sends the SIP message via
    // a SIP UDP or TCP client as dictated by policy and the address
    // specified in the message
    virtual UtlBoolean send(SipMessage& message,
        OsMsgQ* responseListener = NULL,
        void* responseListenerData = NULL) = 0;
    //! param: message - the SIP message to be sent
    // Fields may be set to default values by this method.
    // Ownership is not taken by SipUserAgent.
    //! param: responseListener - the queue on which to place SipMessageEvents containing SIP responses from the same transaction as the request sent in message
    //! param: responseListenerData - data to be passed back with responses

    //: Dispatch the SIP message to the message consumer(s)
    // All incoming SIP message need to be dispatched via the user agent
    // server so that it can provide the reliablity for UDP
    // (i.e. resend requests when no response is received)
    // messageType - is as define by SipMessageEvent::MessageStatusTypes
    // APPLICATION type are normal incoming messages
    // TRANSPORT_ERROR type are notification of failures to send messages
    virtual void dispatch(SipMessage* message,
                              int messageType = SipMessageEvent::APPLICATION) = 0;

    //! Request User Agent to notify all output processors that a new
    //! SIP message is about to be sent.
    virtual void executeAllSipOutputProcessors( SipMessage& message,
                                                const char* address,
                                                int port ) = 0;

    void addConfigChangeConsumer(OsMsgQ& messageQueue);
    //: Register to find out when UA config changes (i.e. contact address)

/* ============================ ACCESSORS ================================= */

    virtual void logMessage(const char* message, int messageLength) = 0;

    virtual void getContactUri(UtlString* contactUri) ;

/* ============================ INQUIRY =================================== */

    virtual UtlBoolean isMessageLoggingEnabled() = 0;
    //: Is message logging enabled?

    virtual UtlBoolean isReady();
    //: Return boolean if the UA is started and initialized

    virtual UtlBoolean waitUntilReady();
    //: Block and wait until the UA is started and initialized

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    int mTcpPort;
    int mUdpPort;
    int mTlsPort;
    UtlBoolean mMessageLogEnabled;
    UtlString mContactURI;
    OsRWMutex mObserverMutex;
    UtlHashBag mConfigChangeObservers;
    SipContactDb mContactDb;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    SipUserAgentBase(const SipUserAgentBase& rSipUserAgent);
     //:Copy constructor

    SipUserAgentBase& operator=(const SipUserAgentBase& rhs);
     //:Assignment operator



};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipUserAgentBase_h_

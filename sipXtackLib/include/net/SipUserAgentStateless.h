//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipUserAgentStateless_h_
#define _SipUserAgentStateless_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <net/SipUserAgentBase.h>
#include <net/SipMessage.h>
#include <net/SipMessageEvent.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipClient;

//:Base class for SipUserAgent
// Class detailed description which may extend to multiple lines
class SipUserAgentStateless : public SipUserAgentBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   SipUserAgentStateless(int sipTcpPort = SIP_PORT, int sipUdpPort = SIP_PORT);
     //:Default constructor


   virtual
   ~SipUserAgentStateless();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

    virtual UtlBoolean handleMessage(OsMsg& eventMessage);

    virtual void addMessageConsumer(OsServerTask* messageConsumer);
    //: Add a SIP message recipient


    //: Send a SIP message over the net
    // This method sends the SIP message via
    // a SIP UDP or TCP client as dictated by policy and the address
    // specified in the message
    virtual UtlBoolean send(SipMessage& message,
        OsMsgQ* responseListener = NULL,
        void* responseListenerData = NULL);
    //! param: message - the sip message to be sent
    //! param: responseListener - the queue on which to place SipMessageEvents containing SIP responses from the same transaction as the request sent in message
    //! param: responseListenerData - data to be passed back with responses

    UtlBoolean sendTo(SipMessage& message,
           const char* sendAddress,
           const char* sendProtocol,
           int sendPort);

    //: Dispatch the SIP message to the message consumer(s)
    // All incoming SIP message need to be dispatched via the user agent
    // server so that it can provide the reliablity for UDP
    // (i.e. resend requests when no response is received)
    // messageType - is as define by SipMessageEvent::MessageStatusTypes
    // APPLICATION type are normal incoming messages
    // TRANSPORT_ERROR type are notification of failures to send messages
    virtual void dispatch(SipMessage* message,
                              int messageType = SipMessageEvent::APPLICATION);

/* ============================ ACCESSORS ================================= */

    virtual void logMessage(const char* message, int messageLength);

/* ============================ INQUIRY =================================== */

    virtual UtlBoolean isMessageLoggingEnabled();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        OsServerTask* mpMessageObserver;
    SipClient* mpUdpServer;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    SipUserAgentStateless(const SipUserAgentStateless& rSipUserAgent);
     //:Copy constructor

    SipUserAgentStateless& operator=(const SipUserAgentStateless& rhs);
     //:Assignment operator



};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipUserAgentStateless_h_

//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipClientTcp_h_
#define _SipClientTcp_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsSocket.h>
#include <os/OsTask.h>
#include <os/OsServerTask.h>
#include <os/OsBSem.h>
#include <net/SipClient.h>
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
class SipUserAgentBase;
class OsEvent;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipClientTcp : public SipClient
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipClientTcp(OsSocket* socket,
                SipProtocolServerBase* pSipServer,
                SipUserAgentBase* sipUA);
     //:Default constructor

   virtual ~SipClientTcp();
   
/* ============================ MANIPULATORS ============================== */

   // Send a message.  Executed by the thread.
   virtual void sendMessage(const SipMessage& message,
                            const char* address,
                            int port);

   // Continue sending stored message content (because the socket
   // is now writable).
   virtual void writeMore(void);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

    // Return the default port for the protocol of this SipClientTcp.
    virtual int defaultPort(void) const;

    virtual UtlContainableType getContainableType(void) const
    {
       return SipClientTcp::TYPE;
    };

    /** Class type used for runtime checking */
    static const UtlContainableType TYPE;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /// Un-sent data to be written.
    UtlString mWriteBuffer;

    /// A complete copy of the message whose tail is buffered in mWriteBuffer.
    //  Used to send error indications to the SipUserAgent for buffered messages
    //  that encounter a transport error.
    //  :TODO: Fold this with mWriteBuffer, keeping a cursor pointer showing
    //  what prefix has already been sent.
    // :TODO: Even better, ::sendMessage() should receive ownership of the message
    // and store it.  This requires changes to ::handleMessage() and maybe ::run().
    // Also need a way to detach the message from the SipClientSendMsg, it's now a field.
    UtlString mWriteQueuedMessage;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    SipClientTcp(const SipClientTcp& rSipClientTcp);
     //:disable Copy constructor

    SipClientTcp& operator=(const SipClientTcp& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipClientTcp_h_

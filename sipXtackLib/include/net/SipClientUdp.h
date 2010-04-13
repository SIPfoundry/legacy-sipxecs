//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipClientUdp_h_
#define _SipClientUdp_h_

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
class SipClientUdp : public SipClient
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipClientUdp(OsSocket* socket,
                SipProtocolServerBase* pSipServer,
                SipUserAgentBase* sipUA,
                UtlBoolean bIsSharedSocket = FALSE);
     //:Default constructor

   virtual ~SipClientUdp();

/* ============================ MANIPULATORS ============================== */

   // Send a message.  Executed by the thread.
   virtual void sendMessage(const SipMessage& message,
                            const char* address,
                            int port);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

    // Return the default port for the protocol of this SipClientUdp.
    virtual int defaultPort(void) const;

    virtual UtlContainableType getContainableType() const
    {
       return SipClientUdp::TYPE;
    };

    /** Class type used for runtime checking */
    static const UtlContainableType TYPE;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    SipClientUdp(const SipClientUdp& rSipClientUdp);
     //:disable Copy constructor

    SipClientUdp& operator=(const SipClientUdp& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipClientUdp_h_

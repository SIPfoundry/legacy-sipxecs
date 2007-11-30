//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipClientTls_h_
#define _SipClientTls_h_

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
class SipClientTls : public SipClient
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipClientTls(OsSocket* socket,
                SipProtocolServerBase* pSipServer,
                SipUserAgentBase* sipUA,
                UtlBoolean bIsSharedSocket = FALSE);
     //:Default constructor

   virtual ~SipClientTls();

/* ============================ MANIPULATORS ============================== */

   // Send a message.  Executed by the thread.
   virtual void sendMessage(const SipMessage& message,
                            const char* address,
                            int port);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

    // Return the default port for the protocol of this SipClientTls.
    virtual int defaultPort(void) const;

    virtual UtlContainableType getContainableType(void) const
    {
       return SipClientTls::TYPE;
    };

    /** Class type used for runtime checking */
    static const UtlContainableType TYPE;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    SipClientTls(const SipClientTls& rSipClientTls);
     //:disable Copy constructor

    SipClientTls& operator=(const SipClientTls& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipClientTls_h_

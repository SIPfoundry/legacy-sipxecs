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
#include <net/SipClient.h>
#include <net/SipClientWriteBuffer.h>
#include <os/OsSocket.h>

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

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipClientTcp : public SipClientWriteBuffer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipClientTcp(OsSocket* socket,
                SipProtocolServerBase* pSipServer,
                SipUserAgentBase* sipUA,
                UtlBoolean bIsSharedSocket = FALSE);
     //:Default constructor

   virtual ~SipClientTcp();

/* ============================ MANIPULATORS ============================== */

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

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    SipClientTcp(const SipClientTcp& rSipClientTcp);
     //:disable Copy constructor

    SipClientTcp& operator=(const SipClientTcp& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipClientTcp_h_

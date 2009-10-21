//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Copyright (C) 2009 Nortel Networks, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <stdio.h>

// APPLICATION INCLUDES
#include <net/SipClientTls.h>
#include <net/SipUserAgentBase.h>

#define LOG_TIME

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// All requests must contain at least 72 characters:
/*
X Y SIP/2.0 \n\r
i: A\n\r
f: B\n\r
t: C\n\r
c: 1\n\r
v: SIP/2.0/UDP D\n\r
l: 0 \n\r
\n\r

*/
// However to be tolerant of malformed messages we allow smaller:
#define MINIMUM_SIP_MESSAGE_SIZE 30
#define MAX_UDP_PACKET_SIZE (1024 * 64)

// STATIC VARIABLE INITIALIZATIONS

const UtlContainableType SipClientTls::TYPE = "SipClientTls";
const int sDefaultPort = SIP_TLS_PORT;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipClientTls::SipClientTls(OsSocket* socket,
                           SipProtocolServerBase* pSipServer,
                           SipUserAgentBase* sipUA,
                           UtlBoolean bIsSharedSocket) :
   SipClientWriteBuffer(socket,
                        pSipServer,
                        sipUA,
                        "SipClientTls-%d",
                        bIsSharedSocket)
{
}

SipClientTls::~SipClientTls()
{
   // Tell the associated thread to shut itself down.
   waitUntilShutDown();
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

// Return the default port for the protocol of this SipClientTls.
int SipClientTls::defaultPort(void) const
{
   return sDefaultPort;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

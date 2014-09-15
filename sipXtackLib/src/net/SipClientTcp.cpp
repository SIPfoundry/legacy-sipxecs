//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
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
#include <net/SipClientTcp.h>
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
const int SHUTDOWN_WAIT_TIME = 5000;

// STATIC VARIABLE INITIALIZATIONS

const UtlContainableType SipClientTcp::TYPE = "SipClientTcp";
const int sDefaultPort = SIP_PORT;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipClientTcp::SipClientTcp(OsSocket* socket,
                           SipProtocolServerBase* pSipServer,
                           SipUserAgentBase* sipUA,
                           UtlBoolean bIsSharedSocket) :
   SipClientWriteBuffer(socket,
                        pSipServer,
                        sipUA,
                        "SipClientTcp-%d",
                        bIsSharedSocket)
{
}

SipClientTcp::~SipClientTcp()
{
  Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipClientTcp[%s]::~ called",
                  mName.data());
   // Tell the associated thread to shut itself down.

  if(mClientSocket)
  {

        // Close the socket to unblock the run method
        // in case it is blocked in a waitForReadyToRead or
        // a read on the mClientSocket.  This should also
        // cause the run method to exit.
        if (!mbSharedSocket)
        {
           Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipClientTcp[%s]::~ %p socket %p closing %s socket",
                         mName.data(), this,
                         mClientSocket, OsSocket::ipProtocolString(mSocketType));
           mClientSocket->close();
        }

  }
  //
  // In a heavy load scenario, 20 ms might not be enough wait time
  // for the thread to exit.  We increase it to 5000 since it will
  // assert anyway.
  //
   waitUntilShutDown(SHUTDOWN_WAIT_TIME);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

// Return the default port for the protocol of this SipClientTcp.
int SipClientTcp::defaultPort(void) const
{
   return sDefaultPort;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

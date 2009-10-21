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
#include <net/SipMessage.h>
#include <net/SipClientUdp.h>
#include <net/SipMessageEvent.h>
#include <net/SipUserAgentBase.h>

#include <os/OsDateTime.h>
#include <os/OsDatagramSocket.h>
#include <os/OsStatus.h>
#include <os/OsSysLog.h>
#include <os/OsEvent.h>

#include <utl/XmlContent.h>

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

const UtlContainableType SipClientUdp::TYPE = "SipClientUdp";
const int sDefaultPort = SIP_PORT;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipClientUdp::SipClientUdp(OsSocket* socket,
                           SipProtocolServerBase* pSipServer,
                           SipUserAgentBase* sipUA,
                           UtlBoolean bIsSharedSocket) :
   SipClient(socket, pSipServer, sipUA, "SipClientUdp-%d", bIsSharedSocket)
{
}

SipClientUdp::~SipClientUdp()
{
   // Tell the associated thread to shut itself down.
   waitUntilShutDown();
}

/* ============================ MANIPULATORS ============================== */

// Send a message.  Executed by the thread.
void SipClientUdp::sendMessage(const SipMessage& message,
                               const char* address,
                               int port)
{
   UtlString buffer;
   ssize_t bufferLen;
   ssize_t bytesWritten;

   message.getBytes(&buffer, &bufferLen);

   // port will not be PORT_NONE, because it would have been replaced by
   // the default port in ::sendTo().
   bytesWritten =
      (dynamic_cast <OsDatagramSocket*> (mClientSocket))->
      write(buffer.data(), bufferLen, address, port);

   if (bufferLen == bytesWritten)
   {
      // If send was successful, update the last-activity time.
      touch();
   }
   else
   {
      // If send was unsuccessful, return the message with a transport
      // error indication.
      // Must make a new copy of the message because our caller
      // owns 'message'.
      mpSipUserAgent->dispatch(new SipMessage(message),
                               SipMessageEvent::TRANSPORT_ERROR);
   }

   return;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

// Return the default port for the protocol of this SipClientUdp.
int SipClientUdp::defaultPort(void) const
{
   return sDefaultPort;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

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
#include <net/SipClientTcp.h>
#include <net/SipMessageEvent.h>
#include <net/SipUserAgentBase.h>

#include <os/OsDateTime.h>
#include <os/OsDatagramSocket.h>
#include <os/OsStatus.h>
#include <os/OsSysLog.h>
#include <os/OsEvent.h>

#include <utl/XmlContent.h>

#define SIP_DEFAULT_RTT 500

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

const UtlContainableType SipClientTcp::TYPE = "SipClientTcp";
const int sDefaultPort = SIP_PORT;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipClientTcp::SipClientTcp(OsSocket* socket,
                           SipProtocolServerBase* pSipServer,
                           SipUserAgentBase* sipUA) :
   SipClient(socket, pSipServer, sipUA, "SipClientTcp-%d")
{
}

SipClientTcp::~SipClientTcp()
{
   // Tell the associated thread to shut itself down.
   waitUntilShutDown();
}

/* ============================ MANIPULATORS ============================== */

// Send a message.  Executed by the thread.
void SipClientTcp::sendMessage(const SipMessage& message,
                               const char* address,
                               int port)
{
   // :NOTE: OsConnectionSocket::reconnect isn't implemented.
   if (!clientSocket->isOk())
   {
      clientSocket->reconnect();
   }

   // Get the text of the message.
   // We can safely extract into mWriteBuffer, because this method is only
   // called when mWriteQueued is false, which happens only when
   // mWriteBuffer is null.
   int length;
   message.getBytes(&mWriteBuffer, &length);

   // Value returned from write attempt.
   // 0 means an error was seen.
   int ret;
   if (clientSocket->isOk())
   {
      // Write what we can.
      ret = clientSocket->write(mWriteBuffer.data(), length, 0L);
   }
   else
   {
      ret = 0;
   }

   if (ret == length)
   {
      // The send of the message was complete and successful.
      // Update the last-activity time.
      touch();
      // Delete the data.
      mWriteBuffer.remove(0);
      // No queued data.
      mWriteQueued = FALSE;
   }
   else if (ret > 0)
   {
      // We successfully sent some data but not all of it.
      // Update the last-activity time.
      touch();
      // Remember the original message, in case we have to return
      // it to the SipUserAgent as a transport error later.
      mWriteQueuedMessage = mWriteBuffer;
      // Delete the data that was sent.
      mWriteBuffer.remove(0, ret);
      // Queued data.
      mWriteQueued = TRUE;
   }
   else
   {
      // Error while writing.
      // Return the message with a transport error indication.
      // Must make a new copy of the message because our caller
      // owns 'message'.
      sipUserAgent->dispatch(new SipMessage(message),
                             SipMessageEvent::TRANSPORT_ERROR);
      // Because TCP is a connection protocol, we know that we cannot
      // send successfully any more and so should shut down this client.
      clientStopSelf();
   }
}

// Continue sending stored message content (because the socket
// is now writable).
void SipClientTcp::writeMore(void)
{
   // Write what we can.
   int ret =
      clientSocket->write(mWriteBuffer.data(), mWriteBuffer.length(), 0L);

   if (ret > 0)
   {
      // Some data was successfully written.
      touch();
      // Delete the data that was sent.
      mWriteBuffer.remove(0, ret);
   }
   else
   {
      // Error while writing.
      // Return the message with a transport error indication.
      // Must make a new copy of the message because our caller
      // owns 'message'.
      sipUserAgent->dispatch(new SipMessage(mWriteQueuedMessage),
                             SipMessageEvent::TRANSPORT_ERROR);
      // Throw away the message text.
      mWriteBuffer.remove(0);
      // Because TCP is a connection protocol, we know that we cannot
      // send successfully any more and so should shut down this client.
      clientStopSelf();
   }

   // Set mWriteQueued to control waiting on the output side of
   // this SipClient.
   mWriteQueued = !mWriteBuffer.isNull();
}

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

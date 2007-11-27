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

// APPLICATION INCLUDES
#include <net/SipClientWriteBuffer.h>
#include <net/SipMessageEvent.h>
#include <net/SipUserAgentBase.h>
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

const UtlContainableType SipClientWriteBuffer::TYPE = "SipClientWriteBuffer";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

SipClientWriteBuffer::SipClientWriteBuffer(OsSocket* socket,
                                           SipProtocolServerBase* pSipServer,
                                           SipUserAgentBase* sipUA,
                                           const char* taskNameString) :
   SipClient(socket, pSipServer, sipUA, taskNameString)
{
   mWriteQueued = FALSE;

   // Initialize mWritePointer for cleanliness.
   // (It won't be referenced because mWriteString is empty.)
   mWritePointer = 0;
}

SipClientWriteBuffer::~SipClientWriteBuffer()
{
   // Delete all the queued messages.
   mWriteBuffer.destroyAll();
}
   
/* ============================ MANIPULATORS ============================== */

// Handles an incoming message (from the message queue).
UtlBoolean SipClientWriteBuffer::handleMessage(OsMsg& eventMessage)
{
   UtlBoolean messageProcessed = FALSE;

   int msgType = eventMessage.getMsgType();
   int msgSubType = eventMessage.getMsgSubType();

   if (msgType == OsMsg::OS_SHUTDOWN)
   {
      // When shutting down, have to return all queued outgoing messages
      // with transport errors.
      emptyBuffer();

      // Continue with shutdown processing.
      messageProcessed = FALSE;
   }
   else if (msgType == OsMsg::OS_EVENT &&
            msgSubType == SipClientSendMsg::SIP_CLIENT_SEND)
   {
      // Queued SIP message to send.

      // Insert the SIP message into the queue, detaching it from
      // the incoming eventMessage.
      SipClientSendMsg* sendMsg =
         dynamic_cast <SipClientSendMsg*> (&eventMessage);
      insertMessage(sendMsg->detachMessage());

      // Write what we can.
      writeMore();

      messageProcessed = TRUE;
      // sendMsg will be deleted by ::run(), as usual.
      // Its destructor will free any storage owned by it.
   }

   return (messageProcessed);
}

// Not used in SipClientWriteBuffer.
void SipClientWriteBuffer::sendMessage(const SipMessage& message,
                                       const char* address,
                                       int port)
{
   assert(FALSE);
}

/// Insert a message into the buffer.
void SipClientWriteBuffer::insertMessage(SipMessage* message)
{
   UtlBoolean wasEmpty = mWriteBuffer.isEmpty();

   // Add the message to the queue.
   mWriteBuffer.insert(message);

   // If the buffer was empty, we need to set mWriteString and
   // mWritePointer.
   if (wasEmpty)
   {
      int length;
      message->getBytes(&mWriteString, &length);
      mWritePointer = 0;
   }

   mWriteQueued = TRUE;

   // Check to see if our internal queue is getting too big, which means
   // that the socket has been blocked for writing for a long time.
   // We use the message queue length of this task as the limit, since
   // both queues are affected by the same traffic load factors.
   if (mWriteBuffer.entries() > (size_t) (getMessageQueue()->maxMsgs()))
   {
      // If so, abort all unsent messages and terminate this client (so
      // as to clear any state of the socket).
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "SipClientWriteBuffer[%s]::insertMessage "
                    "mWriteBuffer has %d entries, exceeding the limit of %d",
                    getName().data(), (int) mWriteBuffer.entries(),
                    (int) getMessageQueue()->maxMsgs());
      emptyBuffer();
      clientStopSelf();
   }
}

/// Write as much of the buffered messages as can be written.
// Executed by the thread.
void SipClientWriteBuffer::writeMore()
{
   while (mWriteQueued)
   {
      if (mWritePointer >= mWriteString.length())
      {
         // We have written all of the first message.
         // Pop it and set up to write the next message.
         delete mWriteBuffer.get();
         mWriteString.remove(0);
         SipMessage* m = dynamic_cast <SipMessage*> (mWriteBuffer.first());
         mWritePointer = 0;
         mWriteQueued = m != NULL;
         if (m != NULL)
         {
            int length;
            m->getBytes(&mWriteString, &length);
         }
      }
      else
      {
         // Some portion of the first message remains to be written.
         
         // If the socket has failed, attempt to reconnect it.
         // :NOTE: OsConnectionSocket::reconnect isn't implemented.
         if (!clientSocket->isOk())
         {
            clientSocket->reconnect();
         }

         // Calculate the length to write.
         int length = mWriteString.length() - mWritePointer;

         // ret is the value returned from write attempt.
         // 0 means an error was seen.
         int ret;
         if (clientSocket->isOk())
         {
            // Write what we can.
            ret = clientSocket->write(mWriteString.data() + mWritePointer,
                                      length, 0L /* nonblocking */);
         }
         else
         {
            // Record the error.
            ret = 0;
         }

         if (ret > 0)
         {
            // We successfully sent some data, perhaps all of the
            // remainder of the first message.
            // Update the last-activity time.
            touch();
            // Update the state variables.
            mWritePointer += ret;
         }
         else
         {
            // Error while writing.
            // Return all buffered messages with a transport error indication.
            emptyBuffer();
            // Because TCP is a connection protocol, we know that we cannot
            // send successfully any more and so should shut down this client.
            clientStopSelf();
         }
      }
   }
}

/// Empty the buffer, returning all messages in the queue to the SipUserAgent
/// as transport errors.
/// This may not generate transport errors for all messages that were not
/// successfully sent -- some may have been written into the kernel
/// (and so were deleted from mWriteBuffer), but not have been successfully
/// sent.
void SipClientWriteBuffer::emptyBuffer()
{
   // Return all buffered messages with transport errors.
   SipMessage* m;
   while ((m = dynamic_cast <SipMessage*> (mWriteBuffer.get())))
   {
      // Return the message with a transport error indication.
      // SipUserAgent::dispatch takes ownership of the SIP message '*m'.
      // SipUserAgent::dispatch does not block -- if its message recipients
      // are overloaded, it discards the message.
      mpSipUserAgent->dispatch(m, SipMessageEvent::TRANSPORT_ERROR);
   }

   // Clear the other variables.
   mWriteString.remove(0);
   mWritePointer = 0;
   mWriteQueued = 0;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

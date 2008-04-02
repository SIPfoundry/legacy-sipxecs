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
                                           const char* taskNameString,
                                           UtlBoolean bIsSharedSocket) :
   SipClient(socket, pSipServer, sipUA, taskNameString, bIsSharedSocket)
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
   else if (msgType == OsMsg::OS_EVENT 
           &&  (msgSubType == SipClientSendMsg::SIP_CLIENT_SEND
             || msgSubType == SipClientSendMsg::SIP_CLIENT_SEND_KEEP_ALIVE))
   {
      // Queued SIP message to send - normal path.
      if (msgSubType == SipClientSendMsg::SIP_CLIENT_SEND)
      {
          // Insert the SIP message into the queue, detaching it from
          // the incoming eventMessage.
          SipClientSendMsg* sendMsg =
             dynamic_cast <SipClientSendMsg*> (&eventMessage);
          insertMessage(sendMsg->detachMessage());
      }
      else // send Keep Alive
      {
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipClientWriteBuffer[%s]::handleMessage send TCP keep-alive CR-LF response, ",
                        mName.data());
          UtlString* pKeepAlive;
          pKeepAlive = new UtlString("\r\n");
          insertMessage(pKeepAlive);
      }

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

/// Insert a keep alive message into the buffer.
void SipClientWriteBuffer::insertMessage(UtlString* keepAlive)
{
   UtlBoolean wasEmpty = mWriteBuffer.isEmpty();
  // NOTE- keep alive works since function only needs UtlContainable
   // Add the message to the queue.
   mWriteBuffer.insert(keepAlive);

   // If the buffer was empty, we need to set mWriteString and
   // mWritePointer.
   if (wasEmpty)
   {
       //const char* keepAliveData = keepAlive->data();

       mWriteString.append(keepAlive->data());
       mWritePointer = 0;
   }
   mWriteQueued = TRUE;
   // Skip check of internal queue size for keep alives,
   // count on real messages to watch it
}



/// Write as much of the buffered messages as can be written.
// Executed by the thread.
void SipClientWriteBuffer::writeMore()
{
   // 'exit_loop' will be set to TRUE if an attempt to write does
   // not write any bytes, and we will then return.
   UtlBoolean exit_loop = FALSE;

   while (mWriteQueued && !exit_loop)
   {
      if (mWritePointer >= mWriteString.length())
      {
         // We have written all of the first message.
         // Pop it and set up to write the next message.
         delete mWriteBuffer.get();
         mWriteString.remove(0);
         UtlContainable* firstWriteString = mWriteBuffer.first();
         SipMessage* m = dynamic_cast <SipMessage*> (firstWriteString);
         mWritePointer = 0;
         mWriteQueued = m != NULL;
         if (m != NULL)
         {
             // first check if this is a CRLF keepalive response
             UtlString* keepAlive = dynamic_cast <UtlString*> (firstWriteString);
             const char* keepAliveData = keepAlive->data();
             if (*keepAliveData++    == '\r' 
                 && *keepAliveData++ == '\n'
                 && *keepAliveData++ ==  0)
             {
                 mWriteString.append("\r\n");
             }
             else   // normal path
             {
                int length;
                m->getBytes(&mWriteString, &length);
             }
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
         // -1 means an error was seen.
         int ret;
         if (clientSocket->isOk())
         {
            // Write what we can.
            ret = clientSocket->write(mWriteString.data() + mWritePointer,
                                      length, 0L /* nonblocking */);
            // Theoretically, ret > 0, since the socket is ready for writing,
            // but it appears that that ret can be 0.
         }
         else
         {
            // Record the error.
            ret = -1;
            // Set a special errno value, which hopefully is not a real value.
            errno = 1000;
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
         else if (ret == 0)
         {
            // No data sent, even though (in our caller) poll()
            // reported the socket was ready to write.
            exit_loop = TRUE;
         }
         else
         {
            // Error while writing.
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "SipClientWriteBuffer[%s]::writeMore "
                          "OsSocket::write() returned %d, errno = %d",
                          getName().data(), ret, errno);
            // Return all buffered messages with a transport error indication.
            emptyBuffer();
            // Because TCP is a connection protocol, we know that we cannot
            // send successfully any more and so should shut down this client.
            clientStopSelf();
            // Exit the loop so handleMessage() can process the stop request.
            exit_loop = TRUE;
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

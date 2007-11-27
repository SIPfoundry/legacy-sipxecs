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
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>

// APPLICATION INCLUDES
#include <net/SipMessage.h>
#include <net/SipClient.h>
#include <net/SipMessageEvent.h>
#include <net/SipProtocolServerBase.h>
#include <net/SipUserAgentBase.h>

#include <os/OsDateTime.h>
#include <os/OsDatagramSocket.h>
#include <os/OsStatus.h>
#include <os/OsSysLog.h>
#include <os/OsEvent.h>

#include <utl/XmlContent.h>

#define SIP_DEFAULT_RTT 500
// The time in milliseconds that we allow poll() to wait.
// This must be short, as the run() loop must wake up periodically to check
// if the client's thread is being shut down.
// And the SipUserAgent, when garbage collecting idle clients, waits for
// the clients to finish shutting down.
#define POLL_TIMEOUT 100

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

const UtlContainableType SipClient::TYPE = "SipClient";

const UtlContainableType SipClientSendMsg::TYPE = "SipClientSendMsg";

// Methods for SipClientSendMsg.

// Constructor
SipClientSendMsg::SipClientSendMsg(const unsigned char msgType,
                                   const unsigned char msgSubType,
                                   const SipMessage& message,
                                   const char* address, int port) :
   OsMsg(msgType, msgSubType),
   mpMessage(new SipMessage(message)),
   mAddress(strdup(address)),
   mPort(port)
{
}

//:Copy constructor
SipClientSendMsg::SipClientSendMsg(const SipClientSendMsg& rOsMsg) :
   OsMsg(rOsMsg),
   mpMessage(new SipMessage(*rOsMsg.mpMessage)),
   mAddress(strdup(rOsMsg.mAddress)),
   mPort(rOsMsg.mPort)
{
}

//:Destructor
SipClientSendMsg::~SipClientSendMsg()
{
   free(mAddress);

   // mpMessage may have been nulled by detachMessage, so we have to
   // test it before deleting it.
   if (mpMessage)
   {
      delete mpMessage;
   }
}

//:Create a copy of this msg object (which may be of a derived type)
OsMsg* SipClientSendMsg::createCopy(void) const
{
   return new SipClientSendMsg(*this);
}

//:Assignment operator
SipClientSendMsg& SipClientSendMsg::operator=(const SipClientSendMsg& rhs)
{
   if (this != &rhs)            // handle the assignment to self case
   {
      OsMsg::operator=(rhs);
      mpMessage = new SipMessage(*rhs.mpMessage);
      free(mAddress);
      mAddress = strdup(rhs.mAddress);
      mPort = rhs.mPort;
   }

   return *this;
}

/// Return the SipMessage component, and NULL the SipMessage component,
/// so the SipClientSendMsg no longer owns it.
SipMessage* SipClientSendMsg::detachMessage(void)
{
   SipMessage* ret = mpMessage;
   mpMessage = NULL;
   return ret;
}

// Component accessors.
const SipMessage* SipClientSendMsg::getMessage(void) const
{
   return mpMessage;
}

const char* SipClientSendMsg::getAddress(void) const
{
   return mAddress;
}

int SipClientSendMsg::getPort(void) const
{
   return mPort;
}

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipClient::SipClient(OsSocket* socket,
                     SipProtocolServerBase* pSipServer,
                     SipUserAgentBase* sipUA,
                     const char* taskNameString) :
   OsServerTaskWaitable(taskNameString),
   clientSocket(socket),
   mSocketType(socket ? socket->getIpProtocol() : OsSocket::UNKNOWN),
   mpSipUserAgent(sipUA),
   mpSipServer(pSipServer),
   mRemoteViaPort(PORT_NONE),
   mRemoteReceivedPort(PORT_NONE),
   mSocketLock(OsBSem::Q_FIFO, OsBSem::FULL),
   mFirstResendTimeoutMs(SIP_DEFAULT_RTT * 4), // for first transcation time out
   mbSharedSocket(FALSE),
   mWriteQueued(FALSE)
{
   touch();

   if (clientSocket)
   {
       clientSocket->getRemoteHostName(&mRemoteHostName);
       clientSocket->getRemoteHostIp(&mRemoteSocketAddress, &mRemoteHostPort);

       OsSysLog::add(FAC_SIP, PRI_INFO,
                     "SipClient[%s]::_ created %s %s socket %d: host '%s' port %d",
                     mName.data(),
                     OsSocket::ipProtocolString(mSocketType),
                     mbSharedSocket ? "shared" : "unshared",
                     socket->getSocketDescriptor(),
                     mRemoteHostName.data(), mRemoteHostPort
                     );
   }
}

// Destructor
SipClient::~SipClient()
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipClient[%s]::~ called",
                  mName.data());

    // Tell the associated thread to shut itself down.
    requestShutdown();

    // Do not delete the event listers, as they are not subordinate.

    // Free the socket
    if(clientSocket)
    {
        // Close the socket to unblock the run method
        // in case it is blocked in a waitForReadyToRead or
        // a read on the clientSocket.  This should also
        // cause the run method to exit.
        if (!mbSharedSocket)
        {
           OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipClient[%s]::~SipClient %p socket %p closing %s socket",
                         mName.data(), this,
                         clientSocket, OsSocket::ipProtocolString(mSocketType));
           clientSocket->close();
        }

        // Wait for the task to exit so that it does not
        // reference the socket or other members after they
        // get deleted.
        if(isStarted() || isShuttingDown())
        {
            waitUntilShutDown();
        }

        if (!mbSharedSocket)
        {
            delete clientSocket;
        }
        clientSocket = NULL;
    }
    else if(isStarted() || isShuttingDown())
    {
        // It should not get here but just in case
        waitUntilShutDown();
    }
}

/* ============================ MANIPULATORS ============================== */

// Handles an incoming message (from the message queue).
UtlBoolean SipClient::handleMessage(OsMsg& eventMessage)
{
   UtlBoolean messageProcessed = FALSE;

   int msgType = eventMessage.getMsgType();
   int msgSubType = eventMessage.getMsgSubType();

   if(msgType == OsMsg::OS_EVENT &&
      msgSubType == SipClientSendMsg::SIP_CLIENT_SEND)
   {
      // Queued SIP message to send.

      // Call sendMessage method to send the SIP message (or to
      // store its contents to be sent).
      SipClientSendMsg* sendMsg =
         dynamic_cast <SipClientSendMsg*> (&eventMessage);
      sendMessage(*sendMsg->getMessage(), sendMsg->getAddress(),
                  sendMsg->getPort());
      messageProcessed = TRUE;
      // sendMsg will be deleted by ::run(), as usual.
      // Its destructor will free any storage owned by it.
   }

   return (messageProcessed);
}

// Queue a message to be sent to the specified address and port.
UtlBoolean SipClient::sendTo(const SipMessage& message,
                             const char* address,
                             int port)
{
   UtlBoolean sendOk;

   if (clientSocket)
   {
      // Create message to queue.
      // If port == PORT_NONE, apply the correct default port for this
      // transport method before assembling the SipClientSendMsg.
      SipClientSendMsg sendMsg(OsMsg::OS_EVENT,
                               SipClientSendMsg::SIP_CLIENT_SEND,
                               message, address,
                               port == PORT_NONE ? defaultPort() : port);

      // Post the message to the task's queue.
      OsStatus status = postMessage(sendMsg, OsTime::NO_WAIT);
      sendOk = status == OS_SUCCESS;
      if (!sendOk)
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipClient[%s]::sendTo attempt to post message failed",
                       mName.data());
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "SipClient[%s]::sendTo called for client without socket",
                    mName.data()
         );
      sendOk = FALSE;
   }

   return sendOk;
}

// Continue sending stored message content (because the socket
// is now writable).
// This is the default, do-nothing, implementation, to be overridden
// by classes that use this functionality.
void SipClient::writeMore(void)
{
   assert(FALSE);
}

void SipClient::setSharedSocket(UtlBoolean bShared)
{
    mbSharedSocket = bShared;
}

void SipClient::touch()
{
   OsTime time;
   OsDateTime::getCurTimeSinceBoot(time);
   touchedTime = time.seconds();
   //OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipClient[%s]::touch client: %p time: %d\n",
   //             mName.data(), this, touchedTime);
}

/* ============================ ACCESSORS ================================= */

void SipClient::getClientNames(UtlString& clientNames) const
{
    char portString[40];

    // host DNS name
    sprintf(portString, "%d", mRemoteHostPort);
    clientNames = " remote host: ";
    clientNames.append(mRemoteHostName);
    clientNames.append(":");
    clientNames.append(portString);

    // host IP address
    clientNames.append(" remote IP: ");
    clientNames.append(mRemoteSocketAddress);
    clientNames.append(":");
    clientNames.append(portString);

    // via address
    clientNames.append(" remote Via address: ");
    clientNames.append(mRemoteViaAddress);
    clientNames.append(":");
    XmlDecimal(clientNames, mRemoteViaPort);

    // recieved address
    clientNames.append(" received address: ");
    clientNames.append(mReceivedAddress);
    clientNames.append(":");
    XmlDecimal(clientNames, mRemoteReceivedPort);
}

long SipClient::getLastTouchedTime() const
{
   return (touchedTime);
}

/* ============================ INQUIRY =================================== */

UtlBoolean SipClient::isOk()
{
   return clientSocket->isOk() && isNotShut();
}

UtlBoolean SipClient::isConnectedTo(UtlString& hostName, int hostPort)
{
    UtlBoolean isSame = FALSE;
    int tempHostPort = portIsValid(hostPort) ? hostPort : defaultPort();

#ifdef TEST_SOCKET
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipClient[%s]::isConnectedTo hostName = '%s', tempHostPort = %d, mRemoteHostName = '%s', mRemoteHostPort = %d, mRemoteSocketAddress = '%s', mReceivedAddress = '%s', mRemoteViaAddress = '%s'",
                  mName.data(),
                  hostName.data(), tempHostPort, mRemoteHostName.data(), mRemoteHostPort, mRemoteSocketAddress.data(), mReceivedAddress.data(), mRemoteViaAddress.data());
#endif

    // If the ports match and the host is the same as either the
    // original name that the socket was constructed with or the
    // name it was resolved to (usually an IP address).
    if (   mRemoteHostPort == tempHostPort
        && (   hostName.compareTo(mRemoteHostName, UtlString::ignoreCase) == 0
            || hostName.compareTo(mRemoteSocketAddress, UtlString::ignoreCase) == 0))
    {
        isSame = TRUE;
    }
    else if (   mRemoteReceivedPort == tempHostPort
             && hostName.compareTo(mReceivedAddress, UtlString::ignoreCase) == 0)
    {
        isSame = TRUE;
    }
    else if (   mRemoteViaPort == tempHostPort
             && hostName.compareTo(mRemoteViaAddress, UtlString::ignoreCase) == 0)
    {
        // Cannot trust what the other side said was their IP address
        // as this is a bad spoofing/denial of service hole
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipClient[%s]::isConnectedTo matches %s:%d but is not trusted",
                      mName.data(),
                      mRemoteViaAddress.data(), mRemoteViaPort);
    }

    return(isSame);
}

const UtlString& SipClient::getLocalIp()
{
    return clientSocket->getLocalIp();
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Thread execution code.
int SipClient::run(void* runArg)
{
   OsMsg*    pMsg = NULL;
   OsStatus  res;
   // Buffer to hold data read from the socket but not yet parsed
   // into incoming SIP messages.
   UtlString readBuffer;

   // Wait structure:
   struct pollfd fds[2];
   // Incoming message on the message queue (to be sent on the socket).
   fds[0].fd = mPipeReadingFd;
   // Socket ready to write (to continue sending message).
   // Socket ready to read (message to be received).

   do
   {
      // The file descriptor for the socket may change, as OsSocket's
      // can be re-opened.
      fds[1].fd = clientSocket->getSocketDescriptor();

      // Initialize the revents members.
      // This may not be necessary (the man page is not clear), but
      // Valgrind flags 'fds[*].revents' as undefined if they aren't
      // initialized.
      fds[0].revents = 0;
      fds[1].revents = 0;

      fds[0].events = POLLIN;
      // Read the socket only if the socket is not shared.
      // If it is shared, the ancestral SipClient will read it.
      // If multiple threads attempt to read the socket, poll() may
      // succeed but another may read the data, leaving us to block on
      // read.
      fds[1].events = mbSharedSocket ? 0 : POLLIN;

      // Set wait for writing the socket if there is queued messages to
      // send.
      if (mWriteQueued)
      {
         // Wait for output on the socket to not block.
         fds[1].events |= POLLOUT;
      }

      // Wait for work to do.
      if (!readBuffer.isNull())
      {
         // If there is residual data in the read buffer, pretend the socket
         // is ready to read.
         fds[1].revents = POLLIN;
      }
      else
      {
         // Otherwise, call poll() to wait.
         int res = poll(&fds[0], sizeof (fds) / sizeof (fds[0]),
                        POLL_TIMEOUT);
         assert(res >= 0 || (res == -1 && errno == EINTR));
      }

      // Check for message queue messages before checking the socket,
      // to make sure that we process shutdown messages promptly, even
      // if we would be spinning trying to service the socket.
      if ((fds[0].revents & POLLIN) != 0)
      {
         // Poll finished because the pipe is ready to read.

         // Receive the message.
         res = receiveMessage((OsMsg*&) pMsg, OsTime::NO_WAIT);
         assert(res == OS_SUCCESS);

         // Read 1 byte from the pipe.
         char buffer[1];
         assert(read(mPipeReadingFd, &buffer, 1) == 1);

         if (!handleMessage(*pMsg))                  // process the message
         {
            OsServerTask::handleMessage(*pMsg);
         }

         if (!pMsg->getSentFromISR())
         {
            pMsg->releaseMsg();                         // free the message
         }
      }
      else if ((fds[1].revents & POLLOUT) != 0)
      {
         // Poll finished because socket is ready to write.

         // Call method to continue writing data.
         writeMore();
      }
      else if ((fds[1].revents & POLLIN) != 0)
      {
         // Poll finished because socket is ready to read.

         // Read message.
         // Must allocate a new message because SipUserAgent::dispatch will
         // take ownership of it.
         SipMessage* msg = new SipMessage;
         int res = msg->read(clientSocket,
                             HTTP_DEFAULT_SOCKET_BUFFER_SIZE,
                             &readBuffer);
         // Use readBuffer to hold any unparsed data after the message
         // we read.
         // Note that if a message was successfully parsed, readBuffer
         // still contains as its prefix the characters of that message.
         // We save them for logging purposes below and will delete them later.

         // Note that input was processed at this time.
         touch();

         if (res > 0)
         {
            // Message successfully read.

            // Do preliminary processing of message to log it,
            // clean up its data, and extract any needed source address.
            preprocessMessage(*msg, readBuffer, res);

            // Dispatch the message.
            // dispatch() takes ownership of *msg.
            mpSipUserAgent->dispatch(msg);

            // Now that logging is done, remove the parsed bytes and
            // remember any unparsed input for later use.
            readBuffer.remove(0, res);
         }
         else
         {
            // Something went wrong while reading the message.
            // (Possibly EOF on a connection-oriented socket.)

            // Delete the SipMessage allocated above, which is no longer needed.
            delete msg;

            // If the socket is not framed, we need to abort the connection.
            // :TODO: This doesn't work right for framed connection-oriented
            // protocols (like SCTP), but OsSocket doesn't have an EOF-query
            // method -- we need to close all connection-oriented
            // sockets as well in case it was an EOF.
            // Define a virtual function that returns the correct bit.
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipClient[%s]::run SipMessage::read returns %d (error or EOF), "
                          "readBuffer = '%.1000s'",
                          mName.data(), res, readBuffer.data());
            if (!OsSocket::isFramed(clientSocket->getIpProtocol()))
            {
               clientStopSelf();
            }
            // Delete the data read so far, which will not have been
            // deleted by HttpMessage::read.
            readBuffer.remove(0);
         }
      }
   }
   while (isStarted());

   return 0;        // and then exit
}

// Do preliminary processing of message to log it,
// clean up its data, and extract any needed source address.
void SipClient::preprocessMessage(SipMessage& msg,
                                  const UtlString& msgText,
                                  int msgLength)
{
   // Canonicalize short field names.
   msg.replaceShortFieldNames();

   // Get the send address.
   UtlString fromIpAddress;
   int fromPort;
   msg.getSendAddress(&fromIpAddress, &fromPort);

   // Log the message.
   // Only bother processing if the logs are enabled
   if (   mpSipUserAgent->isMessageLoggingEnabled()
          || OsSysLog::willLog(FAC_SIP_INCOMING, PRI_INFO)
      )
   {
      UtlString logMessage;
      logMessage.append("Read SIP message:\n");
      logMessage.append("----Remote Host:");
      logMessage.append(fromIpAddress);
      logMessage.append("---- Port: ");
      XmlDecimal(logMessage,
                 portIsValid(fromPort) ? fromPort : defaultPort());
      logMessage.append("----\n");

      logMessage.append(msgText.data(), msgLength);
      UtlString messageString;
      logMessage.append(messageString);
      logMessage.append("====================END====================\n");

      // Send the message to the SipUserAgent for its internal log.
      mpSipUserAgent->logMessage(logMessage.data(), logMessage.length());
      // Write the message to the syslog.
      OsSysLog::add(FAC_SIP_INCOMING, PRI_INFO, "%s", logMessage.data());
   }

   // Set the date field if not present
   long epochDate;
   if (!msg.getDateField(&epochDate))
   {
      msg.setDateField();
   }

   // Set the protocol and time.
   msg.setSendProtocol(mSocketType);
   msg.setTransportTime(touchedTime);

   // Keep track of where this message came from
   msg.setSendAddress(fromIpAddress.data(), fromPort);
                    
   // Keep track of the interface on which this message was
   // received.               
   msg.setLocalIp(clientSocket->getLocalIp());

   if (mReceivedAddress.isNull())
   {
      mReceivedAddress = fromIpAddress;
      mRemoteReceivedPort = fromPort;
   }

   // If this is a request...
   if (!msg.isResponse())
   {
      UtlString lastAddress;
      int lastPort;
      UtlString lastProtocol;
      int receivedPort;
      UtlBoolean receivedSet;
      UtlBoolean maddrSet;
      UtlBoolean receivedPortSet;

      // Fill in 'received' and 'rport' in top Via if needed.
      msg.setReceivedViaParams(fromIpAddress, fromPort);

      // Derive information about the other end of the connection
      // from the Via that the other end provided.

      // Get the addresses from the topmost Via.
      msg.getLastVia(&lastAddress, &lastPort, &lastProtocol,
                     &receivedPort, &receivedSet, &maddrSet,
                     &receivedPortSet);

      // :QUERY: Should this test be clientSocket->isConnected()?
      if (   (   mSocketType == OsSocket::TCP
                 || mSocketType == OsSocket::SSL_SOCKET
                )
             && !receivedPortSet
         )
      {
         // we can use this socket as if it were
         // connected to the port specified in the
         // via field
         mRemoteReceivedPort = lastPort;
      }

      // Keep track of the address the other
      // side said they sent from.  Note, this cannot
      // be trusted unless this transaction is
      // authenticated
      if (mRemoteViaAddress.isNull())
      {
         mRemoteViaAddress = lastAddress;
         mRemoteViaPort =
            portIsValid(lastPort) ? lastPort : defaultPort();
      }
   }
}

// Test whether the socket is ready to read. (Does not block.)
UtlBoolean SipClient::isReadyToRead()
{
   return clientSocket->isReadyToRead(0);
}

// Wait until the socket is ready to read (or has an error).
UtlBoolean SipClient::waitForReadyToRead()
{
   return clientSocket->isReadyToRead(-1);
}

// Called by the thread to shut the SipClient down and signal its
// owning server that it has done so.
void SipClient::clientStopSelf()
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipClient[%s]::clientStopSelf called",
                 mName.data());

   // Stop the run loop.
   OsTask::requestShutdown();
   // Signal the owning SipServer to check for dead clients.
   if (mpSipServer)
   {
      OsMsg message(OsMsg::OS_EVENT,
                    SipProtocolServerBase::SIP_SERVER_GC);
      // If the SipServer's queue is full, don't wait, since
      // that can cause deadlocks.  The SipServer will eventually
      // garbage-collect terminated clients spontaneously.
      mpSipServer->postMessage(message,
                               OsTime::NO_WAIT);
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

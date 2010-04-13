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
#include <assert.h>

// APPLICATION INCLUDES
#include <net/SipUserAgentStateless.h>
#include <net/SipClientUdp.h>
#include <net/Url.h>
#include <os/OsDatagramSocket.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipUserAgentStateless::SipUserAgentStateless(int sipTcpPort, int sipUdpPort) :
   SipUserAgentBase(sipTcpPort, sipUdpPort)
{
   mpMessageObserver = NULL;
   UtlString localAddress;
   OsSocket::getHostIp(&localAddress);
   Url contact;
   contact.setHostAddress(localAddress.data());
   contact.setHostPort(sipUdpPort);
   contact.toString(mContactURI);

   OsDatagramSocket* serverSocket =
       new OsDatagramSocket(0, NULL,sipUdpPort, NULL);
   mpUdpServer = new SipClientUdp(serverSocket, NULL, this);
   assert(mpUdpServer->isOk());
   mpUdpServer->start();
}

// Copy constructor
SipUserAgentStateless::SipUserAgentStateless(const SipUserAgentStateless& rSipUserAgentStateless)
{
}

// Destructor
SipUserAgentStateless::~SipUserAgentStateless()
{
    if(mpUdpServer) delete mpUdpServer;
    mpUdpServer = NULL;
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipUserAgentStateless&
SipUserAgentStateless::operator=(const SipUserAgentStateless& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean SipUserAgentStateless::handleMessage(OsMsg& eventMessage)
{
    return(TRUE);
}

void SipUserAgentStateless::addMessageConsumer(OsServerTask* messageConsumer)
{
    mpMessageObserver = messageConsumer;
}

UtlBoolean SipUserAgentStateless::send(SipMessage& message,
        OsMsgQ* responseListener,
        void* responseListenerData)
{
    UtlString sendAddress;
    UtlString sendProtocol;
    int sendPort;

    if(message.isResponse())
    {
        int receivedPort;
        UtlBoolean receivedSet;
        UtlBoolean maddrSet;
        UtlBoolean receivedPortSet;
        message.getTopVia(&sendAddress, &sendPort, &sendProtocol, &receivedPort,
                          &receivedSet, &maddrSet, &receivedPortSet);
        if(receivedPortSet && portIsValid(receivedPort))
        {
            sendPort = receivedPort;
        }
    }
    else
    {
        UtlString uriString;
        message.getRequestUri(&uriString);
        Url uri(uriString);
        uri.getHostAddress(sendAddress);

        // Check for maddr
        UtlString maddr;
        uri.getUrlParameter("maddr", maddr);
        if(!maddr.isNull())
        {
            // Use maddr if present
            sendAddress = maddr;
        }

        uri.getUrlParameter("transport", sendProtocol);
        sendPort = uri.getHostPort();
    }

    UtlBoolean sendOk =
        sendTo(message, sendAddress.data(), sendProtocol.data(), sendPort);

    return(sendOk);
}

UtlBoolean SipUserAgentStateless::sendTo(SipMessage& message,
                 const char* sendAddress,
                 const char* sendProtocol,
                 int sendPort)
{
    UtlBoolean sendOk = FALSE;

    if(sendAddress && *sendAddress && mpUdpServer)
    {
        if (!portIsValid(sendPort))
        {
           sendPort = SIP_PORT;
        }

        sendOk = mpUdpServer->sendTo(message, sendAddress, sendPort);
    }

    // Log the message
    if (isMessageLoggingEnabled())
    {
        UtlString messageStatus;
        char messageChars[200];
        if(sendOk)
            sprintf(messageChars, "Sent message %s port:%d:\n",
                sendAddress, sendPort);
        else
            sprintf(messageChars, "Failed to send message %s port:%d:\n",
                sendAddress, sendPort);
        messageStatus = messageChars;
        UtlString msgBytes;
        ssize_t msgLen;
        message.getBytes(&msgBytes, &msgLen);
        msgBytes.insert(0, messageStatus);
        msgBytes.append("--------------------END--------------------\n");
        logMessage(msgBytes.data(), msgBytes.length());
    }

    return(sendOk);
}

void SipUserAgentStateless::dispatch(SipMessage* message,
                              int messageType)
{
    // Log the message
    //if(message)
    //{
    //    UtlString msgBytes;
    //    ssize_t msgLen;
    //    message->getBytes(&msgBytes, &msgLen);
    //    msgBytes.insert(0, "Received message:\n");
    //    msgBytes.append("++++++++++++++++++++END++++++++++++++++++++\n");
    //    logMessage(msgBytes.data(), msgBytes.length());
    //}

    if(mpMessageObserver)
    {
        SipMessageEvent event(message);
        event.setMessageStatus(messageType);
        mpMessageObserver->postMessage(event);
    }
}
/* ============================ ACCESSORS ================================= */

void SipUserAgentStateless::logMessage(const char* message, int messageLength)
{
    osPrintf("%s", message);
}

/* ============================ INQUIRY =================================== */

UtlBoolean SipUserAgentStateless::isMessageLoggingEnabled()
{
    return true ;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsDefs.h"

// APPLICATION INCLUDES
#include "tao/TaoTransportAgent.h"
#include "tao/TaoMessage.h"
#include "os/OsDateTime.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
TaoTransportAgent::TaoTransportAgent(OsSocket* pSocket, OsServerTask* pServer)
    : OsTask("TaoTrsptAgent-%d")
        , mWriteSem(OsBSem::Q_PRIORITY, OsBSem::FULL)

{
        mpSocket = pSocket;
        mpServer = pServer;
}

TaoTransportAgent::TaoTransportAgent(OsSocket* socket, const char* remoteHost,
                                         const char* callId, const char* toField,
                                         const char* fromField)
        : OsTask("TaoTrsptAgent-%d")
        , mWriteSem(OsBSem::Q_PRIORITY, OsBSem::FULL)

{
#ifdef TEST
   if(socket)
                osPrintf("TaoTransportAgent created with socket descriptor: %d\n",
                        socket->getSocketDescriptor());
#endif
        mpSocket = socket;
        if(remoteHost)
        {
                viaName.remove(0);
                viaName.append(remoteHost);
        }
        if(callId)
        {
                callIdLabel.append(callId);
        }
        if(toField)
        {
                toFieldLabel.append(toField);
        }
        if(fromField)
        {
                fromFieldLabel.append(fromField);
        }

        touchedTime = OsDateTime::getSecsSinceEpoch();
}

// Copy constructor
TaoTransportAgent::TaoTransportAgent(const TaoTransportAgent& rTaoTransportAgent)
        : OsTask("TaoTrsptAgent-%d")
        , mWriteSem(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
}

// Destructor
TaoTransportAgent::~TaoTransportAgent()
{
        // Do not delete the event listers they are not subordinate

        // Free the socket
        if(mpSocket)
        {
                osPrintf("<<**>> TaoTransportAgent::tearing down mpSocket\n") ;
                // The destructer will close the socket
//              delete mpSocket;
//              mpSocket = NULL;
        }
}

/* ============================ MANIPULATORS ============================== */

int TaoTransportAgent::run(void* runArg)
{
        UtlString remoteHostName;
        UtlString viaProtocol;
    UtlString fromIpAddress;

        while (mpSocket && mpSocket->isOk() && !isShuttingDown())
        {
                char          buffer[DEF_TAO_MAX_SOCKET_SIZE];
                unsigned long bytesRead;
                unsigned long cookie ;
                unsigned long length ;

                memset(buffer, 0, (DEF_TAO_MAX_SOCKET_SIZE * sizeof(char)));

                // Look for our next message, it should start with a '1234ABCD' marker
                bytesRead = mpSocket->read((char*) &cookie, sizeof(unsigned long)) ;
                while ((bytesRead > 0) && (cookie != 0x1234ABCD)) {
                        osPrintf("<<**>> Invalid data read from socket, trying to resynchronize...\n") ;
                        bytesRead = readUntilDone(mpSocket, (char*) &cookie, sizeof(unsigned long)) ;
                }
                // Okay, now read length
                if (bytesRead > 0) {
                        bytesRead = readUntilDone(mpSocket, (char*) &length, sizeof(unsigned long)) ;
                }
                // Finally read data
                if (bytesRead > 0) {
                        bytesRead = readUntilDone(mpSocket, buffer, length) ;

                        if (bytesRead != length) {
                                osPrintf("<<**>> TaoTransportAgent READ MISMATCH %lu != %lu\n", bytesRead, length) ;
                                bytesRead = 0 ;
                        }
                }

                if(bytesRead > 0)
                {
                        TaoMessage msg = TaoMessage(UtlString(buffer));
                        msg.setSocket((TaoObjHandle)this);      //stores pointer to this class

                        mpServer->postMessage(msg);
                }
                else if(bytesRead <= 0 || !mpSocket->isOk())
                {
                        // The socket has gone sour close down the client
                        mpSocket->getRemoteHostName(&remoteHostName);
                        osPrintf("Shutting down TaoTransportAgent: %s due to failed socket\n",
                                remoteHostName.data());
                        break;
                }
        }

#ifdef TEST_PRINT
        osPrintf("TaoTransportAgent: %s/%s exiting\r\n", remoteHostName.data(),
                viaName.data());
#endif
        return(0);
}

int TaoTransportAgent::send(TaoMessage& rMsg)
{
        UtlString buffer;
        ssize_t bufferLen;
        rMsg.getBytes(&buffer, &bufferLen);

        // send the msg to the transport, receive the response
        size_t sent = 0;
        if (mpSocket->isOk() && (bufferLen > 0))
        {
                mWriteSem.acquire() ;

                size_t iSendSize = bufferLen + (sizeof(uint32_t)*2) ;

                char* pBuf = new char[iSendSize] ;

                uint32_t cookie = 0x1234ABCD ;
                uint32_t length = bufferLen ;
                memcpy(&pBuf[0], &cookie, sizeof(uint32_t)) ;
                memcpy(&pBuf[sizeof(uint32_t)], &length, sizeof(uint32_t)) ;
                memcpy(&pBuf[sizeof(uint32_t)*2], buffer.data(), bufferLen) ;
                sent = mpSocket->write(pBuf, iSendSize) ;

                delete pBuf ;

                if (sent > (sizeof(uint32_t)*2))
                        sent -= sizeof(uint32_t)*2 ;

                mWriteSem.release() ;

                if (sent != length) {
                        osPrintf("<<**>> TaoTransportAgent WRITE MISMATCH %zu != %u\n", sent, length) ;
                        sent = 0 ;
                }
        }

        return sent;
}


// Assignment operator
TaoTransportAgent&
TaoTransportAgent::operator=(const TaoTransportAgent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */

void TaoTransportAgent::getHostIp(UtlString* hostAddress) const
{
        mpSocket->getRemoteHostIp(hostAddress);
}

void TaoTransportAgent::getAgentName(UtlString* pAgentName) const
{
        pAgentName->remove(0);
        pAgentName->append(viaName.data());
}

void TaoTransportAgent::getCallId(UtlString* callId) const
{
        callId->remove(0);
        callId->append(callIdLabel.data());
}

void TaoTransportAgent::getFromField(UtlString* fromField) const
{
        fromField->remove(0);
        fromField->append(fromFieldLabel.data());
}

void TaoTransportAgent::getToField(UtlString* toField) const
{
        toField->remove(0);
        toField->append(toFieldLabel.data());
}

long TaoTransportAgent::getLastTouchedTime() const
{
        return(touchedTime);
}

int TaoTransportAgent::getHostPort() const
{
        int port = PORT_NONE;
        if(mpSocket)
        {
                port = mpSocket->getRemoteHostPort();
        }
        return(port);
}

// Set the errno status for the task.
// This call has no effect under Windows NT and, if the task has been
// started, will always returns OS_SUCCESS
OsStatus TaoTransportAgent::setErrno(int errno)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

UtlBoolean TaoTransportAgent::isOk()
{
        return(mpSocket->isOk() && !isShuttingDown());
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

int TaoTransportAgent::readUntilDone(OsSocket* pSocket, char *pBuf, int iLength)
{
        int iTotalRead = 0 ;
        int iRead = iLength ;

        while ((iRead > 0) && (iTotalRead < iLength) &&
                        (pSocket->isReadyToRead(HTTP_READ_TIMEOUT_MSECS))) {
                iRead = pSocket->read(&pBuf[iTotalRead], iLength-iTotalRead);
                iTotalRead += iRead ;
        }

        return iTotalRead ;
}

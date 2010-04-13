//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

#include <assert.h>
#include "os/OsConnectionSocket.h"
#include "os/OsLock.h"
#include "os/OsProtectEvent.h"
#include "tao/TaoClientTask.h"
#include "tao/TaoTransportTask.h"
#include "tao/TaoString.h"
#include "tao/TaoListenerClient.h"
#include "ptapi/PtTerminalConnectionEvent.h"
#include "ptapi/PtTerminalConnectionListener.h"
#include "ptapi/PtTerminalComponentListener.h"
#include "ptapi/PtTerminalComponentEvent.h"
#include "ptapi/PtConnectionEvent.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoClientTask::TaoClientTask(int port,
                                  UtlString host,
                                  TaoServerTask *pTaoServerTask,
                                  const UtlString& name,
                                  const int maxRequestQMsgs)
: OsServerTask(name, NULL, maxRequestQMsgs),
mRemotePort(port),
mRemoteHost(host),
mMutex(OsRWMutex::Q_PRIORITY)
{
        initInstance();
        mpConnectionSocket = 0;
        mpAgent = NULL;
        mpTaoServerTask = pTaoServerTask;
        if (!isStarted())
        {
                start();
        }

}

// Constructor
TaoClientTask::TaoClientTask(TaoServerTask *pTaoServerTask,
                const UtlString& name,
                void* pArg,
                const int maxRequestQMsgs,
                const int priority,
                const int options,
                const int stackSize)
: OsServerTask(name, pArg, maxRequestQMsgs, priority, options, stackSize),
mMutex(OsRWMutex::Q_PRIORITY)
{
        mpConnectionSocket = 0;
        mpTaoServerTask = pTaoServerTask;
        mpAgent = NULL;
        initInstance();
        if (!isStarted())
        {
                start();
        }
}

TaoClientTask::TaoClientTask(const int maxIncomingQMsgs, TaoServerTask *pTaoServerTask)
: OsServerTask("TaoClient", NULL, maxIncomingQMsgs),
mMutex(OsRWMutex::Q_PRIORITY)
{
        mpConnectionSocket = 0;
        mpTaoServerTask = pTaoServerTask;
        mpAgent = NULL;
        initInstance();
        if (!isStarted())
        {
                start();
        }
}

// Destructor
TaoClientTask::~TaoClientTask()
{
        if (mpTransport)
        {
                delete mpTransport;
                mpTransport = 0;
        }

        if (mpConnectionSocket)
        {
                mpConnectionSocket->close();    // shut down the transport task
                delete mpConnectionSocket;
                mpConnectionSocket = 0;
        }

        if (mpListenerTask)
        {
                delete mpListenerTask;
                mpListenerTask = 0;
        }

        if (mpAgent)
        {
                delete mpAgent;
                mpAgent = NULL;
        }
}

void TaoClientTask::requestShutdown()
{
        // Need to put a Mutex on the call stack


        // Pass the shut down to itself
        OsServerTask::requestShutdown();
        yield();

        if (mpConnectionSocket)
        {
                TaoMessage msg = TaoMessage(TaoMessage::REQUEST_PROVIDER,
                                                                                        TaoMessage::SHUTDOWN,
                                                                                        0,
                                                                                        0,
                                                                                        (TaoObjHandle)mpConnectionSocket,
                                                                                        0,
                                                                                        "");

                postMessage(msg);
        }

        if(mpTransport)
        {
                osPrintf("---- TaoClientTask::initInstance: stoping transport\n");
                mpTransport->stopListening();    // shut down the transport task
                mpTransport->requestShutdown();    // shut down the transport task
        }
}

//////////////////////////////////////////////////////////////////////
// MANIPULATORS
//////////////////////////////////////////////////////////////////////

// Initialization, called by constructor
TaoStatus TaoClientTask::initInstance()
{
        mpTransport = new TaoTransportTask(DEF_TAO_EVENT_PORT);

        if (!mpTransport)
        {
                return TAO_FAILURE;
        }
        mpListenerTask = new TaoListenerClientTask(this);

        mpTransport->setServer(mpListenerTask); // let TaoListenerClientTask handle events
        if (!(mpTransport->isStarted()))
        {
                mpTransport->start();
        }
        mpTransport->startListening();

        return TAO_SUCCESS;
}


UtlBoolean TaoClientTask::handleMessage(OsMsg& rMsg)
{
///////////////////////////////////////////////////////////
//
        UtlBoolean handled = TRUE;
//      This should really initialized as FALSE.  But for now
//      we use this to avoid assertion when the scroll wheel
//      spins too fast and receiveEvent fails.
//
//      Need to be fixed later. ------ Feng 3/26/2000
//
///////////////////////////////////////////////////////////

   switch (rMsg.getMsgSubType())
   {
        case TaoMessage::REQUEST_ADDRESS:
        case TaoMessage::REQUEST_CALL:
        case TaoMessage::REQUEST_CONNECTION:
        case TaoMessage::REQUEST_PHONECOMPONENT:
        case TaoMessage::REQUEST_PROVIDER:
        case TaoMessage::REQUEST_TERMINAL:
        case TaoMessage::REQUEST_TERMCONNECTION:
                if (sendRequest((TaoMessage&) rMsg, 0))
                {
                        handled = TRUE;
                }
                break;

        case TaoMessage::RESPONSE_ADDRESS:
        case TaoMessage::RESPONSE_CALL:
        case TaoMessage::RESPONSE_CONNECTION:
        case TaoMessage::RESPONSE_PROVIDER:
        case TaoMessage::RESPONSE_TERMCONNECTION:
        case TaoMessage::RESPONSE_TERMINAL:
        case TaoMessage::RESPONSE_PHONECOMPONENT:
                handled = receiveMsg((TaoMessage&) rMsg);
                if (!handled)
                {
                        osPrintf("TaoClientTask::handleMessage response msg not handled msg subtype = %d\n", rMsg.getMsgSubType());
                        UtlString buffer;
                        ssize_t bufferLen;
                        ((TaoMessage&) rMsg).getBytes(&buffer, &bufferLen);
                        osPrintf("%s\n", buffer.data());

                        ///////////  WE DONT KNOW WHY IT WOULDN"T BE HANDLED
                }

                break;

        case TaoMessage::UNSPECIFIED:
        default:
//              assert(FALSE);
                handled = FALSE;
                osPrintf("\n ERROR! TaoClientTask::handleMessage - UNKNOWN MESSAGE TYPE %d\n", rMsg.getMsgSubType());
                UtlString buffer;
                ssize_t bufferLen;
                ((TaoMessage&) rMsg).getBytes(&buffer, &bufferLen);
                osPrintf("%s\n", buffer.data());
                break;
   }

   return handled;
}

UtlBoolean TaoClientTask::receiveMsg(TaoMessage& rMsg)
{
        TaoObjHandle appHandle = 0;

        appHandle = rMsg.getSocket();   // where the app pointer is stored
        if (appHandle)
        {
                OsProtectedEvent*               pEvent;
                TaoObjHandle    handle;
                UtlString               argList;
                int                             data;

                pEvent  = (OsProtectedEvent *)appHandle;
                handle  = rMsg.getTaoObjHandle();
                data    = rMsg.getArgCnt();
                argList = rMsg.getArgList();

                pEvent->setIntData(data);

                data = rMsg.getCmd();
                pEvent->setIntData2(data);
                pEvent->setStringData(argList);
                // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pEvent->signal(handle))
        {
            OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
            eventMgr->release(pEvent);
        }

                return TRUE;
        }
        else
        {
                osPrintf("<<<< TaoClientTask::receiveMsg: no appHandle!! >>>>\n");
                return FALSE;
        }
}


int TaoClientTask::sendRequest(TaoMessage& rMsg, OsMutex* pMutex, const OsTime& rTimeout)
{
        if (mpTaoServerTask)
        {
                rMsg.setMsgQueueHandle((TaoObjHandle) this);
                mpTaoServerTask->postMessage(rMsg);
                return 1;
        }
        else
        {
                osPrintf("\n++++++ TaoClientTask::sendRequest mpTaoServerTask = 0x%p +++++\n", mpTaoServerTask);
                osPrintf("\n++++++ %d %d +++++\n", rMsg.getMsgSubType(), rMsg.getCmd());
        }

        osPrintf("\n++++++ TaoClientTask::sendRequest %p %s : %d+++++\n", mpConnectionSocket, mRemoteHost.data(), mRemotePort);
        if (!mpConnectionSocket)
        {
                mMutex.acquireWrite();
                mpConnectionSocket = new OsConnectionSocket(mRemotePort, mRemoteHost);
                if (mpAgent)
                {
                        delete mpAgent;
                }
                mpAgent = new TaoTransportAgent(mpConnectionSocket, this);
                mpAgent->start();
                mMutex.releaseWrite();
        }

        int recvd = 0;
        if (mpConnectionSocket && mpConnectionSocket->isOk())
        {
                unsigned int sent;
                mMutex.acquireWrite();

            UtlString buffer;
            ssize_t bufferLen;
            rMsg.getBytes(&buffer, &bufferLen);

                size_t iSendSize = bufferLen + (sizeof(uint32_t)*2) ;

                char* pBuf = new char[iSendSize] ;

                uint32_t cookie = 0x1234ABCD ;
                uint32_t length = bufferLen ;
                memcpy(&pBuf[0], &cookie, sizeof(uint32_t)) ;
                memcpy(&pBuf[sizeof(uint32_t)], &length, sizeof(uint32_t)) ;
                memcpy(&pBuf[sizeof(uint32_t)*2], buffer.data(), bufferLen) ;
                sent = mpConnectionSocket->write(pBuf, iSendSize) ;

                delete pBuf ;

                if (sent > sizeof(uint32_t)*2)
                        sent -= sizeof(uint32_t)*2 ;

                mMutex.releaseWrite();
        }

        return recvd;
}

void TaoClientTask::addEventListener(PtEventListener* pListener, const char* callId)
{
        if (mpListenerTask)
        {
                mpListenerTask->addEventListener(pListener, callId);
        }
}

void TaoClientTask::removeEventListener(PtEventListener& rListener)
{
        if (mpListenerTask)
        {
                mpListenerTask->removeEventListener(rListener);
        }
}

int TaoClientTask::readUntilDone(OsConnectionSocket* pSocket, char *pBuf, int iLength)
{
        int iTotalRead = 0 ;
        int iRead = iLength ;

        if (!pSocket)
                return 0;

        while ((iRead > 0) && (iTotalRead < iLength) &&
                        (pSocket->isReadyToRead(HTTP_READ_TIMEOUT_MSECS))) {
                iRead = pSocket->read(&pBuf[iTotalRead], iLength-iTotalRead);
                iTotalRead += iRead ;
        }

        return iTotalRead ;
}


int TaoClientTask::resetConnectionSocket(int transactionId)
{
        if (mpConnectionSocket)
        {
                mpConnectionSocket->close();
                delete mpConnectionSocket;
                mpConnectionSocket = 0;
        }

        return 0;
}

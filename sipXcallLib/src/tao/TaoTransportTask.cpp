//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <assert.h>

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

#include "tao/TaoTransportTask.h"
#include "tao/TaoServerTask.h"
#include "tao/TaoListeningTask.h"
#include "tao/TaoTransportAgent.h"

TaoReference*   TaoTransportTask::mpTransactionCnt = 0;
int                             TaoTransportTask::mRef = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoTransportTask::TaoTransportTask(const char *host, const char *port) :
 OsServerTask("TaoTransport-%d")
{
        mRemoteHost = host;
        mRemotePort = atoi(port);
        mListenerPort = DEF_TAO_LISTEN_PORT;

        initialize();

        mClientHandle = 0;
        mpListenSocket = NULL;
        mpTaoListeningTask = NULL;
}

TaoTransportTask::TaoTransportTask(const int port) :
 OsServerTask("TaoTransport-%d")
{
        mRemotePort = port;
        mListenerPort = port;

        initialize();

        mClientHandle = 0;
        mpListenSocket = NULL;
        mpTaoListeningTask = NULL;
}

TaoTransportTask::TaoTransportTask() :
 OsServerTask("TaoTransport-%d")
{

        initialize();

        mClientHandle = 0;
        mpListenSocket = NULL;
        mpTaoListeningTask = NULL;
}

TaoTransportTask::~TaoTransportTask()
{
        stopListening();

        if (--mRef < 1)
        {
                if (mpTransactionCnt)
                {
                        delete mpTransactionCnt;
                        mpTransactionCnt = 0;
                }
        }
}

void TaoTransportTask::initialize()
{
        mRef++;

        if (!mpTransactionCnt)
                mpTransactionCnt = new TaoReference();
}

//////////////////////////////////////////////////////////////////////
// MANIPULATORS
//////////////////////////////////////////////////////////////////////
int TaoTransportTask::startListening( void )
{
#ifdef TAO_REMOTING
        stopListening();
        mpListenSocket = new OsServerSocket(64, mListenerPort);

        mpTaoListeningTask = new TaoListeningTask((OsServerTask *)mpServer, mpListenSocket);
        mpTaoListeningTask->start();
#endif
        return 0;
}


int TaoTransportTask::stopListening( void )
{

        if (!isStarted() || !mpTaoListeningTask)
          return 0;

        osPrintf("---- TaoTransportTask::stopListening socket %p\n", mpTaoListeningTask);
        if (mpTaoListeningTask->isStarted())
        {
                mpTaoListeningTask->requestShutdown();
                delete mpTaoListeningTask;
                mpTaoListeningTask = NULL;
        }

        if ( mpListenSocket != NULL )
        {
                mpListenSocket->close();
                delete mpListenSocket;
                mpListenSocket = NULL;
        }
        return 0;
}

UtlBoolean TaoTransportTask::handleMessage(OsMsg& rMsg)
{
        UtlBoolean handled = FALSE;

    // osPrintf("************* MsgType = %d Subtype = %d\n",rMsg.getMsgType(),rMsg.getMsgSubType());
    // printf("************* MsgType = %d Subtype = %d\n",rMsg.getMsgType(),rMsg.getMsgSubType());

        switch (rMsg.getMsgSubType())
        {
        case TaoMessage::RESPONSE_ADDRESS:
        case TaoMessage::RESPONSE_CALL:
        case TaoMessage::RESPONSE_CONNECTION:
        case TaoMessage::RESPONSE_PROVIDER:
        case TaoMessage::RESPONSE_TERMCONNECTION:
        case TaoMessage::RESPONSE_TERMINAL:
        case TaoMessage::RESPONSE_PHONECOMPONENT:
                if (send((TaoMessage&) rMsg) > 0)
                {
                        handled = TRUE;
                }
                else
                {
                        osPrintf("TaoTransportTask::handleMessage: Response send returns negative or zero bytes.\n");
                }
                break;
        case TaoMessage::EVENT:
                if (send((TaoMessage&) rMsg) > 0)
                {
                        handled = TRUE;
                }
                else
                {
                        osPrintf("TaoTransportTask::handleMessage: Event send returns negative or zero bytes.\n");
                }
                break;
        case TaoMessage::UNSPECIFIED:
        default:
        osPrintf("\n\n\n\n\nUNHANDLED MESSAGE in TaoTransportTask:  MsgType = %d Subtype = %d\n\n\n\n",rMsg.getMsgType(),rMsg.getMsgSubType());
        assert(FALSE);
                break;
        }

        return handled;
}

int TaoTransportTask::send(TaoMessage& rMsg)
{
        unsigned int type = rMsg.getMsgSubType();
        unsigned int cmd = rMsg.getCmd();
        if ((TaoMessage::RESPONSE_PROVIDER == type) && (cmd == TaoMessage::SHUTDOWN))
        {
                TaoTransportAgent* pAgent = (TaoTransportAgent *) rMsg.getSocket();

                mpTaoListeningTask->shutdownAgent(pAgent);
                return 0;
        }

        // record this transaction in the transaction db
        rMsg.getMsgID();

        mpTransactionCnt->add();
        mpTransactionCnt->getRef();

        TaoObjHandle remoteServer = rMsg.getMsgQueueHandle();

        if (mClientHandle)
        {
                ((OsServerTask *) mClientHandle)->postMessage(rMsg);
                return 1;
        }
        else if (remoteServer)
        {
                ((OsServerTask *) remoteServer)->postMessage(rMsg);
                return 1;
        }

        // send the msg to the transport, receive the response
        int sent = 0;
        TaoTransportAgent* pAgent = (TaoTransportAgent *) rMsg.getSocket();
        if (pAgent && !pAgent->isShuttingDown())
        {
                sent = pAgent->send(rMsg);
        }

        return sent;
}

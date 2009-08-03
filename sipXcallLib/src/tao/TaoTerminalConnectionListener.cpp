//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////


// TaoTerminalConnectionListener.cpp: implementation of the TaoTerminalConnectionListener class.
//
//////////////////////////////////////////////////////////////////////
#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

#include "tao/TaoTerminalConnectionListener.h"
#include "tao/TaoTransportTask.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
TaoTerminalConnectionListener::TaoTerminalConnectionListener(PtEventMask* pMask):
PtTerminalConnectionListener(pMask)
{
        mTerminalName = 0;
}

TaoTerminalConnectionListener::TaoTerminalConnectionListener(int objId,
                                                                                                                         TaoObjHandle clientSocket,
                                                                                                                         TaoTransportTask* pSvrTransport,
                                                                                                                         const char * terminalName)
: PtTerminalConnectionListener(NULL)
{
        mObjId = objId;
        mhClientSocket = clientSocket;
        mpSvrTransport = pSvrTransport;
        int len = strlen(terminalName);
        osPrintf("TaoTerminalConnectionListener: objId %d terminal name: %s\n", objId, terminalName);
        if (len > 0)
        {
                mTerminalName = new char[len];
                strcpy(mTerminalName, terminalName);
        }
        else
        {
                mTerminalName = 0;
        }
        mpConnectionSocket = new OsConnectionSocket(DEF_TAO_EVENT_PORT, mTerminalName);

}

TaoTerminalConnectionListener::~TaoTerminalConnectionListener()
{
        if (mTerminalName)
        {
                delete[] mTerminalName;
                mTerminalName = 0;
        }
}

PtStatus TaoTerminalConnectionListener::getLocation(UtlString*& rpLocation)
{
        if (rpLocation)
                rpLocation->append(mTerminalName);

        return PT_SUCCESS;
}

void TaoTerminalConnectionListener::terminalConnectionCreated(const PtTerminalConnectionEvent& rEvent)
{
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::EVENT,
                                                                        TaoMessage::ADD_CALL_LISTENER,
                                                                        mObjId,
                                                                        PtEvent::TERMINAL_CONNECTION_CREATED,
                                                                        (TaoObjHandle)mpConnectionSocket,
                                                                        0,
                                                                        "");

        mpSvrTransport->postMessage(*pMsg);
        delete pMsg;
}

void TaoTerminalConnectionListener::terminalConnectionIdle(const PtTerminalConnectionEvent& rEvent)
{
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::EVENT,
                                                                        TaoMessage::ADD_CALL_LISTENER,
                                                                        mObjId,
                                                                        PtEvent::TERMINAL_CONNECTION_IDLE,
                                                                        (TaoObjHandle)mpConnectionSocket,
                                                                        0,
                                                                        "");

        mpSvrTransport->postMessage(*pMsg);
        delete pMsg;
}

void TaoTerminalConnectionListener::terminalConnectionRinging(const PtTerminalConnectionEvent& rEvent)
{
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::EVENT,
                                                                        TaoMessage::ADD_CALL_LISTENER,
                                                                        mObjId,
                                                                        PtEvent::TERMINAL_CONNECTION_RINGING,
                                                                        (TaoObjHandle)mpConnectionSocket,
                                                                        0,
                                                                        "");

        mpSvrTransport->postMessage(*pMsg);
        delete pMsg;
}

void TaoTerminalConnectionListener::terminalConnectionDropped(const PtTerminalConnectionEvent& rEvent)
{
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::EVENT,
                                                                        TaoMessage::ADD_CALL_LISTENER,
                                                                        mObjId,
                                                                        PtEvent::TERMINAL_CONNECTION_DROPPED,
                                                                        (TaoObjHandle)mpConnectionSocket,
                                                                        0,
                                                                        "");

        mpSvrTransport->postMessage(*pMsg);
        delete pMsg;
}

void TaoTerminalConnectionListener::terminalConnectionUnknown(const PtTerminalConnectionEvent& rEvent)
{
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::EVENT,
                                                                        TaoMessage::ADD_CALL_LISTENER,
                                                                        mObjId,
                                                                        PtEvent::TERMINAL_CONNECTION_UNKNOWN,
                                                                        (TaoObjHandle)mpConnectionSocket,
                                                                        0,
                                                                        "");

        mpSvrTransport->postMessage(*pMsg);
        delete pMsg;
}

void TaoTerminalConnectionListener::terminalConnectionHeld(const PtTerminalConnectionEvent& rEvent)
{
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::EVENT,
                                                                        TaoMessage::ADD_CALL_LISTENER,
                                                                        mObjId,
                                                                        PtEvent::TERMINAL_CONNECTION_HELD,
                                                                        (TaoObjHandle)mpConnectionSocket,
                                                                        0,
                                                                        "");

        mpSvrTransport->postMessage(*pMsg);
        delete pMsg;
}

void TaoTerminalConnectionListener::terminalConnectionTalking(const PtTerminalConnectionEvent& rEvent)
{
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::EVENT,
                                                                        TaoMessage::ADD_CALL_LISTENER,
                                                                        mObjId,
                                                                        PtEvent::TERMINAL_CONNECTION_TALKING,
                                                                        (TaoObjHandle)mpConnectionSocket,
                                                                        0,
                                                                        "");

        mpSvrTransport->postMessage(*pMsg);
        delete pMsg;
}

void TaoTerminalConnectionListener::terminalConnectionInUse(const PtTerminalConnectionEvent& rEvent)
{
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::EVENT,
                                                                        TaoMessage::ADD_CALL_LISTENER,
                                                                        mObjId,
                                                                        PtEvent::TERMINAL_CONNECTION_IN_USE,
                                                                        (TaoObjHandle)mpConnectionSocket,
                                                                        0,
                                                                        "");

        mpSvrTransport->postMessage(*pMsg);
        delete pMsg;
}

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

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include "ptapi/PtConnection.h"
#include "ptapi/PtAddress.h"
#include "ptapi/PtCall.h"
#include "ptapi/PtTerminal.h"
#include "ptapi/PtAddress.h"
#include "ptapi/PtTerminalConnection.h"
#include "cp/Connection.h"
#include "cp/CpGatewayManager.h"
#include "os/OsStatus.h"
#include "tao/TaoClientTask.h"
#include "tao/TaoServerTask.h"
#include "tao/TaoEvent.h"
#include "tao/TaoString.h"
#include "net/SipMessage.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
OsBSem               PtConnection::semInit(OsBSem::Q_PRIORITY, OsBSem::FULL) ;
TaoReference            *PtConnection::mpTransactionCnt = 0;
unsigned int            PtConnection::mRef = 0;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtConnection::PtConnection()
{
        initialize();
        mpClient = 0;
        mState = IDLE;

}

PtConnection::PtConnection(TaoClientTask *pClient, const char* address, const char* callId)
{
        mpClient = pClient;

        if (address)
                mAddress = address;

        if (callId)
                mCallId = callId;

        mState = ESTABLISHED;

        initialize();

        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }
}

// Copy constructor
PtConnection::PtConnection(const PtConnection& rPtConnection)
{
        mAddress = rPtConnection.mAddress;
        mCallId = rPtConnection.mCallId;
        mState = rPtConnection.mState;
        mpClient = rPtConnection.mpClient;

        initialize();
}

// Destructor
PtConnection::~PtConnection()
{
   semInit.acquire() ;
        mRef--;
        if (mRef < 1)
        {
                if(mpTransactionCnt)
                {
                        delete mpTransactionCnt;
                        mpTransactionCnt = 0;
                }
        }
   semInit.release() ;
}

void PtConnection::initialize()
{
        mpEventMgr = OsProtectEventMgr::getEventMgr();
        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
    semInit.acquire() ;

        mRef++;

        if (!mpTransactionCnt)
                mpTransactionCnt = new TaoReference();

   semInit.release() ;

}
/* ============================ MANIPULATORS ============================== */
// Assignment operator
PtConnection&
PtConnection::operator=(const PtConnection& rhs)
{
        if (this == &rhs)            // handle the assignment to self case
          return *this;

        mAddress.remove(0);
        mCallId.remove(0);
        if (!rhs.mAddress.isNull())
                mAddress = rhs.mAddress;
        if (!rhs.mCallId.isNull())
                mCallId = rhs.mCallId;
        mState = rhs.mState;
        mpClient = rhs.mpClient;
        mTimeOut = rhs.mTimeOut;

        return *this;
}

PtStatus PtConnection::accept()
{
        EVENT_TRACE("Entering PtConnection::accept\n") ;
        UtlString arg = mCallId + TAOMESSAGE_DELIMITER + mAddress;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CONNECTION,
                                                                        TaoMessage::ACCEPT,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::ACCEPT);
#endif
        mpEventMgr->release(pe);
        EVENT_TRACE("Exiting PtConnection::accept\n") ;

        return PT_SUCCESS;
}

PtStatus PtConnection::disconnect()
{
        EVENT_TRACE("Entering PtConnection::disconnect\n") ;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        UtlString arg(mCallId);

        arg += TAOMESSAGE_DELIMITER + mAddress;

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CONNECTION,
                                                                        TaoMessage::DISCONNECT,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::DISCONNECT);
#endif
        mpEventMgr->release(pe);

        mState = DISCONNECTED;
        EVENT_TRACE("Exiting PtConnection::disconnect\n") ;

        return PT_SUCCESS;
}

PtStatus PtConnection::getTerminalConnections(PtTerminalConnection termConnections[],
                                                                                           int size,
                                                                                           int& nItems)
{
        EVENT_TRACE("Entering PtConnection::getTerminalConnections\n") ;
        PtStatus statusRC = PT_NO_MORE_DATA ;
        UtlString arg = mCallId + TAOMESSAGE_DELIMITER + mAddress;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CONNECTION,
                                                                        TaoMessage::GET_TERM_CONNECTIONS,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        arg);
        mpClient->sendRequest(msg);

        arg.remove(0);
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
                EVENT_TRACE("Exiting PtConnection::getTerminalConnections: TIME OUT!!\n") ;
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_TERM_CONNECTIONS);
#endif
        mpEventMgr->release(pe);

        TaoString argList(arg, TAOMESSAGE_DELIMITER);

        nItems = (argList.getCnt() - 2) / 2 ;

        int cnt = (nItems > size) ? size : nItems ;

        nItems = 0;
        for (int i = 0; i < cnt; i++)
        {
                int j = 2 * i;
                int isLocal = atoi(argList[j + 3]);
                PtTerminalConnection tc = PtTerminalConnection(mpClient,
                                                                                                           mAddress.data(),
                                                                                                           argList[i+2],
                                                                                                           mCallId.data(),
                                                                                                           isLocal);
#ifdef TEST
                osPrintf("~~~ PtConnection::getTerminalConnections connection %s terminal %s is local: %d ~~~\n", mAddress.data(), argList[j+2].data(), isLocal);
#endif
                termConnections[nItems++] = tc;
                statusRC = PT_SUCCESS ;
        }

#ifdef TEST
        osPrintf("~~~ number of tc %d ~~~\n", nItems);
#endif

        EVENT_TRACE("Exiting PtConnection::getTerminalConnections\n") ;
        return statusRC ;
}


PtStatus PtConnection::getAddress(PtAddress& rAddress)
{
        EVENT_TRACE("Entering PtConnection::getAddress\n") ;
        PtStatus rc = PT_SUCCESS;
        if (mpClient && !mAddress.isNull())
        {
                rAddress = PtAddress(mpClient, mAddress.data());
        }
        else
        {
                rc = PT_NO_MORE_DATA;
        }

        EVENT_TRACE("Exiting PtConnection::getAddress\n") ;
        return rc;
}

PtStatus PtConnection::getCall(PtCall& rCall)
{
        EVENT_TRACE("Entering PtConnection::getCall\n") ;
        PtStatus rc = PT_SUCCESS;
        if (mpClient && !mCallId.isNull())
        {
                rCall = PtCall(mpClient, mCallId.data());
        }
        else
        {
                rc = PT_NO_MORE_DATA;
        }
        EVENT_TRACE("Exiting PtConnection::getCall\n");
        return rc;
}

PtStatus PtConnection::getSessionInfo(PtSessionDesc& rSession)
{
        EVENT_TRACE("Entering PtCall::getSessionInfo:\n") ;
        PtStatus ret = PT_FAILED;

        UtlString arg = mCallId + TAOMESSAGE_DELIMITER + mAddress;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CONNECTION,
                                                                        TaoMessage::GET_SESSION_INFO,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        arg);
        mpClient->sendRequest(msg);

        arg.remove(0);
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_FROM_FIELD);
#endif
        mpEventMgr->release(pe);

        TaoString argList(arg, TAOMESSAGE_DELIMITER);

        if (argList.getCnt() == 7)
        {
                rSession = PtSessionDesc(mCallId,
                                                                 argList[0],                    //toUrl
                                                                 argList[1],                    //fromUrl
                                                                 argList[2],    // localContact
                                                                 atoi(argList[3]),      // next CSeq
                                                                 atoi(argList[4]),      // lastFromCseq
                                                                 atoi(argList[5]),      // lastToCseq
                         atoi(argList[6]));     // sessionState

                ret = PT_SUCCESS;
        }

        EVENT_TRACE("Exiting PtCall::getSessionInfo\n") ;
        return ret;
}

PtStatus PtConnection::getState(int& rState)
{
        EVENT_TRACE("Entering PtConnection::getState\n") ;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        UtlString arg(mCallId);

        arg += TAOMESSAGE_DELIMITER + mAddress;

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CONNECTION,
                                                                        TaoMessage::GET_STATE,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        arg);
        mpClient->sendRequest(msg);

        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        intptr_t rc;
        intptr_t argCnt = 0;
        arg.remove(0);
        pe->getEventData(rc);
        pe->getIntData(argCnt);
        pe->getStringData(arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_STATE);
#endif
        mpEventMgr->release(pe);

        PtStatus ret = PT_SUCCESS;
        if (argCnt == 1)
        {
                mState = rState = atoi(arg.data());
        }
        else
        {
                mState = rState = UNKNOWN;
                ret = PT_NOT_FOUND;
        }
        EVENT_TRACE("Exiting PtConnection::getState\n") ;
        return ret;
}

PtStatus PtConnection::getToField(char* pName, int len)
{
        EVENT_TRACE("Entering PtConnection::getToField\n") ;
        PtStatus ret = PT_FAILED;
        if (pName == 0)
                return ret;

        UtlString arg = mCallId + TAOMESSAGE_DELIMITER + mAddress;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CONNECTION,
                                                                        TaoMessage::GET_TO_FIELD,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        arg);
        mpClient->sendRequest(msg);

        arg.remove(0);
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_TO_FIELD);
#endif
        mpEventMgr->release(pe);

        TaoString argList(arg, TAOMESSAGE_DELIMITER);

        if (argList.getCnt() == 2)
        {
                ret = (PtStatus) atoi(argList[0]);
                arg = argList[1];
                int l = arg.length();
                if (l > len) l = len;
                strncpy(pName, arg.data(), l);
                pName[l] = 0;
        }

        EVENT_TRACE("Exiting PtConnection::getToField\n") ;
        return ret;
}

PtStatus PtConnection::getFromField(char* pName, int len)
{
        EVENT_TRACE("Entering PtConnection::getFromField\n") ;
        PtStatus ret = PT_FAILED;
        if (pName == 0)
                return ret;

        UtlString arg = mCallId + TAOMESSAGE_DELIMITER + mAddress;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CONNECTION,
                                                                        TaoMessage::GET_FROM_FIELD,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        arg);
        mpClient->sendRequest(msg);

        arg.remove(0);
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_FROM_FIELD);
#endif
        mpEventMgr->release(pe);

        TaoString argList(arg, TAOMESSAGE_DELIMITER);

        if (argList.getCnt() == 2)
        {
                ret = (PtStatus) atoi(argList[0]);
                arg = argList[1];
                int l = arg.length();
                if (l > len) l = len;
                strncpy(pName, arg.data(), l);
                pName[l] = 0;
        }

        EVENT_TRACE("Exiting PtConnection::getFromField\n") ;
        return ret;
}


PtStatus PtConnection::numTerminalConnections(int& count)
{
        EVENT_TRACE("Entering PtConnection::numTerminalConnections\n") ;
        UtlString arg = mCallId + TAOMESSAGE_DELIMITER + mAddress;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CONNECTION,
                                                                        TaoMessage::NUM_TERM_CONNECTIONS,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        arg);
        mpClient->sendRequest(msg);

        arg.remove(0);
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::NUM_TERM_CONNECTIONS);
#endif
        mpEventMgr->release(pe);

        TaoString argList(arg, TAOMESSAGE_DELIMITER);

        if (argList.getCnt() > 2)
        {
                count = atoi(argList[2]);
        }
        else
        {
                count = 0;
        }

        EVENT_TRACE("Exiting PtConnection::numTerminalConnections\n") ;
        return PT_SUCCESS;
}

PtStatus PtConnection::park(char* destinationURL, PtConnection& rNewConnection)
{
        EVENT_TRACE("Entering PtConnection::park\n") ;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        UtlString arg = destinationURL + TAOMESSAGE_DELIMITER + mCallId;

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CONNECTION,
                                                                        TaoMessage::PARK,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::PARK);
#endif
        mpEventMgr->release(pe);

        EVENT_TRACE("Exiting PtConnection::park\n") ;
        return PT_SUCCESS;
}

PtStatus PtConnection::redirect(char* destinationURL, PtConnection& rNewConnection)
{
        EVENT_TRACE("Entering PtConnection::redirect\n") ;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        UtlString arg = mAddress + TAOMESSAGE_DELIMITER + destinationURL + TAOMESSAGE_DELIMITER + mCallId;

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CONNECTION,
                                                                        TaoMessage::REDIRECT,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        3,
                                                                        arg);
        mpClient->sendRequest(msg);

        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        intptr_t rc;
        arg.remove(0);

        pe->getEventData(rc);
        pe->getStringData(arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::REDIRECT);
#endif
        mpEventMgr->release(pe);

        TaoString args = TaoString(arg, TAOMESSAGE_DELIMITER);
        UtlString callId = args[2];
        UtlString retCode = args[3];

        rNewConnection = PtConnection(mpClient, destinationURL, callId.data());
        EVENT_TRACE("Exiting PtConnection::redirect\n") ;

        rc = atoi(retCode.data());
        return (PtStatus) rc;
}

PtStatus PtConnection::reject()
{
        EVENT_TRACE("Entering PtConnection::reject\n") ;
        UtlString arg = mCallId + TAOMESSAGE_DELIMITER + mAddress;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CONNECTION,
                                                                        TaoMessage::REJECT,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::REJECT);
#endif
        mpEventMgr->release(pe);
        EVENT_TRACE("Exiting PtConnection::reject\n") ;

        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
// Protected constructor.
PtConnection::PtConnection(const char* address, const char* callId)
{
        if (address)
                mAddress = address;

        if (callId)
                mCallId = callId;

        mpClient = 0;
        mState = IDLE;

        initialize();

}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

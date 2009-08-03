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
#include "ptapi/PtTerminalConnection.h"
#include "ptapi/PtTerminal.h"
#include "ptapi/PtConnection.h"
#include "ptapi/PtTerminal.h"
#include "cp/CpGatewayManager.h"
#include "cp/Connection.h"
#include "tao/TaoClientTask.h"
#include "tao/TaoServerTask.h"
#include "tao/TaoEvent.h"
#include "tao/TaoString.h"


//#define EVENT_TRACE(x) osPrintf(x)
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
OsBSem               PtTerminalConnection::semInit(OsBSem::Q_PRIORITY, OsBSem::FULL) ;
TaoReference            *PtTerminalConnection::mpTransactionCnt = 0;
int                                      PtTerminalConnection::mRef = 0;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtTerminalConnection::PtTerminalConnection()
        : mIsLocal(-1),
   mpClient(NULL)
{
        initialize();

}

// Copy constructor
PtTerminalConnection::PtTerminalConnection(const PtTerminalConnection& rPtTerminalConnection)
{
        mAddress = rPtTerminalConnection.mAddress;
        mTerminalName = rPtTerminalConnection.mTerminalName;
        mCallId = rPtTerminalConnection.mCallId;

        mpClient = rPtTerminalConnection.mpClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        mIsLocal = rPtTerminalConnection.mIsLocal;

        initialize();
}

PtTerminalConnection::PtTerminalConnection(TaoClientTask *pClient, const char* address, const char* termName, const char* callId, int nIsLocal)
{
        mpClient = pClient;
        mIsLocal = nIsLocal;

        if (address)
                mAddress = address;

        if (termName)
                mTerminalName = termName;


        if (callId)
                mCallId = callId;

        initialize();

        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }
        UtlBoolean local;
        if ((mIsLocal == (-1)) && (PT_SUCCESS == isLocal(local)))
                mIsLocal = local;
}

void PtTerminalConnection::initialize()
{
        mpEventMgr = OsProtectEventMgr::getEventMgr();
        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
    semInit.acquire() ;

        mRef++;

        if (!mpTransactionCnt)
                mpTransactionCnt = new TaoReference();

    semInit.release() ;

}
// Destructor
PtTerminalConnection::~PtTerminalConnection()
{
   semInit.acquire() ;
        mRef--;
        if (mRef < 1)
        {
                if (mpTransactionCnt)
                {
                        delete mpTransactionCnt;
                        mpTransactionCnt = 0;
                }
        }

    semInit.release() ;
}

/* ============================ MANIPULATORS ============================== */
// Assignment operator
PtTerminalConnection&
PtTerminalConnection::operator=(const PtTerminalConnection& rhs)
{
        if (this == &rhs)            // handle the assignment to self case
          return *this;

        mAddress = rhs.mAddress;
        mTerminalName = rhs.mTerminalName;
        mCallId = rhs.mCallId;

        mpClient = rhs.mpClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        mTimeOut = rhs.mTimeOut;
        mIsLocal = rhs.mIsLocal;

        return *this;
}

// Answer an incoming telephone call on this PtTerminalConnection
PtStatus PtTerminalConnection::answer()
{
        EVENT_TRACE("Entering PtTerminalConnection::answer\n") ;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->add();

        UtlString arg = mTerminalName + TAOMESSAGE_DELIMITER + mCallId;

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::ANSWER,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        UtlString callId;

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
        pe->getStringData(callId);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::ANSWER);
#endif
        mpEventMgr->release(pe);
        EVENT_TRACE("Exiting PtTerminalConnection::answer\n") ;

        return PT_SUCCESS;
}

PtStatus PtTerminalConnection::getConnection(PtConnection& rConnection)
{
        EVENT_TRACE("Entering PtTerminalConnection::getConnection\n") ;
        UtlString arg = mCallId + TAOMESSAGE_DELIMITER + mAddress;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::GET_CONNECTION,
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
        assert(cmd == TaoMessage::GET_CONNECTION);
#endif
        mpEventMgr->release(pe);

        PtStatus rc = PT_SUCCESS;
        if (!arg.isNull())
        {
                rConnection = PtConnection(mpClient, mAddress, mCallId);
        }
        else
        {
                rc = PT_NO_MORE_DATA;
        }

        EVENT_TRACE("Exiting PtTerminalConnection::getConnection\n") ;
        return rc;
}

PtStatus PtTerminalConnection::hold()
{
        EVENT_TRACE("Entering PtTerminalConnection::hold\n") ;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        UtlString arg = mTerminalName;

        arg += TAOMESSAGE_DELIMITER + mAddress + TAOMESSAGE_DELIMITER + mCallId;
        osPrintf("\nPtTerminalConnection::hold %s %s %s\n", mAddress.data(), mCallId.data(), mTerminalName.data());

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::HOLD,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        3,
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
        assert(cmd == TaoMessage::HOLD);
#endif
        mpEventMgr->release(pe);

        EVENT_TRACE("Exiting PtTerminalConnection::hold\n") ;

        return PT_SUCCESS;
}

PtStatus PtTerminalConnection::unhold()
{
        EVENT_TRACE("Entering PtTerminalConnection::unhold\n") ;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        UtlString arg = mTerminalName;

        arg += TAOMESSAGE_DELIMITER + mAddress + TAOMESSAGE_DELIMITER + mCallId;

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::UNHOLD,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        3,
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
        assert(cmd == TaoMessage::UNHOLD);
#endif
        mpEventMgr->release(pe);

        EVENT_TRACE("Exiting PtTerminalConnection::unhold\n") ;

        return PT_SUCCESS;
}

PtStatus PtTerminalConnection::getState(int& rState)
{
        EVENT_TRACE("Entering PtTerminalConnection::getState\n") ;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        UtlString arg;
        if (!mCallId.isNull() && (!mAddress.isNull() || !mTerminalName.isNull()))
        {
                arg = mCallId + TAOMESSAGE_DELIMITER + mAddress + TAOMESSAGE_DELIMITER + mTerminalName;
        }
        else
                return PT_NO_MORE_DATA;

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::GET_STATE,
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
        intptr_t argCnt = 0;
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
                ret = PT_NOT_FOUND;

        EVENT_TRACE("Exiting PtTerminalConnection::getState\n") ;
        return ret;
}

PtStatus PtTerminalConnection::getTerminal(PtTerminal& rTerminal)
{
        EVENT_TRACE("Entering PtTerminalConnection::getTerminal\n") ;
        UtlString arg = mCallId + TAOMESSAGE_DELIMITER + mTerminalName;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::GET_TERMINAL,
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

        arg.remove(0);
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_TERMINAL);
#endif
        mpEventMgr->release(pe);

        PtStatus rc = PT_SUCCESS;
        if (!arg.isNull())
        {
                rTerminal = PtTerminal(mTerminalName.data(), mpClient);
        }
        else
        {
                rc = PT_NO_MORE_DATA;
        }

        EVENT_TRACE("Exiting PtTerminalConnection::getTerminal\n") ;
        return rc;
}

PtStatus PtTerminalConnection::getTerminalName(UtlString& rTerminalName)
{
        rTerminalName.remove(0);
        rTerminalName.append(mTerminalName);
        return PT_SUCCESS;
}

PtStatus PtTerminalConnection::getAddressName(UtlString& rAddress)
{
        rAddress.remove(0);
        rAddress.append(mAddress);
        return PT_SUCCESS;
}

PtStatus PtTerminalConnection::getCallId(UtlString& rCallId)
{
        rCallId.remove(0);
        rCallId.append(mCallId);
        return PT_SUCCESS;
}

PtStatus PtTerminalConnection::startTone(int toneId, UtlBoolean local,
                      UtlBoolean remote, const char* locale)
{
        EVENT_TRACE("Entering PtTerminalConnection::startTone\n") ;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", toneId);
        UtlString arg(buff);

    sprintf(buff, "%d", (int)local);
        arg += TAOMESSAGE_DELIMITER + buff;

    sprintf(buff, "%d", (int)remote);
        arg += TAOMESSAGE_DELIMITER + buff;

        arg += TAOMESSAGE_DELIMITER + mCallId;

        int     cnt = 4;
        if (locale)
        {
                cnt = 5;
                arg += TAOMESSAGE_DELIMITER + locale;
        }

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::START_TONE,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        cnt,
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
        assert(cmd == TaoMessage::START_TONE);
#endif
        mpEventMgr->release(pe);

        EVENT_TRACE("Exiting PtTerminalConnection::startTone\n") ;

        return PT_SUCCESS;
}

PtStatus PtTerminalConnection::stopTone(void)
{
        EVENT_TRACE("Entering PtTerminalConnection::stopTone\n") ;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::STOP_TONE,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        mCallId);
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
        assert(cmd == TaoMessage::STOP_TONE);
#endif
        mpEventMgr->release(pe);
        EVENT_TRACE("Exiting PtTerminalConnection::stopTone\n") ;

        return PT_SUCCESS;
}

PtStatus PtTerminalConnection::playFile(const char* audioFileName, UtlBoolean repeat,
                     UtlBoolean local, UtlBoolean remote)
{
        EVENT_TRACE("Entering PtTerminalConnection::playFile\n") ;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        UtlString arg(audioFileName);

    sprintf(buff, "%d", (int)repeat);
        arg += TAOMESSAGE_DELIMITER + buff;

    sprintf(buff, "%d", (int)local);
        arg += TAOMESSAGE_DELIMITER + buff;

    sprintf(buff, "%d", (int)remote);
        arg += TAOMESSAGE_DELIMITER + buff;

        arg += TAOMESSAGE_DELIMITER + mCallId;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::PLAY_FILE_NAME,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        5,
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
        assert(cmd == TaoMessage::PLAY_FILE_NAME);
#endif
        mpEventMgr->release(pe);

        EVENT_TRACE("Exiting PtTerminalConnection::playFile\n") ;

        return PT_SUCCESS;
}

PtStatus PtTerminalConnection::playFile(FILE* audioFilePtr, UtlBoolean repeat,
                 UtlBoolean local, UtlBoolean remote)
{
        EVENT_TRACE("Entering PtTerminalConnection::playFile\n") ;
   char buff[MAXIMUM_INTEGER_STRING_LENGTH];

   sprintf(buff, "%" PRIdPTR, (intptr_t)audioFilePtr);
        UtlString arg(buff);

    sprintf(buff, "%d", (int)repeat);
        arg += TAOMESSAGE_DELIMITER + buff;

    sprintf(buff, "%d", (int)local);
        arg += TAOMESSAGE_DELIMITER + buff;

    sprintf(buff, "%d", (int)remote);
        arg += TAOMESSAGE_DELIMITER + buff;

        arg += TAOMESSAGE_DELIMITER + mCallId;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::PLAY_FILE_URL,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        mCallId);
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
        assert(cmd == TaoMessage::PLAY_FILE_URL);
#endif
        mpEventMgr->release(pe);

        EVENT_TRACE("Exiting PtTerminalConnection::playFile\n") ;

        return PT_SUCCESS;
}

PtStatus PtTerminalConnection::stopPlay(UtlBoolean closeFile)
{
        EVENT_TRACE("Entering PtTerminalConnection::stopPlay\n") ;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::STOP_PLAY,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        mCallId);
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
        assert(cmd == TaoMessage::STOP_PLAY);
#endif
        mpEventMgr->release(pe);
        EVENT_TRACE("Exiting PtTerminalConnection::stopPlay\n") ;

        return PT_SUCCESS;
}

  //:Creates a Player
PtStatus PtTerminalConnection::createPlayer(MpStreamPlayer** pPlayer, const char* szStream, int flags)
{
        EVENT_TRACE("Entering PtTerminalConnection::createPlayer\n") ;
   char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

   UtlString args;

   sprintf(buff, "%" PRIdPTR, (intptr_t) pPlayer);
   args.append(buff);
   args.append(TAOMESSAGE_DELIMITER) ;
   args.append(szStream) ;
   args.append(TAOMESSAGE_DELIMITER) ;
   sprintf(buff, "%d", flags);
   args.append(buff);
   args.append(TAOMESSAGE_DELIMITER);
   args.append(mCallId);

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::CREATE_PLAYER,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        4,
                                                                        args);
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
        assert(cmd == TaoMessage::CREATE_PLAYER);
#endif
        mpEventMgr->release(pe);
        EVENT_TRACE("Exiting PtTerminalConnection::createPlayer\n") ;

        return PT_SUCCESS;
}

//:Destroys a player
PtStatus PtTerminalConnection::destroyPlayer(MpStreamPlayer* pPlayer)
{
        EVENT_TRACE("Entering PtTerminalConnection::destroyPlayer\n") ;

   char buff[MAXIMUM_INTEGER_STRING_LENGTH];
        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

   UtlString args;


   sprintf(buff, "%" PRIdPTR, (intptr_t) pPlayer);
   args.append(buff);
   args.append(TAOMESSAGE_DELIMITER) ;
   args.append(mCallId);

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::DESTROY_PLAYER,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        args);
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
        assert(cmd == TaoMessage::DESTROY_PLAYER);
#endif
        mpEventMgr->release(pe);
        EVENT_TRACE("Exiting PtTerminalConnection::destroyPlayer\n") ;

        return PT_SUCCESS;
}

//:Creates a playlist Player
PtStatus PtTerminalConnection::createPlaylistPlayer(MpStreamPlaylistPlayer** pPlayer)
{
        EVENT_TRACE("Entering PtTerminalConnection::createPlaylistPlayer\n") ;

   char buff[MAXIMUM_INTEGER_STRING_LENGTH];
        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

   UtlString args;

   sprintf(buff, "%" PRIdPTR, (intptr_t) pPlayer);
   args.append(buff);
   args += TAOMESSAGE_DELIMITER + mCallId ;

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::CREATE_PLAYLIST_PLAYER,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        args);
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
        assert(cmd == TaoMessage::CREATE_PLAYLIST_PLAYER);
#endif
        mpEventMgr->release(pe);
        EVENT_TRACE("Exiting PtTerminalConnection::createPlaylistPlayer\n") ;

        return PT_SUCCESS;
}

//:Destroys a playlist player
PtStatus PtTerminalConnection::destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer)
{
        EVENT_TRACE("Entering PtTerminalConnection::destroyPlaylistPlayer\n") ;

   char buff[MAXIMUM_INTEGER_STRING_LENGTH];
        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

   UtlString args;

   sprintf(buff, "%" PRIdPTR, (intptr_t) pPlayer);
   args.append(buff);
   args.append(TAOMESSAGE_DELIMITER) ;
   args.append(mCallId);

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::DESTROY_PLAYLIST_PLAYER,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        args);
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
        assert(cmd == TaoMessage::DESTROY_PLAYLIST_PLAYER);
#endif
        mpEventMgr->release(pe);
        EVENT_TRACE("Exiting PtTerminalConnection::destroyPlaylistPlayer\n") ;

        return PT_SUCCESS;
}


/* ============================ INQUIRY =================================== */
PtStatus PtTerminalConnection::isLocal(UtlBoolean& local)
{
        EVENT_TRACE("Entering PtTerminalConnection::isLocal\n") ;
        if (mIsLocal != (-1))
        {
                local = mIsLocal;
                EVENT_TRACE("Exiting PtTerminalConnection::isLocal -1\n") ;
                return PT_SUCCESS;
        }

        osPrintf("PtTerminalConnection::isLocal getting status from lower layer....\n");
        UtlString arg;

        arg = mCallId + TAOMESSAGE_DELIMITER + mAddress + TAOMESSAGE_DELIMITER + mTerminalName;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMCONNECTION,
                                                                        TaoMessage::IS_LOCAL,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        3,
                                                                        arg);
        mpClient->sendRequest(msg);

        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
                EVENT_TRACE("Exiting PtTerminalConnection::isLocal 0 \n") ;
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        arg.remove(0);
        pe->getStringData(arg);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::IS_LOCAL);
#endif
        mpEventMgr->release(pe);

        TaoString argList(arg, TAOMESSAGE_DELIMITER);

        int cnt = argList.getCnt();
        if (cnt == 3)
        {
                local = TRUE;
        }
        else
        {
                local = FALSE;
        }

        mIsLocal = local;
        osPrintf("PtTerminalConnection::isLocal returned from lower layer %d\n", local);

        EVENT_TRACE("Exiting PtTerminalConnection::isLocal 1\n") ;
        return PT_SUCCESS;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */
// Protected Constructor
PtTerminalConnection
::PtTerminalConnection(const char* address, const char* terminalName, const char* callId, int isLocal)
{
        mAddress = address;
        mTerminalName = terminalName;
        mCallId = callId;

        mIsLocal = isLocal;
        initialize();
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

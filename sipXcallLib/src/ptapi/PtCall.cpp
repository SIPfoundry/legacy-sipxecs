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
#include "cp/CpGatewayManager.h"
#include "ptapi/PtTerminalConnection.h"
#include "ptapi/PtAddress.h"
#include "ptapi/PtCall.h"
#include "ptapi/PtTerminal.h"
#include "ptapi/PtProvider.h"
#include "ptapi/PtCallListener.h"
#include "ptapi/PtConnection.h"
#include "tao/TaoClientTask.h"
#include "tao/TaoServerTask.h"
#include "tao/TaoEvent.h"
#include "tao/TaoString.h"
#include "tao/TaoReference.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//#define EVENT_TRACE(x) osPrintf(x)
// STATIC VARIABLE INITIALIZATIONS
OsBSem               PtCall::semInit(OsBSem::Q_PRIORITY, OsBSem::FULL) ;
TaoReference            *PtCall::mpTransactionCnt = 0;
int                                      PtCall::mRef = 0;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtCall::PtCall()
{
        mpClient = 0 ;
    mState = IDLE;
        mpConfController = 0;

        initialize() ;

}

// Copy constructor
PtCall::PtCall(const PtCall& rPtCall)
{
        mCallId = rPtCall.mCallId;
        mpClient = rPtCall.mpClient;
        mState = rPtCall.mState;
        mpConfController = rPtCall.mpConfController;

        initialize();
}

PtCall::PtCall(TaoClientTask *pClient, const char* callId)
{
    mpClient   = pClient;

    mCallId.remove(0);
    if (callId)
            mCallId = callId;

    initialize();

    if (mpClient && !(mpClient->isStarted()))
    {
            mpClient->start();
    }
    mState = ACTIVE;
    mpConfController = 0;
}

PtCall::~PtCall()
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

void PtCall::initialize()
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
PtCall&
PtCall::operator=(const PtCall& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        mCallId.remove(0);
        mCallId = rhs.mCallId;
        mpClient = rhs.mpClient;
        mState = rhs.mState;
        mpConfController = rhs.mpConfController;
        mTimeOut = rhs.mTimeOut;

        return *this;
}

PtStatus PtCall::addCallListener(PtCallListener& rCallListener)
{
        EVENT_TRACE("Entering PtCall::addCallListener:\n") ;

        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        TaoObjHandle handle = (TaoObjHandle) &rCallListener;
        UtlString name;
        UtlString arg;

        if (PT_SUCCESS == rCallListener.getLocation(&name)) // must have the terminal name
        {
                arg = name.data() + TAOMESSAGE_DELIMITER + mCallId;
        }
        else
                return PT_INVALID_ARGUMENT;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();

        if (pe == 0)
                return PT_RESOURCE_UNAVAILABLE;

        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::ADD_CALL_LISTENER,
                                                                        transactionId,
                                                                        (TaoObjHandle)handle,
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
        assert(cmd == TaoMessage::ADD_CALL_LISTENER);
#endif
        mpEventMgr->release(pe);

        mpClient->addEventListener(&rCallListener, mCallId.data());

        EVENT_TRACE("Exiting PtCall::addCallListener:\n") ;
        return PT_SUCCESS;
}

PtStatus PtCall::addParty(const char* newPartyURL,
                     PtSessionDesc* pSessionDesc,
                     PtConnection& rConnection)
{
        EVENT_TRACE("Entering PtCall::addParty:\n") ;
        if (!newPartyURL)
                return PT_INVALID_ARGUMENT;

        char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        sprintf(buff, "%" PRIdPTR, (intptr_t)pSessionDesc);
        UtlString arg;
        arg = newPartyURL + TAOMESSAGE_DELIMITER + buff
                        + TAOMESSAGE_DELIMITER + mCallId;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::ADD_PARTY,
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

        UtlString retCode;
        pe->getEventData(rc);
        pe->getStringData(retCode);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::ADD_PARTY);
#endif
        mpEventMgr->release(pe);

        rConnection = PtConnection(mpClient, newPartyURL, mCallId.data());

        EVENT_TRACE("Exiting PtCall::addParty:\n") ;
        return (PtStatus) atoi(retCode);
}

PtStatus PtCall::conference(PtCall& rOtherCall)
{
        EVENT_TRACE("Entering PtCall::conference:\n") ;
    char buff[128];
        rOtherCall.getCallId(buff, 127);

        UtlString arg(buff);

        // not the same call
        if (arg.isNull() || arg != mCallId)
                return PT_INVALID_PARTY;


        arg += TAOMESSAGE_DELIMITER + mCallId;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::CONFERENCE,
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
        assert(cmd == TaoMessage::CONFERENCE);
#endif
        mpEventMgr->release(pe);

        EVENT_TRACE("Exiting PtCall::conference:\n") ;
        return PT_SUCCESS;
}

PtStatus PtCall::connect(PtTerminal& rTerminal,
                                                        PtAddress& rAddress,
                                                        const char* destinationURL,
                                                        PtSessionDesc* pSessionDesc)
{
        EVENT_TRACE("Entering PtCall::connect:\n") ;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%" PRIdPTR, (intptr_t)&rTerminal);
        UtlString arg(buff);

    sprintf(buff, "%" PRIdPTR, (intptr_t)&rAddress);
        arg += TAOMESSAGE_DELIMITER + buff;

        arg += TAOMESSAGE_DELIMITER + destinationURL;

    sprintf(buff, "%" PRIdPTR, (intptr_t)pSessionDesc);
        arg += TAOMESSAGE_DELIMITER + buff;

        arg += TAOMESSAGE_DELIMITER + mCallId;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::CONNECT,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        5,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        UtlString retCode;

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
        pe->getStringData(retCode);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::CONNECT);
#endif
        mpEventMgr->release(pe);

    mState = ACTIVE;

        EVENT_TRACE("Exiting PtCall::connect:\n") ;
        return (PtStatus) atoi(retCode);
}

PtStatus PtCall::consult(PtTerminalConnection& rTerminalConnection,
                    const char* destinationURL,
                    PtSessionDesc* pSessionDesc,
                    PtConnection& rSrcConnection,
                    PtConnection& rDstConnection)
{
        EVENT_TRACE("Entering PtCall::consult:\n") ;

        UtlString arg;
        UtlString str;

        rTerminalConnection.getTerminalName(str);
        arg.append(str);

        rTerminalConnection.getAddressName(str);
        arg += TAOMESSAGE_DELIMITER + str;

        rTerminalConnection.getCallId(str);
        arg += TAOMESSAGE_DELIMITER + str;

        arg += TAOMESSAGE_DELIMITER + mCallId;

        arg += TAOMESSAGE_DELIMITER + destinationURL;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::CONSULT,
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
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::CONSULT);
#endif
        mpEventMgr->release(pe);

        TaoString params(arg, TAOMESSAGE_DELIMITER);

        int cnt = params.getCnt();
        rc = (PtStatus)atoi(params[2]);

        if (cnt >= 2 && rc == PT_SUCCESS)
        {
                rSrcConnection = PtConnection(mpClient,
                                                                                params[0],
                                                                                mCallId.data());
                rDstConnection = PtConnection(mpClient,
                                                                                params[1],
                                                                                mCallId.data());
        }


        EVENT_TRACE("Exiting PtCall::consult:\n") ;
        return (PtStatus)rc;
}

PtStatus PtCall::drop()
{
        EVENT_TRACE("Entering PtCall::drop:\n") ;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::DROP,
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
        assert(cmd == TaoMessage::DROP);
#endif
        mpEventMgr->release(pe);

    mState = INVALID;

        EVENT_TRACE("Exiting PtCall::drop:\n") ;
        return PT_SUCCESS;
}


// Sets the CPU codec limit for this call.
PtStatus PtCall::setCodecCPULimit(const int limit,
                                  const UtlBoolean bAutoRenegotiate)
{
        EVENT_TRACE("Entering PtCall::setCodecCPULimit:\n") ;

        unsigned int transactionId = mpTransactionCnt->add();
   char buff[MAXIMUM_INTEGER_STRING_LENGTH];

   UtlString arg(mCallId) ;

   // Add Limit
   arg += TAOMESSAGE_DELIMITER ;
   sprintf(buff, "%d", limit) ;
   arg += buff ;

   // Add Auto Renegotiate
   arg += TAOMESSAGE_DELIMITER ;
   sprintf(buff, "%d", bAutoRenegotiate) ;
   arg += buff ;

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
         TaoMessage::SET_CODEC_CPU_LIMIT,
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

#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::SET_CODEC_CPU_LIMIT);
#endif
        mpEventMgr->release(pe);

        EVENT_TRACE("Exiting PtCall::setCodecCPULimit:\n") ;
        return PT_SUCCESS;
}


// Forces the renegotation of all connections for this call.
PtStatus PtCall::forceCodecRenegotiation()
{
        EVENT_TRACE("Entering PtCall::forceCodecRenegotiation:\n") ;

        unsigned int transactionId = mpTransactionCnt->add();

   OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
         TaoMessage::CODEC_RENEGOTIATE,
                        transactionId,
                        0,
                        (TaoObjHandle)pe,
                        1,
                        mCallId);

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

#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::CODEC_RENEGOTIATE);
#endif
        mpEventMgr->release(pe);

        EVENT_TRACE("Exiting PtCall::forceCodecRenegotiation:\n") ;
        return PT_SUCCESS;
}



/* ============================ ACCESSORS ================================= */

PtStatus PtCall::getCallListeners(PtCallListener* callListeners[],
                                                                   int size,
                                                                   int& rNumItems)
{
        EVENT_TRACE("Entering PtCall::getCallListeners:\n") ;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        arg += TAOMESSAGE_DELIMITER + mCallId;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::GET_CALL_LISTENERS,
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
        assert(cmd == TaoMessage::GET_CALL_LISTENERS);
#endif
        mpEventMgr->release(pe);

        EVENT_TRACE("Exiting PtCall::getCallListeners:\n") ;
        return PT_SUCCESS;
}

PtStatus PtCall::getCalledAddress(PtAddress& rAddress)
{
        EVENT_TRACE("Entering PtCall::getCalledAddress:\n") ;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::GET_CALLED_ADDRESS,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        mCallId);
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

        UtlString arg;
        pe->getStringData(arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_CALLED_ADDRESS);
#endif
        mpEventMgr->release(pe);

        TaoString addresses(arg, TAOMESSAGE_DELIMITER);
        UtlString address;

        PtStatus rc = PT_SUCCESS;
        int cnt = atoi(addresses[0]);
        if (cnt > 2)
        {
                address = addresses[2];
                rAddress = PtAddress(mpClient, address.data());
        }
        else
        {
                rc = PT_NO_MORE_DATA;
        }

        EVENT_TRACE("Exiting PtCall::getCalledAddress:\n") ;
        return rc;
}

PtStatus PtCall::getCallingAddress(PtAddress& rAddress)
{
        EVENT_TRACE("Entering PtCall::getCallingAddress:\n") ;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::GET_CALLING_ADDRESS,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        mCallId);
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

        UtlString arg;
        pe->getStringData(arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_CALLING_ADDRESS);
#endif
        mpEventMgr->release(pe);

        TaoString addresses(arg, TAOMESSAGE_DELIMITER);
        UtlString address;
        PtStatus rc = PT_SUCCESS;
        int cnt = atoi(addresses[0]);
        if (cnt > 2)
        {
                address = addresses[2];
                rAddress = PtAddress(mpClient, address.data());
        }
        else
        {
                rc = PT_NO_MORE_DATA;
        }
        EVENT_TRACE("Exiting PtCall::getCallingAddress:\n") ;
        return rc;
}

PtStatus PtCall::getCallingTerminal(PtTerminal& rTerminal)
{
        EVENT_TRACE("Entering PtCall::getCallingTerminal:\n") ;

        unsigned int transactionId = mpTransactionCnt->add();

        UtlString       terminalName;
        OsSocket::getHostIp(&terminalName);

        terminalName += TAOMESSAGE_DELIMITER + mCallId;

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::GET_CALLING_TERMINAL,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        terminalName);
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

        UtlString arg;
        pe->getStringData(arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_CALLING_TERMINAL);
#endif
        mpEventMgr->release(pe);

        TaoString addresses(arg, TAOMESSAGE_DELIMITER);
        UtlString address;
        PtStatus rc = PT_SUCCESS;
        int cnt = addresses.getCnt();
        if (cnt > 2)
        {
                address = addresses[2];
                rTerminal = PtTerminal(address.data(), mpClient);
        }
        else
        {
                rc = PT_NO_MORE_DATA;
        }
        EVENT_TRACE("Entering PtCall::getCallingTerminal\n") ;
        return rc;
}

PtStatus PtCall::getConferenceController(PtTerminalConnection& rController)
{
        EVENT_TRACE("Entering PtCall::getConferenceController:\n") ;
        if (mpConfController)
                rController = PtTerminalConnection(*mpConfController);
        else
                return PT_NO_MORE_DATA;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::GET_CONF_CONTROLLER,
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
        assert(cmd == TaoMessage::GET_CONF_CONTROLLER);
#endif
        mpEventMgr->release(pe);

        EVENT_TRACE("Exiting PtCall::getConferenceController:\n") ;
        return PT_SUCCESS;
}

PtStatus PtCall::getConnections(PtConnection connections[],
                                                                 int size,
                                                                 int& rNumItems)
{
        EVENT_TRACE("Entering PtCall::getConnections:\n") ;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        arg += TAOMESSAGE_DELIMITER + mCallId;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::GET_CONNECTIONS,
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
        pe->getStringData(arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_CONNECTIONS);
#endif
        mpEventMgr->release(pe);

        TaoString addresses(arg, TAOMESSAGE_DELIMITER);
        UtlString address;
        PtStatus rc = PT_SUCCESS;
        int cnt = addresses.getCnt();
        if (cnt > 2)
        {
                rNumItems = cnt - 2;
                rNumItems =  (rNumItems > size) ? size : rNumItems;

                for (int i = 0; i < rNumItems; i++)
                {
                        address = addresses[i + 2];
                        connections[i] = PtConnection(mpClient, address.data(), mCallId.data());
                }
        }
        else
        {
                rNumItems = 0;
                rc = PT_NO_MORE_DATA;
        }
        EVENT_TRACE("Exiting PtCall::getConnections\n") ;
        return rc;
}

PtStatus PtCall::getLastRedirectedAddress(PtAddress& rAddress)
{
        EVENT_TRACE("Entering PtCall::getLastRedirectedAddress:\n") ;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::GET_LAST_REDIRECTED_ADDRESS,
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
        assert(cmd == TaoMessage::GET_LAST_REDIRECTED_ADDRESS);
#endif
        mpEventMgr->release(pe);
        EVENT_TRACE("Exiting PtCall::getLastRedirectedAddress:\n") ;

        return PT_SUCCESS;
}

PtStatus PtCall::getTransferController(PtTerminalConnection& rController)
{
        EVENT_TRACE("Entering PtCall::getTransferController:\n") ;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::GET_TRANSFER_CONTROLLER,
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
        assert(cmd == TaoMessage::GET_TRANSFER_CONTROLLER);
#endif
        mpEventMgr->release(pe);

        EVENT_TRACE("Exiting PtCall::getTransferController:\n") ;

        return PT_SUCCESS;
}

PtStatus PtCall::getProvider(PtProvider& rProvider)
{
        EVENT_TRACE("Entering PtCall::getProvider:\n") ;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::GET_PROVIDER,
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
        assert(cmd == TaoMessage::GET_PROVIDER);
#endif
        mpEventMgr->release(pe);
        EVENT_TRACE("Exiting PtCall::getProvider:\n") ;

        return PT_SUCCESS;
}

PtStatus PtCall::numCallListeners(int& rCount)
{
        EVENT_TRACE("Entering PtCall::numCallListeners:\n") ;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::NUM_CALL_LISTENERS,
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
        assert(cmd == TaoMessage::NUM_CALL_LISTENERS);
#endif
        mpEventMgr->release(pe);
        EVENT_TRACE("Exiting PtCall::numCallListeners:\n") ;

        return PT_SUCCESS;
}

PtStatus PtCall::numConnections(int& rCount)
{
        EVENT_TRACE("Entering PtCall::numConnections:\n") ;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::NUM_CONNECTIONS,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        mCallId);
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

        UtlString arg;
        pe->getStringData(arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::NUM_CONNECTIONS);
#endif
        mpEventMgr->release(pe);

        TaoString argList(arg, TAOMESSAGE_DELIMITER);
        rCount = atoi(argList[1]);
        EVENT_TRACE("Exiting PtCall::numConnections:\n") ;

        return PT_SUCCESS;
}

PtStatus PtCall::removeCallListener(PtCallListener& rCallListener)
{
        EVENT_TRACE("Entering PtCall::removeCallListener:\n") ;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];
        sprintf(buff, "%" PRIdPTR, (intptr_t)&rCallListener);

        UtlString name;
        UtlString arg;
        if (PT_SUCCESS == rCallListener.getLocation(&name)) // must have the terminal name
        {
                arg = name.data() + TAOMESSAGE_DELIMITER + buff;
        }
        else
                return PT_INVALID_ARGUMENT;

        mpClient->removeEventListener(rCallListener);

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::REMOVE_CALL_LISTENER,
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
        assert(cmd == TaoMessage::REMOVE_CALL_LISTENER);
#endif
        mpEventMgr->release(pe);
        EVENT_TRACE("Exiting PtCall::removeCallListener:\n") ;

        return PT_SUCCESS;
}

PtStatus PtCall::setConferenceController(PtTerminalConnection* pController)
{
        EVENT_TRACE("Entering PtCall::setConferenceController:\n") ;
        mpConfController = pController;
        char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        sprintf(buff, "%" PRIdPTR, (intptr_t)pController);
        UtlString arg(buff);

        arg += TAOMESSAGE_DELIMITER + mCallId;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::SET_CONF_CONTROLLER,
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
        assert(cmd == TaoMessage::SET_CONF_CONTROLLER);
#endif
        mpEventMgr->release(pe);
        EVENT_TRACE("Exiting PtCall::setConferenceController:\n") ;

        return PT_SUCCESS;
}

PtStatus PtCall::setTransferController(PtTerminalConnection* pController)
{
        EVENT_TRACE("Entering PtCall::setTransferController:\n") ;
        char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        sprintf(buff, "%" PRIdPTR, (intptr_t)pController);
        UtlString arg(buff);

        arg += TAOMESSAGE_DELIMITER + mCallId;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::SET_TRANSFER_CONTROLLER,
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
        assert(cmd == TaoMessage::SET_TRANSFER_CONTROLLER);
#endif
        mpEventMgr->release(pe);
        EVENT_TRACE("Exiting PtCall::setTransferController:\n") ;

        return PT_SUCCESS;
}

//Transfers all participants currently on this PtCall, with the
//exception of the transfer controller participant, to the address
//indicated by the destination URL.
PtStatus PtCall::transfer(const char* destinationURL,
                     PtSessionDesc* pSessionDesc,
                     PtConnection& rNewConnection,
                                         int transferType)
{
        EVENT_TRACE("Entering PtCall::transfer:\n") ;
        char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        sprintf(buff, "%" PRIdPTR, (intptr_t)pSessionDesc);
        UtlString arg(buff);

        arg += TAOMESSAGE_DELIMITER + destinationURL;

        arg += TAOMESSAGE_DELIMITER + mCallId;

    sprintf(buff, "%d", transferType);
        arg += TAOMESSAGE_DELIMITER + buff;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::TRANSFER_CON,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        4,
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
        intptr_t argCnt;
        pe->getEventData(rc);
        pe->getIntData(argCnt);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::TRANSFER_CON);
#endif
        pe->getStringData(arg);
        mpEventMgr->release(pe);

        PtStatus ret = PT_SUCCESS;
        if (argCnt > 1)
        {
                TaoString argList(arg, TAOMESSAGE_DELIMITER);
                UtlString callId = argList[0];
                UtlString address = argList[1];

                ret = (PtStatus) atoi(argList[2]);

                if (!callId.isNull())
                        rNewConnection = PtConnection(mpClient, address.data(), callId.data());
        }
        else
        {
                ret = PT_RESOURCE_UNAVAILABLE;
        }

        EVENT_TRACE("Exiting PtCall::transfer\n") ;
        return ret;
}

PtStatus PtCall::transfer(PtCall& rOtherCall)
{
        EVENT_TRACE("Entering PtCall::transfer:\n") ;
        UtlString arg;

        arg = mCallId;

        char callId[128];

        rOtherCall.getCallId(callId, 127);

        if (callId[0])
        {
                arg += TAOMESSAGE_DELIMITER + callId;
        }
        else
        {
                return PT_INVALID_ARGUMENT;
        }

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::TRANSFER_SEL,
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
        assert(cmd == TaoMessage::TRANSFER_SEL);
#endif
        mpEventMgr->release(pe);

        EVENT_TRACE("Exiting PtCall::transfer\n") ;
        return PT_SUCCESS;
}


PtStatus PtCall::hold(UtlBoolean bBridgeParticipants /*= FALSE*/)
{
   EVENT_TRACE("Entering PtCall::hold:\n") ;

   char buff[MAXIMUM_INTEGER_STRING_LENGTH];
        UtlString arg(mCallId) ;
        sprintf(buff, "%d", bBridgeParticipants) ;
   arg += TAOMESSAGE_DELIMITER + buff ;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::CALL_HOLD,
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
        assert(cmd == TaoMessage::HOLD);
#endif
        mpEventMgr->release(pe);

   EVENT_TRACE("Exiting PtCall::hold\n") ;
        return PT_SUCCESS;
}


PtStatus PtCall::unhold(UtlBoolean bRemoteParticipants /*= TRUE*/)
{
   EVENT_TRACE("Entering PtCall::unhold:\n") ;

   char buff[MAXIMUM_INTEGER_STRING_LENGTH];
        UtlString arg(mCallId) ;
        sprintf(buff, "%d", bRemoteParticipants) ;
   arg += TAOMESSAGE_DELIMITER + buff ;

   unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::CALL_UNHOLD,
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
        assert(cmd == TaoMessage::HOLD);
#endif
        mpEventMgr->release(pe);

   EVENT_TRACE("Exiting PtCall::unhold\n") ;

   return PT_SUCCESS;
}


PtStatus PtCall::getState(int& rState)
{
        EVENT_TRACE("Entering PtCall::getState:\n") ;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::GET_STATE,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        mCallId);
        mpClient->sendRequest(msg);

        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
                EVENT_TRACE("Exiting PtCall::getState: time out\n") ;
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        intptr_t rc;
        intptr_t argCnt = 0;
        UtlString arg;
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
                mState = rState = INVALID;
                ret = PT_NOT_FOUND;
        }

        EVENT_TRACE("Exiting PtCall::getState\n") ;
        return ret;
}

PtStatus PtCall::getCallId(char* callId, int len)
{
        if (!mCallId.isNull())
        {
                int bytes = mCallId.length();

                bytes = (bytes > len) ? len : bytes;

                strncpy(callId, mCallId.data(), bytes);
                callId[bytes] = 0;
                return PT_SUCCESS;
        }

        return PT_RESOURCE_UNAVAILABLE;
}


// Gets the current CPU cost given the negotiated codecs.
PtStatus PtCall::getCodecCPUCost(int& cost)
{
   EVENT_TRACE("Entering PtCall::getCodecCPUCost:\n") ;

        UtlString arg(mCallId) ;
   unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::GET_CODEC_CPU_COST,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
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

        intptr_t argCnt = 0;
        UtlString argResponse;
        pe->getIntData(argCnt);
        pe->getStringData(argResponse);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_CODEC_CPU_COST);
#endif
   mpEventMgr->release(pe);
   if (argCnt == 1)
   {
      cost = atoi(argResponse.data()) ;
   }
   else
   {
      return PT_NOT_FOUND;
   }

   EVENT_TRACE("Exiting PtCall::getCodecCPUCost\n") ;

   return PT_SUCCESS;
}


// Gets the current CPU cost limit given the negotiated codecs.
PtStatus PtCall::getCodecCPULimit(int& cost)
{
   EVENT_TRACE("Entering PtCall::getCodecCPULimit:\n") ;

        UtlString arg(mCallId) ;
   unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_CALL,
                                                                        TaoMessage::GET_CODEC_CPU_LIMIT,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
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

        intptr_t argCnt = 0;
        UtlString argResponse;
        pe->getIntData(argCnt);
        pe->getStringData(argResponse);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_CODEC_CPU_LIMIT);
#endif
   mpEventMgr->release(pe);
   if (argCnt == 1)
   {
      cost = atoi(argResponse.data()) ;
   }
   else
   {
      return PT_NOT_FOUND;
   }

   EVENT_TRACE("Exiting PtCall::getCodecCPULimit\n") ;

   return PT_SUCCESS;
}
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
PtCall::PtCall(const char* callId)
{
        if (callId)
                mCallId = callId;

    mState = IDLE;
        mpConfController = 0;
}
/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

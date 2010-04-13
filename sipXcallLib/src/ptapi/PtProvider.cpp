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
#include <string.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include "ptapi/PtProvider.h"
#include "ptapi/PtProvider.h"
#include "ptapi/PtTerminal.h"
#include "ptapi/PtAddress.h"
#include "ptapi/PtCall.h"
#include "tao/TaoClientTask.h"
#include "tao/TaoServerTask.h"
#include "tao/TaoEvent.h"
#include "ptapi/PtProviderListener.h"
#include "tao/TaoString.h"
#include <cp/CpGatewayManager.h>


// EXTERNAL FUNCTIONS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define USE_MESSAGE_QUEUE 1

// STATIC VARIABLE INITIALIZATIONS
OsBSem               PtProvider::semInit(OsBSem::Q_PRIORITY, OsBSem::FULL) ;
PtProvider                      *PtProvider::spInstance = 0L;       // pointer to the single instance of
OsBSem                          PtProvider::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
TaoClientTask           *PtProvider::mpClient = 0;

TaoReference            *PtProvider::mpTransactionCnt = 0;

TaoObjectMap            *PtProvider::mpCalls   = 0;
TaoReference            *PtProvider::mpCallCnt = 0;

TaoObjectMap            *PtProvider::mpAddresses  = 0;
TaoReference            *PtProvider::mpAddressCnt = 0;

UtlBoolean                       PtProvider::mbInvalidIP = FALSE;

unsigned int            PtProvider::mRef = 0;

int FORCE_REFERENCE_PtProvider = 0 ;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
// Obtaining access to a Provider (Pingtel Server)
PtStatus PtProvider::
getProvider(const char* login, const char* password,
                        const char* server, const char* options,
                        CpCallManager* pCallMgr,
                        PtProvider*& rpProvider)
{
        // If the task object already exists, and the corresponding low-level task
        // has been started, then use it
        if (spInstance != NULL)
        {
                rpProvider = spInstance;
                return PT_SUCCESS;
        }

        // If the task does not yet exist or hasn't been started, then acquire
        // the lock to ensure that only one instance of the task is started
        sLock.acquire();
        if (spInstance == NULL)
       spInstance = new PtProvider(login, password, server, options, pCallMgr);

        sLock.release();

        rpProvider = spInstance;
        return PT_SUCCESS;
}

PtProvider::PtProvider(const char* login, const char* password,
                        const char* server, const char* options, CpCallManager* pCallMgr)
{
        mpEventMgr = OsProtectEventMgr::getEventMgr();
        if (PT_SUCCESS == createProvider(login, password, server,  options, pCallMgr))
                mState = IN_SERVICE;
        else
                mState = OUT_OF_SERVICE;

}

PtStatus PtProvider::createProvider(const char* login, const char* password,
                        const char* server, const char* options,
         CpCallManager* pCallMgr)
{
        ssize_t   pos;
        UtlString svr = server;
        UtlString host;
        int port;

        pos = svr.index(":", (size_t)0);
        if (pos != UTL_NOT_FOUND)
        {
                host = svr(0, pos);
                port = atoi(svr((pos + 1), (svr.length() - pos - 1)).data());
                if (host.isNull() || 0 == host.compareTo("0.0.0.0"))
                {
                        host = "127.0.0.1";
                        mbInvalidIP = TRUE;
                        osPrintf("WARNING - PtProvider::createProvider: using invalid host, replaced with %s\n", host.data());
                }

                if (!portIsValid(port))
                        port = DEF_TAO_LISTEN_PORT;
        }
        else
        {
                return PT_INVALID_ARGUMENT;
        }

        if (!mpClient)
        {
                TaoServerTask *pTaoServerTask = NULL;
                if (USE_MESSAGE_QUEUE && isLocal(host))
                {
            // Part of decoupling Call Manager from Phone library
            // is to make this argument required
            assert(pCallMgr != NULL);

                        pTaoServerTask = TaoServerTask::getTaoServerTask(pCallMgr);
                        mpClient = new TaoClientTask(port, host, pTaoServerTask);
                        if (mpClient && pTaoServerTask)
                        {
                                pTaoServerTask->setClientHandle((TaoObjHandle) mpClient);
                                TaoObjHandle eventServer = mpClient->getEventServer();
                                pTaoServerTask->setEventClient((TaoObjHandle) eventServer);
                        }
                }
                else
                {
                        osPrintf("Ptprovider::createProvider: NOT a local host %s\n", host.data());
                        mpClient = new TaoClientTask(port, host, 0);
                }
        }

        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        initialize();

        // formulate the message to be sent to the server
        UtlString       argList = login + TAOMESSAGE_DELIMITER + password;
        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::GET_PROVIDER,
                                                                        0,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        argList);
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
        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_PROVIDER);
#endif
        mpEventMgr->release(pe);

#ifdef _VXWORKS
        if (mbInvalidIP)
        {
        char *title = "Error" ;
                char *text = "The network settings of this xpressa are incorrect or there is no connection to your network. Please check the network connection and xpressa preferences." ;

        // Call processing is now decoupled from Pinger library
        // this will have to be removed, but for now this is
        // only an issue for the hardphone

        JXAPI_MessageBox(MBT_ERROR, title, text, 0) ;
                //int delay = 30 * sysClkRateGet();
                //taskDelay(delay);
                //JNI_ClearStatus();
        }
#endif

        return PT_SUCCESS;
}

void PtProvider::initialize()
{
        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
    semInit.acquire() ;
        mRef++;

        if (!mpTransactionCnt)
                mpTransactionCnt    = new TaoReference();

        if (!mpCalls)
                mpCalls   = new TaoObjectMap();
        if (!mpCallCnt)
                mpCallCnt = new TaoReference();

        if (!mpAddresses)
                mpAddresses  = new TaoObjectMap();
        if (!mpAddressCnt)
                mpAddressCnt = new TaoReference();

   semInit.release() ;
}

// Default Constructor
PtProvider::PtProvider()
{
   initialize();
}

// Copy constructor
PtProvider::PtProvider(const PtProvider& rPtProvider)
{
        spInstance = rPtProvider.spInstance;      // pointer to the single instance of
        mpClient = rPtProvider.mpClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        mLogin = rPtProvider.mLogin;
        mPass = rPtProvider.mPass;

        mState = rPtProvider.mState;
        initialize();
        mpEventMgr = OsProtectEventMgr::getEventMgr();
}

PtProvider::PtProvider(UtlString& rLogin, UtlString& rPass)
{
        mLogin = rLogin;
        mPass = rPass;

        mpClient = 0;

        mState = IN_SERVICE;

        mpEventMgr = OsProtectEventMgr::getEventMgr();
        initialize();
}

// Destructor
PtProvider::~PtProvider()
{
    semInit.acquire() ;
        if (--mRef < 1)
        {
                if (mpClient)
                {
                        mpClient->requestShutdown();
                        delete mpClient;
                        mpClient = NULL;
                }

                if (mpTransactionCnt)
                {
                        delete mpTransactionCnt;
                        mpTransactionCnt = 0;
                }

                if (mpCalls)
                {
                        delete mpCalls;
                        mpCalls = 0;
                }

                if (mpCallCnt)
                {
                        delete mpCallCnt;
                        mpCallCnt = 0;
                }

                if (mpAddresses)
                {
                        delete mpAddresses;
                        mpAddresses = 0;
                }

                if (mpAddressCnt)
                {
                        delete mpAddressCnt;
                        mpAddressCnt = 0;
                }

                if (spInstance)
                {
                        spInstance = 0;
                }
        }
    semInit.release() ;
}


UtlBoolean PtProvider::isLocal(const char * host)
{
        UtlString strServer;
        OsSocket::getHostIp(&strServer);

//      osPrintf("%s isLocal %s?\n", host, strServer.data());
        if (strServer.length() == 0 || 0 == strServer.compareTo("0.0.0.0"))
        {
                mbInvalidIP = TRUE;
                return TRUE;
        }

        if (strServer.compareTo(host, UtlString::ignoreCase) == 0)
                return TRUE;

        OsSocket::getHostName(&strServer);
        if (strServer.compareTo(host, UtlString::ignoreCase) == 0)
                return TRUE;

        return FALSE;
}

/* ============================ MANIPULATORS ============================== */

PtStatus PtProvider::addProviderListener(PtProviderListener& rListener)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        sprintf(buff, "%" PRIdPTR, (intptr_t)&rListener);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::ADD_PROVIDER_LISTENER,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
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
        assert(cmd == TaoMessage::ADD_PROVIDER_LISTENER);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtProvider::createCall(PtCall& rCall)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::CREATE_CALL,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
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
        UtlString callId;
        pe->getEventData(rc);
        pe->getStringData(callId);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::CREATE_CALL);
#endif
        mpEventMgr->release(pe);

        if (!callId.isNull())
                rCall = PtCall(mpClient, callId.data());

        return PT_SUCCESS;
}

PtStatus PtProvider::getAddress(const char* phoneURL, PtAddress& rAddress)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::GET_ADDRESS,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        phoneURL);
        mpClient->sendRequest(msg);

        intptr_t rc;
        UtlString name;
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
        pe->getStringData(name);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_ADDRESS);
#endif
        mpEventMgr->release(pe);

        rAddress = PtAddress(mpClient, (const char*) name);

        return PT_SUCCESS;
}

PtStatus PtProvider::getAddresses(PtAddress arAddresses[], int size, int& nItems)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::GET_ADDRESSES,
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

        intptr_t temp;
        pe->getEventData(temp);
        nItems = (int) temp;
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_ADDRESSES);
#endif
        mpEventMgr->release(pe);

        int actual = ((size < nItems) ? size : nItems);

        TaoString taoStr(arg, TAOMESSAGE_DELIMITER);;

        for (int i = 0; i < actual; i++)
        {
                arAddresses[i] = PtAddress(mpClient, taoStr[i]);
        }


        return PT_SUCCESS;
}

PtStatus PtProvider::getCalls(PtCall arCalls[], int size, int& nItems)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::GET_CALLS,
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

        intptr_t temp;
        pe->getEventData(temp);
        nItems = (int)temp;
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_CALLS);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtProvider::getProviderListeners(PtProviderListener* listeners[],
                                                                          int size,
                                                                          int& nItems)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::GET_PROVIDER_LISTENERS,
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

        intptr_t temp;
        pe->getEventData(temp);
        nItems = (int)temp;
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_PROVIDER_LISTENERS);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

int PtProvider::getState(void)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::GET_STATE,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
        mpClient->sendRequest(msg);


        int state;
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

        intptr_t temp;
        pe->getEventData(temp);
        state = (int)temp;
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_STATE);
#endif
        mpEventMgr->release(pe);

        return state;
}

PtStatus PtProvider::getTerminal(const char* name, PtTerminal& rTerminal)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::GET_TERMINAL,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        name);
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

        TaoObjHandle rObjHandle;
        intptr_t temp;
        pe->getEventData(temp);
        rObjHandle = (TaoObjHandle)temp;

        UtlString terminalName;
        pe->getStringData(terminalName);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_TERMINAL);
#endif
        mpEventMgr->release(pe);

        rTerminal = PtTerminal(name, mpClient);

        return PT_SUCCESS;
}

PtStatus PtProvider::getTerminals(PtTerminal arTerms[], int size, int& nItems)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::GET_TERMINALS,
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

        intptr_t temp;
        pe->getEventData(temp);
        nItems = (int) temp;
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_TERMINALS);
#endif
        mpEventMgr->release(pe);

        int actual = ((size < nItems) ? size : nItems);

        TaoString taoStr(arg, TAOMESSAGE_DELIMITER);;

        for (int i = 0; i < actual; i++)
        {
                arTerms[i] = PtTerminal(taoStr[i], mpClient);
        }

        return PT_SUCCESS;
}

PtStatus PtProvider::numAddresses(int& count)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::NUM_ADDRESSES,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
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

        intptr_t temp;
        pe->getEventData(temp);
        count = (int)temp;
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::NUM_ADDRESSES);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtProvider::numCalls(int& count)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::NUM_CALLS,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
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

        intptr_t temp;
        pe->getEventData(temp);
        count = (int)temp;
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::NUM_CALLS);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtProvider::numProviderListeners(int& count)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::NUM_PROVIDER_LISTENERS,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
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

        intptr_t temp;
        pe->getEventData(temp);
        count = (int)temp;
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::NUM_PROVIDER_LISTENERS);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtProvider::numTerminals(int& count)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::NUM_TERMINALS,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
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

        intptr_t temp;
        pe->getEventData(temp);
        count = (int)temp;
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::NUM_TERMINALS);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtProvider::removeProviderListener(PtProviderListener& rListener)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        sprintf(buff, "%" PRIdPTR, (intptr_t)&rListener);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::REMOVE_PROVIDER_LISTENER,
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

        intptr_t rc;
        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::REMOVE_PROVIDER_LISTENER);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtProvider::shutdown(void)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PROVIDER,
                                                                        TaoMessage::SHUTDOWN,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
        mpClient->sendRequest(msg);

    // Should something be done with pe?

        return PT_SUCCESS;
}
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Assignment operator
PtProvider&
PtProvider::operator=(const PtProvider& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        spInstance = rhs.spInstance;      // pointer to the single instance of
        mpClient = rhs.mpClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        mLogin = rhs.mLogin;
        mPass = rhs.mPass;

        mState = rhs.mState;
        mTimeOut = rhs.mTimeOut;

        return *this;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

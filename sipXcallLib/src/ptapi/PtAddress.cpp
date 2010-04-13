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
#include "ptapi/PtAddress.h"
#include "ptapi/PtTerminal.h"
#include "ptapi/PtAddressForwarding.h"
#include "tao/TaoReference.h"
#include "tao/TaoClientTask.h"
#include "tao/TaoServerTask.h"
#include "tao/TaoEvent.h"
#include "tao/TaoString.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
OsBSem               PtAddress::semInit(OsBSem::Q_PRIORITY, OsBSem::FULL) ;
TaoReference            *PtAddress::mpTransactionCnt = 0;
unsigned int             PtAddress::mRef = 0;
PtAddressForwarding     *PtAddress::mpAddressForwards = 0;
int                                      PtAddress::mAddressForwardCnt = 0;
OsBSem               PtAddress::mAddressForwardDbSem(OsBSem::Q_PRIORITY, OsBSem::FULL);
PtBoolean                        PtAddress::mbMessageWaiting = FALSE;
PtBoolean                        PtAddress::mbDoNotDisturb = FALSE;
int                                      PtAddress::mOfferedTimeout = 0;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtAddress::PtAddress()
{
        mpClient = 0;
        mpEventMgr = OsProtectEventMgr::getEventMgr();
        initialize();
}

PtAddress::PtAddress(TaoClientTask *pClient, const char* name)
{
        mpClient   = pClient;

        if (name)
                mAddress = name;

        mpEventMgr = OsProtectEventMgr::getEventMgr();
        initialize();
}

// Copy constructor
PtAddress::PtAddress(const PtAddress& rPtAddress)
{
        mAddress = rPtAddress.mAddress;
        mpClient = rPtAddress.mpClient;
        mpTransactionCnt = rPtAddress.mpTransactionCnt;
        mpAddressForwards = rPtAddress.mpAddressForwards ;
        mAddressForwardCnt = rPtAddress.mAddressForwardCnt ;

        mpEventMgr = OsProtectEventMgr::getEventMgr();
        initialize();

}

PtAddress::PtAddress(const char* address)
{
        if (address)
                mAddress = address;

        mpClient = 0;

        mpEventMgr = OsProtectEventMgr::getEventMgr();
        initialize();
}

PtAddress::PtAddress(PtProvider *pProvider, const char* address)
{
        if (address)
                mAddress = address;

        mpClient = 0;

        mpEventMgr = OsProtectEventMgr::getEventMgr();
        initialize();
}

// Destructor
PtAddress::~PtAddress()
{

    semInit.acquire() ;

        mRef--;

        if (mRef < 1)
        {
                if (!mpTransactionCnt)
                {
                        delete mpTransactionCnt;
                        mpTransactionCnt  = 0;
                }

                if (!mpAddressForwards)
                {
                        mAddressForwardDbSem.acquire();
                        delete[] mpAddressForwards;
                        mpAddressForwards = 0;
                        mAddressForwardCnt = 0;
                        mAddressForwardDbSem.release();
                }
        }

    semInit.release() ;
}

void PtAddress::initialize()
{
        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
    semInit.acquire() ;

        mRef++;

        if (!mpTransactionCnt)
                mpTransactionCnt = new TaoReference();

        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

    semInit.release() ;
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtAddress&
PtAddress::operator=(const PtAddress& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        mAddress = rhs.mAddress;
        mpTransactionCnt = rhs.mpTransactionCnt;
        mpClient = rhs.mpClient;
        mpAddressForwards = rhs.mpAddressForwards ;
        mAddressForwardCnt = rhs.mAddressForwardCnt ;

        // initialize();

        return *this;
}

PtStatus PtAddress::addAddressListener(PtAddressListener& rAddressListener)
{
        TaoObjHandle handle = (TaoObjHandle)&rAddressListener;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];
    sprintf(buff, "%ld", handle);

        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,
                                                                        TaoMessage::ADD_ADDRESS_LISTENER,
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
        assert(cmd == TaoMessage::ADD_ADDRESS_LISTENER);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtAddress::addCallListener(PtCallListener& rCallListener)
{
        TaoObjHandle handle = (TaoObjHandle)&rCallListener;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];
    sprintf(buff, "%ld", handle);

        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,
                                                                        TaoMessage::ADD_CALL_LISTENER,
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
        assert(cmd == TaoMessage::ADD_CALL_LISTENER);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtAddress::cancelForwarding()
{
        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,
                                                                        TaoMessage::CANCEL_ALL_FORWARDING,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
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
        assert(cmd == TaoMessage::CANCEL_ALL_FORWARDING);
#endif
        mpEventMgr->release(pe);

        mAddressForwardDbSem.acquire();
        if (mpAddressForwards)
        {
                delete[] mpAddressForwards;
                mpAddressForwards = 0;
        }
        mAddressForwardCnt = 0;
        mAddressForwardDbSem.release();

        return PT_SUCCESS;
}

PtStatus PtAddress::cancelForwarding(PtAddressForwarding forwards[], int size)
{
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        osPrintf("PtAddress::cancelForwarding(size=%d)\n", size) ;
        int i;
        for (i = 0; i < size; i++)
        {
                osPrintf("%02d: type=%d filter=%d, url=%s, caller=%s\n", i, forwards[i].mForwardingType, forwards[i].mFilterType, forwards[i].mDestinationUrl.data(), forwards[i].mCallerUrl.data()) ;

                sprintf(buff, "%d", forwards[i].mForwardingType);
                arg += TAOMESSAGE_DELIMITER + UtlString(buff);

                sprintf(buff, "%d", forwards[i].mFilterType);
                arg += TAOMESSAGE_DELIMITER + UtlString(buff);

                arg += TAOMESSAGE_DELIMITER + forwards[i].mDestinationUrl
                                + TAOMESSAGE_DELIMITER + forwards[i].mCallerUrl;

                sprintf(buff, "%d", forwards[i].mNoAnswerTimeout);
                arg += TAOMESSAGE_DELIMITER + UtlString(buff);
        }

        unsigned int transactionId = 0;
        if (mpTransactionCnt)
        {
                mpTransactionCnt->add();
                transactionId = mpTransactionCnt->getRef();
        }
        else {
      // I HAVE NO IDEA WHAT THIS IS ABOUT, BUT THE FORMAT IS WRONG!!!
      // And, we only got here because mpTransactionCnt is NULL.
           osPrintf("\nPtAddress::cancelForwarding - mpTransactionCnt = %p\n", mpTransactionCnt);
   }

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,TaoMessage::CANCEL_FORWARDING,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        (5 * size + 1),
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
        assert(cmd == TaoMessage::CANCEL_FORWARDING);
#endif
        mpEventMgr->release(pe);

        mAddressForwardDbSem.acquire();
        if (mpAddressForwards)
        {
                for (i = 0; i < size; i++)
                {
                        for (int j = 0; j < mAddressForwardCnt; j++)
                        {
                                if (forwards[i] == (PtAddressForwarding&) mpAddressForwards[j])
                                {
                                        mAddressForwardCnt--;
                                        for (int k = j; k < mAddressForwardCnt; k++)
                                                mpAddressForwards[k] = mpAddressForwards[k + 1];
                                        break;
                                }
                        }
                }
                if (mAddressForwardCnt <= 0)
                {
                        delete[] mpAddressForwards;
                        mpAddressForwards = 0;
                        mAddressForwardCnt = 0;
                }
        }

        mAddressForwardDbSem.release();

        return PT_SUCCESS;
}

PtStatus PtAddress::removeAddressListener(PtAddressListener& rAddressListener)
{
    TaoObjHandle listener = 0x00000008; //fake
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%ld", listener);
    UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,
                                                                        TaoMessage::REMOVE_ADDRESS_LISTENER,
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
        assert(cmd == TaoMessage::REMOVE_ADDRESS_LISTENER);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtAddress::removeCallListener(PtCallListener& rCallListener)
{
        TaoObjHandle listener = (TaoObjHandle)&rCallListener;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%ld", listener);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,
                                                                        TaoMessage::REMOVE_CALL_LISTENER,
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
        assert(cmd == TaoMessage::REMOVE_CALL_LISTENER);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtAddress::setDoNotDisturb(PtBoolean flag)
{
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", flag);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,
                                                                        TaoMessage::SET_DONOT_DISTURB,
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
        assert(cmd == TaoMessage::SET_DONOT_DISTURB);
#endif
        mpEventMgr->release(pe);

        mbDoNotDisturb = flag;
        return PT_SUCCESS;
}

PtStatus PtAddress::setForwarding(PtAddressForwarding forwards[], int size)
{
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        osPrintf("PtAddress::setForwarding(size=%d)\n", size) ;
        int i;
        for (i = 0; i < size; i++)
        {
                osPrintf("%02d: type=%d filter=%d, url=%s, caller=%s\n", i, forwards[i].mForwardingType, forwards[i].mFilterType, forwards[i].mDestinationUrl.data(), forwards[i].mCallerUrl.data()) ;

                sprintf(buff, "%d", forwards[i].mForwardingType);
                arg += TAOMESSAGE_DELIMITER + UtlString(buff);

                sprintf(buff, "%d", forwards[i].mFilterType);
                arg += TAOMESSAGE_DELIMITER + UtlString(buff);

                arg += TAOMESSAGE_DELIMITER + forwards[i].mDestinationUrl
                                + TAOMESSAGE_DELIMITER + forwards[i].mCallerUrl;

                sprintf(buff, "%d", forwards[i].mNoAnswerTimeout);
                arg += TAOMESSAGE_DELIMITER + UtlString(buff);
        }

        unsigned int transactionId = 0;
        if (mpTransactionCnt)
        {
                mpTransactionCnt->add();
                transactionId = mpTransactionCnt->getRef();
        }
        else {
      // I HAVE NO IDEA WHAT THIS IS ABOUT, BUT THE FORMAT IS WRONG!!!
      // And, we only got here because mpTransactionCnt is NULL.
           osPrintf("\nPtAddress::setForwarding - mpTransactionCnt = %p\n", mpTransactionCnt);
   }

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,TaoMessage::SET_FORWARDING,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        (5 * size + 1),
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
        assert(cmd == TaoMessage::SET_FORWARDING);
#endif
        mpEventMgr->release(pe);

        mAddressForwardDbSem.acquire();
        if (!mpAddressForwards && mAddressForwardCnt == 0)
        {
                if (size > 0) {
                        mpAddressForwards = new PtAddressForwarding[size];
                        mAddressForwardCnt = size;
                        for (i = 0; i < size; i++)
                                mpAddressForwards[i] = PtAddressForwarding(forwards[i]);
                }
        }
        else
        {
                // Dump the old list
                delete[] mpAddressForwards ;
                mpAddressForwards = NULL ;
                mAddressForwardCnt = 0 ;

                // Create a new list
                if (size > 0) {
                        mpAddressForwards = new PtAddressForwarding[size];
                        mAddressForwardCnt = size;
                        for (int k = 0; k < size; k++)
                                mpAddressForwards[k] = PtAddressForwarding(forwards[k]);
                }
        }
        mAddressForwardDbSem.release();

        return PT_SUCCESS;
}

PtStatus PtAddress::setMessageWaiting(PtBoolean flag)
{
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", flag);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,TaoMessage::SET_MESSAGE_WAITING,
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
        assert(cmd == TaoMessage::SET_MESSAGE_WAITING);
#endif
        mpEventMgr->release(pe);

        mbMessageWaiting = flag;

        return PT_SUCCESS;
}

PtStatus PtAddress::setOfferedTimeout(int milliSecs)
{
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", milliSecs);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,TaoMessage::SET_OFFERED_TIMEOUT,
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
        assert(cmd == TaoMessage::SET_OFFERED_TIMEOUT);
#endif
        mpEventMgr->release(pe);


        mOfferedTimeout = milliSecs;
        return PT_SUCCESS;
}

/* ============================ ACCESSORS ================================= */
// Get the name associated with the address.
PtStatus PtAddress::getName(char* rpName, int len)
{
        enum PtStatus returnCode = PT_RESOURCE_UNAVAILABLE;

        if (rpName && len > 0)
        {
                if (!mAddress.isNull())
                {
                        int bytes = mAddress.length();
                        bytes = (bytes > len) ? len : bytes;

                        memset(rpName, 0, len);
                        strncpy (rpName, mAddress.data(), bytes);
                        returnCode = PT_SUCCESS;
                }
        }

        return returnCode;
}

PtStatus PtAddress::getAddressListeners(PtAddressListener* addrListeners[],
                                                                         int size,
                                                                         int& nItems)
{
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,
                                                                        TaoMessage::GET_ADDRESS_LISTENERS,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t items;
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

        pe->getEventData(items);
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_ADDRESS_LISTENERS);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtAddress::getCallListeners(PtCallListener* callListeners[],
                                                                  int size,
                                                                  int& nItems)
{
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,
                                                                        TaoMessage::GET_CALL_LISTENERS,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t items;
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

        pe->getEventData(items);
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_CALL_LISTENERS);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtAddress::getConnections(PtConnection connections[],
                                                                int size,
                                                                int& nItems)
{
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,
                                                                        TaoMessage::GET_CONNECTIONS,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t items;
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

        pe->getEventData(items);
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_CONNECTIONS);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtAddress::getDoNotDisturb(PtBoolean& rFlag)
{
        rFlag = mbDoNotDisturb;
        return PT_SUCCESS;
}

PtStatus PtAddress::getForwarding(PtAddressForwarding forwards[],
                                                           int size,
                                                           int& nItems)
{
        PtStatus ret = PT_SUCCESS;

        nItems = mAddressForwardCnt;
        mAddressForwardDbSem.acquire();

        if (mAddressForwardCnt && mpAddressForwards)
        {
                if (size < nItems)
                        nItems = size;
                for (int i = 0; i < nItems; i++)
                {
                        forwards[i] = PtAddressForwarding(mpAddressForwards[i]);
                }
        }
        else if (nItems > 0)
                ret = PT_INVALID_STATE;

        mAddressForwardDbSem.release();

        return ret;
}

PtStatus PtAddress::getMessageWaiting(PtBoolean& rFlag)
{
        rFlag = mbMessageWaiting;
        return PT_SUCCESS;
}

PtStatus PtAddress::getOfferedTimeout(int& rMilliSecs)
{
        rMilliSecs = mOfferedTimeout;
        return PT_SUCCESS;
}

PtStatus PtAddress::getTerminals(PtTerminal terms[], int size, int& nItems)
{
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,
                                                                        TaoMessage::GET_TERMINALS,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t items;
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

        pe->getEventData(items);
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_TERMINALS);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtAddress::getProvider(PtProvider& rProvider)
{
        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        // formulate the message to be sent to the server, the format of the msg over
        // the wire is following:
        // TaoMessageType$D$TransactionID$D$ArgCnt$D$Arg1$D$Arg2........
        //              UtlString       argList = (UtlString)m_strServer + (UtlString)TAOMESSAGE_DELIMITER + (UtlString)m_strPort;
        UtlString       argList = "";

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,
                                                                TaoMessage::GET_PROVIDER,
                                                                        transactionId,
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

        return PT_SUCCESS;
}

PtStatus PtAddress::numAddressListeners(int& count)
{
        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,
                                                                        TaoMessage::NUM_ADDRESS_LISTENERS,
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
        assert(cmd == TaoMessage::NUM_ADDRESS_LISTENERS);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtAddress::numCallListeners(int& count)
{
        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,
                                                                        TaoMessage::NUM_CALL_LISTENERS,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
        mpClient->sendRequest(msg);

        return PT_SUCCESS;

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
        assert(cmd == TaoMessage::NUM_CALL_LISTENERS);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtAddress::numConnections(int& count)
{
        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,
                                                                        TaoMessage::NUM_CONNECTIONS,
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
        pe->getEventData((intptr_t &)temp);
        count = (int)temp;
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::NUM_CONNECTIONS);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtAddress::numForwards(int& count)
{
        count = mAddressForwardCnt;
        return PT_SUCCESS;
}

PtStatus PtAddress::numTerminals(int& count)
{
        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_ADDRESS,
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

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
// Protected constructor.
/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

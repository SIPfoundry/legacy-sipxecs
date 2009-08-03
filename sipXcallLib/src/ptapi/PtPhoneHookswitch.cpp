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
#include "ptapi/PtPhoneHookswitch.h"
#include "ptapi/PtCall.h"
#include "ptapi/PtProvider.h"
#include "tao/TaoClientTask.h"
#include "tao/TaoEvent.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtPhoneHookswitch::PtPhoneHookswitch()
: PtComponent(PtComponent::HOOKSWITCH),
  mState(ON_HOOK),
  mpProvider(0),
  mpCall(0)
{
        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();

}

PtPhoneHookswitch::PtPhoneHookswitch(PtProvider*& rpProvider)
: PtComponent(PtComponent::HOOKSWITCH),
  mState(ON_HOOK),
  mpProvider(0),
  mpCall(0)
{
        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();

}

PtPhoneHookswitch::PtPhoneHookswitch(TaoClientTask *pClient)
: PtComponent(PtComponent::HOOKSWITCH),
  mState(ON_HOOK),
  mpProvider(0),
  mpCall(0)
{

        mpClient   = pClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();
}

// Copy constructor
PtPhoneHookswitch::PtPhoneHookswitch(const PtPhoneHookswitch& rPtPhoneHookswitch)
: PtComponent(rPtPhoneHookswitch)
{
        mpClient   = rPtPhoneHookswitch.mpClient;
        mState   = rPtPhoneHookswitch.mState;
        mpProvider   = rPtPhoneHookswitch.mpProvider;
        mpCall   = rPtPhoneHookswitch.mpCall;

        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();
}

// Destructor
PtPhoneHookswitch::~PtPhoneHookswitch()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtPhoneHookswitch&
PtPhoneHookswitch::operator=(const PtPhoneHookswitch& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        mpClient   = rhs.mpClient;
        mState   = rhs.mState;
        mpProvider   = rhs.mpProvider;
        mpCall   = rhs.mpCall;

        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        mTimeOut = rhs.mTimeOut;

        return *this;
}

PtStatus PtPhoneHookswitch::setHookswitchState(int state)
{
        char arg[20];

        sprintf(arg, "%d", state);

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                        TaoMessage::HOOKSWITCH_SET_STATE,
                                        0,
                                        (TaoObjHandle)0,
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
        assert(cmd == TaoMessage::HOOKSWITCH_SET_STATE);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

PtStatus PtPhoneHookswitch::getHookswitchState(int& state)
{

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                        TaoMessage::HOOKSWITCH_SET_STATE,
                                        0,
                                        (TaoObjHandle)0,
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

        intptr_t stateIntptr;
        pe->getEventData(stateIntptr);
        state=(int)stateIntptr;
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::HOOKSWITCH_SET_STATE);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtPhoneHookswitch::getCall(PtCall& rCall)
{
        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                        TaoMessage::HOOKSWITCH_GET_CALL,
                                        0,
                                        (TaoObjHandle)0,
                                        (TaoObjHandle)pe,
                                        0,
                                        "");
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
        assert(cmd == TaoMessage::HOOKSWITCH_GET_CALL);
#endif
        mpEventMgr->release(pe);

        rCall = PtCall(mpClient, callId.data());

        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

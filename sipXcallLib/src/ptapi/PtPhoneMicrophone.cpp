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
#include "ptapi/PtComponent.h"
#include "ptapi/PtPhoneMicrophone.h"
#include "ptapi/PtPhoneMicrophone.h"
#include "ps/PsButtonTask.h"
#include "tao/TaoClientTask.h"
#include "tao/TaoEvent.h"
#include "tao/TaoString.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtPhoneMicrophone::PtPhoneMicrophone()
: PtComponent(PtComponent::MICROPHONE)
{
        mpClient = 0;
        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();

}

// Copy constructor
PtPhoneMicrophone::PtPhoneMicrophone(const PtPhoneMicrophone& rPtPhoneMicrophone)
: PtComponent(rPtPhoneMicrophone)
{
        mpClient   = rPtPhoneMicrophone.mpClient;
        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();
}

PtPhoneMicrophone::PtPhoneMicrophone(TaoClientTask *pClient)
: PtComponent(PtComponent::MICROPHONE)
{
        mpClient   = pClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }
        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();
}

// Destructor
PtPhoneMicrophone::~PtPhoneMicrophone()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtPhoneMicrophone&
PtPhoneMicrophone::operator=(const PtPhoneMicrophone& rhs)
{
        if (this == &rhs)            // handle the assignment to self case
          return *this;

        mpClient   = rhs.mpClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        mTimeOut = rhs.mTimeOut;

        return *this;
}

PtStatus PtPhoneMicrophone::setGain(int gain)
{
        if (gain < 0)
                gain = 0;

        if (gain > 10)
                gain = 10;

        char buf[MAXIMUM_INTEGER_STRING_LENGTH];
        sprintf(buf, "%d", mGroupType);

        UtlString arg(buf);

        sprintf(buf, "%d", gain);
        arg += TAOMESSAGE_DELIMITER + buf;

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::MIC_SET_GAIN,
                                                                        0,
                                                                        (TaoObjHandle)0,
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
        assert(cmd == TaoMessage::MIC_SET_GAIN);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

PtStatus PtPhoneMicrophone::getGain(int& rGain)
{
        char buf[16];
        memset(buf, 0, 16);

        sprintf(buf, "%d", mGroupType);

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::MIC_GET_GAIN,
                                                                        0,
                                                                        (TaoObjHandle)0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        buf);
        mpClient->sendRequest(msg);

        UtlString arg;
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
        assert(cmd == TaoMessage::MIC_GET_GAIN);
#endif
        mpEventMgr->release(pe);

        TaoString value(arg, TAOMESSAGE_DELIMITER);
        rGain = atoi(value[1]);

        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

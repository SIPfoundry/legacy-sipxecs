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
#include "ptapi/PtPhoneSpeaker.h"
#include "ptapi/PtPhoneSpeaker.h"
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
PtPhoneSpeaker::PtPhoneSpeaker()
: PtComponent(PtComponent::SPEAKER)
{
        mpClient = 0;
        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();
}

// Copy constructor
PtPhoneSpeaker::PtPhoneSpeaker(const PtPhoneSpeaker& rPtPhoneSpeaker)
: PtComponent(rPtPhoneSpeaker)
{
        mpClient   = rPtPhoneSpeaker.mpClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();
}

PtPhoneSpeaker::PtPhoneSpeaker(TaoClientTask *pClient)
: PtComponent(PtComponent::SPEAKER)
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
PtPhoneSpeaker::~PtPhoneSpeaker()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtPhoneSpeaker&
PtPhoneSpeaker::operator=(const PtPhoneSpeaker& rhs)
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

PtStatus PtPhoneSpeaker::setVolume(int volume)
{
        if (volume < 0)
                volume = 0;

        if (volume > 10)
                volume = 10;

        char buf[MAXIMUM_INTEGER_STRING_LENGTH];
        sprintf(buf, "%d", volume);

        UtlString arg;
        arg.append(buf);

        sprintf(buf, "%d", mGroupType);
        arg += TAOMESSAGE_DELIMITER + buf;

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::SPEAKER_SET_VOLUME,
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
        assert(cmd == TaoMessage::SPEAKER_SET_VOLUME);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}


/* ============================ ACCESSORS ================================= */

PtStatus PtPhoneSpeaker::getVolume(int& rVolume)
{
        char buf[MAXIMUM_INTEGER_STRING_LENGTH];
        sprintf(buf, "%d", mGroupType);

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::SPEAKER_GET_VOLUME,
                                                                        0,
                                                                        (TaoObjHandle)0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        buf);
        mpClient->sendRequest(msg);

        intptr_t rc;
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

        pe->getEventData(rc);
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::SPEAKER_GET_VOLUME);
#endif
        mpEventMgr->release(pe);

        rVolume = atoi(arg);

        return PT_SUCCESS;
}

PtStatus PtPhoneSpeaker::getNominalVolume(int& rVolume)
{
        char buf[MAXIMUM_INTEGER_STRING_LENGTH];
        sprintf(buf, "%d", mGroupType);

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::SPEAKER_GET_NOMINAL_VOLUME,
                                                                        0,
                                                                        (TaoObjHandle)0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        buf);
        mpClient->sendRequest(msg);

        intptr_t rc;
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

        pe->getEventData(rc);
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::SPEAKER_GET_NOMINAL_VOLUME);
#endif
        mpEventMgr->release(pe);

        rVolume = atoi(arg);

        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

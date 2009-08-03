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
#include "ptapi/PtPhoneRinger.h"
#include "ptapi/PtPhoneRinger.h"
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
PtPhoneRinger::PtPhoneRinger()
: PtComponent(PtComponent::RINGER)
{
        mpClient = 0;
        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();
}

// Copy constructor
PtPhoneRinger::PtPhoneRinger(const PtPhoneRinger& rPtPhoneRinger)
: PtComponent(rPtPhoneRinger)
{
        mpClient   = rPtPhoneRinger.mpClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();
}

PtPhoneRinger::PtPhoneRinger(TaoClientTask *pClient)
: PtComponent(PtComponent::RINGER)
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
PtPhoneRinger::~PtPhoneRinger()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtPhoneRinger&
PtPhoneRinger::operator=(const PtPhoneRinger& rhs)
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

PtStatus PtPhoneRinger::setRingerInfo(int patternIndex, char* rpInfo)
{

        char buf[MAXIMUM_INTEGER_STRING_LENGTH];
        sprintf(buf, "%d", patternIndex);
        UtlString arg = buf;

        if (rpInfo)
                arg += rpInfo;

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::RINGER_SET_INFO,
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
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::RINGER_SET_INFO);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtPhoneRinger::setRingerPattern(int patternIndex)
{

        char buf[MAXIMUM_INTEGER_STRING_LENGTH];
        sprintf(buf, "%d", patternIndex);

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::RINGER_SET_PATTERN,
                                                                        0,
                                                                        (TaoObjHandle)0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        buf);
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
        assert(cmd == TaoMessage::RINGER_SET_PATTERN);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtPhoneRinger::setRingerVolume(int volume)
{
        if (volume < 0)
                volume = 0;

        if (volume > 10)
                volume = 10;

        char buf[MAXIMUM_INTEGER_STRING_LENGTH];
        sprintf(buf, "%d", volume);

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::RINGER_SET_VOLUME,
                                                                        0,
                                                                        (TaoObjHandle)0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        buf);
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
        assert(cmd == TaoMessage::RINGER_SET_VOLUME);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

PtStatus PtPhoneRinger::getMaxRingPatternIndex(int& rMaxIndex)
{
        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::RINGER_GET_MAX_PATTERN_INDEX,
                                                                        0,
                                                                        (TaoObjHandle)0,
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
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
        assert(cmd == TaoMessage::RINGER_GET_MAX_PATTERN_INDEX);
#endif
        mpEventMgr->release(pe);

        rMaxIndex = atoi(arg);

        return PT_SUCCESS;
}

PtStatus PtPhoneRinger::getNumberOfRings(int& rNumRingCycles)
{
        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::RINGER_GET_NUMBER_OF_RINGS,
                                                                        0,
                                                                        (TaoObjHandle)0,
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
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
        assert(cmd == TaoMessage::RINGER_GET_NUMBER_OF_RINGS);
#endif
        mpEventMgr->release(pe);

        rNumRingCycles = atoi(arg);

        return PT_SUCCESS;
}

PtStatus PtPhoneRinger::getRingerInfo(int patternIndex, char*& rpInfo)
{
        char buf[MAXIMUM_INTEGER_STRING_LENGTH];
        sprintf(buf, "%d", patternIndex);

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::RINGER_GET_INFO,
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
        assert(cmd == TaoMessage::RINGER_GET_INFO);
#endif
        mpEventMgr->release(pe);

        if (rpInfo)
        {
                strcpy(rpInfo, arg.data());
                return PT_SUCCESS;
        }

        return PT_RESOURCE_UNAVAILABLE;
}

PtStatus PtPhoneRinger::getRingerPattern(int& rPatternIndex)
{
        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::RINGER_GET_PATTERN,
                                                                        0,
                                                                        (TaoObjHandle)0,
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
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
        assert(cmd == TaoMessage::RINGER_GET_PATTERN);
#endif
        mpEventMgr->release(pe);

        rPatternIndex = atoi(arg);

        return PT_SUCCESS;
}

PtStatus PtPhoneRinger::getRingerVolume(int& rVolume)
{
        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::RINGER_GET_VOLUME,
                                                                        0,
                                                                        (TaoObjHandle)0,
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
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
        assert(cmd == TaoMessage::RINGER_GET_VOLUME);
#endif
        mpEventMgr->release(pe);

        rVolume = atoi(arg);

        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */

PtStatus PtPhoneRinger::isRingerOn(PtBoolean& rIsOn)
{
        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::RINGER_IS_ON,
                                                                        0,
                                                                        (TaoObjHandle)0,
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
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
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::RINGER_IS_ON);
#endif
        mpEventMgr->release(pe);

        rIsOn = atoi(arg);

        return PT_SUCCESS;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

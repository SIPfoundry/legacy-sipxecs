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
#include "ptapi/PtPhoneDisplay.h"
#include "ptapi/PtPhoneLamp.h"
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
PtPhoneDisplay::PtPhoneDisplay(int type)
: PtComponent(type)
{
        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();
}

PtPhoneDisplay::PtPhoneDisplay(TaoClientTask *pClient, int type)
: PtComponent(type)
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
PtPhoneDisplay::PtPhoneDisplay(const PtPhoneDisplay& rPtPhoneDisplay)
: PtComponent(rPtPhoneDisplay)
{
        mpClient   = rPtPhoneDisplay.mpClient;

        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();
}

// Destructor
PtPhoneDisplay::~PtPhoneDisplay()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtPhoneDisplay&
PtPhoneDisplay::operator=(const PtPhoneDisplay& rhs)
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

PtStatus PtPhoneDisplay::setContrast(int level)
{
        char buf[MAXIMUM_INTEGER_STRING_LENGTH];
        sprintf(buf, "%d", level);

        UtlString arg;
        arg.append(buf);

        sprintf(buf, "%d", mGroupType);
        arg += TAOMESSAGE_DELIMITER + buf;

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::DISPLAY_SET_CONTRAST,
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
        assert(cmd == TaoMessage::DISPLAY_SET_CONTRAST);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}



/* ============================ ACCESSORS ================================= */

PtStatus PtPhoneDisplay::getContrast(int& rLevel, int& rLow, int& rHigh, int& rNominal)
{
        char buf[MAXIMUM_INTEGER_STRING_LENGTH];
        sprintf(buf, "%d", mGroupType);

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::DISPLAY_GET_CONTRAST,
                                                                        0,
                                                                        (TaoObjHandle)0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        buf);
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

        UtlString args;
        pe->getStringData(args);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::DISPLAY_GET_CONTRAST);
#endif
        mpEventMgr->release(pe);

        TaoString arg(args, TAOMESSAGE_DELIMITER);
        rLevel = atoi(arg[0]);
        rLow = atoi(arg[1]);
        rHigh = atoi(arg[2]);
        rNominal = atoi(arg[3]);

        return PT_SUCCESS;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

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
#include "ptapi/PtComponent.h"
#include "ptapi/PtPhoneLamp.h"
#include "ptapi/PtPhoneLamp.h"
#include "ptapi/PtPhoneButton.h"
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
PtPhoneLamp::PtPhoneLamp()
: PtComponent(PtComponent::LAMP)
{
        mpAssociatedButton = 0;
        mMode = MODE_OFF;
        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();
}

PtPhoneLamp::PtPhoneLamp(TaoClientTask *pClient)
: PtComponent(PtComponent::LAMP)
{
        mpClient   = pClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        mMode = MODE_OFF;
        mpAssociatedButton = 0;

        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();
}

// Copy constructor
PtPhoneLamp::PtPhoneLamp(const PtPhoneLamp& rPtPhoneLamp)
: PtComponent(rPtPhoneLamp)
{
        mpClient   = rPtPhoneLamp.mpClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        if (rPtPhoneLamp.mpAssociatedButton != NULL)
                mpAssociatedButton = new PtPhoneButton(*rPtPhoneLamp.mpAssociatedButton);
        else
                mpAssociatedButton = NULL ;
        mMode = rPtPhoneLamp.mMode;

        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();
}

// Destructor
PtPhoneLamp::~PtPhoneLamp()
{
        if (mpAssociatedButton != NULL) {
                delete mpAssociatedButton ;
                mpAssociatedButton = NULL ;
        }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtPhoneLamp&
PtPhoneLamp::operator=(const PtPhoneLamp& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        mpClient   = rhs.mpClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        if (mpAssociatedButton != NULL) {
                delete mpAssociatedButton ;
        }

        if (rhs.mpAssociatedButton != NULL)
                mpAssociatedButton = new PtPhoneButton(*rhs.mpAssociatedButton) ;
        else
                mpAssociatedButton = NULL ;
        mMode = rhs.mMode;

        return *this;
}

PtStatus PtPhoneLamp::setMode(int mode)
{
        if (mpAssociatedButton &&
                (mode == MODE_OFF || mode == MODE_STEADY || mode == MODE_FLASH ||
                mode ==  MODE_FLUTTER || mode == MODE_BROKENFLUTTER || mode == MODE_WINK))
        {
                char buf[DEF_TAO_MAX_BUFFER_SIZE];

                if (mpClient && PT_SUCCESS == mpAssociatedButton->getInfo(buf, (DEF_TAO_MAX_BUFFER_SIZE - 1)))
                {
                        UtlString arg;
                        arg.append(buf);

                        sprintf(buf, "%d", mode);
                        arg += TAOMESSAGE_DELIMITER + buf;

                        OsProtectedEvent *pe = mpEventMgr->alloc();
                        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                                        TaoMessage::LAMP_SET_MODE,
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
        assert(cmd == TaoMessage::LAMP_SET_MODE);
#endif
                        mpEventMgr->release(pe);

                        mMode = mode;
                        return PT_SUCCESS;
                }
                else
                        return PT_RESOURCE_UNAVAILABLE;
        }
        else
                return PT_INVALID_ARGUMENT;
}

/* ============================ ACCESSORS ================================= */

PtStatus PtPhoneLamp::getAssociatedPhoneButton(PtPhoneButton& rButton)
{
        if (mpAssociatedButton)
        {
                rButton = PtPhoneButton(*mpAssociatedButton);
                return PT_SUCCESS;
        }
        else
                return PT_RESOURCE_UNAVAILABLE;
}

PtStatus PtPhoneLamp::getMode(int& rMode)
{
        rMode = mMode;
        return PT_SUCCESS;
}

PtStatus PtPhoneLamp::getSupportedModes(int& rModeMask)
{
        rModeMask = mSupportedModes;
        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
void PtPhoneLamp::setAssociatedButton(PtPhoneButton* pButton)
{
        if (mpAssociatedButton != NULL)
                delete mpAssociatedButton ;

        mpAssociatedButton = new PtPhoneButton(*pButton);
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

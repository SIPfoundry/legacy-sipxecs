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
#include "ptapi/PtPhoneButton.h"
#include "ptapi/PtPhoneLamp.h"
#include "os/OsUtil.h"
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
PtPhoneButton::PtPhoneButton() :
PtComponent(PtComponent::BUTTON)
{
        memset(mpInfo, 0, MAX_NAME_LENGTH + 1);

        mpClient = 0;
        mpLamp = 0;

        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
        mpEventMgr = OsProtectEventMgr::getEventMgr();
}

PtPhoneButton::PtPhoneButton(TaoClientTask *pClient, const char* name) :
PtComponent(PtComponent::BUTTON)
{
        mpEventMgr = OsProtectEventMgr::getEventMgr();
        memset(mpInfo, 0, MAX_NAME_LENGTH + 1);

        if (name )
        {
                int len = strlen(name);
                len = (len <= MAX_NAME_LENGTH) ? len : MAX_NAME_LENGTH;
                strncpy(mpInfo, name, len);
        }

        mpLamp = 0;
        mpClient   = pClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
}

// Copy constructor
PtPhoneButton::PtPhoneButton(const PtPhoneButton& rPtPhoneButton) :
PtComponent(rPtPhoneButton)
{
        mpEventMgr = OsProtectEventMgr::getEventMgr();
        if (rPtPhoneButton.mpLamp)
                mpLamp = new PtPhoneLamp(*(rPtPhoneButton.mpLamp));
        else
                mpLamp = 0;

        memset(mpInfo, 0, MAX_NAME_LENGTH + 1);

        if (rPtPhoneButton.mpInfo[0])
        {
                int len = strlen(rPtPhoneButton.mpInfo);

                if (len > MAX_NAME_LENGTH)
                        len = MAX_NAME_LENGTH;

                strncpy(mpInfo, rPtPhoneButton.mpInfo, len);
                mpInfo[len] = 0;
        }

        mpClient   = rPtPhoneButton.mpClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
}

// Destructor
PtPhoneButton::~PtPhoneButton()
{
        if (mpLamp)
        {
                delete mpLamp;
                mpLamp = 0;
        }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtPhoneButton&
PtPhoneButton::operator=(const PtPhoneButton& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        memset(mpInfo, 0, MAX_NAME_LENGTH + 1);

        if (rhs.mpInfo[0])
        {
                int len = strlen(rhs.mpInfo);

                if (len > MAX_NAME_LENGTH)
                        len = MAX_NAME_LENGTH;

                strncpy(mpInfo, rhs.mpInfo, len);
                mpInfo[len] = 0;
        }

        if (mpLamp != NULL) {
                delete mpLamp ;
        }

        if (rhs.mpLamp)
                mpLamp = new PtPhoneLamp(*(rhs.mpLamp));
        else
                mpLamp = 0;

        mpClient   = rhs.mpClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }
        mTimeOut = rhs.mTimeOut;

    return *this;
}

PtStatus PtPhoneButton::buttonPress(void)
{
        if (mpInfo[0])
        {
                UtlString arg(mpInfo);

                OsProtectedEvent *pe = mpEventMgr->alloc();
                TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                                TaoMessage::BUTTON_PRESS,
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
        assert(cmd == TaoMessage::BUTTON_PRESS);
#endif
                mpEventMgr->release(pe);
                return PT_SUCCESS;
        }
        return PT_RESOURCE_UNAVAILABLE;
}

PtStatus PtPhoneButton::setInfo(char* buttonInfo)
{
        if (buttonInfo)
        {
                int len = strlen(buttonInfo);

                memset(mpInfo, 0, (len + 1));
                strcpy(mpInfo, buttonInfo);
                return PT_SUCCESS;
        }
        return PT_RESOURCE_UNAVAILABLE;
}

// set button info locally
PtStatus PtPhoneButton::setInfo2(char* buttonInfo)
{
        if (buttonInfo)
        {
                int len = strlen(buttonInfo);

                if (len > MAX_NAME_LENGTH)
                        len = MAX_NAME_LENGTH;

                strncpy(mpInfo, buttonInfo, len);
                mpInfo[len] = 0;
                return PT_SUCCESS;
        }
        return PT_RESOURCE_UNAVAILABLE;
}

/* ============================ ACCESSORS ================================= */

PtStatus PtPhoneButton::getAssociatedPhoneLamp(PtPhoneLamp& rLamp)
{
        if (OsUtil::getPlatformType() < OsUtil::PLATFORM_TCAS4)
                return PT_RESOURCE_UNAVAILABLE;

        if (!mpLamp)
        {
                mpLamp = new PtPhoneLamp(mpClient);
                mpLamp->setAssociatedButton(this);
        }

        rLamp = PtPhoneLamp(*mpLamp);
        return PT_SUCCESS;
}

PtStatus PtPhoneButton::getInfo(char* rpInfo, int maxLen)
{
        if (rpInfo && maxLen > 0)
        {
                if (mpInfo)
                {
                        int bytes = strlen(mpInfo);
                        bytes = (bytes > maxLen) ? maxLen : bytes;

                        memset(rpInfo, 0, maxLen);
                        strncpy (rpInfo, mpInfo, bytes);
                        return PT_SUCCESS;
                }
        }

        return PT_RESOURCE_UNAVAILABLE;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

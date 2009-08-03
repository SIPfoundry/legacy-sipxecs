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
#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif
// APPLICATION INCLUDES
#include "ptapi/PtTerminalComponentEvent.h"
#include "ptapi/PtPhoneRinger.h"
#include "ptapi/PtPhoneSpeaker.h"
#include "ptapi/PtPhoneMicrophone.h"
#include "ptapi/PtPhoneLamp.h"
#include "ptapi/PtPhoneButton.h"
#include "ptapi/PtPhoneHookswitch.h"
#include "ptapi/PtPhoneDisplay.h"
#include "tao/TaoClientTask.h"
#include "tao/TaoObjectMap.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:PtTerminalComponentEvent contains PtComponent-associated event data

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
PtTerminalComponentEvent::PtTerminalComponentEvent(PtEvent::PtEventId eventId,
                                                                                                   const char* termName,
                                                                                                   TaoClientTask *pClient) :
PtTerminalEvent(eventId, termName, pClient)
{
        mpRinger        = NULL;
        mpSpeaker       = NULL;
        mpMic           = NULL;
        mpLamp          = NULL;
        mpButton        = NULL;
        mpHooksw        = NULL;
        mpDisplay       = NULL;
}

PtTerminalComponentEvent::PtTerminalComponentEvent(
        const PtTerminalComponentEvent& rPtTerminalComponentEvent)
{
        mEventId = rPtTerminalComponentEvent.mEventId;

        mIntData1 = rPtTerminalComponentEvent.mIntData1;
        mIntData2 = rPtTerminalComponentEvent.mIntData2;
        mIntData3 = rPtTerminalComponentEvent.mIntData3;

        mStringData1 = rPtTerminalComponentEvent.mStringData1;
        mStringData2 = rPtTerminalComponentEvent.mStringData2;
        mStringData3 = rPtTerminalComponentEvent.mStringData3;

        mpRinger        = rPtTerminalComponentEvent.mpRinger;
        mpSpeaker       = rPtTerminalComponentEvent.mpSpeaker;
        mpMic           = rPtTerminalComponentEvent.mpMic;
        mpLamp          = rPtTerminalComponentEvent.mpLamp;
        mpButton        = rPtTerminalComponentEvent.mpButton;
        mpHooksw        = rPtTerminalComponentEvent.mpHooksw;
        mpDisplay       = rPtTerminalComponentEvent.mpDisplay;
}

PtTerminalComponentEvent::~PtTerminalComponentEvent()
{
        if (mpRinger)
        {
                delete mpRinger;
                mpRinger = 0;
        }

        if (mpSpeaker)
        {
                delete mpSpeaker;
                mpSpeaker = 0;
        }

        if (mpMic)
        {
                delete mpMic;
                mpMic = 0;
        }

        if (mpLamp)
        {
                delete mpLamp;
                mpLamp = 0;
        }

        if (mpButton)
        {
                delete mpButton;
                mpButton = 0;
        }

        if (mpHooksw)
        {
                delete mpHooksw;
                mpHooksw = 0;
        }

        if (mpDisplay)
        {
                delete mpDisplay;
                mpDisplay = 0;
        }
}

/* ============================ MANIPULATORS ============================== */

PtTerminalComponentEvent&
PtTerminalComponentEvent::operator=(const PtTerminalComponentEvent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        mpRinger        = rhs.mpRinger;
        mpSpeaker       = rhs.mpSpeaker;
        mpMic           = rhs.mpMic;
        mpLamp          = rhs.mpLamp;
        mpButton        = rhs.mpButton;
        mpHooksw        = rhs.mpHooksw;
        mpDisplay       = rhs.mpDisplay;

        return *this;
}

void PtTerminalComponentEvent::setStringData1(const char* stringData)
{
    mStringData1.remove(0);
    if(stringData) mStringData1.append(stringData);
}

void PtTerminalComponentEvent::setStringData2(const char* stringData)
{
    mStringData2.remove(0);
    if(stringData) mStringData2.append(stringData);
}

void PtTerminalComponentEvent::setStringData3(const char* stringData)
{
    mStringData3.remove(0);
    if(stringData) mStringData3.append(stringData);
}

void PtTerminalComponentEvent::setIntData1(int intData)
{
    mIntData1 = intData;
}

void PtTerminalComponentEvent::setIntData2(int intData)
{
    mIntData2 = intData;
}

void PtTerminalComponentEvent::setIntData3(int intData)
{
    mIntData3 = intData;
}

/* ============================ ACCESSORS ================================= */

void PtTerminalComponentEvent::getStringData1(char* stringData)
{
        if (stringData)
                strcpy(stringData, mStringData1.data());
}

void PtTerminalComponentEvent::getStringData2(char* stringData)
{
        if (stringData)
                strcpy(stringData, mStringData2.data());
}

void PtTerminalComponentEvent::getStringData3(char* stringData)
{
        if (stringData)
                strcpy(stringData, mStringData3.data());
}


int PtTerminalComponentEvent::getIntData1()
{
    return(mIntData1);
}

int PtTerminalComponentEvent::getIntData2()
{
    return(mIntData2);
}

int PtTerminalComponentEvent::getIntData3()
{
    return(mIntData3);
}

PtStatus PtTerminalComponentEvent::getComponent(PtComponent*& rpComponent)
{
        PtEventId id = PtEvent::EVENT_INVALID;

        if (PT_SUCCESS == getId(id))
        {
                switch (id)
                {
                case PtEvent::PHONE_RINGER_VOLUME_CHANGED:
        case PtEvent::PHONE_RINGER_PATTERN_CHANGED:
        case PtEvent::PHONE_RINGER_INFO_CHANGED:
                        if (!mpRinger)
                        {
                                mpRinger = new PtPhoneRinger(mpClient);
                        }
                        rpComponent = mpRinger;
                        break;
        case PtEvent::PHONE_SPEAKER_VOLUME_CHANGED:
                        if (!mpSpeaker)
                        {
                                mpSpeaker = new PtPhoneSpeaker(mpClient);
                        }
                        rpComponent = mpSpeaker;
                        break;
        case PtEvent::PHONE_MICROPHONE_GAIN_CHANGED:
                        if (!mpMic)
                        {
                                mpMic = new PtPhoneMicrophone(mpClient);
                        }
                        rpComponent = mpMic;
                        break;
        case PtEvent::PHONE_LAMP_MODE_CHANGED:
                        if (!mpLamp)
                        {
                                mpLamp = new PtPhoneLamp(mpClient);
                        }
                        rpComponent = mpLamp;
                        break;
        case PtEvent::PHONE_BUTTON_INFO_CHANGED:
        case PtEvent::PHONE_BUTTON_UP:
        case PtEvent::PHONE_BUTTON_DOWN:
        case PtEvent::PHONE_BUTTON_REPEAT:
                        if (!mpButton)
                        {
                                mpButton = new PtPhoneButton(mpClient);
                                mpButton->setInfo2((char*)mStringData1.data());
                        }
                        rpComponent = mpButton;
                        break;
        case PtEvent::PHONE_HOOKSWITCH_OFFHOOK:
        case PtEvent::PHONE_HOOKSWITCH_ONHOOK:
                        if (!mpHooksw)
                        {
                                mpHooksw = new PtPhoneHookswitch(mpClient);
                        }
                        rpComponent = mpHooksw;
                        break;
        case PtEvent::PHONE_DISPLAY_CHANGED:
                        if (!mpDisplay)
                        {
                                mpDisplay = new PtPhoneDisplay(mpClient);
                        }
                        rpComponent = mpDisplay;
                        break;
        case PtEvent::PHONE_HANDSET_VOLUME_CHANGED:
                        break;
        case PtEvent::PHONE_HANDSETMIC_GAIN_CHANGED:
                        break;
        case PtEvent::EVENT_INVALID:
                default:
                        break;
                }
        }

        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

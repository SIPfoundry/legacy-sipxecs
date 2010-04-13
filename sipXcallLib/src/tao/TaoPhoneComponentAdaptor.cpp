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
#include "tao/TaoPhoneComponentAdaptor.h"
#include "tao/TaoTransportTask.h"
#include "tao/TaoMessage.h"
#include "tao/TaoString.h"
#include "ptapi/PtPhoneLamp.h"
#include "ptapi/PtComponentGroup.h"
#include "ps/PsButtonTask.h"
#include "ps/PsPhoneTask.h"
#include "ps/PsHookswTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
TaoPhoneComponentAdaptor::TaoPhoneComponentAdaptor(TaoTransportTask*& rpSvrTransport,
                                           TaoMessage& rMsg,
                                           const UtlString& name ,
                                           const int maxRequestQMsgs)
        : TaoAdaptor("TaoPhnAdaptor-%d", maxRequestQMsgs)
{
        mpSvrTransport = rpSvrTransport;
        mpButtonTask = PsButtonTask::getButtonTask();
        mpPhoneTask = PsPhoneTask::getPhoneTask();
        mpHookswTask = PsHookswTask::getHookswTask();

#ifdef _VXWORKS
        MpCodec_getLCDContrastRange(mLCDLow, mLCDHigh, mLCDContrast);
        mLCDOffset = mLCDLow - 1;
#else
        mLCDOffset = -1;
        mLCDHigh = 10;
        mLCDLow = 0;
        mLCDContrast = 10;
#endif

        if (!isStarted())
        {
                start();
        }

}

// Copy constructor
TaoPhoneComponentAdaptor::TaoPhoneComponentAdaptor(const TaoPhoneComponentAdaptor& rTaoPhoneComponentAdaptor)
        : TaoAdaptor("TaoPhnAdaptor-%d")
{
        mpSvrTransport = rTaoPhoneComponentAdaptor.mpSvrTransport;
        mpButtonTask = PsButtonTask::getButtonTask();
        mpPhoneTask = PsPhoneTask::getPhoneTask();
        mpHookswTask = PsHookswTask::getHookswTask();
}

// Destructor
TaoPhoneComponentAdaptor::~TaoPhoneComponentAdaptor()
{
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean TaoPhoneComponentAdaptor::handleMessage(OsMsg& rMsg)
{
        UtlBoolean handled = FALSE;
        int msgType = ((TaoMessage&)rMsg).getMsgSubType();

        if( TaoMessage::REQUEST_PHONECOMPONENT == msgType)
        {
                switch (((TaoMessage&)rMsg).getCmd())
                {
                case TaoMessage::BUTTON_PRESS:
                        if (TAO_SUCCESS == buttonPress((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::BUTTON_UP:
                        if (TAO_SUCCESS == buttonUp((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::BUTTON_DOWN:
                        if (TAO_SUCCESS == buttonDown((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::BUTTON_GET_INFO:
                        if (TAO_SUCCESS == getButtonInfo((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::BUTTON_SET_INFO:
                        if (TAO_SUCCESS == setButtonInfo((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::BUTTON_GET_PHONELAMP:
                        if (TAO_SUCCESS == getAssociatedPhoneLamp((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::HOOKSWITCH_GET_STATE:
                        if (TAO_SUCCESS == getHookswState((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::HOOKSWITCH_SET_STATE:
                        if (TAO_SUCCESS == setHookswState((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::HOOKSWITCH_GET_CALL:
                        if (TAO_SUCCESS == getHookswCall((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::LAMP_GET_MODE:
                        if (TAO_SUCCESS == getHookswCall((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::LAMP_GET_SUPPORTED_MODES:
                        if (TAO_SUCCESS == getHookswCall((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::LAMP_GET_BUTTON:
                        if (TAO_SUCCESS == getHookswCall((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::LAMP_SET_MODE:
                        if (TAO_SUCCESS == setLampMode((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
        /* ----------------------------- PHONEDISPALY --------------------------------- */
                case TaoMessage::DISPLAY_GET_DISPLAY:
                        if (TAO_SUCCESS == getDisplay((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::DISPLAY_GET_ROWS:
                        if (TAO_SUCCESS == getDisplayRows((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::DISPLAY_GET_COLS:
                        if (TAO_SUCCESS == getDisplayColumns((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::DISPLAY_SET_DISPLAY:
                        if (TAO_SUCCESS == setDisplay((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::DISPLAY_GET_CONTRAST:
                        if (TAO_SUCCESS == getDisplayContrast((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::DISPLAY_SET_CONTRAST:
                        if (TAO_SUCCESS == setDisplayContrast((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
        /* ----------------------------- PHONEMIC --------------------------------- */
                case TaoMessage::MIC_GET_GAIN:
                        if (TAO_SUCCESS == getMicGain((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::MIC_SET_GAIN:
                        if (TAO_SUCCESS == setMicGain((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
        /* ----------------------------- PHONERINGER --------------------------------- */
                case TaoMessage::RINGER_SET_INFO:
                        if (TAO_SUCCESS == setRingerInfo((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::RINGER_SET_PATTERN:
                        if (TAO_SUCCESS == setRingerPattern((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::RINGER_SET_VOLUME:
                        if (TAO_SUCCESS == setRingerVolume((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::RINGER_GET_INFO:
                        if (TAO_SUCCESS == getRingerInfo((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::RINGER_GET_PATTERN:
                        if (TAO_SUCCESS == getRingerPattern((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::RINGER_GET_VOLUME:
                        if (TAO_SUCCESS == getRingerVolume((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::RINGER_GET_MAX_PATTERN_INDEX:
                        if (TAO_SUCCESS == getMaxRingPatternIndex((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::RINGER_GET_NUMBER_OF_RINGS:
                        if (TAO_SUCCESS == getNumberOfRings((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::RINGER_IS_ON:
                        if (TAO_SUCCESS == isRingerOn((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
        /* ----------------------------- PHONESPEAKER --------------------------------- */
                case TaoMessage::SPEAKER_SET_VOLUME:
                        if (TAO_SUCCESS == setSpeakerVolume((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::SPEAKER_GET_VOLUME:
                        if (TAO_SUCCESS == getSpeakerVolume((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::SPEAKER_GET_NOMINAL_VOLUME:
                        if (TAO_SUCCESS == getSpeakerNominalVolume((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
        /* ----------------------------- PHONEEXTSPEAKER --------------------------------- */
                case TaoMessage::EXTSPEAKER_SET_VOLUME:
                        if (TAO_SUCCESS == setExtSpeakerVolume((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::EXTSPEAKER_GET_VOLUME:
                        if (TAO_SUCCESS == getExtSpeakerVolume((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::EXTSPEAKER_GET_NOMINAL_VOLUME:
                        if (TAO_SUCCESS == getExtSpeakerNominalVolume((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
        /* ----------------------------- PHONEGROUP --------------------------------- */
                case TaoMessage::PHONEGROUP_ACTIVATE:
                        if (TAO_SUCCESS == activateGroup((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::PHONEGROUP_DEACTIVATE:
                        if (TAO_SUCCESS == deactivateGroup((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::PHONEGROUP_GET_COMPONENTS:
                        if (TAO_SUCCESS == getGroupComponents((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::PHONEGROUP_GET_DESCRIPTION:
                        if (TAO_SUCCESS == getGroupDescription((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::PHONEGROUP_GET_TYPE:
                        if (TAO_SUCCESS == getGroupType((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::PHONEGROUP_IS_ACTIVATED:
                        if (TAO_SUCCESS == isGroupActivated((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::COMPONENT_RESULT:
                        if (TAO_SUCCESS == returnResult((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;

                default:
                  break;
                }
        }
        else if (TaoMessage::RESPONSE_PHONECOMPONENT == msgType)
        {
                if (TAO_SUCCESS == returnResult((TaoMessage&)rMsg))
                        handled = TRUE;
        }

        if (!handled)
        {
                TaoMessage*     pMsg = (TaoMessage*) &rMsg;

                pMsg->setArgCnt(1);
                pMsg->setArgList("-1");

                if (mpSvrTransport->postMessage(*pMsg))
                        handled = TRUE;
        }

        return handled;
}

// Assignment operator
TaoPhoneComponentAdaptor&
TaoPhoneComponentAdaptor::operator=(const TaoPhoneComponentAdaptor& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

TaoStatus TaoPhoneComponentAdaptor::buttonPress(TaoMessage& rMsg)
{
        mInfo = rMsg.getArgList();

        int keyIndex = mpButtonTask->getButtonIndex(mInfo.data());
        if (keyIndex >= 0)
        {
                mpButtonTask->postEvent(PsMsg::BUTTON_DOWN, this, keyIndex);
                mpButtonTask->postEvent(PsMsg::BUTTON_UP, this, keyIndex);
        }
        else
                rMsg.setObjHandle((TaoObjHandle)TAO_INVALID_ARGUMENT);


        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::buttonUp(TaoMessage& rMsg)
{
        mInfo = rMsg.getArgList();

        int keyIndex = mpButtonTask->getButtonIndex(mInfo.data());
        if (keyIndex >= 0)
        {
                mpButtonTask->postEvent(PsMsg::BUTTON_UP, this, keyIndex);
        }
        else
                rMsg.setObjHandle((TaoObjHandle)TAO_INVALID_ARGUMENT);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::buttonDown(TaoMessage& rMsg)
{
        mInfo = rMsg.getArgList();

        int keyIndex = mpButtonTask->getButtonIndex(mInfo.data());
        if (keyIndex >= 0)
        {
                mpButtonTask->postEvent(PsMsg::BUTTON_DOWN, this, keyIndex);
        }
        else
                rMsg.setObjHandle((TaoObjHandle)TAO_INVALID_ARGUMENT);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::setButtonInfo(TaoMessage& rMsg)
{
        if (rMsg.getArgList())
        {
                mInfo = rMsg.getArgList();

                int keyIndex = mpButtonTask->getButtonIndex(mInfo.data());
                if (keyIndex >= 0)
                {
                        mpButtonTask->postEvent(PsMsg::BUTTON_SET_INFO, this, keyIndex);
                }
                else
                        rMsg.setObjHandle((TaoObjHandle)TAO_INVALID_ARGUMENT);

                rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
                if (mpSvrTransport->postMessage(rMsg))
                        return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::setHookswState(TaoMessage& rMsg)
{
        if (rMsg.getArgCnt())
        {
                mHookswState = atoi(rMsg.getArgList());

                PsMsg hookswMsg(PsMsg::HOOKSW_STATE, this, mHookswState, mHookswState);
                mpPhoneTask->postMessage(hookswMsg);

                rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
                if (mpSvrTransport->postMessage(rMsg))
                        return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::setRingerInfo(TaoMessage& rMsg)
{
        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);

        int index = atoi(arg[0]);
        UtlString info = arg[1];

        if (info)
        {
                PsMsg msg(PsMsg::RINGER_SET_INFO, this, index, 0);
                msg.setStringParam1(info.data());

                mpPhoneTask->postMessage(msg);

                rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
                if (mpSvrTransport->postMessage(rMsg))
                        return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::setRingerPattern(TaoMessage& rMsg)
{
        int index = atoi(rMsg.getArgList());

        PsMsg msg(PsMsg::RINGER_SET_PATTERN, this, index, 0);

        mpPhoneTask->postMessage(msg);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::setRingerVolume(TaoMessage& rMsg)
{
        int volume = atoi(rMsg.getArgList());

        mpPhoneTask->taoSetVolume(volume, PtComponentGroup::RINGER);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);

        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::setMicGain(TaoMessage& rMsg)
{
        if (rMsg.getArgCnt() != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        int groupType = atoi(arg[0]);
        int gain = atoi(arg[1]);

        mpPhoneTask->taoSetMicGain(groupType, gain);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::setSpeakerVolume(TaoMessage& rMsg)
{
        if (rMsg.getArgCnt() != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        int groupType = atoi(arg[1]);
        int volume = atoi(arg[0]);

        mpPhoneTask->taoSetVolume(volume, groupType);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);

        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::setExtSpeakerVolume(TaoMessage& rMsg)
{
        if (rMsg.getArgCnt() != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        int groupType = atoi(arg[1]);
        int volume = atoi(arg[0]);

        mpPhoneTask->taoSetVolume(volume, groupType);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);

        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::setLampMode(TaoMessage& rMsg)
{
        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString buttonInfo = arg[0];
        int mode = atoi(arg[1]);

        PsMsg msg(PsMsg::LAMP_SET_MODE, this, mode, 0);
        msg.setStringParam1(buttonInfo.data());

        mpPhoneTask->postMessage(msg);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::setDisplay(TaoMessage& rMsg)
{
        if (rMsg.getArgList())
        {
                rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
                if (mpSvrTransport->postMessage(rMsg))
                        return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::setDisplayContrast(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

#ifdef _VXWORKS
        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        int level = atoi(arg[0]);

        if (!mLCDLow && !mLCDHigh)
        {
                int nominal;
                MpCodec_getLCDContrastRange(mLCDLow, mLCDHigh, nominal);
                mLCDOffset = mLCDLow - 1;
        }

        level += mLCDOffset;
        if (level < mLCDLow)
                level = mLCDLow;
        if (level > mLCDHigh)
                level = mLCDHigh;

        if (OS_SUCCESS == MpCodec_setLCDContrast(level))
        {
                osPrintf("--- set LCD contrast level %d -> %d ---\n", mLCDContrast, level);
                mLCDContrast = level;
                rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);

                if (mpSvrTransport->postMessage(rMsg))
                {
                        return TAO_SUCCESS;
                }
        }
#endif

        return TAO_FAILURE;
}


TaoStatus TaoPhoneComponentAdaptor::activateGroup(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        mpPhoneTask->activateGroup((PsMsg&) rMsg);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

    return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::deactivateGroup(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

//      assert
        (mpPhoneTask->deactivateGroup((PsMsg&) rMsg));

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

    return TAO_FAILURE;
}

/* ============================ ACCESSORS ================================= */

TaoStatus TaoPhoneComponentAdaptor::getAssociatedPhoneLamp(TaoMessage& rMsg)
{
        //// need work
        TaoPhoneLamp* rpLamp;
        if (mpLamp)
        {
                rpLamp = mpLamp;

                rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
                if (mpSvrTransport->postMessage(rMsg))
                        return TAO_SUCCESS;
        }

    return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getButtonInfo(TaoMessage& rMsg)
{
        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getHookswState(TaoMessage& rMsg)
{
        PsMsg hookswMsg(PsMsg::HOOKSW_GET_STATE, this, mHookswState, mHookswState);
        mpPhoneTask->postMessage(hookswMsg);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getHookswCall(TaoMessage& rMsg)
{

        PsMsg hookswMsg(PsMsg::HOOKSW_GET_CALL, this, mHookswState, mHookswState);
        mpPhoneTask->postMessage(hookswMsg);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getMaxRingPatternIndex(TaoMessage& rMsg)
{
        PsMsg msg(PsMsg::RINGER_GET_MAX_PATTERN_INDEX, this, 0, 0);
        mpPhoneTask->postMessage(msg);

        return TAO_SUCCESS;
}

TaoStatus TaoPhoneComponentAdaptor::getNumberOfRings(TaoMessage& rMsg)
{
        PsMsg msg(PsMsg::RINGER_GET_NUMBER_OF_RINGS, this, 0, 0);
        mpPhoneTask->postMessage(msg);

        return TAO_SUCCESS;
}

TaoStatus TaoPhoneComponentAdaptor::getRingerInfo(TaoMessage& rMsg)
{
        if (rMsg.getArgCnt())
        {
                int index = atoi(rMsg.getArgList());

                PsMsg msg(PsMsg::RINGER_GET_INFO, this, index, 0);
                mpPhoneTask->postMessage(msg);

                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getRingerPattern(TaoMessage& rMsg)
{
        PsMsg msg(PsMsg::RINGER_GET_PATTERN, this, 0, 0);
        mpPhoneTask->postMessage(msg);

        return TAO_SUCCESS;
}

TaoStatus TaoPhoneComponentAdaptor::getRingerVolume(TaoMessage& rMsg)
{
        int volume = 0;
        mpPhoneTask->taoGetVolume(volume, PtComponentGroup::RINGER);

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];
    sprintf(buff, "%d", volume);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        rMsg.setArgCnt(1);
        rMsg.setArgList(buff);


        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getMicGain(TaoMessage& rMsg)
{
        if (rMsg.getArgCnt() != 1)
                return TAO_FAILURE;

        UtlString arg = rMsg.getArgList();
        int groupType = atoi(arg.data());

        int gain = mpPhoneTask->taoGetMicGain(groupType);

        char buf[32];
        sprintf(buf, "%d", gain);
        arg += TAOMESSAGE_DELIMITER + buf;

        rMsg.setArgCnt(2);
        rMsg.setArgList(arg);
        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getSpeakerVolume(TaoMessage& rMsg)
{
        int volume = 0;
        int groupType = atoi(rMsg.getArgList());

        mpPhoneTask->taoGetVolume(volume, groupType);

        if (volume > 10)
                volume = 10;
        if (volume < 0)
                volume = 0;

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];
    sprintf(buff, "%d", volume);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        rMsg.setArgCnt(1);
        rMsg.setArgList(buff);


        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getSpeakerNominalVolume(TaoMessage& rMsg)
{
        int volume = 0;
        int groupType = atoi(rMsg.getArgList());

        mpPhoneTask->taoGetNominalVolume(volume, groupType);

        if (volume > 10)
                volume = 10;
        if (volume < 0)
                volume = 0;

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];
    sprintf(buff, "%d", volume);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        rMsg.setArgCnt(1);
        rMsg.setArgList(buff);


        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getExtSpeakerVolume(TaoMessage& rMsg)
{
        int volume = 0;
        int groupType = atoi(rMsg.getArgList());

        mpPhoneTask->taoGetVolume(volume, groupType);

        if (volume > 10)
                volume = 10;
        if (volume < 0)
                volume = 0;

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];
    sprintf(buff, "%d", volume);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        rMsg.setArgCnt(1);
        rMsg.setArgList(buff);


        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getExtSpeakerNominalVolume(TaoMessage& rMsg)
{
        int volume = 0;
        int groupType = atoi(rMsg.getArgList());

        mpPhoneTask->taoGetNominalVolume(volume, groupType);

        if (volume > 10)
                volume = 10;
        if (volume < 0)
                volume = 0;

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];
    sprintf(buff, "%d", volume);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        rMsg.setArgCnt(1);
        rMsg.setArgList(buff);


        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getAssociatedPhoneButton(TaoMessage& rMsg)
{
        if (rMsg.getArgList())
        {
                rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
                if (mpSvrTransport->postMessage(rMsg))
                        return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getLampMode(TaoMessage& rMsg)
{
        PsMsg msg(PsMsg::LAMP_GET_MODE, this, 0, 0);
        mpPhoneTask->postMessage(msg);

        return TAO_SUCCESS;
}

TaoStatus TaoPhoneComponentAdaptor::getSupportedLampModes(TaoMessage& rMsg)
{
        PsMsg msg(PsMsg::LAMP_GET_SUPPORTED_MODES, this, 0, 0);
        mpPhoneTask->postMessage(msg);

        return TAO_SUCCESS;
}

TaoStatus TaoPhoneComponentAdaptor::getDisplayRows(TaoMessage& rMsg)
{
        if (rMsg.getArgList())
        {
                rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
                if (mpSvrTransport->postMessage(rMsg))
                        return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getDisplayColumns(TaoMessage& rMsg)
{
        if (rMsg.getArgList())
        {
                rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
                if (mpSvrTransport->postMessage(rMsg))
                        return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getDisplay(TaoMessage& rMsg)
{
        if (rMsg.getArgList())
        {
                rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
                if (mpSvrTransport->postMessage(rMsg))
                        return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getDisplayContrast(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

#ifdef _VXWORKS
        int nominal = 0;

        if (OS_SUCCESS == MpCodec_getLCDContrastRange(mLCDLow, mLCDHigh, nominal))
        {
                mLCDContrast = MpCodec_getLCDContrast();
                mLCDOffset = mLCDLow - 1;
                osPrintf("--- get LCD contrast level %d ---\n", mLCDContrast);
        }

        UtlString arg;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        int level = mLCDContrast - mLCDOffset;
        if (level < 1)
                level = 1;

    sprintf(buff, "%d", level);
        arg = buff + TAOMESSAGE_DELIMITER;

    sprintf(buff, "%d", 1);
        arg += buff + TAOMESSAGE_DELIMITER;

    sprintf(buff, "%d", (mLCDHigh - mLCDOffset));
        arg += buff + TAOMESSAGE_DELIMITER;

    sprintf(buff, "%d", (nominal - mLCDOffset));
        arg += buff;

        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        rMsg.setArgCnt(4);
        rMsg.setArgList(arg.data());
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
#endif

    return TAO_FAILURE;
}

TaoStatus TaoPhoneComponentAdaptor::getGroupComponents(TaoMessage& rMsg)
{
        int size = atoi(rMsg.getArgList());

        PsMsg msg(PsMsg::PHONEGROUP_GET_COMPONENTS, this, size, 0);
        mpPhoneTask->postMessage(msg);

        return TAO_SUCCESS;
}

TaoStatus TaoPhoneComponentAdaptor::getGroupDescription(TaoMessage& rMsg)
{
        PsMsg msg(PsMsg::PHONEGROUP_GET_DESCRIPTION, this, 0, 0);
        mpPhoneTask->postMessage(msg);

        return TAO_SUCCESS;
}


TaoStatus TaoPhoneComponentAdaptor::getGroupType(TaoMessage& rMsg)
{
        PsMsg msg(PsMsg::PHONEGROUP_GET_TYPE, this, 0, 0);
        mpPhoneTask->postMessage(msg);

        return TAO_SUCCESS;
}

// get feedback from ps, return to client via transport, used
// by all get and set calls
TaoStatus TaoPhoneComponentAdaptor::returnResult(TaoMessage& rMsg)
{

        osPrintf("->>entering TaoPhoneComponentAdaptor:: returnResult<<-\n");
        rMsg.setMsgSubType(TaoMessage::RESPONSE_PHONECOMPONENT);
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

/* ============================ INQUIRY =================================== */

TaoStatus TaoPhoneComponentAdaptor::isRingerOn(TaoMessage& rMsg)
{
        PsMsg msg(PsMsg::RINGER_IS_ON, this, 0, 0);
        mpPhoneTask->postMessage(msg);

        return TAO_SUCCESS;
}

TaoStatus TaoPhoneComponentAdaptor::isGroupActivated(TaoMessage& rMsg)
{
        PsMsg msg(PsMsg::PHONEGROUP_IS_ACTIVATED, this, 0, 0);
        mpPhoneTask->postMessage(msg);

        return TAO_SUCCESS;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

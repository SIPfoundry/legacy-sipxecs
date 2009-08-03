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
#include <time.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

#ifdef _VXWORKS
#   include <hostLib.h>
#   include <bootLib.h>
#   include <sysLib.h>
#   include <resolvLib.h>
#   include <socket.h>
#   include <../config/pingtel/pingtel.h>
#   include <dosFsLib.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>


// APPLICATION INCLUDES
#ifdef _VXWORKS
   int touch(char *name);
#endif
#include "os/OsLock.h"
#include "os/OsReadLock.h"
#include "os/OsTime.h"
#include "os/OsUtil.h"
#include "os/OsWriteLock.h"
#include "os/OsSocket.h"
#include "ps/PsButtonId.h"
#include "ps/PsButtonTask.h"
#include "ps/PsHookswTask.h"
#include "ps/PsLampTask.h"
#include "ps/PsPhoneTask.h"
#include "ps/PsButtonInfo.h"
#include "ps/PsTaoComponent.h"
#include "ps/PsTaoComponentGroup.h"
#include "ps/PsTaoButton.h"
#include "ps/PsTaoHookswitch.h"
#include "ps/PsTaoDisplay.h"
#include "ps/PsTaoLamp.h"
#include "ps/PsTaoRinger.h"
#include "ps/PsTaoMicrophone.h"
#include "ps/PsTaoSpeaker.h"
#include "ptapi/PtTerminalConnection.h"
#include "ptapi/PtComponent.h"
#include "ptapi/PtComponentGroup.h"
#include "ptapi/PtConnection.h"
#include "tao/TaoDefs.h"
#include "tao/TaoMessage.h"
#include "tao/TaoPhoneComponentAdaptor.h"
#include "tao/TaoTerminalAdaptor.h"
#include "tao/TaoObjectMap.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
PsPhoneTask* PsPhoneTask::spInstance = 0;
OsBSem       PsPhoneTask::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the Phone task, creating it if necessary
PsPhoneTask* PsPhoneTask::getPhoneTask(void)
{
   UtlBoolean isStarted;

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (spInstance != NULL && spInstance->isStarted())
      return spInstance;

   // If the task does not yet exist or hasn't been started, then acquire
   // the lock to ensure that only one instance of the task is started
   sLock.acquire();
   if (spInstance == NULL)
       spInstance = new PsPhoneTask();

   isStarted = spInstance->isStarted();
   if (!isStarted)
   {
      isStarted = spInstance->start();
      assert(isStarted);
   }
   sLock.release();

   return spInstance;
}

// Destructor
PsPhoneTask::~PsPhoneTask()
{
   mpButtonTask->requestShutdown();   // shut down the button task
   delete mpButtonTask;

   mpHookswTask->requestShutdown();   // shut down the hookswitch task
   delete mpHookswTask;

   mpLampTask->requestShutdown();     // shut down the lamp task
   delete mpLampTask;

   delete mpListenerCnt;

   if (mpListeners)
   {
                delete mpListeners;
                mpListeners = 0;
   }

   if (mpActiveListeners)
   {
                delete[] mpActiveListeners;
                mpActiveListeners = 0;
   }

   spInstance = NULL;
}

/* ============================ MANIPULATORS ============================== */

// Register as a listener for phone messages.
// For now, the phone task allows at most one listener and forwards
// all keypad and hookswitch messages to that listener.
OsStatus PsPhoneTask::addListener(OsServerTask* pListener)
{
        if (!mpListenerCnt)
                mpListenerCnt = new TaoReference();
        if (!mpListeners)
                mpListeners = new TaoObjectMap();

        int cnt = mpListenerCnt->add();
        mpListeners->insert((TaoObjHandle)cnt, (TaoObjHandle)pListener);

        if (mpActiveListeners)
                delete[] mpActiveListeners;
        mpActiveListeners = new TaoObjHandle[cnt];

        mpListeners->getActiveObjects(mpActiveListeners, cnt);

        return OS_SUCCESS;
}

// Create a phone message and post it to the Phone task
// Return the result of the message send operation.
OsStatus PsPhoneTask::postEvent(const int msg, void* source,
                                const int param1, const int param2,
                                const OsTime& rTimeout)
{
   PsMsg    phoneMsg(msg, source, param1, param2);
   OsStatus res;

   res = postMessage(phoneMsg, rTimeout);
   return res;
}

int PsPhoneTask::taoGetMicGain(int group)
{
        OsStatus rc = OS_INVALID_ARGUMENT;
        int level = 0;

        switch(group)
        {
        case PtComponentGroup::HAND_SET:
           if (mpHandSetGroup)
              rc = mpHandSetGroup->getMicGain(PsTaoComponentGroup::HAND_SET, level);
           break;
        case PtComponentGroup::SPEAKER_PHONE:
           if (mpSpeakerPhoneGroup)
              rc = mpSpeakerPhoneGroup->getMicGain(PsTaoComponentGroup::SPEAKER_PHONE, level);
           break;
        case PtComponentGroup::RINGER:
           if (mpPhoneSetGroup)
              rc = mpPhoneSetGroup->getMicGain(PsTaoComponentGroup::PHONE_SET, level);
           break;
        case PtComponentGroup::PHONE_SET:
           if (mpPhoneSetGroup)
              rc = mpPhoneSetGroup->getMicGain(PsTaoComponentGroup::PHONE_SET, level);
           break;
        case PtComponentGroup::HEAD_SET:
           if (mpHeadSetGroup)
              rc = mpHeadSetGroup->getMicGain(PsTaoComponentGroup::HEAD_SET, level);
           break;
        default:
           level = getGain();
           break;
        }

        return level;
}

OsStatus PsPhoneTask::taoSetMicGain(int group, int level)
{
        OsStatus rc = OS_INVALID_ARGUMENT;

        switch(group)
        {
        case PtComponentGroup::HAND_SET:
                if (mpHandSetGroup)
                        rc = mpHandSetGroup->setMicGain(PsTaoComponentGroup::HAND_SET, level);
           break;
        case PtComponentGroup::SPEAKER_PHONE:
           if (mpSpeakerPhoneGroup)
                        rc = mpSpeakerPhoneGroup->setMicGain(PsTaoComponentGroup::SPEAKER_PHONE, level);
           break;
        case PtComponentGroup::RINGER:
           if (mpPhoneSetGroup)
                        rc = mpPhoneSetGroup->setMicGain(PsTaoComponentGroup::PHONE_SET, level);
           break;
        case PtComponentGroup::PHONE_SET:
           if (mpPhoneSetGroup)
                        rc = mpPhoneSetGroup->setMicGain(PsTaoComponentGroup::PHONE_SET, level);
           break;
        case PtComponentGroup::HEAD_SET:
           if (mpHeadSetGroup)
                        rc = mpHeadSetGroup->setMicGain(PsTaoComponentGroup::HEAD_SET, level);
           break;
        default:
                setGain(level);
          break;
        }

        return rc;
}

// Set the microphone gain to a level between 0 (low) and 100 (high)
OsStatus PsPhoneTask::setGain(int level)
{
        OsStatus rc = OS_UNSPECIFIED;

   osPrintf("PsPhoneTask::setGain(%d)\n", level);

   if (level < 0)   level = 0;   // constrain level to be between
   if (level > mHigh) level = mHigh; // 0 and 100;

   switch(mSpkrMode)
   {
   case HANDSET_ENABLED:
                if (mpHandSetGroup)
                {
                        rc = mpHandSetGroup->setMicGain(PsTaoComponentGroup::HAND_SET, level);
#ifdef _VXWORKS
                        MpCodec_setMicMode(CODEC_ENABLE_HANDSET_MIC);
#endif
                }
           break;
   case SPEAKERPHONE_ENABLED:
           if (mpSpeakerPhoneGroup)
           {
                        rc = mpSpeakerPhoneGroup->setMicGain(PsTaoComponentGroup::SPEAKER_PHONE, level);
#ifdef _VXWORKS
                        MpCodec_setMicMode(CODEC_ENABLE_BASE_MIC);
#endif
                }
           break;
   case RINGER_ENABLED:
           if (mpPhoneSetGroup)
           {
                        rc = mpPhoneSetGroup->setMicGain(PsTaoComponentGroup::PHONE_SET, level);
#ifdef _VXWORKS
                        MpCodec_setMicMode(CODEC_ENABLE_BASE_MIC);
#endif
                }
           break;
   case SOUND_ENABLED:
           if (mpPhoneSetGroup)
           {
                        rc = mpPhoneSetGroup->setMicGain(PsTaoComponentGroup::PHONE_SET, level);
#ifdef _VXWORKS
                        MpCodec_setMicMode(CODEC_ENABLE_BASE_MIC);
#endif
                }
           break;
   case HEADSET_ENABLED:
           if (mpHeadSetGroup)
           {
                        rc = mpHeadSetGroup->setMicGain(PsTaoComponentGroup::HEAD_SET, level);
#ifdef _VXWORKS
                        MpCodec_setMicMode(CODEC_ENABLE_HEADSET_MIC);
#endif
                }
           break;
   default:
//      assert(FALSE);
      break;
   }

   osPrintf("PsPhoneTask::setGain - MpCodec_setGain(%d)\n", level);
   assert(false);

   return rc;
}

// Sets the speaker mode for the phone (which speakers are enabled).
// and returns the previous setting.
void PsPhoneTask::setSpeakerMode(int mode)
{

   assert((mode & ~(HANDSET_ENABLED | SPEAKERPHONE_ENABLED | RINGER_ENABLED |
      HEADSET_ENABLED | SOUND_ENABLED | EXTSPEAKER_ENABLED)) == 0);
   mSpkrMode = mode;

#ifdef _VXWORKS
        {
                if (mode & HANDSET_ENABLED)
                {
                  MpCodec_setSpeakerMode(CODEC_ENABLE_HANDSET_SPKR);
                  MpCodec_setMicMode(CODEC_ENABLE_HANDSET_MIC);
                  return;
                }

                if (mode & SPEAKERPHONE_ENABLED)
                {
                  MpCodec_setSpeakerMode(CODEC_ENABLE_BASE_SPKR);
                  MpCodec_setMicMode(CODEC_ENABLE_BASE_MIC);
                  return;
                }

                if (mode & RINGER_ENABLED)
                {
                  MpCodec_setSpeakerMode(CODEC_ENABLE_RINGER_SPKR);
                  MpCodec_setMicMode(CODEC_DISABLE_MIC);
                  return;
                }

                if (mode & SOUND_ENABLED)
                {
                  MpCodec_setSpeakerMode(CODEC_ENABLE_BASE_SPKR);
                  MpCodec_setMicMode(CODEC_DISABLE_MIC);
                  return;
                }

                if (mode & HEADSET_ENABLED)
                {
                  MpCodec_setSpeakerMode(CODEC_ENABLE_HEADSET_SPKR);
                  MpCodec_setMicMode(CODEC_ENABLE_HEADSET_MIC);
                  return;
                }

                if (mode & EXTSPEAKER_ENABLED)
                {
                        if (MpCodec_isExtSpkrPresent())
                        {
                          MpCodec_setSpeakerMode(CODEC_ENABLE_EXTERNAL_SPKR_MONO);
                          return;
                        }
                        else
                        {
                          MpCodec_setSpeakerMode(CODEC_ENABLE_BASE_SPKR);
                          MpCodec_setMicMode(CODEC_ENABLE_BASE_MIC);
                          return;
                        }
                }

                MpCodec_setSpeakerMode(CODEC_DISABLE_SPKR);
                MpCodec_setMicMode(CODEC_DISABLE_MIC);
        }
#endif
}

void PsPhoneTask::speakerModeEnable(int mode)
{
   mode |= mSpkrMode;
   setSpeakerMode(mode);
}

void PsPhoneTask::speakerModeDisable(int mode)
{
    mode = (mSpkrMode & (0xffffffff - mode));
    if (mode == 0)
    {
        mSpkrMode = mode;
        #ifdef _VXWORKS
        MpCodec_setSpeakerMode(CODEC_DISABLE_SPKR);
        MpCodec_setMicMode(CODEC_DISABLE_MIC);
        #endif
    }
   else
   {
       setSpeakerMode(mode);
   }
}

// Set the speaker volume to a level between 0 (low) and 100 (high)
OsStatus PsPhoneTask::setVolume(int level)
{
    if (mSpkrMode & HANDSET_ENABLED ||
    mSpkrMode & SPEAKERPHONE_ENABLED ||
    mSpkrMode & RINGER_ENABLED ||
    mSpkrMode & SOUND_ENABLED ||
    mSpkrMode & HEADSET_ENABLED)
    {
        osPrintf("< PsPhoneTask::setVolume: %d mode: %d>\n", level, mSpkrMode);
        assert(false);
    }

    return OS_SUCCESS;
}

void PsPhoneTask::extSpeakerConnect(UtlBoolean connected)
{
        int state = PsMsg::EXTSPEAKER_DISCONNECT;
        if (connected)
                state = PsMsg::EXTSPEAKER_CONNECT;

        PsMsg    msg(state, 0, 0, 0);

        postListenerMessage(msg);
}

/* ============================ ACCESSORS ================================= */

// Return the current microphone gain setting (a value from 0..100)
int PsPhoneTask::getGain(void) const
{
        OsStatus rc = OS_UNSPECIFIED;

        int level1 = 0;
        int level = 0;

#ifdef _VXWORKS
   level1 = MpCodec_getGain();
#endif

   switch(mSpkrMode)
   {
   case HANDSET_ENABLED:
                if (mpHandSetGroup)
                        rc = mpHandSetGroup->getMicGain(PsTaoComponentGroup::HAND_SET, level);
           break;
   case SPEAKERPHONE_ENABLED:
           if (mpSpeakerPhoneGroup)
                        rc = mpSpeakerPhoneGroup->getMicGain(PsTaoComponentGroup::SPEAKER_PHONE, level);
           break;
   case RINGER_ENABLED:
           if (mpPhoneSetGroup)
                        rc = mpPhoneSetGroup->getMicGain(PsTaoComponentGroup::PHONE_SET, level);
           break;
   case SOUND_ENABLED:
           if (mpPhoneSetGroup)
                        rc = mpPhoneSetGroup->getMicGain(PsTaoComponentGroup::PHONE_SET, level);
           break;
   case HEADSET_ENABLED:
           if (mpHeadSetGroup)
                        rc = mpHeadSetGroup->getMicGain(PsTaoComponentGroup::HEAD_SET, level);
           break;
   default:
//      assert(FALSE);
      break;
   }

   // :BUG: [XCL-15] This expression is almost certainly wrong, as it
   // returns level only when level == 0.  Bob A. says that getGain is
   // not used these days, so there is no point fixing this code, which is
   // VxWorks-specific anyway.
   return (level ? level1 : level);
}

// Returns the speaker mode for the phone (which speakers are enabled).
int PsPhoneTask::getSpeakerMode(void)
{
   return mSpkrMode;
}

void PsPhoneTask::taoGetNominalVolume(int& volume, int type)
{
        int isNominal = 1;

        switch (type)
        {
        case PtComponentGroup::HEAD_SET:
           if (mpHeadSetGroup)
           {
                        mpHeadSetGroup->getHeadsetVolume(volume, isNominal);
           }
                break;
        case PtComponentGroup::HAND_SET:
           if (mpHandSetGroup)
           {
                        mpHandSetGroup->getHandsetVolume(volume, isNominal);
           }
                break;
        case PtComponentGroup::SPEAKER_PHONE:
           if (mpSpeakerPhoneGroup)
           {
                        mpSpeakerPhoneGroup->getSpeakerVolume(volume, isNominal);
           }
                break;
        case PtComponentGroup::EXTERNAL_SPEAKER:
           if (mpExtSpeakerGroup)
           {
                        mpExtSpeakerGroup->getExtSpeakerVolume(volume, isNominal);
           }
                break;
        case PtComponentGroup::PHONE_SET:
           if (mpPhoneSetGroup)
           {
                        mpPhoneSetGroup->getRingerVolume(volume, isNominal);
           }
                break;
        case PtComponentGroup::RINGER:
        default:
           if (mpPhoneSetGroup)
           {
                        mpPhoneSetGroup->getRingerVolume(volume, isNominal);
           }
                break;
        }
}

void PsPhoneTask::taoGetVolume(int& volume, int type)
{
        switch (type)
        {
        case PtComponentGroup::HEAD_SET:
           if (mpHeadSetGroup)
           {
                        mpHeadSetGroup->getHeadsetVolume(volume);
           }
                break;
        case PtComponentGroup::HAND_SET:
           if (mpHandSetGroup)
           {
                        mpHandSetGroup->getHandsetVolume(volume);
           }
                break;
        case PtComponentGroup::SPEAKER_PHONE:
           if (mpSpeakerPhoneGroup)
           {
                        mpSpeakerPhoneGroup->getSpeakerVolume(volume);
           }
                break;
        case PtComponentGroup::EXTERNAL_SPEAKER:
           if (mpExtSpeakerGroup)
           {
                        mpExtSpeakerGroup->getExtSpeakerVolume(volume);
           }
                break;
        case PtComponentGroup::PHONE_SET:
           if (mpPhoneSetGroup)
           {
                        mpPhoneSetGroup->getRingerVolume(volume);
           }
                break;
        case PtComponentGroup::RINGER:
        default:
           if (mpPhoneSetGroup)
           {
                        mpPhoneSetGroup->getRingerVolume(volume);
           }
                break;
        }
}

void PsPhoneTask::taoSetVolume(int volume, int type)
{
        volume = (volume <= 0) ? 0 : volume;

    osPrintf("PsPhoneTask::taoSetVolume  volume = %d  grouptype = %d  \n",volume,type);

        switch (type)
        {
        case PtComponentGroup::HEAD_SET:
                if (mpHeadSetGroup)
                {
                        mpHeadSetGroup->setHeadsetVolume(volume);
                }
                break;
        case PtComponentGroup::HAND_SET:
                if (mpHandSetGroup)
                {
                        mpHandSetGroup->setHandsetVolume(volume);
                }
                break;
        case PtComponentGroup::SPEAKER_PHONE:
                if (mpSpeakerPhoneGroup)
                {
                        mpSpeakerPhoneGroup->setSpeakerVolume(volume);
                }
                break;
        case PtComponentGroup::EXTERNAL_SPEAKER:
                if (mpExtSpeakerGroup)
                {
                        mpExtSpeakerGroup->setExtSpeakerVolume(volume);
                }
                break;
        case PtComponentGroup::PHONE_SET:
                if (mpPhoneSetGroup)
                {
                        mpPhoneSetGroup->setRingerVolume(volume);
                }
                break;
        case PtComponentGroup::RINGER:
                if (mpPhoneSetGroup)
                {
                        mpPhoneSetGroup->setRingerVolume(volume);
                }
                break;
        default:
                break;
        }

    //if the group type is correct for the given mSpkMode then it's ok to REALLY change the value.

    if ((type == PtComponentGroup::HEAD_SET && (mSpkrMode & HEADSET_ENABLED)) ||
        (type == PtComponentGroup::HAND_SET && (mSpkrMode & HANDSET_ENABLED)) ||
        (type == PtComponentGroup::SPEAKER_PHONE && (mSpkrMode & SPEAKERPHONE_ENABLED)) ||
        (type == PtComponentGroup::PHONE_SET && (mSpkrMode & RINGER_ENABLED)) ||
        (type == PtComponentGroup::RINGER && (mSpkrMode & RINGER_ENABLED)) ||
        (type == PtComponentGroup::EXTERNAL_SPEAKER && (mSpkrMode & SPEAKERPHONE_ENABLED)))
    {
        setVolume(volume);
            osPrintf("<-- taoSetVolume %d -->\n", volume);
    }
}

// Return the current speaker volume setting (a value from 0..100)
int PsPhoneTask::getVolume()
{
        int level = 0;

   switch(mSpkrMode)
   {
   case HANDSET_ENABLED:
           if (mpHandSetGroup)
           {
                        mpHandSetGroup->getHandsetVolume(level);
                        osPrintf("->>PsPhoneTask::getVolume: HANDSET_ENABLED: %d<<-\n", level);
           }
           break;
   case SPEAKERPHONE_ENABLED:
           if (mpSpeakerPhoneGroup)
           {
                        mpSpeakerPhoneGroup->getSpeakerVolume(level);
                        osPrintf("->>PsPhoneTask::getVolume: SPEAKERPHONE_ENABLED: %d<<-\n", level);
           }
           break;
   case EXTSPEAKER_ENABLED:
           if (mpExtSpeakerGroup)
           {
                        mpExtSpeakerGroup->getExtSpeakerVolume(level);
                        osPrintf("->>PsPhoneTask::getVolume: EXTSPEAKER_ENABLED: %d<<-\n", level);
           }
           break;
   case SOUND_ENABLED:
           if (mpPhoneSetGroup)
           {
                        mpPhoneSetGroup->getSpeakerVolume(level);
                        osPrintf("->>PsPhoneTask::getVolume: SOUND_ENABLED: %d<<-\n", level);
           }
           break;
   case RINGER_ENABLED:
   default:
           if (mpPhoneSetGroup)
           {
                        mpPhoneSetGroup->getRingerVolume(level);
                        osPrintf("->>PsPhoneTask::getVolume: RINGER_ENABLED: %d<<-\n", level);
           }
           break;
   }

        return level;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Default constructor (called only indirectly via getPhoneTask())
PsPhoneTask::PsPhoneTask()
:  OsServerTask("PsPhone"),
   mMutex(OsRWMutex::Q_PRIORITY),  // create mutex for protecting data
   mpButtonTask(NULL),
   mpHookswTask(NULL),
   mpLampTask(NULL),
   mSpkrMode(0),

   mpListeners(NULL),
   mpListenerCnt(NULL),
   mpActiveListeners(NULL),

   mpTaoHooksw(NULL),
   mpTaoButton(NULL),
   mpTaoLamp(NULL),
   mpTaoRinger(NULL),
   mpTaoDisplay(NULL),

   mpHeadSetGroup(NULL),
   mpTaoHeadsetMic(NULL),
   mpTaoHeadsetSpeaker(NULL),

   mpHandSetGroup(NULL),
   mpTaoHandsetMic(NULL),
   mpTaoHandsetSpeaker(NULL),

   mpExtSpeakerGroup(NULL),
   mpSpeakerPhoneGroup(NULL),
   mpPhoneSetGroup(NULL),
   mpTaoBaseMic(NULL),
   mpTaoBaseSpeaker(NULL),
   mpTaoExtSpeaker(NULL),

   mpOtherGroup(NULL),
   mStepSize(10),
   mHigh(100),
   mLow(0),
   mNominal(5),
   mSplash(4)
{
   mpHookswTask = PsHookswTask::getHookswTask();  // hook switch handling

   mpButtonTask = PsButtonTask::getButtonTask();  // keypad handling

   mpLampTask = PsLampTask::getLampTask();        // lamp handling

   // initialize platform-specific settings
   initPlatformButtonSettings(mpButtonTask);
   initComponentGroups();
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Handle an incoming message.
// Return TRUE if the message was handled, otherwise FALSE.
UtlBoolean PsPhoneTask::handleMessage(OsMsg& rMsg)
{
   UtlBoolean   processed;
   OsWriteLock lock(mMutex);            // acquire a write lock

   switch (rMsg.getMsgType())
   {
   case OsMsg::PS_MSG:
      processed = handlePhoneMessage((PsMsg&) rMsg);
      break;
   default:
      assert(FALSE);
      processed = FALSE;                // unexpected message type
      break;
   }

   return processed;
}

// Handle an incoming phone message (class PsMsg or descendants).
// Return TRUE if the message was handled, otherwise FALSE.
// A write lock should be acquired before calling this method.
UtlBoolean PsPhoneTask::handlePhoneMessage(const PsMsg& rMsg)
{
   UtlBoolean processed;

   void* msgSrc;
   int   msgType;
   int   param1;
   int   param2;

   processed = TRUE;

   msgType = rMsg.getMsg();
   msgSrc  = rMsg.getMsgSource();
   param1  = rMsg.getParam1();
   param2  = rMsg.getParam2();
   TaoMessage *pMsg = (TaoMessage *) &rMsg;

   switch (msgType)
   {
   case PsMsg::BUTTON_DOWN:
   case PsMsg::BUTTON_REPEAT:
   case PsMsg::BUTTON_UP:
   case PsMsg::KEY_DOWN:
   case PsMsg::KEY_UP:


      if (mpListeners != NULL)
      {
         postListenerMessage(rMsg);
      }
      else
      {
/*
         cout << "Phone message:" << endl;
         cout << "  type="   << msgType <<
                 ", source=" << msgSrc <<
                 ", param1=" << param1 <<
                 ", param2=" << param2 << endl;
*/
      }
      break;
   case PsMsg::BUTTON_SET_INFO:
      if (mpTaoButton)
      {
         char buf[MAXIMUM_INTEGER_STRING_LENGTH];
         sprintf(buf, "%d", param1);

         if (mpTaoButton->setInfo(buf) && mpListeners != NULL)
            postListenerMessage(rMsg);
      }
      else if (mpListeners != NULL)
      {
         postListenerMessage(rMsg);
      }
      break;
   case PsMsg::HOOKSW_STATE:
   case PsMsg::HOOKSW_SET_STATE:
      if (mpListeners != NULL)
         postListenerMessage(rMsg);
      else
      {
/*
         cout << "Phone message:" << endl;
         cout << "  type="   << msgType <<
                 ", source=" << msgSrc <<
                 ", param1=" << param1 <<
                 ", param2=" << param2 << endl;
*/
      }
      break;
   case PsMsg::TERMINAL_GET_COMPONENT:
      processed = getComponent((PsMsg&)rMsg);
      break;
   case PsMsg::TERMINAL_GET_COMPONENTS:
      processed = getComponents((PsMsg&)rMsg);
      break;
   case PsMsg::TERMINAL_GET_COMPONENTGROUPS:
      processed = getComponentGroups((PsMsg&)rMsg);
      break;
   case PsMsg::HOOKSW_GET_CALL:
   case PsMsg::HOOKSW_GET_STATE:
      if (mpListeners != NULL)
      {
         postListenerMessage(rMsg);
      }
      break;
   case PsMsg::MIC_GET_GAIN:
      {
         int gain = getGain();
         char buf[MAXIMUM_INTEGER_STRING_LENGTH];
         sprintf(buf, "%d", gain);

         osPrintf("->>PsPhoneTask::MIC_GET_GAIN: %d<<-\n", gain);
         pMsg->setCmd(TaoMessage::COMPONENT_RESULT);
         pMsg->setArgList(buf);

         ((TaoPhoneComponentAdaptor*) msgSrc)->postMessage(*pMsg);
      }
      break;
   case PsMsg::MIC_SET_GAIN:
      osPrintf("->>PsPhoneTask::MIC_SET_GAIN: %d<<-\n", param1);
      setGain(param1);
      pMsg->setCmd(TaoMessage::COMPONENT_RESULT);

      ((TaoPhoneComponentAdaptor*) msgSrc)->postMessage(*pMsg);
      break;
/* ----------------------------- PHONERINGER --------------------------------- */
   case PsMsg::RINGER_SET_INFO:
      {
         // do nothing for now
         pMsg->setCmd(TaoMessage::COMPONENT_RESULT);

         ((TaoPhoneComponentAdaptor*) msgSrc)->postMessage(*pMsg);
         break;
      }
   case PsMsg::RINGER_SET_PATTERN:
      {// do nothing for now
         pMsg->setCmd(TaoMessage::COMPONENT_RESULT);

         ((TaoPhoneComponentAdaptor*) msgSrc)->postMessage(*pMsg);
      }
      break;
   case PsMsg::RINGER_SET_VOLUME:
   case PsMsg::SPEAKER_SET_VOLUME:
      {
         osPrintf("->>PsPhoneTask::SPEAKER/RINGER_SET_VOLUME: %d<<-\n", param1);
         setVolume(param1);
         pMsg->setCmd(TaoMessage::COMPONENT_RESULT);

         ((TaoPhoneComponentAdaptor*) msgSrc)->postMessage(*pMsg);
      }
      break;
   case PsMsg::RINGER_GET_INFO:
      {// do nothing for now
         pMsg->setCmd(TaoMessage::COMPONENT_RESULT);

         ((TaoPhoneComponentAdaptor*) msgSrc)->postMessage(*pMsg);
      }
      break;
   case PsMsg::RINGER_GET_PATTERN:
      {// do nothing for now
         pMsg->setCmd(TaoMessage::COMPONENT_RESULT);

         ((TaoPhoneComponentAdaptor*) msgSrc)->postMessage(*pMsg);
      }
      break;
   case PsMsg::RINGER_GET_VOLUME:
   case PsMsg::SPEAKER_GET_VOLUME:
      {
         int volume = getVolume();
         char buf[MAXIMUM_INTEGER_STRING_LENGTH];
         sprintf(buf, "%d", volume);

         pMsg->setCmd(TaoMessage::COMPONENT_RESULT);
         pMsg->setArgList(buf);

         ((TaoPhoneComponentAdaptor*) msgSrc)->postMessage(*pMsg);
      }
      break;
   case PsMsg::RINGER_GET_MAX_PATTERN_INDEX:
      {// do nothing for now
         pMsg->setCmd(TaoMessage::COMPONENT_RESULT);
         ((TaoPhoneComponentAdaptor*) msgSrc)->postMessage(*pMsg);
      }
      break;
   case PsMsg::RINGER_GET_NUMBER_OF_RINGS:
      {// do nothing for now
         pMsg->setCmd(TaoMessage::COMPONENT_RESULT);

         ((TaoPhoneComponentAdaptor*) msgSrc)->postMessage(*pMsg);
      }
      break;
   case PsMsg::RINGER_IS_ON:
/* ----------------------------- PHONESPEAKER --------------------------------- */
      {// do nothing for now
         pMsg->setCmd(TaoMessage::COMPONENT_RESULT);

         ((TaoPhoneComponentAdaptor*) msgSrc)->postMessage(*pMsg);
      }
      break;
   case PsMsg::LAMP_SET_MODE:
      {
         UtlString buttonInfo;
         ((PsMsg) rMsg).getStringParam1(buttonInfo);

         mpLampTask->setMode(buttonInfo.data(), (PsLampInfo::LampMode) param1);
      }
      break;
   default:
      assert(FALSE);
      processed = FALSE;            // unexpected message
      break;
   }

   return processed;
}

// getComponent just checks if the required component is in the
// list ps supports, if supported, it returns the type back.
UtlBoolean PsPhoneTask::getComponent(PsMsg& rMsg)
{
        UtlString arg;
        rMsg.getStringParam1(arg);
        int type;

        if (!arg.compareTo("button", UtlString::ignoreCase))
                type = PtComponent::BUTTON;
        else if (!arg.compareTo("display", UtlString::ignoreCase))
                type = PtComponent::DISPLAY;
        else if(!arg.compareTo("graphic_display", UtlString::ignoreCase))
                type = PtComponent::GRAPHIC_DISPLAY;
        else if(!arg.compareTo("hookswitch", UtlString::ignoreCase))
                type = PtComponent::HOOKSWITCH;
        else if(!arg.compareTo("lamp", UtlString::ignoreCase))
                type = PtComponent::LAMP;
        else if(!arg.compareTo("microphone", UtlString::ignoreCase))
                type = PtComponent::MICROPHONE;
        else if(!arg.compareTo("ringer", UtlString::ignoreCase))
                type =  PtComponent::RINGER;
        else if(!arg.compareTo("speaker", UtlString::ignoreCase))
                type = PtComponent::SPEAKER;
        else if(!arg.compareTo("text_display", UtlString::ignoreCase))
                type = PtComponent::TEXT_DISPLAY;
        else
                type = PtComponent::UNKNOWN;

        char buf[MAXIMUM_INTEGER_STRING_LENGTH];
        sprintf(buf, "%d", type);

        TaoMessage *pMsg = (TaoMessage *) &rMsg;
        pMsg->setArgCnt(2);
        arg = UtlString("1") + TAOMESSAGE_DELIMITER + UtlString(buf);
        pMsg->setArgList(arg);

        return TRUE;
}

UtlBoolean PsPhoneTask::getComponents(PsMsg& rMsg)
{
        UtlString arg;
        char buf[MAXIMUM_INTEGER_STRING_LENGTH];
        int platformType = OsUtil::getPlatformType();
        int cnt;

        sprintf(buf, "%d", (int)PsTaoComponent::DISPLAY);
        arg = UtlString(buf) + TAOMESSAGE_DELIMITER;
        sprintf(buf, "%d", (int)PsTaoComponent::GRAPHIC_DISPLAY);
        arg += UtlString(buf) + TAOMESSAGE_DELIMITER;
        sprintf(buf, "%d", (int)PsTaoComponent::HOOKSWITCH);
        arg += UtlString(buf) + TAOMESSAGE_DELIMITER;
        sprintf(buf, "%d", (int)PsTaoComponent::LAMP);
        arg += UtlString(buf) + TAOMESSAGE_DELIMITER;
        sprintf(buf, "%d", (int)PsTaoComponent::MICROPHONE);
        arg += UtlString(buf) + TAOMESSAGE_DELIMITER;
        sprintf(buf, "%d", (int)PsTaoComponent::RINGER);
        arg += UtlString(buf) + TAOMESSAGE_DELIMITER;
        sprintf(buf, "%d", (int)PsTaoComponent::SPEAKER);
        arg += UtlString(buf) + TAOMESSAGE_DELIMITER;
        sprintf(buf, "%d", (int)PsTaoComponent::BUTTON);
        arg += UtlString(buf);
        if ((platformType == OsUtil::PLATFORM_TCAS5) ||
            (platformType == OsUtil::PLATFORM_TCAS6) ||
            (platformType == OsUtil::PLATFORM_TCAS7))
        {
                sprintf(buf, "%d", (int)PsTaoComponent::EXTERNAL_SPEAKER);
                arg += TAOMESSAGE_DELIMITER + UtlString(buf);
                cnt = 8;
        }
        else
                cnt = 7;

        int numButtons = mpButtonTask->getMaxButtonIndex() + 1;

        PsButtonInfo btnInfo;
        char *name;
        for (int i = 0; i < numButtons; i++)
        {
                btnInfo = mpButtonTask->getButtonInfo(i);
                name = btnInfo.getName();
                arg += TAOMESSAGE_DELIMITER + UtlString(name);
        }

        TaoMessage *pMsg = (TaoMessage *) &rMsg;
        pMsg->setArgCnt(numButtons + cnt);
        pMsg->setArgList(arg);

    return TRUE;
}

UtlBoolean PsPhoneTask::numComponents(PsMsg& rMsg)
{
        int cnt = mpButtonTask->getMaxButtonIndex();
        int platformType = OsUtil::getPlatformType();

        if ((platformType == OsUtil::PLATFORM_TCAS5) ||
            (platformType == OsUtil::PLATFORM_TCAS6) ||
            (platformType == OsUtil::PLATFORM_TCAS7))
                cnt += 9;
        else
                cnt += 8;

        UtlString arg;
        char buf[MAXIMUM_INTEGER_STRING_LENGTH];

        sprintf(buf, "%d", cnt);
        arg = UtlString(buf);

        TaoMessage *pMsg = (TaoMessage *) &rMsg;
        pMsg->setArgCnt(1);
        pMsg->setArgList(arg);

    return TRUE;
}


UtlBoolean PsPhoneTask::getComponentGroups(PsMsg& rMsg)
{
        UtlString arg;
        char buf[MAXIMUM_INTEGER_STRING_LENGTH];
        int platformType = OsUtil::getPlatformType();
        int cnt;

        sprintf(buf, "%d", (int)PsTaoComponentGroup::HEAD_SET);
        arg = UtlString(buf) + TAOMESSAGE_DELIMITER;
        sprintf(buf, "%d", (int)PsTaoComponentGroup::HAND_SET);
        arg += UtlString(buf) + TAOMESSAGE_DELIMITER;
        sprintf(buf, "%d", (int)PsTaoComponentGroup::SPEAKER_PHONE);
        arg += UtlString(buf) + TAOMESSAGE_DELIMITER;
        sprintf(buf, "%d", (int)PsTaoComponentGroup::PHONE_SET);
        arg += UtlString(buf);

        if ((platformType == OsUtil::PLATFORM_TCAS5) ||
            (platformType == OsUtil::PLATFORM_TCAS6) ||
            (platformType == OsUtil::PLATFORM_TCAS7))
        {
                sprintf(buf, "%d", (int)PsTaoComponentGroup::EXTERNAL_SPEAKER);
                arg += TAOMESSAGE_DELIMITER + UtlString(buf);
                cnt = 5;
        }
        else
                cnt = 4;


        TaoMessage *pMsg = (TaoMessage *) &rMsg;
        sprintf(buf, "%d", cnt);
        UtlString argList = UtlString(buf) + TAOMESSAGE_DELIMITER + arg;
        pMsg->setArgCnt(cnt + 1);
        pMsg->setArgList(argList);

    return TRUE;
}

int PsPhoneTask::activateGroup(PsMsg& rMsg)
{
        TaoMessage *pMsg = (TaoMessage *) &rMsg;

        int type = atoi(pMsg->getArgList());
        return activateGroup(type);
}

int PsPhoneTask::activateGroup(int type)
{
        UtlBoolean rc = FALSE;
        int level = 0;

        switch(type)
        {
        case PtComponentGroup::HEAD_SET:
                if (mpHeadSetGroup)
                {
                        rc = mpHeadSetGroup->activate();
                        speakerModeEnable(HEADSET_ENABLED);
                        mpHeadSetGroup->getVolume(PsTaoComponentGroup::HEAD_SET, level);
                        setVolume(level);
                        mpHeadSetGroup->getMicGainValue(PsTaoComponentGroup::HEAD_SET, level);
                        setGainValue(level);
                }
                break;
        case PtComponentGroup::HAND_SET:
                if (mpHandSetGroup)
                {
                        rc = mpHandSetGroup->activate();
                        speakerModeEnable(HANDSET_ENABLED);
                        mpHandSetGroup->getVolume(PsTaoComponentGroup::HAND_SET, level);
                        setVolume(level);
                        mpHandSetGroup->getMicGainValue(PsTaoComponentGroup::HAND_SET, level);
                        setGainValue(level);
                }
                break;
        case PtComponentGroup::SPEAKER_PHONE:
                if (mpSpeakerPhoneGroup)
                {
                        rc = mpSpeakerPhoneGroup->activate();
                        speakerModeEnable(SPEAKERPHONE_ENABLED);
                        mpSpeakerPhoneGroup->getVolume(PsTaoComponentGroup::SPEAKER_PHONE, level);
                        setVolume(level);
                        mpSpeakerPhoneGroup->getMicGainValue(PsTaoComponentGroup::SPEAKER_PHONE, level);
                        setGainValue(level);
                }
                break;
        case PtComponentGroup::EXTERNAL_SPEAKER:
                if (mpExtSpeakerGroup)
                {
                        rc = mpExtSpeakerGroup->activate();
                        speakerModeEnable(EXTSPEAKER_ENABLED);
                        mpExtSpeakerGroup->getVolume(PsTaoComponentGroup::EXTERNAL_SPEAKER, level);
                        setVolume(level);
                        mpExtSpeakerGroup->getMicGainValue(PsTaoComponentGroup::EXTERNAL_SPEAKER, level);
                        setGainValue(level);
                }
                break;
        case PtComponentGroup::RINGER:
        case PtComponentGroup::PHONE_SET:
        if (mpPhoneSetGroup)
        {
            assert(false);
        }
        break;
        case PtComponentGroup::SOUND:
                if (mpSpeakerPhoneGroup)
                {
                   int low;
                   int high;
                   int nominal;     // low <= nominal <= high
                   int stepsize;    // in .1 dB
                   int mute;

                  assert(false);
                }
                break;
        case PtComponentGroup::OTHER:
        default:
                break;
        }

        return 1;
}

void PsPhoneTask::setGainValue(int value)
{
#ifdef _VXWORKS
   MpCodec_setGain(value);
   osPrintf("PsPhoneTask::setGain - MpCodec_setGain(%d)\n", value);
#endif
}


UtlBoolean PsPhoneTask::deactivateGroup(PsMsg& rMsg)
{
        TaoMessage *pMsg = (TaoMessage *) &rMsg;

        int type = atoi(pMsg->getArgList());
        return deactivateGroup(type);
}


UtlBoolean PsPhoneTask::deactivateGroup(int type)
{
        UtlBoolean rc = FALSE;
        switch(type)
        {
        case PtComponentGroup::HEAD_SET:
           if (mpHeadSetGroup)
           {
                        rc = mpHeadSetGroup->deactivate();
                        speakerModeDisable(HEADSET_ENABLED);
           }
           break;
        case PtComponentGroup::HAND_SET:
           if (mpHandSetGroup)
           {
                        rc = mpHandSetGroup->deactivate();
                        speakerModeDisable(HANDSET_ENABLED);
           }
           break;
        case PtComponentGroup::SPEAKER_PHONE:
           if (mpSpeakerPhoneGroup)
           {
                        rc = mpSpeakerPhoneGroup->deactivate();
                        speakerModeDisable(SPEAKERPHONE_ENABLED);
           }
           break;
        case PtComponentGroup::EXTERNAL_SPEAKER:
           if (mpExtSpeakerGroup)
           {
                        rc = mpExtSpeakerGroup->deactivate();
                        speakerModeDisable(EXTSPEAKER_ENABLED);
           }
           break;
        case PtComponentGroup::PHONE_SET:
        case PtComponentGroup::RINGER:
            if (mpPhoneSetGroup)
            {
                assert(false);
                rc = mpPhoneSetGroup->deactivate();
                speakerModeDisable(RINGER_ENABLED | SOUND_ENABLED);
            }
            break;
        case PtComponentGroup::SOUND:
           if (mpSpeakerPhoneGroup)
           {
                        rc = mpSpeakerPhoneGroup->deactivate();
                        speakerModeDisable(SOUND_ENABLED);
           }
           break;
        case PtComponentGroup::OTHER:
        default:
                break;
        }

        return TRUE;
}

void PsPhoneTask::postListenerMessage(const PsMsg& rMsg)
{
        int cnt = mpListenerCnt->getRef();

        for (int i = 0; i < cnt; i++)
        {
                ((OsServerTask*) mpActiveListeners[i])->postMessage(rMsg);
        }
}

// Initialize the platform-specific settings for the keyboard buttons
void PsPhoneTask::initPlatformButtonSettings(PsButtonTask* pButtonTask)
{
   int btnEvtMask;
   int btnIndex;
   int numButtons;
   int platformType;

   // $$$ (rschaaf)
   // The button information should be maintained in a dictionary so that
   // we can access the information more efficiently

   platformType = OsUtil::getPlatformType();
   switch (platformType)
   {
   case OsUtil::PLATFORM_WIN32:
   case OsUtil::PLATFORM_LINUX:
   case OsUtil::PLATFORM_SOLARIS:
      numButtons = 35; // DWW changed it for all keys on a keyboard....
                            // really, this should be dynamically allocated.
      break;
   case OsUtil::PLATFORM_BRUTUS:
   case OsUtil::PLATFORM_TCAS1:
      numButtons = 16;
      break;
   case OsUtil::PLATFORM_TCAS2:
      numButtons = 58;
      break;
   case OsUtil::PLATFORM_TCAS3:
   case OsUtil::PLATFORM_TCAS4:      // Note: On the TCAS4,
   case OsUtil::PLATFORM_TCAS5:      //          TCAS5,
   case OsUtil::PLATFORM_TCAS6:      //          TCAS6 and
   case OsUtil::PLATFORM_TCAS7:      //          TCAS7 we turn scroll wheel
      numButtons = 35;               //    events into SCROLL_UP/DOWN button
      break;                         //    presses.
   default:
      assert(FALSE);
      break;
   }

   pButtonTask->init(numButtons-1);  // set max button index to numButtons-1

   btnIndex = 0;

   btnEvtMask = PsButtonInfo::BUTTON_DOWN | PsButtonInfo::BUTTON_UP;

   // The following buttons are on all of different versions of the phone
   pButtonTask->setButtonInfo(btnIndex++, DIAL_0,     "0",     btnEvtMask);
   pButtonTask->setButtonInfo(btnIndex++, DIAL_1,     "1",     btnEvtMask);
   pButtonTask->setButtonInfo(btnIndex++, DIAL_2,     "2",     btnEvtMask);
   pButtonTask->setButtonInfo(btnIndex++, DIAL_3,     "3",     btnEvtMask);
   pButtonTask->setButtonInfo(btnIndex++, DIAL_4,     "4",     btnEvtMask);
   pButtonTask->setButtonInfo(btnIndex++, DIAL_5,     "5",     btnEvtMask);
   pButtonTask->setButtonInfo(btnIndex++, DIAL_6,     "6",     btnEvtMask);
   pButtonTask->setButtonInfo(btnIndex++, DIAL_7,     "7",     btnEvtMask);
   pButtonTask->setButtonInfo(btnIndex++, DIAL_8,     "8",     btnEvtMask);
   pButtonTask->setButtonInfo(btnIndex++, DIAL_9,     "9",     btnEvtMask);
   pButtonTask->setButtonInfo(btnIndex++, DIAL_POUND, "POUND", btnEvtMask);
   pButtonTask->setButtonInfo(btnIndex++, DIAL_STAR,  "STAR",  btnEvtMask);


  // The following buttons are on the Brutus and TCAS1
   if ((platformType == OsUtil::PLATFORM_BRUTUS) ||
       (platformType == OsUtil::PLATFORM_TCAS1))

   {
      pButtonTask->setButtonInfo(btnIndex++, FKEY_VOL_UP,      "VOL_UP",      btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_VOL_DOWN,    "VOL_DOWN",    btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SCROLL_UP,   "SCROLL_UP",   btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SCROLL_DOWN, "SCROLL_DOWN", btnEvtMask);
   }

   // The following buttons are on the TCAS2..TCAS7
   if ((platformType == OsUtil::PLATFORM_TCAS2) ||
       (platformType == OsUtil::PLATFORM_TCAS3) ||
       (platformType == OsUtil::PLATFORM_TCAS4) ||
       (platformType == OsUtil::PLATFORM_TCAS5) ||
       (platformType == OsUtil::PLATFORM_TCAS6) ||
       (platformType == OsUtil::PLATFORM_TCAS7) ||
       (platformType == OsUtil::PLATFORM_WIN32) ||
       (platformType == OsUtil::PLATFORM_LINUX) ||
       (platformType == OsUtil::PLATFORM_SOLARIS))
   {
      // Note: Scroll wheel events on the TCAS4 and higher are translated into
      //       SCROLL_UP and SCROLL_DOWN button events
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SCROLL_UP,  "SCROLL_UP",  btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SCROLL_DOWN,"SCROLL_DOWN",btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_VOL_UP,     "VOL_UP",     btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_VOL_DOWN,   "VOL_DOWN",   btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_HOLD,       "HOLD",       btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_CONFERENCE, "CONFERENCE", btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_TRANSFER,   "TRANSFER",   btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_MUTE,       "MUTE",       btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SPEAKER,    "SPEAKER",    btnEvtMask);
   }

   // The following additional buttons are on the TCAS2
   if (platformType == OsUtil::PLATFORM_TCAS2)
   {
      pButtonTask->setButtonInfo(btnIndex++, FKEY_REDIAL,         "REDIAL",                     btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_MENU,           "MENU",                               btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SKEY1,          "SKEY1",                              btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SKEY2,          "SKEY2",                              btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SKEY3,          "SKEY3",                    btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_PICKUP,         "PICKUP",         btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SPEED_DIAL,     "SPEED_DIAL",     btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_CANCEL,         "CANCEL",         btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_ACCEPT,         "ACCEPT",         btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_VOICE_MAIL,     "VOICE_MAIL",     btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_PRIVACY,        "PRIVACY",        btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_FORWARD,        "FORWARD",        btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_DO_NOT_DISTURB, "DO_NOT_DISTURB", btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_CALL1,          "CALL1",          btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_CALL2,          "CALL2",          btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_CALL3,          "CALL3",          btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_CALL4,          "CALL4",          btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_CALL5,          "CALL5",          btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_CALL6,          "CALL6",          btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER1,          "USER1",          btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER2,          "USER2",          btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER3,          "USER3",          btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER4,          "USER4",          btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER5,          "USER5",          btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER6,          "USER6",          btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER7,          "USER7",          btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER8,          "USER8",          btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER9,          "USER9",          btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER10,         "USER10",         btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER11,         "USER11",         btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER12,         "USER12",         btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER13,         "USER13",         btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER14,         "USER14",         btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER15,         "USER15",         btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER16,         "USER16",         btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER17,         "USER17",         btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_USER18,         "USER18",         btnEvtMask);
   }

   // The following additional buttons are on the TCAS3..TCAS7
   if ((platformType == OsUtil::PLATFORM_TCAS3) ||
       (platformType == OsUtil::PLATFORM_TCAS4) ||
       (platformType == OsUtil::PLATFORM_TCAS5) ||
       (platformType == OsUtil::PLATFORM_TCAS6) ||
       (platformType == OsUtil::PLATFORM_TCAS7) ||
       (platformType == OsUtil::PLATFORM_WIN32) ||
       (platformType == OsUtil::PLATFORM_LINUX) ||
       (platformType == OsUtil::PLATFORM_SOLARIS))
   {
      pButtonTask->setButtonInfo(btnIndex++, FKEY_HEADSET, "HEADSET", btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_MORE,    "MORE",    btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SKEY_L1, "SKEYL1",  btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SKEY_L2, "SKEYL2",  btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SKEY_L3, "SKEYL3",  btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SKEY_L4, "SKEYL4",  btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SKEY_R1, "SKEYR1",  btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SKEY_R2, "SKEYR2",  btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SKEY_R3, "SKEYR3",  btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SKEY_R4, "SKEYR4",  btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SKEY_B1, "SKEYB1",  btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SKEY_B2, "SKEYB2",  btnEvtMask);
      pButtonTask->setButtonInfo(btnIndex++, FKEY_SKEY_B3, "SKEYB3",  btnEvtMask);
          pButtonTask->setButtonInfo(btnIndex++, FKEY_VOICE_MAIL, "VOICE_MAIL", btnEvtMask);
   }
}

void PsPhoneTask::initComponentGroups()
{
   osPrintf("          About to create component groups!\n");
   PsTaoComponent* pComponents[6];
   int nItems = 5;

   // Handset group
   mpTaoButton = new PsTaoButton("Button", PsTaoComponent::BUTTON);

   mpTaoHooksw = new PsTaoHookswitch("Hookswitch",
                                     PsTaoComponent::HOOKSWITCH);

   mpTaoLamp = new PsTaoLamp("Lamp", PsTaoComponent::LAMP);

   mpTaoHandsetSpeaker = new PsTaoSpeaker("Handset Speaker",
                                          PsTaoComponent::SPEAKER);

   mpTaoExtSpeaker = new PsTaoSpeaker("External Speaker",
                                          PsTaoComponent::SPEAKER);

   mpTaoHandsetMic = new PsTaoMicrophone("Handset Mic",
                                         PsTaoComponent::MICROPHONE);

   pComponents[0] = mpTaoButton;
   pComponents[1] = mpTaoLamp;
   pComponents[2] = mpTaoHandsetSpeaker;
   pComponents[3] = mpTaoHandsetMic;
   pComponents[4] = mpTaoHooksw;

   mpHandSetGroup = new PsTaoComponentGroup(PsTaoComponentGroup::HAND_SET,
                                            "Handset Group",
                                            pComponents,
                                            nItems);

   // Speakerphone group
   if(!mpTaoBaseSpeaker)
      mpTaoBaseSpeaker = new PsTaoSpeaker("Base Speaker",
                                          PsTaoComponent::SPEAKER);

   if(!mpTaoBaseMic)
      mpTaoBaseMic = new PsTaoMicrophone("Base Mic",
                                         PsTaoComponent::MICROPHONE);

   nItems = 4;
   pComponents[2] = mpTaoBaseSpeaker;
   pComponents[3] = mpTaoBaseMic;

   mpSpeakerPhoneGroup = new PsTaoComponentGroup(PsTaoComponentGroup::SPEAKER_PHONE,
                                                 "Speakerphone Group",
                                                 pComponents,
                                                 nItems);

   if(!mpTaoRinger)
      mpTaoRinger = new PsTaoRinger("Ringer", PsTaoComponent::RINGER);

   nItems = 5;
   pComponents[4] = mpTaoRinger;

   mpPhoneSetGroup = new PsTaoComponentGroup(PsTaoComponentGroup::PHONE_SET,
                                             "Phoneset Group",
                                             pComponents,
                                             nItems);

   // Headset group
   if(!mpTaoHeadsetSpeaker)
      mpTaoHeadsetSpeaker = new PsTaoSpeaker("Headset Speaker",
                                             PsTaoComponent::SPEAKER);

   if(!mpTaoHeadsetMic)
      mpTaoHeadsetMic = new PsTaoMicrophone("Headset Mic",
                                            PsTaoComponent::MICROPHONE);

   nItems = 2;
   pComponents[0] = mpTaoHeadsetSpeaker;
   pComponents[1] = mpTaoHeadsetMic;

   mpHeadSetGroup = new PsTaoComponentGroup(PsTaoComponentGroup::HEAD_SET,
                                            "Headset Group",
                                            pComponents,
                                            nItems);
   nItems = 1;
   pComponents[0] = mpTaoExtSpeaker;

   mpExtSpeakerGroup = new PsTaoComponentGroup(PsTaoComponentGroup::EXTERNAL_SPEAKER,
                                            "ExtSpeaker Group",
                                            pComponents,
                                            nItems);
#ifdef _VXWORKS
   int low;
   int high;
   int nominal;     // low <= nominal <= high
   int stepsize;    // in .1 dB
   int splash;
   int mute;

   MpCodec_getVolumeRange(low, high, nominal, stepsize, mute, splash, CODEC_ENABLE_HANDSET_SPKR);
   mpHandSetGroup->setVolumeRange(low, high, nominal, stepsize, mute);

   MpCodec_getGainRange(low, high, nominal, stepsize, mute, CODEC_ENABLE_HANDSET_MIC);
   mpHandSetGroup->setGainRange(low, high, nominal, stepsize, mute);

   MpCodec_getVolumeRange(low, high, nominal, stepsize, mute, splash, CODEC_ENABLE_HEADSET_SPKR);
   mpHeadSetGroup->setVolumeRange(low, high, nominal, stepsize, mute);

   MpCodec_getGainRange(low, high, nominal, stepsize, mute, CODEC_ENABLE_HEADSET_MIC);
   mpHeadSetGroup->setGainRange(low, high, nominal, stepsize, mute);

   MpCodec_getVolumeRange(low, high, nominal, stepsize, mute, splash, CODEC_ENABLE_BASE_SPKR);
   mpSpeakerPhoneGroup->setVolumeRange(low, high, nominal, stepsize, mute);
   mNominal = nominal;
   mSplash = splash;
        printf("~~~ initComponentGroups: splash %d\n", mSplash);

   MpCodec_getGainRange(low, high, nominal, stepsize, mute, CODEC_ENABLE_BASE_MIC);
   mpSpeakerPhoneGroup->setGainRange(low, high, nominal, stepsize, mute);
   mpPhoneSetGroup->setGainRange(low, high, nominal, stepsize, mute);

   MpCodec_getVolumeRange(low, high, nominal, stepsize, mute, splash, CODEC_ENABLE_RINGER_SPKR);
   mpPhoneSetGroup->setVolumeRange(low, high, nominal, stepsize, mute);
#else /* Not running on the actual phone */
   mpHandSetGroup->setVolumeRange(0, 100, 50, 10, 0);
   mpHeadSetGroup->setVolumeRange(0, 100, 50, 10, 0);
   mpSpeakerPhoneGroup->setVolumeRange(0, 100, 50, 10, 0);
   mpPhoneSetGroup->setVolumeRange(0, 100, 50, 10, 0);
   mpHandSetGroup->setGainRange(0, 10, 5, 1, 0);
   mpHeadSetGroup->setGainRange(0, 10, 5, 1, 0);
   mpSpeakerPhoneGroup->setGainRange(0, 100, 50, 10, 0);
   mpPhoneSetGroup->setGainRange(0, 100, 50, 10, 0);
#endif
}

/* ============================ FUNCTIONS ================================= */

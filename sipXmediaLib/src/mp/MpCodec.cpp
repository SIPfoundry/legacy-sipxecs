//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include "mp/MpCodec.h"

#define DEF_HSVOLMAX 54
#define DEF_HSVOLSTEP 2
static int hsVolMax = DEF_HSVOLMAX;
static int hsVolStep = DEF_HSVOLSTEP;

// TODO
// The rest of this file is really two independent platform-specific
// implementations of the MpCodec module.  These should be broken out into
// separate files.

#if defined(_WIN32) /* [ */

#include <stdio.h>
#include <string.h>
#include <assert.h>


#include "os/OsUtil.h"
#include "mp/dmaTask.h"
#include "mp/MpCodec.h"

extern "C" void setHandsetMuteState(UtlBoolean state);
static int s_iGainLevel = 5 ;


OsStatus MpCodec_setGain(int level)
{
   s_iGainLevel = level ;
   OsStatus ret = OS_UNSPECIFIED;

   // Open the mixer device
   HMIXER hmx;
   mixerOpen(&hmx, 0, 0, 0, 0);

   // Get the line info for the wave in destination line
   MIXERLINE mxl;
   mxl.cbStruct = sizeof(mxl);
   mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
   mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE);

   // Now find the microphone source line connected to this wave in
   // destination
   DWORD cConnections = mxl.cConnections;
   for(DWORD j=0; j<cConnections; j++)
   {
      mxl.dwSource = j;
      mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_SOURCE);
      if (MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE == mxl.dwComponentType)
         break;
   }

   // Find a volume control, if any, of the microphone line
   LPMIXERCONTROL pmxctrl = (LPMIXERCONTROL)malloc(sizeof MIXERCONTROL);
   MIXERLINECONTROLS mxlctrl = {sizeof mxlctrl, mxl.dwLineID, MIXERCONTROL_CONTROLTYPE_VOLUME, 1, sizeof MIXERCONTROL, pmxctrl};

   if(!mixerGetLineControls((HMIXEROBJ) hmx, &mxlctrl, MIXER_GETLINECONTROLSF_ONEBYTYPE))
   {
      // Found!
      DWORD cChannels = mxl.cChannels;
      if (MIXERCONTROL_CONTROLF_UNIFORM & pmxctrl->fdwControl)
         cChannels = 1;

      LPMIXERCONTROLDETAILS_UNSIGNED pUnsigned =
      (LPMIXERCONTROLDETAILS_UNSIGNED)  malloc(cChannels * sizeof MIXERCONTROLDETAILS_UNSIGNED);

      MIXERCONTROLDETAILS mxcd = {sizeof(mxcd), pmxctrl->dwControlID, cChannels, (HWND)0,
         sizeof MIXERCONTROLDETAILS_UNSIGNED, (LPVOID) pUnsigned};
      mixerGetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

      // Set the volume to the the level  (for both channels as needed)
      pUnsigned[0].dwValue = pUnsigned[cChannels - 1].dwValue = (DWORD)((float)pmxctrl->Bounds.dwMinimum+(float)pmxctrl->Bounds.dwMaximum) * ( (float)((float)level-1.0f) / 9.0f )  ;
      mixerSetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

      free(pmxctrl);
      free(pUnsigned);
   }
   else
   {
      free(pmxctrl);
   }
   mixerClose(hmx);

   DmaTask::setMuteEnabled(s_iGainLevel <= 1); // if Mic level is 1 or less, mute
   ret = OS_SUCCESS;

   return ret;
}


int MpCodec_getGain()
{
   return s_iGainLevel;
}


extern HWAVEOUT audioOutH;
extern HWAVEOUT audioOutCallH;

//setVolume for Win32
OsStatus MpCodec_setVolume(int level)
{
   OsStatus ret = OS_INVALID;

   //scale to our max value
   unsigned short volSeg = 0xFFFF/100;
   unsigned short newLeftVal = level * volSeg;
   unsigned short newRightVal = newLeftVal;

   //now set this to the left and right speakers
   DWORD bothVolume = (newRightVal << 16) + newLeftVal ;

   HWAVEOUT hOut = NULL ;
   if (DmaTask::isRingerEnabled())
   {
      hOut = audioOutH ;
   }
   else
   {
	  hOut = audioOutCallH ;
   }


   if (hOut != NULL)
   {
      waveOutSetVolume(hOut, bothVolume) ;
      ret = OS_SUCCESS;
   }

   return ret;
}

int MpCodec_getVolume()
{
   DWORD bothVolume;
   int volume;
   unsigned short volSeg = 0xFFFF/100;

   HWAVEOUT hOut = NULL ;
   if (DmaTask::isRingerEnabled())
   {
      hOut = audioOutH ;
   }
   else
   {
	  hOut = audioOutCallH ;
   }


   if (hOut != NULL)
   {
      waveOutGetVolume(audioOutH, &bothVolume) ;

      //mask out one
      unsigned short rightChannel = ((unsigned short) bothVolume & 0xFFFF) ;
      volume = rightChannel /volSeg;
   }

   return volume;
}


/* dummy routine */
int MpCodec_isSpeakerOn() { return 0;}

/* dummy routine */
OsStatus MpCodec_doProcessFrame() { return OS_SUCCESS;}

#elif defined(__pingtel_on_posix__) /* WIN32 ] [ */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "os/OsUtil.h"
#include "mp/dmaTask.h"
#include "mp/MpCodec.h"

/* Variable to save the gain level that has been set.
 * This is only significant in that level 0 means "muted" and level > 0 means
 * "not muted", but our callers expect that MpCodec_getGain() will return the
 * value that was set with MpCodec_setGain().
 * The initial value of 1 is an arbitrary number > 0.
 */
static int s_iGainLevel = 1;

/* FUNCTION IS CALLED ONLY BY SOFTWARE PHONE */
OsStatus MpCodec_setGain(int level)
{
   OsStatus ret = OS_UNSPECIFIED;

   osPrintf("MpCodec_setGain (softphone version) GAIN: %d\n",level);

   DmaTask::setMuteEnabled(level == 0);
   /* Save level so it can be returned by MpCodec_getGain(). */
   s_iGainLevel = level;
   ret = OS_SUCCESS;

   return ret;
}

int MpCodec_getGain()
{
   return s_iGainLevel;
}

OsStatus MpCodec_setVolume(int level)
{
   OsStatus ret = OS_SUCCESS;
   return ret;
}


/* dummy routine */
OsStatus MpCodec_doProcessFrame() { return OS_SUCCESS;}


#endif /* ] */

OsStatus MpCodec_getVolumeRange( int& low, int& high, int& nominal,
            int& stepsize, int& mute, int& splash, MpCodecSpkrChoice mask)
{
    OsStatus ret = OS_INVALID;
    int stepsPerInc = 2;
    int nomStep = 5;
    mute = -1;

    stepsize = 10;
    splash = -1;
    switch (OsUtil::getPlatformType()) {
    case OsUtil::PLATFORM_TCAS1:
    case OsUtil::PLATFORM_TCAS2:
        low = high = nominal = stepsize = mute = 0;
        ret = OS_INVALID;
        break;
    case OsUtil::PLATFORM_TCAS3:
    case OsUtil::PLATFORM_TCAS4:
        if((mask & CODEC_ENABLE_HANDSET_SPKR)||
           (mask & CODEC_ENABLE_HEADSET_SPKR)) {
            high     = 50;
            low      = 20;
            nominal  = 35;
            mute     = -1;
        }
        else if (mask&CODEC_ENABLE_BASE_SPKR) {
            high     = 55;
            low      = 35;
            nominal  = 45;
            splash   = 35;
            mute     = -1;
        }
        else if (mask&CODEC_ENABLE_RINGER_SPKR) {
            high     = 55;
            low      = 35;
            nominal  = 45;
            splash   = 35;
            mute     = 15;
        }
        else {
            high     = 50;
            low      = 20;
            nominal  = 35;
            mute     = -1;
        }
        ret = OS_SUCCESS;
        break;
    case OsUtil::PLATFORM_TCAS5:
    case OsUtil::PLATFORM_WIN32:
        if (mask & CODEC_ENABLE_HANDSET_SPKR) {
            // Revised 6Aug01 for hearing aid compatibility, per Mark G.
            high     = hsVolMax;
            stepsPerInc = hsVolStep;
        }
        else if((mask & CODEC_ENABLE_HEADSET_SPKR)) {
            high     = 40;
        }
        else if((mask&CODEC_ENABLE_BASE_SPKR)) {
            high     = 50;
            splash   = 32;
        }
        else if((mask&CODEC_ENABLE_RINGER_SPKR)) {
            stepsPerInc = 3;
            nomStep  = 5;
            high     = 59;
            splash   = 32;
            mute     = 24;
        }
        else {
            high     = 50;
        }
        nominal  = high - ((10 - nomStep) * stepsPerInc);
        low      = high - 10 * stepsPerInc;
        ret = OS_SUCCESS;
        break;

    case OsUtil::PLATFORM_TCAS6:
        if (mask & CODEC_ENABLE_HANDSET_SPKR) {
            // Revised 6Aug01 for hearing aid compatibility, per Mark G.
            high     = hsVolMax - 3; // -3dB compensate for +6dB codec output boost,
                                     // which is only needed for base speaker.
            stepsPerInc = hsVolStep;
        }
        else if((mask & CODEC_ENABLE_HEADSET_SPKR)) {
            high     = 40;
        }
        else if((mask&CODEC_ENABLE_BASE_SPKR)) {
            high     = 50;
            splash   = 32;
        }
        else if((mask&CODEC_ENABLE_RINGER_SPKR)) {
            stepsPerInc = 3;
            nomStep  = 5;
            high     = 59;
            splash   = 32;
            mute     = 24;
        }
        else {
            high     = 50;
        }
        nominal  = high - ((10 - nomStep) * stepsPerInc);
        low      = high - 10 * stepsPerInc;
        ret = OS_SUCCESS;
        break;
    case OsUtil::PLATFORM_TCAS7:
    default:
        if (mask & CODEC_ENABLE_HANDSET_SPKR) {
            // Revised 6Aug01 for hearing aid compatibility, per Mark G.
            high     = hsVolMax - 3;   // -3dB compensate for +6dB codec output boost,
                                       // which is only needed for base speaker.
            stepsPerInc = hsVolStep;
        }
        else if((mask & CODEC_ENABLE_HEADSET_SPKR)) {
            high     = 40;
        }
        else if((mask&CODEC_ENABLE_BASE_SPKR)) {
            high     = 50;
            splash   = 32;
        }
        else if((mask&CODEC_ENABLE_RINGER_SPKR)) {
            stepsPerInc = 3;
            nomStep  = 5;
            high     = 59;
            splash   = 32;
            mute     = 24;
        }
        else {
            high     = 50;
        }
        nominal  = high - ((10 - nomStep) * stepsPerInc);
        low      = high - 10 * stepsPerInc;
        ret = OS_SUCCESS;
        break;
     }
    if (-1 == splash) splash = nominal;
#ifdef DEBUG_MPCODEC /* [ */
    osPrintf(
        "MpCodec_getVolumeRange for 0x%02X: %d, %d, %d, %d, %d, %d; ret %d\n",
        mask, low, high, nominal, stepsize, mute, splash);
#endif /* DEBUG_MPCODEC ] */
    return ret;
}

//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _INCLUDED_MPCODEC_H /* [ */
#define _INCLUDED_MPCODEC_H

#define TUNING_AUDIO_POP_DELAY
#undef TUNING_AUDIO_POP_DELAY

#include <mp/MpMisc.h>
#include <os/OsStatus.h>
/*************************************************************************/

/* MCP/Codec interface: */

enum MpCodecSpkrChoice {
    CODEC_DISABLE_SPKR=0,
    CODEC_ENABLE_SPKR1=(1<<0),
    CODEC_ENABLE_SPKR2=(1<<1),
    CODEC_ENABLE_SPKR3=(1<<2),
    CODEC_ENABLE_SPKR4=(1<<3),
    CODEC_ENABLE_SPKR5=(1<<4),
    CODEC_ENABLE_SPKR6=(1<<5),
};

#define CODEC_ENABLE_HANDSET_SPKR         CODEC_ENABLE_SPKR1
#define CODEC_ENABLE_BASE_SPKR            CODEC_ENABLE_SPKR2
#define CODEC_ENABLE_HEADSET_SPKR         CODEC_ENABLE_SPKR3
#define CODEC_ENABLE_EXTERNAL_SPKR_MONO   CODEC_ENABLE_SPKR4
#define CODEC_ENABLE_EXTERNAL_SPKR_STEREO CODEC_ENABLE_SPKR5
#define CODEC_ENABLE_EXTERNAL_SPKR    (CODEC_ENABLE_SPKR4 | CODEC_ENABLE_SPKR5)
#define CODEC_ENABLE_RINGER_SPKR          CODEC_ENABLE_SPKR6

extern OsStatus MpCodec_setSpeakerMode(MpCodecSpkrChoice mask);

enum MpCodecMicChoice {
    CODEC_DISABLE_MIC=0,
    CODEC_ENABLE_MIC1=(1<<0),
    CODEC_ENABLE_MIC2=(1<<1),
    CODEC_ENABLE_MIC3=(1<<2),
};

#define CODEC_ENABLE_HANDSET_MIC CODEC_ENABLE_MIC1
#define CODEC_ENABLE_BASE_MIC CODEC_ENABLE_MIC2
#define CODEC_ENABLE_HEADSET_MIC CODEC_ENABLE_MIC3

extern OsStatus MpCodec_setMicMode(MpCodecMicChoice mask);
extern MpCodecMicChoice MpCodec_getMicMode(int quiet=1);

#define CODEC_SPKR1_OFF (1<<5)
#define CODEC_SPKR1_ON  (1<<4)
#define CODEC_SPKR2_OFF (1<<3)
#define CODEC_SPKR2_ON  (1<<2)
#define CODEC_SEL_MIC1  (1<<1)
#define CODEC_SEL_MIC2  (1<<0)

#define CODEC_SPKR2_SEL (1<<2)
#define CODEC_SPKR1_SEL (1<<1)
#define CODEC_MIC_SEL (1<<0)
extern void setCodecIO(int options);

#define START_GAIN 25
#define START_VOLUME 60

#define SPEAKER_VOLUME_LEVELS                10

extern OsStatus MpCodecOpen(int sampleRate, int gain, int volume);
extern OsStatus MpCodecEnableOutput(int turnOn);
extern OsStatus MpCodecClose(void);

extern MpCodecSpkrChoice MpCodec_getSpeakerMode(void); /* both speakers */
extern UtlBoolean MpCodec_isBaseSpeakerOn(void); /* the speakerphone speaker */
extern UtlBoolean MpCodec_isHeadsetSpeakerOn(void); /* the headset speaker */
extern UtlBoolean MpCodec_isHandsetSpeakerOn(void); /* the handset speaker */

#ifdef USE_DEV_AUDIO /* [ */
extern int DevAudio_getGain(void);
extern int DevAudio_getVolume(void);
extern OsStatus DevAudio_setGain(int newgain);
extern OsStatus DevAudio_setVolume(int newvolume);
#endif /* USE_DEV_AUDIO ] */

extern OsStatus MpCodec_getVolumeRange( // NEW WAY!
                      int& low,         // lowest value
                      int& high,        // highest value
                      int& nominal,     // low <= nominal <= high
                      int& stepsize,    // in .1 dB
                      int& mute,        // input value to mute
                      int& splash,      // value to use during startup
                      MpCodecSpkrChoice Choice);

extern OsStatus MpCodec_getGainRange(
                      int& low,         // lowest value
                      int& high,        // highest value
                      int& nominal,     // low <= nominal <= high
                      int& stepsize,    // in .1 dB
                      int& mute,        // input value to mute
                      MpCodecMicChoice Choice);

extern OsStatus MpCodec_getSidetoneRange(
                      int& low,         // lowest value
                      int& high,        // highest value
                      int& nominal);    // low <= nominal <= high

extern OsStatus MpCodec_getLCDContrastRange(
                      int& low,         // lowest value
                      int& high,        // highest value
                      int& nominal);    // low <= nominal <= high

extern OsStatus MpCodec_getLCDBrightnessRange(
                      int& low,         // lowest value
                      int& high,        // highest value
                      int& nominal);    // low <= nominal <= high

extern int MpCodec_getGain(void);
extern int MpCodec_getVolume(void);
extern int MpCodec_getSidetone(void);
extern int MpCodec_getLCDBrightness(void);
extern int MpCodec_getLCDContrast(void);
extern OsStatus MpCodec_setGain(int newgain);
extern OsStatus MpCodec_setVolume(int newvolume);
extern OsStatus MpCodec_setSidetone(int level);
extern OsStatus MpCodec_setLCDBrightness(int level);
extern OsStatus MpCodec_setLCDContrast(int level);
extern UtlBoolean MpCodec_isExtSpkrPresent(void);

extern OsStatus MpCodec_doProcessFrame(void);

#endif /* _INCLUDED_MPCODEC_H ] */

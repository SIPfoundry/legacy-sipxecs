//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _INCLUDED_DTMFLIB_H /* [ */
#define _INCLUDED_DTMFLIB_H

#include "os/OsStatus.h"

#define DTMF_TONES_BASE 512

enum toneName {
DTMF_TONE_DIALTONE   = (DTMF_TONES_BASE + 0),
DTMF_TONE_BUSY,
DTMF_TONE_RINGBACK,
DTMF_TONE_RINGTONE,
DTMF_TONE_CALLFAILED,
DTMF_TONE_SILENCE,
DTMF_TONE_BACKSPACE,
DTMF_TONE_CALLWAITING,
DTMF_TONE_CALLHELD,
DTMF_TONE_LOUD_FAST_BUSY
};

typedef enum toneName toneID;

typedef struct __MpToneGen_tag *MpToneGenPtr;

extern void MpToneGen_startTone(MpToneGenPtr p, int toneID);
extern void MpToneGen_stopTone(MpToneGenPtr p);
extern OsStatus MpToneGen_getNextBuff(MpToneGenPtr thisobj, int16_t *b, int N);
extern void MpToneGen_delete(MpToneGenPtr p);
extern MpToneGenPtr MpToneGen_MpToneGen(int samprate, const char* toneLocale=NULL);

#endif /* _INCLUDED_DTMFLIB_H ] */

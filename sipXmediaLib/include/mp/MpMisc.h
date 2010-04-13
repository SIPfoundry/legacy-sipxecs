//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _INCLUDED_MPMISC_H /* [ */
#define _INCLUDED_MPMISC_H

// Forward declarations
class OsConfigDb;

#include "os/OsMsgQ.h"
#include "mp/MpTypes.h"

#define KBHIT
#undef  KBHIT

#ifndef MUTEX_OPS
#define MUTEX_OPS (SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE)
#endif

#define FRAME_SAMPS  320

#ifndef max
#define max(x,y) (((x)>(y))?(x):(y))
#endif

#ifndef min
#define min(x,y) (((x)<(y))?(x):(y))
#endif

/* miscellaneous debugging support: */

#define Zprintf printf
#undef Zprintf
#define Zprintf(fmt, a, b, c, d, e, f)
#define Nprintf(fmt, a, b, c, d, e, f)
#define Lprintf(fmt, a, b, c, d, e, f)

/* mpStartUp initializes the MpMisc struct and other MP data, for */
/*    example, the buffer pools and tables */
extern OsStatus mpStartUp(int sampleRate, int samplesPerFrame,
		  int numAudioBuffers, OsConfigDb* pConfigDb);

/* tears down whatever was created in mpStartUp */
extern OsStatus mpShutdown(void);

/* mpStartTasks spawns the low level MP tasks for DMA and network I/O */
extern OsStatus mpStartTasks(void);

/* mpStopTasks stops the low level MP tasks */
extern OsStatus mpStopTasks(void);

struct __MpGlobals {
        OsMsgQ* pMicQ;
        OsMsgQ* pSpkQ;
        OsMsgQ* pEchoQ;
        int micMuteStatus;
        int spkrMuteStatus;
        int audio_on;
        int frameSamples;
        int frameBytes;
        int sampleBytes;
        int rtpMaxBytes;
        MpBufPoolPtr UcbPool;
        MpBufPoolPtr DMAPool;
        MpBufPoolPtr RtpPool;
        MpBufPoolPtr RtcpPool;
        MpBufPtr XXXsilence;
        MpBufPtr XXXlongSilence;
        MpBufPtr comfortNoise;
        int max_mic_buffers;
        int max_spkr_buffers;
        int min_rtp_packets;
        int micSawTooth;
};

extern struct __MpGlobals MpMisc;

#endif /* _INCLUDED_MPMISC_H ] */

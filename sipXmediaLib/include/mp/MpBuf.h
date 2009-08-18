//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _INCLUDED_MPBUF_H /* [ */
#define _INCLUDED_MPBUF_H

#define BUFFER_INSTRUMENTATION
#undef  BUFFER_INSTRUMENTATION

#ifdef BUFFER_INSTRUMENTATION /* [ */
#ifdef _VXWORKS /* [ */
#include <taskLib.h>
#include <msgQLib.h>
#endif /* _VXWORKS ] */
#endif /* BUFFER_INSTRUMENTATION ] */

#include "mp/MpTypes.h"
#include "mp/MpMisc.h"

/* Access Methods */

#define MpBuf_getStorage(b) ((b)->pStorage)
#define MpBuf_getSamples(b) ((b)->pSamples)
#define MpBuf_getByteLen(b) ((b)->byteLen)
#define MpBuf_getNumSamples(b) ((b)->numSamples)
#define MpBuf_getContentLen(b) ((b)->contentLen)
#define MpBuf_getFormat(b) ((b)->format)
#define MpBuf_setFormat(b, f) ((b)->format = (f))
#define MpBuf_getPool(b) ((b)->pPool)
#define MpBuf_getOffset(b) ((b)->offset)

#define MpBuf_getOsTC(b) ((b)->ostc)
#define MpBuf_setOsTC(b, v) ((b)->ostc = (v))

#define MpBuf_getAtten(b) ((b)->attenDb)
#define MpBuf_setAtten(b, v) ((b)->attenDb = (v))

#define MpBuf_getSpeech(b) ((b)->speech)
#define MpBuf_setSpeech(b, v) ((b)->speech = (v))

enum MpBufFormat {
        MP_FMT_UNKNOWN,        /* as yet unspecified */
        MP_FMT_T12,            /* Raw UCB1200 data, high 12 bits of 16 */
        MP_FMT_RTPPKT,         /* Raw RTP packet */
        MP_FMT_RTCPPKT,        /* Raw RTCP packet */
        MP_FMT_G711M,          /* G711 mu, mu-law */
        MP_FMT_G711A,          /* G711 A, A-law */
        MP_FMT_L16,            /* Linear 16, in "network byte order" */
        MP_FMT_G729,           /* ITU-G729A standard */
};
typedef enum MpBufFormat MpBufFormat;

enum MpBufSpeech {
        MP_SPEECH_UNKNOWN,        /* as yet undetermined */
        MP_SPEECH_SILENT,         /* found to contain no speech */
        MP_SPEECH_COMFORT_NOISE,  /* to be replaced by comfort noise */
        MP_SPEECH_ACTIVE,         /* found to contain speech */
        MP_SPEECH_MUTED,          /* may contain speech, but must be muted */
        MP_SPEECH_TONE,           /* our tonegen filled with active (not silent) tone data */
};
typedef enum MpBufSpeech MpBufSpeech;

/* Buffer management: */

struct __MpBuf_tag {
        int     byteLen;    /* length in bytes; fixed when created */
        int     numSamples; /* actual current use, unit depends on format */
        int     contentLen; /* only for RTP packet, total length of packet */
        char   *pStorage;   /* pointer to beginning of storage; fixed */
        Sample *pSamples;   /* pointer to beginning of data */
        int     offset;     /* offset to first sample, eg. sizeof(RTPheader) */
        short   status;     /* currently, 0 => free */
        short   attenDb;    /* attenuation applied at speakerphone speaker */
        MpBufSpeech speech; /* if we know, whether buffer contains speech */
        MpBufPoolPtr pPool; /* the pool that this buffer belongs to */
        int     refCnt;     /* use count */
        MpBufFormat format; /* the type of the current content */
        int     ostc;       /* OS Timer/Counter (3.6864 MHz) value */
#ifdef BUFFER_INSTRUMENTATION /* [ */
/*
 * These are used to track the usage of each buffer.  Each of the
 * following pairs has a source line number and a time stamp (which
 * is a sequence number incremented in each buffer operation).
 */
        int line_taken; /* who took it last, and when */
        int time_taken;
        int line_freed; /* who freed it last, and when */
        int time_freed;
        int touched_by; /* who touched it last, and when */
        int touched_at; /* set by "voluntary" calls to bufTouch() */
#endif /* BUFFER_INSTRUMENTATION ] */
};

extern OsStatus MpBuf_init(int samplesPerFrame, int numUcbBufs);
extern void MpBuf_close(void);

extern OsStatus MpBuf_setOffset(MpBufPtr b, int offset);
extern OsStatus MpBuf_setSamples(MpBufPtr b, Sample *first);
extern OsStatus MpBuf_setNumSamples(MpBufPtr b, int num);
extern OsStatus MpBuf_setContentLen(MpBufPtr b, int num);

extern int MpBufPool_getNumBufs(MpBufPoolPtr p);
extern int MpBuf_bufNum(MpBufPtr pBuf);

extern MpBufPoolPtr MpBufPool_MpBufPool(
   int poolSize,       // number of bytes to malloc (including headers?)
   int max_buffer_len, // bytes per buffer
   int numBuffers,     // used if poolSize is 0
   int cacheAlignment  // 0=>not aligned, otherwise power of 2 boundary
);

extern STATUS MpBufPool_delete(MpBufPoolPtr pool, int force);

extern void MpBuf_insertSawTooth(MpBufPtr buf);

extern MpBufPtr MpBuf_allowMods(MpBufPtr buf);

extern MpBufPtr MpBuf_getFgSilence(void);
extern MpBufPtr MpBuf_getDmaSilence(void);

extern long MpBuf_setMVE(long newMin);

extern unsigned long MpBuf_getVAD(MpBufPtr buf);
extern MpBufSpeech MpBuf_doVAD(MpBufPtr buf);
extern UtlBoolean MpBuf_isActiveAudio(MpBufPtr buf);
extern UtlBoolean MpBuf_isSilence(MpBufPtr buf);
extern UtlBoolean MpBuf_isPoolSilent(MpBufPtr buf);

extern int MpBuf_getTotalBufferCount(void);

#ifdef BUFFER_INSTRUMENTATION /* [ */

/* prototypes for instrumented versions of routines: */

#define MpBuf_getBuf(Pool, Samples, Offset, Format) \
                 (MpBuf_getBufX(Pool, Samples, Offset, Format, __LINE__))

#define MpBuf_touch(b) (MpBuf_touchX((b), __LINE__))
#define MpBuf_delRef(b) (MpBuf_delRefX((b), __LINE__))
#define MpBuf_addRef(b) (MpBuf_addRefX((b), __LINE__))

extern MpBufPtr MpBuf_getBufX(MpBufPoolPtr pool,
        int InitSamples, int Offset, MpBufFormat Format, int line_no);
extern void MpBuf_touchX(MpBufPtr b, int line_no);
extern void MpBuf_delRefX(MpBufPtr buf, int line);
extern void MpBuf_addRefX(MpBufPtr buf, int line);

#else /* BUFFER_INSTRUMENTATION ] [ */

/* prototypes for production versions of routines: */

#define MpBuf_touch(b)
#define MpBuf_getBuf(Pool, Samples, Offset, Format) \
                        (MpBuf_getBufY(Pool, Samples, Offset, Format))

extern MpBufPtr MpBuf_getBufY(MpBufPoolPtr pool,
                        int InitSamples, int Offset, MpBufFormat Format);
extern void MpBuf_delRef(MpBufPtr buf);
extern void MpBuf_addRef(MpBufPtr buf);

extern MpBufSpeech MpBuf_checkSpeech(MpBufPtr buf);

#endif /* BUFFER_INSTRUMENTATION ] */

/*************************************************************************/

#endif /* _INCLUDED_MPBUF_H ] */

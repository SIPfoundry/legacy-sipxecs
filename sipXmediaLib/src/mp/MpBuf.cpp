//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <limits>

// APPLICATION INCLUDES

#include "os/OsStatus.h"
#include "os/OsMutex.h"
#include "os/OsLock.h"
#include "mp/MpBuf.h"
#include "mp/MpMisc.h"
#include "mp/NetInTask.h"

#undef max

// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES
extern volatile int* pOsTC;

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

#define N_SPARE_BUFS 10
#undef N_SPARE_BUFS

#ifdef N_SPARE_BUFS /* [ */
#define MP_SPEECH_SPARE 0x1234567
static MpBufPtr spareBufs[N_SPARE_BUFS];
int showSpareBufs(int fixIt, char* tag)
{
    MpBufPtr sb;
    Sample* src;
    Sample* dst;
    int i, j, n;
    int ok;
    int OK = 1;
    int fixed;

    if (NULL == tag) tag = "";
    osPrintf("\n");
    for (i=0; i<N_SPARE_BUFS; i++) {
        ok = 1;
        fixed = 0;
        sb = spareBufs[i];
        if (NULL != sb) {
            src = MpBuf_getSamples(sb);
            n = MpBuf_getNumSamples(sb);
            for (j=0; ((j<n) && ok); j++) {
                if (*src++ != ((i<<8) + j)) ok = 0;
            }
            if ((!ok) & fixIt) {
                dst = MpBuf_getSamples(sb);
                n = MpBuf_getNumSamples(sb);
                for (j=0; j<n; j++) {
                    *dst++ = (i<<8) + j;
                }
                fixed = 1;
            }
            src = MpBuf_getSamples(sb);
        } else {
            ok = 0;
            src = NULL;
        }
        OK &= ok;
        osPrintf("%sspare[%d] = 0x%p->0x%p (%sOK)%s\n",
            tag, i, sb, src,
            (ok ? "" : "NOT "),
            (fixed ? " (Fixed)" : ""));
    }
    return OK;
}
#endif /* N_SPARE_BUFS ] */

static uintptr_t LowBufTable = std::numeric_limits<uintptr_t>::max();
static uintptr_t HighBufTable = 0;

#ifdef BUFFER_INSTRUMENTATION /* [ */
// global mutex, used for thread safety when updating sequence counter
static OsMutex* spCounterMutex = NULL;
static int buffer_time_stamp = 0;
#endif /* BUFFER_INSTRUMENTATION ] */

typedef struct __MpBufPool_tag
{
   int      size;        // Amount of memory reserved for the pool (in bytes)
   int      allocCnt;    // Number of allocated buffers and buffer headers
   int      allocSize;   // Amount of memory currently allocated to buffers
                         //  and headers (in bytes)
   int      maxAllocCnt; // Max allocCnt (for the lifetime of the pool)
   int      maxAllocSize;// Max allocSize (for the lifetime of the pool)
   // other fields needed by the allocator
   int      eachLen;     // the number of bytes in each buffer's storage block.
   void    *MpBufPoolData; /* the malloc'ed space for data */
   MpBufPtr table;       // malloc'ed array of buffer headers
   int      lastTaken;   // used by allocator to reduce searching
   int      cacheAlignment;// 0 if not aligned, otherwise boundary size.
   OsMutex* mpMutex;     // mutex semaphore for thread safety
#ifdef BUFFER_INSTRUMENTATION /* [ */
        int      nfree;         /* current number of free buffers in pool */
        int      minfree;       /* minimum number of free buffers in pool */
#endif /* BUFFER_INSTRUMENTATION ] */
} MpBufPool;

typedef struct __MpBufPool_tag MpBufPool;
typedef struct __MpBufPool_tag *MpBufPoolPtr;

/************************************************************************/

int showBufs(MpBufPoolPtr l, int line)
{

        int i, col, num_used = 0;
        int i0, i1, i2, i3, i4, i5;
        const char *fmt;
#ifdef _VXWORKS /* [ */
static  int last_counter = 0;
#endif /* _VXWORKS ] */

        if (0 == l) l = MpMisc.UcbPool;
        i0 = i1 = i2 = i3 = i4 = i5 = 0;
#ifdef _VXWORKS /* [ */
        i = *pOsTC;
        Zprintf("%d:(0x%08X-0x%08X)\n  ", line, i, i - last_counter, 0,0,0);
        last_counter = i;
#else /* _VXWORKS ] */
        Zprintf("%d:\n  ", line, 0,0,0,0,0);
#endif /* _VXWORKS ] */
        col = 0;
        for (i=0; i<l->allocCnt; ) {
            switch(l->allocCnt - i) {
            case 1: fmt = "%d\n";
                i0 = l->table[i++].status;
                break;
            case 2: fmt = "%d%d\n";
                i0 = l->table[i++].status;
                i1 = l->table[i++].status;
                break;
            case 3: fmt = "%d%d%d\n";
                i0 = l->table[i++].status;
                i1 = l->table[i++].status;
                i2 = l->table[i++].status;
                break;
            case 4: fmt = "%d%d%d%d\n";
                i0 = l->table[i++].status;
                i1 = l->table[i++].status;
                i2 = l->table[i++].status;
                i3 = l->table[i++].status;
                break;
            case 5: fmt = "%d%d%d%d%d\n";
                i0 = l->table[i++].status;
                i1 = l->table[i++].status;
                i2 = l->table[i++].status;
                i3 = l->table[i++].status;
                i4 = l->table[i++].status;
                break;
            case 6: fmt = "%d%d%d%d%d%d\n";
                i0 = l->table[i++].status;
                i1 = l->table[i++].status;
                i2 = l->table[i++].status;
                i3 = l->table[i++].status;
                i4 = l->table[i++].status;
                i5 = l->table[i++].status;
                break;
            default: fmt = "%d%d%d%d%d%d";
                i0 = l->table[i++].status;
                i1 = l->table[i++].status;
                i2 = l->table[i++].status;
                i3 = l->table[i++].status;
                i4 = l->table[i++].status;
                i5 = l->table[i++].status;
                break;
            }
            col += Zprintf(fmt, i0, i1, i2, i3, i4, i5) + 0;
            if (col > 70) {
                col = Zprintf("\n          ", 0,0,0,0,0,0) - 1;
            }
        }
        for (i=0; i<l->allocCnt; ) {
                if (l->table[i++].status) num_used++;
        }
        return num_used;
}

void dump1Buf(MpBufPtr t)
{
#ifdef BUFFER_INSTRUMENTATION /* [ */
        MpBufPoolPtr l = t->pPool;
        int i;
        char str[80], state;
        OsLock lock(*(t->pPool->mpMutex));

        i = t - l->table;
        switch (t->status) {
        case  0: state = '-'; break;
        case  1: state = '+'; break;
        default: state = '!'; break;
        }
        sprintf(str, "%3d: %c 0x%p->0x%p %4d(%06d)",
            i, state, t, t->pStorage, t->line_taken, t->time_taken);
        Zprintf("%s %4d(%06d) %4d(%06d)\n", (int) str, t->line_freed,
            t->time_freed, t->touched_by, t->touched_at, 0);
#endif /* BUFFER_INSTRUMENTATION ] */
}

void dump1Pool(MpBufPoolPtr P, int skipFree)
{
        // MpBufPoolPtr l = pool;
#ifdef BUFFER_INSTRUMENTATION /* [ */
        MpBufPtr t;
        int i;
#endif /* BUFFER_INSTRUMENTATION ] */
        OsLock lock(*(P->mpMutex));

        Zprintf(" Buffer table Dump(0x%p): count=%d mutex=0x%p *table=0x%p\n",
            P, P->allocCnt, P->mpMutex, P->table, 0,0);

        Zprintf("    size=%d eachLen=%d cacheAlign=%d last=%d",
            P->size, P->eachLen, P->cacheAlignment, P->lastTaken, 0, 0);

#ifdef BUFFER_INSTRUMENTATION /* [ */
        Zprintf(" nf=%d mf=%d\n", P->nfree, P->minfree, 0,0,0,0);
#else /* BUFFER_INSTRUMENTATION ] [ */
        Zprintf("\n", 0,0,0,0,0,0);
#endif /* BUFFER_INSTRUMENTATION ] */

#ifdef BUFFER_INSTRUMENTATION /* [ */
        t = &(P->table[0]);
        for (i=0; i<P->allocCnt; i++) {
            if (-1 != (t->line_taken)) {
                if ((!skipFree) || (0 != t->status)) dump1Buf(t);
            }
            t++;
        }
#endif /* BUFFER_INSTRUMENTATION ] */
}

void dumpBufs(int skipFree)
{
        if (NULL != MpMisc.UcbPool) {
            dump1Pool(MpMisc.UcbPool, skipFree);
        }

        if (NULL != MpMisc.RtpPool) {
            dump1Pool(MpMisc.RtpPool, skipFree);
        }

        if (NULL != MpMisc.RtcpPool) {
            dump1Pool(MpMisc.RtcpPool, skipFree);
        }

        if (NULL != MpMisc.DMAPool) {
            dump1Pool(MpMisc.DMAPool, skipFree);
        }
}

/* static */ void init_bufs(int num_bufs, int len, MpBufPoolPtr pool, char *b,
        MpBufPtr bufs)
{
        int i;

        for (i=0; i<num_bufs; i++) {
                bufs[i].byteLen = len;
                bufs[i].numSamples = 0;
                bufs[i].contentLen = 0;
                bufs[i].pStorage = b;
                bufs[i].status = 0;
                bufs[i].speech = MP_SPEECH_UNKNOWN;
                bufs[i].pPool = pool;
                bufs[i].refCnt = 0;
                bufs[i].offset = 0;
                bufs[i].pSamples = (Sample *) b;
                bufs[i].ostc = 0;
                bufs[i].attenDb = 0;
#ifdef BUFFER_INSTRUMENTATION /* [ */
                bufs[i].line_taken = -1;
                bufs[i].time_taken = -1;
                bufs[i].line_freed = -1;
                bufs[i].time_freed = -1;
                bufs[i].touched_by = -1;
                bufs[i].touched_at = -1;
#endif /* BUFFER_INSTRUMENTATION ] */
                b += len;
        }
}

int MpBufPool_getNumBufs(MpBufPoolPtr pool)
{
   return pool->allocCnt;
}

STATUS MpBufPool_delete(MpBufPoolPtr pool, int force)
{
        int i;
        int refs;
        MpBufPtr buf;

        if (NULL == pool) {
            return OS_SUCCESS;
        }

        if (NULL == pool->mpMutex) {
            free(pool);
            return OS_SUCCESS;
        }

        pool->mpMutex->acquire();
        refs = 0;
        buf = pool->table;
        if (NULL != buf) {
            for (i=0; i<pool->allocCnt; i++) {
                refs += (0 == buf++->refCnt) ? 0 : 1;
            }
        }
        if (0 != refs) {
            if (0 == force) {
                return OS_BUSY;
            } else {
                Zprintf("MpBufPool_delete(0x%p): %d buffers still in use!\n",
                    pool, refs, 0,0,0,0);
            }
        }

        if (NULL != pool->table) {
            free(pool->table);
        }
        pool->table = NULL;
        if (NULL != pool->MpBufPoolData) {
            free(pool->MpBufPoolData);
        }
        pool->MpBufPoolData = NULL;
        pool->mpMutex->release();

        delete pool->mpMutex;
        free(pool);
        return OS_SUCCESS;
}

#ifdef BUFFER_INSTRUMENTATION /* [ */
static int bufferPoolCreates = 0;
#endif /* BUFFER_INSTRUMENTATION ] */

#define MIN_BUFFER_COUNT 1

MpBufPoolPtr MpBufPool_MpBufPool(int poolSize, int maxBufferLen,
      int numBuffers, int cacheAlign)
{
        intptr_t i;
        int n;
        int totalBytes;
        int numBufs;
        int numBytes;
        int bufBytes;
        char *b;
        char *storage;
        MpBufPoolPtr pool;
        MpBufPtr bufs;

#ifdef BUFFER_INSTRUMENTATION /* [ */
        if (20 > bufferPoolCreates++) {
            Zprintf("MpBufPool_MpBufPool(%d, %d, %d, %d)\n",
                poolSize, maxBufferLen, numBuffers, cacheAlign, 0,0);
        }
#endif /* BUFFER_INSTRUMENTATION ] */

        pool = (MpBufPoolPtr) malloc(sizeof(MpBufPool));
        if (NULL == pool) {
            Zprintf("MpBufPool: unable to malloc %zu bytes!\n",
                sizeof(MpBufPool), 0,0,0,0,0);
            return NULL;
        }
        memset((char *) pool, 0, sizeof(MpBufPool));
        pool->cacheAlignment = cacheAlign;

        pool->mpMutex = new OsMutex(OsMutex::Q_PRIORITY);
        if (NULL == pool->mpMutex) {
            Zprintf("MpBufPool: unable to create Mutex!\n", 0,0,0,0,0,0);
            free(pool);
            return NULL;
        }

        pool->mpMutex->acquire();

        numBufs = poolSize / maxBufferLen;
        if (numBufs < MIN_BUFFER_COUNT) {
            numBufs = numBuffers;
        }
        if (numBufs < MIN_BUFFER_COUNT) {
            Zprintf("MpBufPool: request to create %d buffers: too few (<%d)\n",
                numBufs, MIN_BUFFER_COUNT, 0,0,0,0);
            pool->mpMutex->release();
            delete pool->mpMutex;
            free(pool);
            return NULL;
        }

        totalBytes = numBufs * sizeof(MpBuf);
        bufs = (MpBufPtr) malloc(totalBytes);
        if (NULL == bufs) {
            Zprintf("MpBufPool: unable to malloc %zu bytes!\n",
                numBufs * sizeof(MpBuf), 0,0,0,0,0);
            pool->mpMutex->release();
            delete pool->mpMutex;
            free(pool);
            return NULL;
        }
        memset((char *) bufs, 0, totalBytes);
        pool->table = bufs;

        if (((uintptr_t) bufs) < LowBufTable) {
            LowBufTable = (uintptr_t) bufs;
        }

        if (((uintptr_t) (bufs + numBufs)) > HighBufTable) {
            HighBufTable = (uintptr_t) (bufs + numBufs);
        }

        if (0 == cacheAlign) {
            bufBytes = maxBufferLen;
            numBytes = numBufs * bufBytes;
        } else {

            /*
             * The number of bytes in each buffer must be a
             * multiple of the data cache line size
             */

             /* round up the buffer size, and the total buffer space */
            i = (maxBufferLen + cacheAlign - 1) / cacheAlign;
            bufBytes = cacheAlign * i;
            numBytes = numBufs * bufBytes + cacheAlign - 1;
        }
        storage = (char *) malloc(numBytes);
        if (NULL == storage) {
            Zprintf("MpBufPool: unable to malloc %d bytes!\n",
                numBytes, 0,0,0,0,0);
            free(bufs);
            pool->mpMutex->release();
            delete pool->mpMutex;
            free(pool);
            return NULL;
        }
        pool->MpBufPoolData = storage;
        totalBytes += numBytes;

        pool->eachLen = bufBytes;
        pool->size = sizeof(MpBufPool);
        pool->allocCnt = numBufs;
        pool->allocSize = totalBytes;
        pool->maxAllocCnt = 0;
        pool->maxAllocSize = 0;
        pool->lastTaken = 0;

        if (0 == cacheAlign) {
            b = storage;
        } else {

             /* round up the base of the first buffer */

            i = (uintptr_t) storage;
            n = i % cacheAlign;
            if (0 != n) {
                n = cacheAlign - n;
            }
            b = storage + n;
        }

#ifdef BUFFER_INSTRUMENTATION /* [ */
        if (20 > bufferPoolCreates) {
           Zprintf("buffer data base address = 0x%p (0x%p) (%d)\n",
              b, (pool->MpBufPoolData),
              b - (char *)(pool->MpBufPoolData), 0,0,0);
        }
#endif /* BUFFER_INSTRUMENTATION ] */

        init_bufs(numBufs, bufBytes, pool, b, bufs);

#ifdef BUFFER_INSTRUMENTATION /* [ */
        pool->nfree = numBufs;
        pool->minfree = numBufs;
        if (20 > bufferPoolCreates) {
#ifdef _VXWORKS /* [ */
            int x;
            x = StartLogging();
#endif /* _VXWORKS ] */
            dumpBufs(1);
#ifdef _VXWORKS /* [ */
            if (0 == x) StopLogging(0);
#endif /* _VXWORKS ] */
        }
#endif /* BUFFER_INSTRUMENTATION ] */

        pool->mpMutex->release();
        return pool;
}

void MpBuf_close()
{

#ifdef BUFFER_INSTRUMENTATION /* [ */
        delete spCounterMutex;
        spCounterMutex = NULL;
#ifdef _VXWORKS /* [ */
        StartLogging();
#endif /* _VXWORKS ] */
        dumpBufs(1);
#ifdef _VXWORKS /* [ */
        while (0<msgQNumMsgs(MpMisc.LogQ)) taskDelay(1);
        taskDelay(12);
#endif /* _VXWORKS ] */
#endif /* BUFFER_INSTRUMENTATION ] */

        MpBuf_delRef(MpMisc.XXXsilence);
        MpMisc.XXXsilence = NULL;
        MpBuf_delRef(MpMisc.XXXlongSilence);
        MpMisc.XXXlongSilence = NULL;

        MpBufPool_delete(MpMisc.UcbPool, TRUE);
        MpMisc.UcbPool = NULL;
        MpBufPool_delete(MpMisc.RtpPool, TRUE);
        MpMisc.RtpPool = NULL;
        MpBufPool_delete(MpMisc.RtcpPool, TRUE);
        MpMisc.RtcpPool = NULL;
        MpBufPool_delete(MpMisc.DMAPool, TRUE);
        MpMisc.DMAPool = NULL;
}

#define MpBuf_invalid(b, m, a) (MpBuf_invalidX((b),(m),(a),__LINE__))
static int MpBuf_invalidX(MpBufPtr b, int forMods, int allocated, int line)
{
   int n;
   MpBufPoolPtr p;

   if ((((uintptr_t) b) < LowBufTable) ||
                         (((uintptr_t) b) > HighBufTable)) {
      Zprintf("\nMpBuf_invalid(0x%p): Outside tables (line: %d)\n",
         b, line, 0,0,0,0);
      return TRUE;
   }

   p = b->pPool;
   // $$$ could add a check that p points to one of the active pools...
   n = b - p->table;
   if ((n < 0 ) || (n > p->allocCnt)) {
      Zprintf("\nMpBuf_invalid(0x%p): Outside table (p=0x%p,"
         " n=%d, t=0x%p) line: %d\n",
         b, p, n, (p->table), line, 0);
      return TRUE;
   }

   if (forMods && (1 != b->refCnt)) {
      Zprintf("\nMpBuf_invalid(0x%p): reference count(%d) != 1 (line: %d)\n",
         b, b->refCnt, line, 0,0,0);
      return TRUE;
   }

   if (allocated && (1 > b->refCnt)) {
      Zprintf("\nMpBuf_invalid(0x%p): reference count(%d) < 1 (line: %d)\n",
         b, b->refCnt, line, 0,0,0);
      return TRUE;
   }

   return FALSE;
}

int MpBuf_bufNum(MpBufPtr pBuf)
{
   int n;
   MpBufPoolPtr p;

   if (MpBuf_invalid(pBuf, FALSE, FALSE)) return -1;
   p = pBuf->pPool;
   n = pBuf - p->table;
   if ((n < 0 ) || (n > p->allocCnt)) {
      n = -1;
   }
   return n;
}

OsStatus MpBuf_setNumSamples(MpBufPtr b, int num)
{
   if (MpBuf_invalid(b, TRUE, TRUE)) {
      return OS_INVALID_ARGUMENT;
   }

   if ((num < 0) || (num > MpBuf_getByteLen(b))) {
      Zprintf("MpBuf_setNumSamples(0x%p, %d): numSamples invalid (max=%d)\n",
         b, num, MpBuf_getByteLen(b), 0,0,0);
      return OS_INVALID_ARGUMENT;
   }

   // $$$ Should we make sure that the num is consistent with the
   //   alignment requirements of the payload type?

   b->numSamples = num;
   return OS_SUCCESS;
}

OsStatus MpBuf_setContentLen(MpBufPtr b, int numBytes)
{
   if (MpBuf_invalid(b, TRUE, TRUE)) {
      return OS_INVALID_ARGUMENT;
   }

   if ((numBytes < 0) || (numBytes > MpBuf_getByteLen(b))) {
      Zprintf("MpBuf_setContentLen(0x%p, %d): contentLen invalid (max=%d)\n",
         b, numBytes, MpBuf_getByteLen(b), 0,0,0);
      return OS_INVALID_ARGUMENT;
   }

   b->contentLen = numBytes;
   return OS_SUCCESS;
}

OsStatus MpBuf_setOffset(MpBufPtr b, int offset)
{
   if (MpBuf_invalid(b, TRUE, TRUE)) {
      return OS_INVALID_ARGUMENT;
   }

   if ((offset < 0) || (offset > MpBuf_getByteLen(b))) {
      Zprintf("MpBuf_setOffset(0x%p, %d): Offset invalid (max=%d)\n",
         b, offset, MpBuf_getByteLen(b), 0,0,0);
      return OS_INVALID_ARGUMENT;
   }

   // $$$ Should we make sure that the offset is consistent with the
   //   alignment requirements of the payload type?

   b->pSamples = (Sample *) (MpBuf_getStorage(b) + offset);
   b->offset = offset;
   return OS_SUCCESS;
}

MpBufSpeech MpBuf_checkSpeech(MpBufPtr buf)
{
   MpBufSpeech ret = MpBuf_getSpeech(buf);

   switch (ret) {
   case MP_SPEECH_UNKNOWN:
      // call silence_detection on buf, then call MpBuf_setSpeech(buf, result)
      // ret = MP_SPEECH_ACTIVE; Should do this, given no silence detector???
      break;
   case MP_SPEECH_SILENT:
   case MP_SPEECH_COMFORT_NOISE:
   case MP_SPEECH_ACTIVE:
   case MP_SPEECH_TONE:
      break;
   case MP_SPEECH_MUTED:
      // maybe call silence_detection on buf, to update background stats
      ret = MP_SPEECH_SILENT;
      break;
   }
   return ret;
}

OsStatus MpBuf_setSamples(MpBufPtr b, Sample *first)
{
   if (MpBuf_invalid(b, TRUE, TRUE)) {
      return OS_INVALID_ARGUMENT;
   }

   return MpBuf_setOffset(b, ((char *) first - MpBuf_getStorage(b)));
}

MpBufPtr MpBuf_getFgSilence()
{
   MpBuf_addRef(MpMisc.XXXsilence);
   return(MpMisc.XXXsilence);
}

MpBufPtr MpBuf_getDmaSilence()
{
   MpBuf_addRef(MpMisc.XXXlongSilence);
   return(MpMisc.XXXlongSilence);
}

UtlBoolean MpBuf_isPoolSilent(MpBufPtr buf)
{
   return ((buf == MpMisc.XXXsilence) || (buf == MpMisc.XXXlongSilence));//$$$
}

static unsigned long MinVoiceEnergy = 0; // trigger threshhold

static int numVads = 0; // for limiting debug output

long MpBuf_setMVE(long newMin)
{
   unsigned long save = MinVoiceEnergy;
   MinVoiceEnergy = newMin;
   numVads = 0;
   return save;
}

MpBufSpeech MpBuf_doVAD(MpBufPtr buf)
{
   int num, i = 1;
   Sample prev;
   Sample* data;
   unsigned long energy = 0;
   unsigned long t;
   MpBufSpeech ret = MP_SPEECH_SILENT;

   assert(!MpBuf_invalid(buf, FALSE, TRUE));

   num = MpBuf_getNumSamples(buf);
   data = MpBuf_getSamples(buf);
   while ((i < num) && (MP_SPEECH_SILENT == ret)) {
      i++;
      prev = *data++;
      t = (prev - *data) >> 1;
      energy += t * t;
      if (energy >= MinVoiceEnergy) ret = MP_SPEECH_ACTIVE;
   }
#ifdef DEBUG
   if (20 > numVads++) osPrintf(" %d %ld\n", i, energy);
#endif
   MpBuf_setSpeech(buf, ret);
   return ret;
}

unsigned long MpBuf_getVAD(MpBufPtr buf)
{
   int num, i = 1;
   Sample prev;
   Sample* data;
   unsigned long energy = 0;
   unsigned long t;

   assert(!MpBuf_invalid(buf, FALSE, TRUE));

   num = MpBuf_getNumSamples(buf);
   data = MpBuf_getSamples(buf);
   while (i < num) {
      i++;
      prev = *data++;
      t = (prev - *data) >> 1;
      energy += t * t;
   }
   return energy;
}

UtlBoolean MpBuf_isActiveAudio(MpBufPtr buf)
{
   MpBufSpeech speech;
   UtlBoolean ret = TRUE;

   assert(!MpBuf_invalid(buf, FALSE, TRUE));

   if (MpBuf_isPoolSilent(buf)) {
      return FALSE;
   }

   speech = MpBuf_getSpeech(buf);
   switch (speech) {
   case MP_SPEECH_UNKNOWN:
      MpBuf_doVAD(buf);
      speech = MpBuf_getSpeech(buf);
      assert(MP_SPEECH_UNKNOWN != speech);
      ret = MpBuf_isActiveAudio(buf);
      break;
   case MP_SPEECH_ACTIVE:
   case MP_SPEECH_TONE:
      break;
   case MP_SPEECH_SILENT:
   case MP_SPEECH_COMFORT_NOISE:
   case MP_SPEECH_MUTED:
      ret = FALSE;
      break;
   }
   return ret;
}

UtlBoolean MpBuf_isSilence(MpBufPtr buf)
{
   return !MpBuf_isActiveAudio(buf);
}

int MpBuf_getTotalBufferCount(void)
{
   int count = 0;
   if (NULL != MpMisc.UcbPool) {
       count += MpBufPool_getNumBufs(MpMisc.UcbPool);
   }

   if (NULL != MpMisc.RtpPool) {
       count += MpBufPool_getNumBufs(MpMisc.RtpPool);
   }

   if (NULL != MpMisc.RtcpPool) {
       count += MpBufPool_getNumBufs(MpMisc.RtcpPool);
   }

   if (NULL != MpMisc.DMAPool) {
       count += MpBufPool_getNumBufs(MpMisc.DMAPool);
   }
   return count;
}

#define NETWORK_MTU 1520
#define MAX_RTCP_PACKET_LEN NETWORK_MTU
#define rtpNBufs 16
#undef  rtpNBufs
#define rtpNBufs 250
#define rtcpNBufs 16

OsStatus MpBuf_init(int samplesPerFrame, int numAudioBuffers)
{
   LowBufTable = 0xffffffff;
   HighBufTable = 0;

#ifdef BUFFER_INSTRUMENTATION /* [ */
   spCounterMutex = new OsMutex(OsMutex::Q_PRIORITY);
#endif /* BUFFER_INSTRUMENTATION ] */

   MpMisc.UcbPool = MpBufPool_MpBufPool(0,
                   samplesPerFrame*sizeof(Sample),
                        numAudioBuffers, 0);
   Nprintf("MpBuf_init: MpMisc.UcbPool = 0x%p\n",
                           MpMisc.UcbPool, 0,0,0,0,0);
   if (NULL == MpMisc.UcbPool) {
      return OS_NO_MEMORY;
   }

   MpMisc.DMAPool = MpBufPool_MpBufPool(0,
                   8*samplesPerFrame*sizeof(Sample),
                        64, 32);
   Nprintf("MpBuf_init: MpMisc.DMAPool = 0x%p\n",
                           MpMisc.DMAPool, 0,0,0,0,0);

   if (NULL == MpMisc.DMAPool) {
      return OS_NO_MEMORY;
   }

/*************************************************************************/

/*
 * Go get a buffer and fill with silence.  We will use this for muting
 * either or both of input and output, and whenever we are starved for
 * audio data.
 */

#ifdef N_SPARE_BUFS /* [ */
    /* For debugging... get a few buffers and set them aside... */
    {
        MpBufPtr sb;
        int i, j, n;
        Sample* dst;

        for (i=0; i<N_SPARE_BUFS; i++) {
            sb = MpBuf_getBuf(MpMisc.UcbPool, samplesPerFrame, 0, MP_FMT_T12);
            if (NULL == sb) {
                Zprintf("\n\nMpBuf_init: MpBuf_getBuf failed, quitting!\n\n\n",
                    0,0,0,0,0,0);
                MpBufPool_delete(MpMisc.UcbPool, 1);
                MpMisc.UcbPool = NULL;
                return OS_LIMIT_REACHED;
            }
            dst = MpBuf_getSamples(sb);
            n = MpBuf_getByteLen(sb) / sizeof(Sample);
            MpBuf_setNumSamples(sb, n);
            for (j=0; j<n; j++) {
                *dst++ = (i<<8) + j;
            }
            MpBuf_setSpeech(sb, (enum MpBufSpeech) MP_SPEECH_SPARE);
            spareBufs[i] = sb;
        }
        showSpareBufs(0, "MpBuf_Init: ");
    }
#endif /* N_SPARE_BUFS ] */
    {
        MpBufPtr sb;

        sb = MpBuf_getBuf(MpMisc.UcbPool, samplesPerFrame, 0, MP_FMT_T12);
        if (NULL == sb) {
            Zprintf("\n\nMpBuf_init: MpBuf_getBuf failed, quitting!\n\n\n",
                0,0,0,0,0,0);
            MpBufPool_delete(MpMisc.UcbPool, 1);
            MpMisc.UcbPool = NULL;
            return OS_LIMIT_REACHED;
        }
        memset(MpBuf_getSamples(sb), 0, MpBuf_getByteLen(sb));
        MpBuf_setSpeech(sb, MP_SPEECH_SILENT);
        MpMisc.XXXsilence = sb;
        Zprintf("MpBuf_init: MpMisc.silence = 0x%p\n",
                                MpMisc.XXXsilence, 0,0,0,0,0);
    }
/*************************************************************************/

/*
 * Go get a DMA buffer and fill with silence.  We will use this for muting
 * either or both of input and output, and whenever we are starved for
 * audio data.
 */

    {
        MpBufPtr sb;

        sb = MpBuf_getBuf(MpMisc.DMAPool, 8*samplesPerFrame, 0, MP_FMT_T12);
        if (NULL == sb) {
            Zprintf("\n\nMpBuf_init: MpBuf_getBuf failed (DMA), quitting!\n\n\n",
                0,0,0,0,0,0);
            MpBufPool_delete(MpMisc.DMAPool, 1);
            MpMisc.DMAPool = NULL;
            return OS_LIMIT_REACHED;
        }
        memset(MpBuf_getSamples(sb), 0, MpBuf_getByteLen(sb));
        MpBuf_setSpeech(sb, MP_SPEECH_SILENT);
        MpMisc.XXXlongSilence = sb;
        Zprintf("MpBuf_init: MpMisc.longSilence = 0x%p\n",
                                MpMisc.XXXlongSilence, 0,0,0,0,0);
    }
/*************************************************************************/
 /*
  * generate a buffer called comfort noise buffer. Even though the zero
  * initiation is not necessary, we do it as the silence buffer for safety.
  */
    {
        MpBufPtr cnb;

        cnb = MpBuf_getBuf(MpMisc.UcbPool, samplesPerFrame, 0, MP_FMT_T12);
        if (NULL == cnb) {
            Zprintf("\n\nMpBuf_init: MpBuf_getBuf failed, quitting!\n\n\n",
                0,0,0,0,0,0);
            MpBufPool_delete(MpMisc.UcbPool, 1);
            MpMisc.UcbPool = NULL;
            return OS_LIMIT_REACHED;
        }
        memset(MpBuf_getSamples(cnb), 0, MpBuf_getByteLen(cnb));
        MpBuf_setSpeech(cnb, MP_SPEECH_COMFORT_NOISE);
        MpMisc.comfortNoise = cnb;
        Zprintf("MpBuf_init: MpMisc.comfortNoise = 0x%p\n",
                                MpMisc.comfortNoise, 0,0,0,0,0);
    }
/*************************************************************************/

   MpMisc.RtpPool = MpBufPool_MpBufPool(0, NETWORK_MTU, rtpNBufs, 0);
   Nprintf("MpBuf_init: MpMisc.RtpPool = 0x%p\n",
                           MpMisc.RtpPool, 0,0,0,0,0);
   if (NULL == MpMisc.RtpPool) {
      MpBufPool_delete(MpMisc.UcbPool, 1);
      MpMisc.UcbPool = NULL;
      return OS_NO_MEMORY;
   }

   MpMisc.RtcpPool = MpBufPool_MpBufPool(0,
      MAX_RTCP_PACKET_LEN, rtcpNBufs, 0);
   Nprintf("MpBuf_init: MpMisc.RtcpPool = 0x%p\n",
                           MpMisc.RtcpPool, 0,0,0,0,0);
   if (NULL == MpMisc.RtcpPool) {
      MpBufPool_delete(MpMisc.UcbPool, 1);
      MpMisc.UcbPool = NULL;
      MpBufPool_delete(MpMisc.RtpPool, 1);
      MpMisc.RtpPool = NULL;
      return OS_NO_MEMORY;
   }

   return OS_SUCCESS;
}

#ifdef BUFFER_INSTRUMENTATION /* [ */
        volatile int bufHang, doHang = 1;
        int we_are_logging = 0;

int bufUnHang(int x)
{
    int was = bufHang;
    bufHang = 0;
    doHang = x;
    return was;
}

int forceLogging(int on)
{
        int was = we_are_logging;

        we_are_logging = on;
        return was;
}

int incr_time_stamp()
{
        OsLock lock(*spCounterMutex);
        return buffer_time_stamp++;
}
#endif /* BUFFER_INSTRUMENTATION ] */

void MpBuf_insertSawTooth(MpBufPtr b)
{
    int i, n;
    Sample *s;

    if (NULL == b) return;
    if (!MpBuf_invalid(b, TRUE, TRUE)) {
        s = MpBuf_getSamples(b);
        n = MpBuf_getNumSamples(b);
        for (i=0; i<n; i++)
            *s++ = ((0xf & i) << 12);
    }
}

#ifdef BUFFER_INSTRUMENTATION /* [ */
MpBufPtr MpBuf_getBufX(MpBufPoolPtr Pool,
        int initSamples, int initOffset, MpBufFormat initFormat, int line)
#else /* BUFFER_INSTRUMENTATION ] [ */
MpBufPtr MpBuf_getBufY(MpBufPoolPtr Pool,
        int initSamples, int initOffset, MpBufFormat initFormat)
#endif /* BUFFER_INSTRUMENTATION ] */
{
        int i;
        int n;
        MpBufPtr ret, t;
        MpBufPoolPtr l = Pool;
        OsLock lock(*(Pool->mpMutex));
#ifdef BUFFER_INSTRUMENTATION /* [ */
        int this_time = incr_time_stamp();
#endif /* BUFFER_INSTRUMENTATION ] */

        ret = NULL;
        n = l->lastTaken;
        t = &(l->table[n]);
        for (i=0; ((NULL==ret) && (i<l->allocCnt)); i++) {
            if (n >= l->allocCnt) {
                n = 0;
                t = l->table;
            }
            if (0 == t->status) {
                n++;
                if (n >= l->allocCnt) {
                    n = 0;
                }
                l->lastTaken = n;
                ret = t;
                ret->status = 1;
                ret->speech = MP_SPEECH_UNKNOWN;
                ret->refCnt = 1;
                MpBuf_setFormat(ret, initFormat);
                MpBuf_setOffset(ret, initOffset);
                MpBuf_setNumSamples(ret, initSamples);
                MpBuf_setOsTC(ret, 0);
#define CLEAR_BUFFER
#undef CLEAR_BUFFER
#ifdef CLEAR_BUFFER /* [ */
                memset(MpBuf_getStorage(ret), 0, MpBuf_getByteLen(ret));
#endif /* CLEAR_BUFFER ] */
#define INSERT_SAWTOOTH
#undef INSERT_SAWTOOTH
#ifdef INSERT_SAWTOOTH /* [ */
                MpBuf_insertSawTooth(ret);
#endif /* INSERT_SAWTOOTH ] */
#ifdef BUFFER_INSTRUMENTATION /* [ */
                ret->line_taken = line;
                ret->time_taken = this_time;
#endif /* BUFFER_INSTRUMENTATION ] */
            }
            t++;
            n++;
        }

#ifdef BUFFER_INSTRUMENTATION /* [ */
        if (NULL != ret) l->nfree--;
        if (l->nfree < l->minfree) {
            l->minfree = l->nfree;
            if (2 < (l->allocCnt)) {
                showBufs(Pool, line);
            }
            if (15 < (l->allocCnt - l->nfree)) {
                /* dumpBufs(0); */
            }
        }
#endif /* BUFFER_INSTRUMENTATION ] */

        Nprintf(" +%02d(%d)\n", i-1, line, 0,0,0,0);
        Nprintf("take: 0x%p (%d), at %d\n",
                                ret, i-1, line, 0,0,0);
        return ret;
}

#ifdef BUFFER_INSTRUMENTATION /* [ */
void MpBuf_delRefX(MpBufPtr b, int line)
#else /* BUFFER_INSTRUMENTATION ] [ */
void MpBuf_delRef(MpBufPtr b)
#define line (-1)
#endif /* BUFFER_INSTRUMENTATION ] */
{
        MpBufPoolPtr l;
#ifdef BUFFER_INSTRUMENTATION /* [ */
        int this_time = incr_time_stamp();
#endif /* BUFFER_INSTRUMENTATION ] */

        int n;

        if (NULL == b) {
            return;
        }

        if (MpBuf_invalid(b, FALSE, TRUE)) {
            Zprintf("MpBuf_delRef(0x%p): invalid! line: %d\n",
                        b, line, 0,0,0,0);
            // assert(!MpBuf_invalid(b, FALSE, TRUE));
            return;
        }

        l = b->pPool;

        n = b - l->table;
        if ((n < 0) || (n >= l->allocCnt)) {
            Zprintf("MpBuf_delRef: attempt to free 0x%p (%d) @%d\n",
                b, n, line, 0,0,0);
        } else if (1 != b->status) {
            l->mpMutex->acquire();
            b->status = 2;
            l->mpMutex->release();
            Zprintf("MpBuf_delRef: attempt to free a free buffer"
                " -- 0x%p (%d) @%d\n", b, n, line, 0,0,0);
        } else {
            Nprintf("!%d(%d)\n ", b - l->table, line,0,0,0,0);
            l->mpMutex->acquire();
            b->refCnt--;
            if (0 == b->refCnt) {
               b->status = 0;
               b->speech = MP_SPEECH_UNKNOWN;
#ifdef BUFFER_INSTRUMENTATION /* [ */
               l->nfree++;
               b->line_freed = line;
               b->time_freed = this_time;
#endif /* BUFFER_INSTRUMENTATION ] */
            }
            l->mpMutex->release();
        }
#undef line
} /* MpBuf_delRef, MpBuf_delRefX */

#ifdef BUFFER_INSTRUMENTATION /* [ */
void MpBuf_addRefX(MpBufPtr b, int line)
#else /* BUFFER_INSTRUMENTATION ] [ */
void MpBuf_addRef(MpBufPtr b)
#define line (-1)
#endif /* BUFFER_INSTRUMENTATION ] */
{
        MpBufPoolPtr l;

        int n;

        if (NULL == b) {
            return;
        }

        if (MpBuf_invalid(b, FALSE, TRUE)) {
            Zprintf("MpBuf_addRef(0x%p): invalid! line: %d\n",
                        b, line, 0,0,0,0);
            return;
        }

        l = b->pPool;

        n = b - l->table;
        if ((n < 0) || (n >= l->allocCnt)) {
            Zprintf("MpBuf_addRef: attempt to free 0x%p (%d) @%d\n",
                b, n, line, 0,0,0);
        } else if (1 != b->status) {
            l->mpMutex->acquire();
            b->status = 2;
            l->mpMutex->release();
            Zprintf("MpBuf_addRef: attempt to add ref to a free buffer"
                " -- 0x%p (%d) @%d\n", b, n, line, 0,0,0);
        } else {
            Nprintf("!%d(%d)\n ", b - l->table, line,0,0,0,0);
            l->mpMutex->acquire();
            b->refCnt++;
            l->mpMutex->release();
        }
#undef line
} /* MpBuf_addRef, MpBuf_addRefX */

#ifdef BUFFER_INSTRUMENTATION /* [ */
void MpBuf_touchX(MpBufPtr b, int line)
{
        MpBufPoolPtr l;
        int this_time;
        int n;

        if (NULL == b) return;  // allow and ignore NULL pointers

        l = b->pPool;
        this_time = incr_time_stamp();
        n = b - l->table;
        if ((n < 0) || (n >= l->allocCnt)) {
            Zprintf(
                "bufTouch: attempt to touch a bad buffer -- 0x%p (%d) @%d\n",
                    b, n, line, 0,0,0);
        } else if (1 != b->status) {
            l->mpMutex->acquire();
            b->status = 2;
            l->mpMutex->release();
            Zprintf(
              "bufTouch: attempt to touch a free buffer -- 0x%p (%d) @%d(%d)\n",
                  b, n, line, buffer_time_stamp, 0,0);
            dump1Buf(b);
        } else {
            l->mpMutex->acquire();
            b->touched_by = line;
            b->touched_at = this_time;
            l->mpMutex->release();
        }
}
#endif /* BUFFER_INSTRUMENTATION ] */

MpBufPtr MpBuf_allowMods(MpBufPtr b)
{
   MpBufPtr t;

   if (NULL == b) {
      return NULL;
   }

   if (MpBuf_invalid(b, FALSE, TRUE)) {
      Zprintf("MpBuf_allowMod(0x%p): invalid!\n", b, 0,0,0,0,0);
      return NULL;
   }

   if (1 == b->refCnt) {
      return b;
   }

   t = MpBuf_getBuf(b->pPool, MpBuf_getNumSamples(b),
                    MpBuf_getOffset(b), MpBuf_getFormat(b));

   if (NULL == t) {
      return NULL;
   }

   memcpy((void*) MpBuf_getSamples(t),
          (void*) MpBuf_getSamples(b),
          MpBuf_getByteLen(t));

   MpBuf_setOsTC(t, MpBuf_getOsTC(b));
   MpBuf_setContentLen(t, MpBuf_getContentLen(b));
   MpBuf_delRef(b);
   return t;

} /* MpBuf_allowMods */

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
#include <assert.h>
#include <string.h>
#ifdef _VXWORKS /* [ */
#include <unistd.h>

#include <msgQLib.h>
#include <taskLib.h>
#include <vmLib.h>
#include <intLib.h>
#include <tickLib.h>
#endif /* _VXWORKS ] */

// APPLICATION INCLUDES

#ifdef _VXWORKS /* [ */
#if CPU == ARMSA110 || (CPU == ARMARCH4) || (CPU == STRONGARM) /* [ [ */
#include "mp/sa1100.h"
#elif (CPU == XSCALE) /* ] [ */
#include "mp/pxa255.h"
#else /* ] [ */
#error Unexpected CPU value
#endif /* ] ] */
#endif /* _VXWORKS ] */

#include "os/OsMsgQ.h"
#include "os/OsConfigDb.h"
#include "mp/MpTypes.h"
#include "mp/MpCodec.h"
#include "mp/dmaTask.h"
#include "mp/MpBuf.h"
#include "mp/MpBufferMsg.h"
#include "mp/MpMisc.h"
#include "mp/NetInTask.h"
#include "mp/MprFromMic.h"
#include "mp/MprToSpkr.h"
#include "mp/MprDejitter.h"
#include "mp/MprDecode.h"

// EXTERNAL FUNCTIONS
extern long MpBuf_setMVE(long newMin);

// EXTERNAL VARIABLES

// CONSTANTS

#define MIC_BUFFER_Q_LEN 10
#define SPK_BUFFER_Q_LEN 14

#ifdef _VXWORKS /* [ */
#define LOG_MSGQ_MAX_MSGS 8000
#define LOG_MSGQ_ITEM_LEN 60
#define ABSOLUTE_MAX_LOG_MSG_LEN 2048
#endif /* _VXWORKS ] */

// STATIC VARIABLE INITIALIZATIONS

    struct __MpGlobals MpMisc;

#ifdef _VXWORKS /* [ */
/************************************************************************/

static int logTaskPrio = 250;
int setLogTaskPrio(int p) {
   int save = logTaskPrio;
   logTaskPrio = 0xff & p;
   return save;
}

static int logging = 1;

static int printingLog = 0;
static int numDiscarded = 0;

int drain_log(int level /* how many to leave */)
{
        char msg[1];
        int l, n;

        if (level < 0) level = 50;
        n = msgQNumMsgs(MpMisc.LogQ);
        while(level < msgQNumMsgs(MpMisc.LogQ)) {
            l = msgQReceive(MpMisc.LogQ, msg, 1, VX_NO_WAIT);
            numDiscarded++;
        }
        return n;
}

int StartLogging()
{
        int i;
        i = logging;
        logging = 1;
        return i;
}

int StopLogging(int opt)
{
        int i;

        logging = 0;
        i = msgQNumMsgs(MpMisc.LogQ);
        if (opt) drain_log(50);
        return i;
}

int Zprintf0(int force, char *buf)
{
        int l;
        int n;
        int ret;
        int msgret;
        char *str;
        char junk;

        str = buf;
        l = min(ABSOLUTE_MAX_LOG_MSG_LEN, strlen(buf));
        ret = l;
        if (0 == MpMisc.LogQ) {
            ret = force ? fwrite(str, 1, ret, stderr) : 0;
            return ret;
        }
        if (force || logging) {
            if ((ret > MpMisc.logMsgSize) && !intContext()) {
                taskLock();
            }
            while (l > 0) {
                n = min(l, MpMisc.logMsgSize);
                msgret = msgQSend(MpMisc.LogQ, buf, n,
                                       VX_NO_WAIT, MSG_PRI_NORMAL);
                if (ERROR == msgret) {
                    // int r2 =
                    msgQReceive(MpMisc.LogQ, &junk, 1, VX_NO_WAIT);
                    // printf("discard message; r1=%d, r2=%d, q=0x%X: '%s'\n",
                        // msgret, r2, MpMisc.LogQ, buf);
                    numDiscarded++;
                    msgret = msgQSend(MpMisc.LogQ, buf, n,
                                       VX_NO_WAIT, MSG_PRI_NORMAL);
                }
                if ((ERROR == msgret) && force) {
                    fwrite(str, 1, ret, stderr);
                    l = 0;
                } else {
                    l -= n;
                    buf += n;
                }
            }
            if ((ret > MpMisc.logMsgSize) && !intContext()) {
                taskUnlock();
            }
        } else {
            ret = 0;
        }
        return ret;
}

int Zprintfx(int force, char *fmt, int a, int b, int c, int d, int e, int f)
{
        int l, ret;
        char buf[256];

        if (force || logging) {
            sprintf(buf, fmt, a, b, c, d, e, f);
            l = Zprintf0(force, buf);
        } else {
            l = 0;
        }
        return l;
}

int NoPrintLog()
{
    int save = printingLog;
    if (printingLog) {
        numDiscarded = 0;
    }
    osPrintf("\n");
    printingLog = 0;
    return save;
}
int PrintLog(int num)
{
    int save = printingLog;
    if (!printingLog) {
        if (num > 0) drain_log(num);
        fprintf(stderr, "%d messages discarded\n", numDiscarded);
        numDiscarded = 0;
    }
    printingLog = 1;
    return save;
}

extern "C" int NPL(){return NoPrintLog();}
extern "C" int PL(int n){return PrintLog(n);}

static int printTask(int t1, int t2, int t3, int t4, int t5,
                int t6, int t7, int t8, int t9, int t10)
{
        char msg[LOG_MSGQ_ITEM_LEN+1];
        int l;
        int lastTick;
        int thisTick;
        int Hz;
        int qHz;

        Hz = sysClkRateGet();
        qHz = Hz / 4;
#define LOG_ANNOUNCE_MINUTES 5

        lastTick = (int) tickGet() - (LOG_ANNOUNCE_MINUTES * 61 * Hz);

        while (1) {
            while (!printingLog) taskDelay(qHz);
            l = msgQReceive(MpMisc.LogQ, msg, LOG_MSGQ_ITEM_LEN, VX_WAIT_FOREVER);
            thisTick = (int) tickGet();
            if ((LOG_ANNOUNCE_MINUTES * 60 * Hz) < (thisTick - lastTick)) {
                lastTick = thisTick;
                fprintf(stderr, "MsgLogger: %d.%03d seconds since boot\n",
                    thisTick/Hz, ((thisTick % Hz) * 1000) / Hz);
            }
            if (l > 0) {
                msg[l] = 0;
                fwrite(msg, 1, l, stderr);
            } else {
                printf("\n\nLogger: quitting!\n\n");
                return 0;
            }
        }
}

MSG_Q_ID startLogging(int nmsgs, int maxlen)
{
        MpMisc.LogQ = msgQCreate(nmsgs, maxlen, MSG_Q_FIFO);
        if (NULL == MpMisc.LogQ) {
                printf("cannot create LogQ!  Quitting\n");
        } else {
            Zprintf("logQ is 0x%p\n", MpMisc.LogQ, 0,0,0,0,0);
            taskSpawn("Logger", logTaskPrio, 0, 8192, (FUNCPTR) printTask,
                           0, 0, 0,0,0,0,0,0,0,0);
        }
        return MpMisc.LogQ;
}

#ifdef DEBUG /* [ */
int jamLog(char *fmt, int count, int a, int b, int c, int d, int e)
{
    int i;

    for (i=0; i<count; i++) {
        Zprintfx(1, fmt, i, a, b, c, d, e);
    }
    return msgQNumMsgs(MpMisc.LogQ);
}
#endif /* DEBUG ] */

OsStatus startLogTask()
{
#ifdef _VXWORKS /* [ */
        MpMisc.logMsgLimit = LOG_MSGQ_MAX_MSGS;
        MpMisc.logMsgSize = LOG_MSGQ_ITEM_LEN;
        if (NULL == startLogging(MpMisc.logMsgLimit, MpMisc.logMsgSize)) {
            return OS_UNSPECIFIED;
        }
#endif /* _VXWORKS ] */
        return OS_SUCCESS;
}

/************************************************************************/

/* Work around for GDB crash... */
static int dontStepInThis()
{
        int ps;

        ps = vmPageSizeGet();
        return ps;
}

static int goGetThePageSize()
{
        return dontStepInThis(); /*!!!*/
}
#endif /* _VXWORKS ] */

/************************************************************************/
extern "C" {
extern intptr_t showMpMisc(int justAddress);
extern int setMaxMic(int v);
extern int setMaxSpkr(int v);
extern int setMinRtp(int v);
extern int mpSetLatency(int maxMic, int maxSpkr, int minRtp);
#ifdef _VXWORKS /* [ */
extern int LoopBack(int on);
extern int LBon();
extern int LBoff();
extern int Spkr1On();
extern int Spkr1Off();
extern int Spkr2On();
extern int Spkr2Off();
#endif /* _VXWORKS ] */
}
/************************************************************************/

#ifdef _VXWORKS /* [ */
#if CPU == ARMSA110 || (CPU == ARMARCH4) || (CPU == STRONGARM) /* [ */

int LoopBack(int on) {
   MpBufferMsg*    pMsg;
   int save = MpMisc.doLoopBack;

   MpMisc.doLoopBack = on;
   while (0 < MpMisc.pLoopBackQ->numMsgs()) {
      if (OS_SUCCESS == MpMisc.pLoopBackQ->receive((OsMsg*&) pMsg,
                                                    OsTime::NO_WAIT)) {
         MpBuf_delRef(pMsg->getTag());
         MpBuf_delRef(pMsg->getTag(1));
         pMsg->releaseMsg();
      }
   }
   return save;
}
int LBon()  {return LoopBack(1);}
int LBoff() {return LoopBack(0);}

static int* gpsr = (int*) SA1100_GPSR;
static int* gpcr = (int*) SA1100_GPCR;
int Spkr1On()  {*gpcr = (1<<25); return 0;}
int Spkr1Off() {*gpsr = (1<<25); return 0;}
int Spkr2On()  {*gpcr = (1<<26); return 0;}
int Spkr2Off() {*gpsr = (1<<26); return 0;}

#endif /* StrongARM ] */
#endif /* _VXWORKS ] */

intptr_t showMpMisc(int justAddress)
{
   Zprintf("&MpMisc = 0x%p\n", &MpMisc, 0,0,0,0,0);
   if (!justAddress) {
      Zprintf(" MicQ=0x%p, SpkQ=0x%p, EchoQ=0x%p, silence=0x%p\n"
         " micMuteStatus=%d, spkrMuteStatus=%d,",
         MpMisc.pMicQ, MpMisc.pSpkQ, MpMisc.pEchoQ,
         MpMisc.XXXsilence, MpMisc.micMuteStatus, MpMisc.spkrMuteStatus);
      Zprintf(" audio_on=%d\n frameSamples=%d,"
         " frameBytes=%d, sampleBytes=%d,",
         MpMisc.audio_on,
         MpMisc.frameSamples, MpMisc.frameBytes, MpMisc.sampleBytes, 0,0);
      Zprintf(" rtpMaxBytes=%d\n UcbPool=0x%p, RtpPool=0x%p, RtcpPool=0x%p\n",
         MpMisc.rtpMaxBytes, MpMisc.UcbPool, MpMisc.RtpPool,
         MpMisc.RtcpPool, 0,0);
#ifdef _VXWORKS /* [ */
      Zprintf(" mem_page_size=%d, mem_page_mask=0x%08X\n"
         "LogQ=0x%p, logMsgLimit=%d, logMsgSize=%d\n",
         MpMisc.mem_page_size, MpMisc.mem_page_mask,
         MpMisc.LogQ, MpMisc.logMsgLimit, MpMisc.logMsgSize, 0);
#endif /* _VXWORKS ] */
      Zprintf(" Latency: maxMic=%d, maxSpkr=%d, minRtp=%d\n",
         MpMisc.max_mic_buffers, MpMisc.max_spkr_buffers,
         MpMisc.min_rtp_packets, 0,0,0);
   }
   return (intptr_t) &MpMisc;
}

int setMaxMic(int v)
{
    int save = MpMisc.max_mic_buffers;

    if (v >= MIC_BUFFER_Q_LEN) {
        int vWas = v;
        v = MIC_BUFFER_Q_LEN - 1;
        osPrintf("\nmax_mic_buffers MUST BE less than %d... setting to"
            " %d instead of %d\n",
            MIC_BUFFER_Q_LEN, v, vWas);
    }
    if (v > 0) MpMisc.max_mic_buffers = v;
    return save;
}

int setMaxSpkr(int v)
{
    int save = MpMisc.max_spkr_buffers;

    if (v >= SPK_BUFFER_Q_LEN) {
        int vWas = v;
        v = SPK_BUFFER_Q_LEN - 1;
        osPrintf("\nmax_spkr_buffers MUST BE less than %d... setting to"
            " %d instead of %d\n",
            SPK_BUFFER_Q_LEN, v, vWas);
    }
    if (v > 0) MpMisc.max_spkr_buffers = v;
    return save;
}

int setMinRtp(int v)
{
    int save = MpMisc.min_rtp_packets;

    if (v >= MprDejitter::MAX_RTP_PACKETS) {
        int vWas = v;
        v = MprDejitter::MAX_RTP_PACKETS - 1;
        osPrintf("\nmin_rtp_packets  MUST BE less than %d... setting to"
            " %d instead of %d\n",
            MprDejitter::MAX_RTP_PACKETS, v, vWas);
    }
    if (v > 0) MpMisc.min_rtp_packets = v;
    return save;
}

int mpSetLatency(int maxMic, int maxSpkr, int minRtp)
{
    setMaxMic(maxMic);
    setMaxSpkr(maxSpkr);
    setMinRtp(minRtp);
    return 0;
}

int mpSetHighLatency()
{
    return mpSetLatency(2, 2, 3);
}

int mpSetMedLatency()
{
    return mpSetLatency(1, 1, 2);
}

int mpSetLowLatency()
{
    return mpSetLatency(1, 1, 1);
}

int mpStartSawTooth()
{
    int save = MpMisc.micSawTooth;
    MpMisc.micSawTooth = 1;
    return save;
}

int mpStopSawTooth()
{
    int save = MpMisc.micSawTooth;
    MpMisc.micSawTooth = 0;
    return save;
}

extern void doFrameLoop(int sampleRate, int frame_samples);
extern STATUS netStartup();

OsStatus mpStartUp(int sampleRate, int samplesPerFrame,
      int numAudioBuffers, OsConfigDb* pConfigDb)
{
#ifdef _VXWORKS
        int defSilenceSuppressLevel = 10000;
#else
        int defSilenceSuppressLevel = 0;
#endif
        OsStatus  resCode;
        UtlBoolean silenceSuppressFlag;
        UtlString  silenceSuppressEnable;
        int       silenceSuppressLevel;

        if (samplesPerFrame < 8) samplesPerFrame = 80;
        samplesPerFrame = min(samplesPerFrame, FRAME_SAMPS);

        showMpMisc(TRUE);
        MpMisc.micMuteStatus = MpMisc.spkrMuteStatus = 0;

#ifdef _VXWORKS /* [ */
        /* Rashly assumes page size is a power of two */
        MpMisc.mem_page_size = goGetThePageSize();
        MpMisc.mem_page_mask = MpMisc.mem_page_size - 1;
#endif /* _VXWORKS ] */

        MpMisc.sampleBytes = sizeof(short);
        MpMisc.frameSamples = samplesPerFrame;
        MpMisc.frameBytes = MpMisc.sampleBytes * MpMisc.frameSamples;
        MpMisc.rtpMaxBytes = /* sizeof(struct rtpHeader) */ 12 +
            (((sampleRate + 24) / 25) * MpMisc.sampleBytes);

        MpMisc.audio_on = 0;

        if (OS_SUCCESS != MpBuf_init(samplesPerFrame, numAudioBuffers)) {
            return OS_UNSPECIFIED;
        }

        // Use the config database to determine the silence supression level
        silenceSuppressFlag  = FALSE;
        silenceSuppressLevel = defSilenceSuppressLevel;
        if (pConfigDb)
        {
           resCode = pConfigDb->get("PHONESET_SILENCE_SUPPRESSION",
                                    silenceSuppressEnable);
           if (resCode == OS_SUCCESS)
           {
              silenceSuppressFlag =
                (silenceSuppressEnable.compareTo("enable",
                                                 UtlString::ignoreCase) == 0);
           }

           resCode = pConfigDb->get("PHONESET_SILENCE_SUPPRESSION_LEVEL",
                                    silenceSuppressLevel);
           if ((resCode != OS_SUCCESS) ||
               (silenceSuppressLevel < 0) ||
               (silenceSuppressLevel > 20000))
           {
              silenceSuppressLevel = defSilenceSuppressLevel;
           }
        }

        if (silenceSuppressFlag)
           MpBuf_setMVE(silenceSuppressLevel);
        else
           MpBuf_setMVE(0);

#ifdef WIN32 /* [ */
        // Adjust initial audio latency if specified in config files:
        if (pConfigDb)
        {
           UtlString  latency;
           static const int MS_PER_FRAME = 10;
           int frames;
           resCode = pConfigDb->get("PHONESET_INITIAL_OUTPUT_LATENCY_MS",
                                    latency);
           if (resCode == OS_SUCCESS)
           {
              frames = atoi(latency.data()) / MS_PER_FRAME;
              DmaTask_setSpkrQPreload(frames);
           }
           resCode = pConfigDb->get("PHONESET_INITIAL_INPUT_LATENCY_MS",
                                    latency);
           if (resCode == OS_SUCCESS)
           {
              frames = atoi(latency.data()) / MS_PER_FRAME;
              DmaTask_setMicQPreload(frames);
           }
        }
#endif /* WIN32 ] */

#ifdef _VXWORKS /* [ */
        if (OS_SUCCESS != MpCodecOpen(sampleRate, START_GAIN, START_VOLUME)) {
            return OS_UNSPECIFIED;
        }
#endif /* _VXWORKS ] */

        if (NULL != MpMisc.pMicQ) {
            OsMsgQ* q = MpMisc.pMicQ;
            MpMisc.pMicQ = NULL;
            delete q;
        }
        if (NULL != MpMisc.pSpkQ) {
            OsMsgQ* q = MpMisc.pSpkQ;
            MpMisc.pSpkQ = NULL;
            delete q;
        }
        if (NULL != MpMisc.pEchoQ) {
            OsMsgQ* q = MpMisc.pEchoQ;
            MpMisc.pEchoQ = NULL;
            delete q;
        }
#ifdef _VXWORKS /* [ */
        if (NULL != MpMisc.pLoopBackQ) {
            OsMsgQ* q = MpMisc.pLoopBackQ;
            MpMisc.pLoopBackQ = NULL;
            delete q;
        }
#endif /* _VXWORKS ] */
        assert(
            (MIC_BUFFER_Q_LEN+SPK_BUFFER_Q_LEN+MIC_BUFFER_Q_LEN) <
                               (MpBufPool_getNumBufs(MpMisc.UcbPool)-3));
        MpMisc.pMicQ = new OsMsgQ(MIC_BUFFER_Q_LEN);
        MpMisc.pSpkQ = new OsMsgQ(SPK_BUFFER_Q_LEN);
        MpMisc.pEchoQ = new OsMsgQ(MIC_BUFFER_Q_LEN);
#ifdef _VXWORKS /* [ */
        MpMisc.doLoopBack = 0;
        MpMisc.pLoopBackQ = new OsMsgQ(MIC_BUFFER_Q_LEN);
#endif /* _VXWORKS ] */

        assert(MprFromMic::MAX_MIC_BUFFERS > 0);
        assert(MprToSpkr::MAX_SPKR_BUFFERS > 0);
        assert(MprDecode::MIN_RTP_PACKETS > 0);

        setMaxMic(MprFromMic::MAX_MIC_BUFFERS - 1);
        setMaxSpkr(MprToSpkr::MAX_SPKR_BUFFERS);
        setMinRtp(MprDecode::MIN_RTP_PACKETS);
        mpStopSawTooth();

        return OS_SUCCESS;
}

OsStatus mpShutdown(void)
{
        if (NULL != MpMisc.pMicQ) {
            OsMsgQ* q = MpMisc.pMicQ;
            MpMisc.pMicQ = NULL;
            delete q;
        }
        if (NULL != MpMisc.pSpkQ) {
            OsMsgQ* q = MpMisc.pSpkQ;
            MpMisc.pSpkQ = NULL;
            delete q;
        }
        if (NULL != MpMisc.pEchoQ) {
            OsMsgQ* q = MpMisc.pEchoQ;
            MpMisc.pEchoQ = NULL;
            delete q;
        }
#ifdef _VXWORKS /* [ */
        if (NULL != MpMisc.pLoopBackQ) {
            OsMsgQ* q = MpMisc.pLoopBackQ;
            MpMisc.pLoopBackQ = NULL;
            delete q;
        }
#endif /* _VXWORKS ] */
    return OS_SUCCESS;
}

OsStatus mpStartTasks(void)
{
        if (OS_SUCCESS != dmaStartup(MpMisc.frameSamples)) {
            return OS_TASK_NOT_STARTED;
        }

        if (OS_SUCCESS != startNetInTask()) {
            return OS_TASK_NOT_STARTED;
        }

        return OS_SUCCESS;
}

OsStatus mpStopTasks(void)
{

    mpShutdown();
    shutdownNetInTask();
    dmaShutdown();

    return OS_SUCCESS;
}

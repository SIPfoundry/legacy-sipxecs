//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/dmaTask.h"
#include "mp/MpBufferMsg.h"
#include "mp/MpBuf.h"
#include "mp/MprToSpkr.h"
#include "mp/MpMediaTask.h"
#include "os/OsMsgPool.h"

// DEFINES
// EXTERNAL FUNCTIONS
extern void showWaveError(char *syscall, int e, int N, int line) ;  // dmaTaskWnt.cpp

// EXTERNAL VARIABLES

extern HANDLE hMicThread;       // dmaTaskWnt.cpp
extern HANDLE hSpkrThread;      // dmaTaskWnt.cpp

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// GLOBAL VARIABLE INITIALIZATIONS
static int gMicDeviceId ;
static HGLOBAL  hInHdr[N_IN_BUFFERS];
static WAVEHDR* pInHdr[N_IN_BUFFERS];
static HGLOBAL  hInBuf[N_IN_BUFFERS];
#ifdef IHISTORY /* [ */
static int histIn[IHISTORY];
static int lastIn;
#endif /* HISTORY ] */
static HWAVEIN  audioInH;
static OsMsgPool* DmaMsgPool = NULL;


/* ============================ FUNCTIONS ================================= */


void CALLBACK micOutCallBackProc(HANDLE h, UINT wMsg, DWORD dwInstance, DWORD dwParam, DWORD unused)
{
#ifdef IHISTORY /* [ */
   if (WIM_DATA == wMsg) {
      histIn[lastIn] = (((WAVEHDR*)dwParam)->dwUser) & USER_BUFFER_MASK;
      lastIn = (lastIn + 1) % IHISTORY;
   }
#endif /* HISTORY ] */

   int retval = PostThreadMessage(dwInstance, wMsg, dwParam, GetTickCount());

   if (retval == 0)
   {
      Sleep(500);
      retval = PostThreadMessage(dwInstance, wMsg, dwParam, GetTickCount());
      if (retval == 0)
         osPrintf("Could not PostTheadMessage after two tries.\n");
   }
}


int openAudioIn(HWAVEIN *pAudioInH,
                int nChannels, int nSamplesPerSec, int nBitsPerSample)
{
   WAVEFORMATEX fmt;
   MMRESULT     res;

   *pAudioInH = NULL;

   fmt.wFormatTag      = WAVE_FORMAT_PCM;
   fmt.nChannels       = nChannels;
   fmt.nSamplesPerSec  = nSamplesPerSec;
   fmt.nAvgBytesPerSec = (nChannels * nSamplesPerSec * nBitsPerSample) / 8;
   fmt.nBlockAlign     = (nChannels * nBitsPerSample) / 8;
   fmt.wBitsPerSample  = nBitsPerSample;
   fmt.cbSize          = 0;

   res = waveInOpen(
      pAudioInH,              // handle (will be filled in)
      gMicDeviceId,            // attempt to open the "in call" device
      &fmt,                   // format
      (DWORD) micOutCallBackProc,// callback entry
      GetCurrentThreadId(),   // instance data
      CALLBACK_FUNCTION);     // callback function specified

   //if we fail to open the audio device above, then we
   //will try to open any device that will hande the requested format
   if (res != MMSYSERR_NOERROR)
   {
      res = waveInOpen(
      pAudioInH,              // handle (will be filled in)
      WAVE_MAPPER,            // select any device able to handle this format
      &fmt,                   // format
      (DWORD) micOutCallBackProc,// callback entry
      GetCurrentThreadId(),   // instance data
      CALLBACK_FUNCTION);     // callback function specified
   }

   if (res != MMSYSERR_NOERROR)
   {
      showWaveError("waveInOpen", res, -1, __LINE__);
      waveInClose(*pAudioInH);
      *pAudioInH = NULL;
      return 0;
   }
   else
   {
      return 1;
   }
}


WAVEHDR* inPrePrep(int n, DWORD bufLen)
{
   WAVEHDR* pWH;
   int doAlloc = (hInHdr[n] == NULL);

   assert((n > -1) && (n < N_IN_BUFFERS));
   if (doAlloc) {
      hInHdr[n] = GlobalAlloc(GPTR, sizeof(WAVEHDR));
      assert(NULL != hInHdr[n]);
      hInBuf[n] = GlobalAlloc(GPTR, bufLen);
      assert(NULL != hInBuf[n]);
   }
   pInHdr[n] = pWH = (WAVEHDR*) GlobalLock(hInHdr[n]);
   assert(NULL != pInHdr[n]);
   pWH->lpData = (char*) GlobalLock(hInBuf[n]);
   pWH->dwBufferLength = bufLen;
   pWH->dwUser = n;
   pWH->dwBytesRecorded = 0;
   pWH->dwFlags = 0;
   pWH->dwLoops = 0;
   pWH->lpNext = 0;
   pWH->reserved = 0;
   if (doAlloc) {
         memset(pWH->lpData, 0, pWH->dwBufferLength);
   }
   return pWH;
}

bool inPostUnprep(int n, int discard, DWORD bufLen, bool bFree)
{
   bool retVal = false;  //assume we didn't succeed for now

   WAVEHDR* pWH;
   MpBufPtr ob = NULL;

   static int iPU = 0;
   static int flushes = 0;

   assert((n > -1) && (n < N_IN_BUFFERS));
   pWH = pInHdr[n];


   // assert(NULL!=pWH);

   if (pWH && bufLen == pWH->dwBytesRecorded)
   {

#ifdef DEBUG_WINDOZE /* [ */
      if (1) {
         static int micQLen[1024];
         int in = iPU % 1024;
         int i, j;

         micQLen[in] = MpMisc.pMicQ->numMsgs();
         if (in == 1023) {
            osPrintf("\n\n Microphone Queue lengths [%d,%d]:\n", iPU, frameCount);
            for (i=0; i<1024; i+=32) {
               for (j=i; j<(i+32); j++) {
                  osPrintf("%3d", micQLen[j]);
               }
               osPrintf("\n");
            }
            osPrintf("\n\n");
         }
      }
#endif /* DEBUG_WINDOZE ] */

      iPU++;

#ifdef DEBUG_WINDOZE /* [ */
      if (0 && (0 == (iPU % 1000))) {
         osPrintf("inPostUnprep(): %d records, %d flushes\n", iPU, flushes);
      }
#endif /* DEBUG_WINDOZE ] */

      if (!discard) {
         ob = MpBuf_getBuf(MpMisc.UcbPool, N_SAMPLES, 0, MP_FMT_T12);
      }
      if (!discard) {
         MpBufferMsg* pFlush;
         MpBufferMsg* pMsg;

   //    DWW took this assert out, because on windows, when you pull the usb dev out
   //    you can receive 0 bytes back
   //    assert(bufLen == pWH->dwBytesRecorded);
		 if (NULL != ob)
		 {
	         memcpy(MpBuf_getSamples(ob), pWH->lpData, pWH->dwBytesRecorded);
		 }
#ifdef INSERT_SAWTOOTH /* [ */
         if (NULL == ob) { /* nothing in Q, or we are disabled */
            ob = MpBuf_getBuf(MpMisc.UcbPool, MpMisc.frameSamples, 0, MP_FMT_T12);
            if (NULL != ob) {
               int i, n;
               Sample *s;

               s = MpBuf_getSamples(ob);
               n = MpBuf_getNumSamples(ob);
               for (i=0; i<n; i++)
                   *s++= ((i % 80) << 10);
            }
         }
#endif /* INSERT_SAWTOOTH ] */


         pMsg = (MpBufferMsg*) DmaMsgPool->findFreeMsg();
         if (NULL == pMsg)
            pMsg = new MpBufferMsg(MpBufferMsg::AUD_RECORDED);

         pMsg->setMsgSubType(MpBufferMsg::AUD_RECORDED);

         pMsg->setTag(ob);
         if (ob)
         {
            pMsg->setBuf(MpBuf_getSamples(ob));
            pMsg->setLen(MpBuf_getNumSamples(ob));
         }

         if (MpMisc.pMicQ && MpMisc.pMicQ->numMsgs() >= MpMisc.pMicQ->maxMsgs())
         {
            // if its full, flush one and send
            OsStatus  res;
            flushes++;
            res = MpMisc.pMicQ->receive((OsMsg*&) pFlush, OsTime::NO_WAIT);
            if (OS_SUCCESS == res) {
               MpBuf_delRef(pFlush->getTag());
               pFlush->releaseMsg();
            } else {
               osPrintf("DmaTask: queue was full, now empty (3)!"
                  " (res=%d)\n", res);
            }
            if (MpMisc.pMicQ && OS_SUCCESS != MpMisc.pMicQ->send(*pMsg, OsTime::NO_WAIT))
            {
               MpBuf_delRef(ob);
            }
         }
         else
         {
             MpMisc.pMicQ->send(*pMsg, OsTime::NO_WAIT);
         }
         if (!pMsg->isMsgReusable()) delete pMsg;
      }
      return true;
   }

   if (bFree)
   {
      if (NULL != hInHdr[n])
      {
         GlobalUnlock(hInHdr[n]);
         pInHdr[n] = NULL;
         GlobalFree(hInHdr[n]);
         hInHdr[n] = NULL;
      }
      if (NULL != hInBuf[n])
      {
         GlobalUnlock(hInBuf[n]);
         GlobalFree(hInBuf[n]);
         hInBuf[n] = NULL;
      }
   }

   return retVal;
}

int openMicDevice(bool& bRunning, WAVEHDR*& pWH)
{
	int        i, ii;
	WAVEINCAPS devcaps;
    DWORD      bufLen = ((N_SAMPLES * BITS_PER_SAMPLE) / 8);
    MMRESULT   ret;
    MSG        tMsg;
    BOOL       bSuccess ;

    gMicDeviceId = WAVE_MAPPER;

    // If the mic device is set to NONE, don't engage
    if (strcasecmp(DmaTask::getMicDevice(), "NONE") == 0)
    {
        ResumeThread(hSpkrThread);
        return 1;
    }


	int numberOfDevicesOnSystem = waveInGetNumDevs();
	for(ii=0; ii<numberOfDevicesOnSystem; ii++)
    {
        waveInGetDevCaps(ii, &devcaps, sizeof(devcaps));
        if (strcmp(devcaps.szPname, DmaTask::getMicDevice())==0)
        {
            gMicDeviceId = ii ;
            osPrintf("MicThread: Selected mic device: %s\n", devcaps.szPname);
        }
    }

    // Open the input wave device
    if (!openAudioIn(&audioInH, 1, SAMPLES_PER_SEC, BITS_PER_SAMPLE))
    {
        osPrintf("MicThread: Failed to open audio input channel\n\n");
        ResumeThread(hSpkrThread);
        bRunning = false ;
        return 1;
    }

    do
    {
        bSuccess = GetMessage(&tMsg, NULL, 0, 0) ;
    } while (bSuccess && (tMsg.message != WIM_OPEN)) ;

    ret = waveInStart(audioInH);
    if (ret != MMSYSERR_NOERROR)
    {
        showWaveError("waveInStart", ret, -1, __LINE__);
        ResumeThread(hSpkrThread);
        bRunning = false ;
        return 1;
    }

    // Preload audio data
    for (i=0; i<N_IN_PRIME; i++)
    {
        pWH = inPrePrep(i, bufLen);

        ret = waveInPrepareHeader(audioInH, pWH, sizeof(WAVEHDR));
        if (ret != MMSYSERR_NOERROR)
        {
            showWaveError("waveInPrepareHeader", ret, i, __LINE__);
            ResumeThread(hSpkrThread);
            bRunning = false ;
            return 1;
        }
        ret = waveInAddBuffer(audioInH, pWH, sizeof(WAVEHDR));
        if (ret != MMSYSERR_NOERROR)
        {
            showWaveError("waveInAddBuffer", ret, i, __LINE__);
            ResumeThread(hSpkrThread);
            bRunning = false ;
            return 1;
        }
    }

    return 0 ;
}


void closeMicDevice()
{
    DWORD      bufLen = ((N_SAMPLES * BITS_PER_SAMPLE) / 8);
    MMRESULT   ret;
    MSG        tMsg;
    BOOL       bSuccess ;
    int        i ;

    // Cleanup
    ret = waveInReset(audioInH);
    if (ret != MMSYSERR_NOERROR)
    {
        showWaveError("waveInReset", ret, -1, __LINE__);
    }
    ret = waveInStop(audioInH);
    if (ret != MMSYSERR_NOERROR)
    {
        showWaveError("waveInStop", ret, -1, __LINE__);
    }
    Sleep(500) ;

    for (i=0; i<N_IN_BUFFERS; i++)
    {
        if (NULL != hInHdr[i])
        {
            ret = waveInUnprepareHeader(audioInH, pInHdr[i], sizeof(WAVEHDR));
            if (ret != MMSYSERR_NOERROR)
            {
                showWaveError("waveInUnprepareHeader", ret, i, __LINE__);
            }
            inPostUnprep(i, TRUE, bufLen, TRUE);
        }
    }
    Sleep(500) ;

    ret = waveInClose(audioInH);
    if (ret != MMSYSERR_NOERROR)
    {
        showWaveError("waveInClose", ret, -1, __LINE__);
    }

    do
    {
        bSuccess = GetMessage(&tMsg, NULL, 0, 0) ;
    } while (bSuccess && (tMsg.message != WIM_CLOSE)) ;

    audioInH = NULL;
}


unsigned int __stdcall MicThread(LPVOID Unused)
{
    int      i;
    DWORD    bufLen = ((N_SAMPLES * BITS_PER_SAMPLE) / 8);
    WAVEHDR* pWH;
    MMRESULT ret;
    int      recorded;
    MSG      tMsg;
    BOOL     bGotMsg ;
    int      n;
    bool     bDone ;
    static bool bRunning = false ;

    // Verify that only 1 instance of the MicThread is running
    if (bRunning)
    {
        ResumeThread(hSpkrThread);
        return 1 ;
    }
    else
    {
        bRunning = true ;
    }

    MpBufferMsg* pMsg = new MpBufferMsg(MpBufferMsg::AUD_RECORDED);
    DmaMsgPool = new OsMsgPool("DmaTask", (*(OsMsg*)pMsg),
            40, 60, 100, 5,
            OsMsgPool::SINGLE_CLIENT);

    // Initialize Buffers
    for (i=0; i<N_IN_BUFFERS; i++)
    {
        hInHdr[i] = hInBuf[i] = NULL;
        pInHdr[i] = NULL;
    }


#ifdef IHISTORY /* [ */
    WAVEHDR* lastWH[IHISTORY];
    int      lastInd[IHISTORY];
    int      last = 0;

    for (i=0;i<IHISTORY;i++)
    {
        lastWH[i] = 0;
        lastInd[i] = 0;
    }

    memset(histIn, 0xff, sizeof(histIn));
    lastIn = 0;

#endif /* IHISTORY ] */

    if (openMicDevice(bRunning, pWH))
    {
        return 1 ;
    }

    // Start up Speaker thread
    ResumeThread(hSpkrThread);

#ifdef DEBUG_WINDOZE
    frameCount = 0;
#endif
    recorded = 0;
    bDone = false ;
    while (!bDone)
    {
        bGotMsg = GetMessage(&tMsg, NULL, 0, 0);
        if (bGotMsg)
        {
            switch (tMsg.message)
            {
            case WIM_DATA:
                pWH = (WAVEHDR *) tMsg.wParam;
                n = (pWH->dwUser) & USER_BUFFER_MASK;

#ifdef IHISTORY /* [ */
                lastWH[last] = pWH;
                lastInd[last] = n;
                last = (last + 1) % IHISTORY;

                if (N_IN_BUFFERS == recorded)
                {
                    osPrintf("after first %d buffers:", recorded + 1);
                    osPrintf("\nCall Backs:");
                    for (i=0; i<IHISTORY; i++)
                    {
                        osPrintf("%c%3d", (0 == (i % 20)) ? '\n' : ' ', histIn[i]);
                    }
                    osPrintf("\n\nMessages:");
                    for (i=0; i<IHISTORY; i++)
                    {
                        osPrintf("%c%3d", (0 == (i % 20)) ? '\n' : ' ', lastInd[i]);
                    }
                    osPrintf("\n");
                }
#endif /* IHISTORY ] */

                if (DmaTask::isInputDeviceChanged())
                {
                    DmaTask::clearInputDeviceChanged() ;

                    closeMicDevice() ;
                    openMicDevice(bRunning, pWH) ;

                    continue ;
                }

                ret = waveInUnprepareHeader(audioInH, pWH, sizeof(WAVEHDR));
                if (ret != MMSYSERR_NOERROR)
                {
                    showWaveError("waveInUnprepareHeader", ret, recorded, __LINE__);
                }

                if (DmaTask::isMuteEnabled())
                {
                    memset(pWH->lpData, 0, pWH->dwBytesRecorded); /* clear it out */
                }


                if (inPostUnprep(n, FALSE, bufLen, FALSE))
                {
                    pWH = inPrePrep(n, bufLen);
                    ret = waveInPrepareHeader(audioInH, pWH, sizeof(WAVEHDR));
                    if (ret != MMSYSERR_NOERROR)
                    {
                        showWaveError("waveInPrepareHeader", ret, recorded, __LINE__);
                    }

                    ret = waveInAddBuffer(audioInH, pWH, sizeof(WAVEHDR));
                    if (ret != MMSYSERR_NOERROR)
                    {
                        showWaveError("waveInAddBuffer", ret, recorded, __LINE__);
                        recorded++;
                    }
                }
                break;
            case WIM_CLOSE:
                bDone = true ;
                break;
            }
        }
        else
        {
            // Failed to get msg; don't spin high priority thread
            bDone = true ;
        }
    }

    closeMicDevice() ;

    bRunning = false ;

    return 0;
}

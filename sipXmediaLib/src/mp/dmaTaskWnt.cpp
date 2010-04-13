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
#include <stdio.h>
#include <string.h>
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/dmaTask.h"
#include "mp/MpBufferMsg.h"
#include "mp/MpBuf.h"
#include "mp/MpMisc.h"
#include "mp/MprToSpkr.h"
#include "mp/MpMediaTask.h"
#include "mp/dmaTask.h"
#include "os/OsMsgPool.h"

// DEFINES

// EXTERNAL FUNCTIONS
extern unsigned int __stdcall SpkrThread(LPVOID Unused) ;
extern unsigned int __stdcall MicThread(LPVOID Unused) ;
extern void closeMicDevice();
extern void closeSpeakerDevices();

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlString DmaTask::mRingDeviceName = "" ;
UtlString DmaTask::mCallDeviceName = "" ;
UtlString DmaTask::mMicDeviceName = "" ;
bool      DmaTask::mbOutputDevicesChanged = false ;
bool      DmaTask::mbInputDeviceChanged = false ;
bool      DmaTask::smIsRingerEnabled = false ;
bool      DmaTask::smIsMuted = false ;

// GLOBAL VARIABLES
MuteListenerFuncPtr DmaTask::smpMuteListener  = NULL ;
int frameCount = 0 ;
int smSpkrQPreload = N_OUT_PRIME ;
int smMicQPreload = N_IN_PRIME ;
HANDLE hMicThread;
HANDLE hSpkrThread;
pthread_t dwMicThreadID;
pthread_t dwSpkrThreadID;

/* ============================ FUNCTIONS ================================= */

int DmaTask_setSpkrQPreload(int qlen)
{
   int save = smSpkrQPreload;
   if (qlen < 1) qlen = 1;
   if (qlen > N_OUT_BUFFERS) qlen = N_OUT_BUFFERS;
   smSpkrQPreload = qlen;
   osPrintf(" Changing spkr preload from %d to %d frames\n", save, qlen);
   return save;
}

int DmaTask_setMicQPreload(int qlen)
{
   int save = smMicQPreload;
   if (qlen < 1) qlen = 1;
   if (qlen > N_IN_BUFFERS) qlen = N_IN_BUFFERS;
   smMicQPreload = qlen;
   osPrintf(" Changing mic preload from %d to %d frames\n", save, qlen);
   return save;
}

void showWaveError(char *syscall, int e, int N, int line)
{
   char *str = "(Unknown return code)";
   char *name = "(unknown)";

   if (MMSYSERR_NOERROR == e) return;
   switch (e) {
   case MMSYSERR_ALLOCATED:
      name = "MMSYSERR_ALLOCATED";
      str = "Specified resource is already allocated. ";
      break;
   case MMSYSERR_BADDEVICEID:
      name = "MMSYSERR_BADDEVICEID";
      str = "Specified device identifier is out of range. ";
      break;
   case MMSYSERR_NODRIVER:
      name = "MMSYSERR_NODRIVER";
      str = "No device driver is present. ";
      break;
   case MMSYSERR_NOMEM:
      name = "MMSYSERR_NOMEM";
      str = "Unable to allocate or lock memory. ";
      break;
   case WAVERR_BADFORMAT:
      name = "WAVERR_BADFORMAT";
      str = "Attempted to open with an unsupported waveform-audio format. ";
      break;
   case WAVERR_SYNC:
      name = "WAVERR_SYNC";
      str = "The device is synchronous but waveOutOpen was called without"
                  " using the WAVE_ALLOWSYNC flag. ";
      break;
   case MMSYSERR_INVALHANDLE:
      name = "MMSYSERR_INVALHANDLE";
      str = "Specified device handle is invalid.";
      break;
   case WAVERR_STILLPLAYING:
      name = "WAVERR_STILLPLAYING";
      str = "There are still buffers in the queue.";
      break;
   case WAVERR_UNPREPARED:
      name = "WAVERR_UNPREPARED";
      str = "The buffer pointed to by the pwh parameter has not been prepared.";
      break;
   case MMSYSERR_NOTSUPPORTED:
      name = "MMSYSERR_NOTSUPPORTED";
      str = "Specified device is synchronous and does not support pausing.";
      break;
   case MMSYSERR_INVALPARAM:
      name = "MMSYSERR_INVALPARAM";
      str = "Invalid parameter passed.";
      break;
   }
   if (-1 == N) {
      osPrintf("%s failed (line %d): res = %s (%d):\n   %s\n\n",
         syscall, line, name, e, str);
   } else {
      osPrintf("%s failed (line %d): res = %s (%d, %d):\n   %s\n\n",
        syscall, line, name, e, N, str);
   }
}

int isFormatSupported(int nChannels, int nSamplesPerSec, int nBitsPerSample)
{
   WAVEFORMATEX fmt;
   MMRESULT     res ;

   fmt.wFormatTag      = WAVE_FORMAT_PCM;
   fmt.nChannels       = nChannels;
   fmt.nSamplesPerSec  = nSamplesPerSec;
   fmt.nAvgBytesPerSec = (nChannels * nSamplesPerSec * nBitsPerSample) / 8;
   fmt.nBlockAlign     = (nChannels * nBitsPerSample) / 8;
   fmt.wBitsPerSample  = nBitsPerSample;
   fmt.cbSize          = 0;

   res = waveOutOpen(
      NULL,                   // handle can be NULL for query
      WAVE_MAPPER,            // select a device able to handle the
                              //  requested format
      &fmt,                   // format
      (DWORD) NULL,           // no callback
      (DWORD) NULL,           // no instance data
      WAVE_FORMAT_QUERY);     // query only, do not open device

   return (res == MMSYSERR_NOERROR) ;
}

int checkFormat(int nChannels, int nSamplesPerSec, int nBitsPerSample)
{
   int good;
   good = isFormatSupported(nChannels, nSamplesPerSec, nBitsPerSample);
   return good;
}

int showFrameCount(int silent)
{
    if (!silent) osPrintf("%d DMA Frames\n", frameCount, 0,0,0,0,0);
    return frameCount;
}

/************************************************************************/
/*
 * dmaStartup -- Start the threads that generate the 10ms interval signal,
 *      and that handle the audio input and output.
 */
OsStatus dmaStartup(int samplesPerFrame)
{
    if (!checkFormat(1, SAMPLES_PER_SEC, BITS_PER_SAMPLE))
    {
        osPrintf(" %3d channels, %5d samples/sec, %2d bits/sample: is NOT supported\n",
                1, SAMPLES_PER_SEC, BITS_PER_SAMPLE) ;

        // 12/16/2004: Allow thread to startup and attempt to open audio
        // channels.  It will likely fail, but the code down stream will
        // fire off alt heartbeat mechanism.
    }


    // start a thread to receive microphone input
    // mic thread will prime the device input queue
    hMicThread = (HANDLE)_beginthreadex(
            NULL,             // pointer to thread security attributes
            16000,            // initial thread stack size, in bytes
            MicThread,        // pointer to thread function
            (LPVOID) 0,       // argument for new thread
            CREATE_SUSPENDED, // creation flags
            (unsigned*)&dwMicThreadID    // pointer to returned thread identifier
    );

    assert(NULL != hMicThread);

    // start a thread to send audio out to the speaker
    // speaker thread will prime the device output queue
    hSpkrThread = (HANDLE)_beginthreadex(
            NULL,             // pointer to thread security attributes
            16000,            // initial thread stack size, in bytes
            SpkrThread,       // pointer to thread function
            (LPVOID) 0,       // argument for new thread
            CREATE_SUSPENDED, // creation flags
            (unsigned*)&dwSpkrThreadID    // pointer to returned thread identifier
    );

    assert(NULL != hSpkrThread);

    // All these threads were started with the SUSPENDED option so that
    // the following thread priority manipulations can happen without
    // yielding the CPU.  They will be resumed soon, but see the comment
    // next below...
    SetThreadPriority(hSpkrThread, THREAD_PRIORITY_TIME_CRITICAL);
    SetThreadPriority(hMicThread, THREAD_PRIORITY_TIME_CRITICAL);


    // Both the Microphone thread and the Speaker thread issue resume
    // commands for the other thread (Mic resumes Spkr, Spkr resumes Mic).
    // (Resuming a running thread is harmless).
    //
    // Exactly one of the two threads should be resumed here, and that one
    // will get the first opportunity to open its side of the audio device.
    // Once it has done so, it will issue the resume command for the other
    // thread, so that on systems with half-duplex audio devices we will be
    // consistent about which device will be opened and which will fail.
    //
    // Currently, we start the Speaker thread first, so that we will be
    // sure to open the output device on a half-duplex system.  If it is
    // decided that we want to open only the input device in such situations,
    // REPLACE the next statement with "ResumeThread(hMicThread);"

    ResumeThread(hSpkrThread);

    return OS_SUCCESS;
}


/************************************************************************/
/*
 * dmaShutdown -- Terminate the threads that generate the 10ms interval signal,
 *      and that handle the audio input and output.
 */
void dmaShutdown()
{
    PostThreadMessage(dwMicThreadID, WIM_CLOSE, 0, 0L);
    WaitForSingleObject(hMicThread, INFINITE);
    PostThreadMessage(dwSpkrThreadID, WOM_CLOSE, 0, 0L);
    WaitForSingleObject(hSpkrThread, INFINITE);
}

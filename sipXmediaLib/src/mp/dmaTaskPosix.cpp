//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifdef __pingtel_on_posix__ /* [ */
/* OK, so here's how this file is supposed to work as I understand it - To
 * start up audio hardware services, the outside world will call dmaStartup()
 * which is supposed to initialize the audio hardware, starting threads if
 * necessary (the thread that calls dmaStartup() needs it to return quickly,
 * and not be interrupted in the future by any timers dmaStartup() might set
 * up), and begin calling MpMediaTask::signalFrameStart() every 10 ms.
*/

/* Only for Linux. Needs root priveleges. */
//#define _REALTIME_LINUX_AUDIO_THREADS

//#define _JITTER_PROFILE
#define JITTER_DUMP     100000  // Dump the jitter histogram after 16.6 minutes.

/* What follows is the generic POSIX sound driver interface. OS-specific
 * details are enclosed in #ifdef blocks further below. */

#include "mp/MpMediaTask.h"
#include "mp/dmaTask.h"

/* These systems have real sound drivers which are very similar. The common
 * functionality is included in this block. */

// SYSTEM INCLUDES
#include <pthread.h> /* we're already fairly os-specific, just use pthreads */
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <semaphore.h>


// APPLICATION INCLUDES

#include "mp/MpBufferMsg.h"
#include "mp/MpBuf.h"
#include "mp/MpMisc.h"
#include "mp/MprToSpkr.h"

#define SAMPLES_PER_SEC 8000
#define MIN_SAMPLE_RATE 100
#define BITS_PER_SAMPLE 16
#define N_SAMPLES 80

#define BUFLEN (N_SAMPLES*(BITS_PER_SAMPLE>>3))

#define timediff(early, late) ((late.tv_sec-early.tv_sec)*1000000+(late.tv_usec-early.tv_usec))

// STATIC VARIABLE INITIALIZATIONS
UtlString DmaTask::mRingDeviceName = "" ;
UtlString DmaTask::mCallDeviceName = "" ;
UtlString DmaTask::mMicDeviceName = "" ;
bool      DmaTask::mbOutputDevicesChanged = false ;
bool      DmaTask::mbInputDeviceChanged = false ;
bool DmaTask::smIsRingerEnabled = false ;
bool DmaTask::smIsMuted = false ;
MuteListenerFuncPtr DmaTask::smpMuteListener  = NULL ;


/* Local variables */

static OsMsgPool* DmaMsgPool = NULL;

/* We want to keep these around to be able to shutdown... */
/* in case we should ever decide to do that. */
static pthread_mutex_t sNotifierMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sNotifierCond = PTHREAD_COND_INITIALIZER;
static struct timespec sNotifierTime;

static bool dmaOnline = 0;
static int frameCount = 1;

#ifdef _INCLUDE_AUDIO_SUPPORT /* [ */
/* Force the threads to alternate read/write */
static sem_t read_sem, write_sem;
static void startAudioSupport(void);
static void stopAudioSupport(void);
#endif /* _INCLUDE_AUDIO_SUPPORT ] */

#ifdef _JITTER_PROFILE /* [ */
static unsigned long long timeA, timeB;
static int jitterHistory[210];
static unsigned targetCpuSpeed;

static unsigned long cpuSpeed(void);
static unsigned long long getRDTSC();
#endif /* _JITTER_PROFILE ] */

// Dummy function to reference DmaMsgPool, which is not referenced if
// _INCLUDE_AUDIO_SUPPORT is not defined.
void DmaMsgPoolDummy()
{
   DmaMsgPool = DmaMsgPool;
}

int showFrameCount(int silent)
{
   if (!silent)
      osPrintf("%d DMA Frames\n", frameCount);
   return frameCount;
}

static void * mediaSignaller(void * arg)
{
#if defined(_REALTIME_LINUX_AUDIO_THREADS) && defined(__linux__) /* [ */
   struct sched_param realtime;
   int res;

   pthread_mutex_init(&sNotifierMutex, NULL);
   pthread_cond_init(&sNotifierCond, NULL);

   if(geteuid() != 0 && getuid() != 0)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING, "_REALTIME_LINUX_AUDIO_THREADS was defined but application does not have ROOT priv.");
   }
   else
   {
      // Set the priority to the maximum allowed for the scheduling polity.
      realtime.sched_priority = sched_get_priority_max(SCHED_FIFO);
      res = sched_setscheduler(0, SCHED_FIFO, &realtime);
      assert(res == 0);

      // keep all memory locked into physical mem, to guarantee realtime-behaviour
      res = mlockall(MCL_CURRENT|MCL_FUTURE);
      assert(res == 0);
   }
#endif /* _REALTIME_LINUX_AUDIO_THREADS ] */

#ifdef __linux__
   clock_gettime(CLOCK_REALTIME, &sNotifierTime);
#else
   struct timeval tv;
   gettimeofday(&tv, NULL);
   sNotifierTime.tv_sec = tv.tv_sec;
   sNotifierTime.tv_nsec = tv.tv_usec * 1000;
#endif
   pthread_mutex_lock(&sNotifierMutex);

   while(dmaOnline)
   {
      // Add 10 milliseconds onto the previous timeout
      sNotifierTime.tv_nsec = sNotifierTime.tv_nsec + 10000000;
      if (sNotifierTime.tv_nsec >= 1000000000)
      {
         ++sNotifierTime.tv_sec;
         sNotifierTime.tv_nsec -= 1000000000;
      }

      pthread_cond_timedwait(&sNotifierCond, &sNotifierMutex, &sNotifierTime);

#ifdef _JITTER_PROFILE /* [ */
      timeB = getRDTSC();
      // Determine the time delta in microseconds then convert and
      // scale to a range of 9.0 milliseconds to +29.9 milliseconds
      int jitter = timeB - timeA;
      timeA = getRDTSC();
      jitter = (jitter - 9000) / 100;
      if (jitter < 0) jitter = 0;
      if (jitter > 209) jitter = 209;
      jitterHistory[jitter]++;
      if (frameCount % JITTER_DUMP == 0)
      {
         for (int i = 0; i < 210; i++)
         {
            osPrintf("%f\t%d\n", 9.0 + (0.1 * i), jitterHistory[i]);
            jitterHistory[i] = 0;
         }
      }
#endif /* _JITTER_PROFILE ] */

      /* here's our 10ms call */
      frameCount++;
      MpMediaTask::signalFrameStart();
   }

   osPrintf(" ***********STOP!**********\n");

   pthread_mutex_unlock(&sNotifierMutex);
   pthread_mutex_destroy(&sNotifierMutex);
   pthread_cond_destroy(&sNotifierCond);

   return NULL;
}


OsStatus dmaStartup(int samplesPerFrame)
{
   int res;
   pthread_t thread;

   dmaOnline = 1;

#ifdef _JITTER_PROFILE /* [ */
   targetCpuSpeed = cpuSpeed();
   timeA = getRDTSC();
#endif /* _JITTER_PROFILE ] */

   // Start the mediaSignaller thread
   res = pthread_create(&thread, NULL, mediaSignaller, NULL);
   assert(res == 0);
   pthread_detach(thread);

#ifdef _INCLUDE_AUDIO_SUPPORT /* [ */
   startAudioSupport();
#endif /* _INCLUDE_AUDIO_SUPPORT ] */

   return OS_SUCCESS;
}

void dmaShutdown(void)
{
   if(dmaOnline)
   {
      dmaOnline = 0;
      /* make sure the thread isn't wedged */
      pthread_cond_signal(&sNotifierCond);

#ifdef _INCLUDE_AUDIO_SUPPORT /* [ */
      stopAudioSupport();
#endif /* _INCLUDE_AUDIO_SUPPORT ] */
   }
}

#ifdef _INCLUDE_AUDIO_SUPPORT /* [ */
/* This will be defined by the OS-specific section below. */
static int setupSoundCard(void);

static void * soundCardReader(void * arg)
{
   MpBufferMsg* pMsg;
   MpBufferMsg* pFlush;
   MpBufPtr ob;
   Sample* buffer;
   int recorded;
   int justRead;

   osPrintf(" **********START MIC!**********\n");

   while(dmaOnline)
   {
      ob = MpBuf_getBuf(MpMisc.UcbPool, N_SAMPLES, 0, MP_FMT_T12);
      assert(ob != NULL);
      buffer = MpBuf_getSamples(ob);
      recorded = 0;
      sem_wait(&read_sem);
      while(recorded < N_SAMPLES)
      {
         justRead = read(soundCard, &buffer[recorded], BUFLEN - (recorded * sizeof(Sample)));

         assert(justRead > 0);
         recorded += justRead/sizeof(Sample);
      }
      sem_post(&write_sem);

      if (DmaTask::isMuteEnabled())
         memset(buffer, 0, sizeof(Sample) * N_SAMPLES); /* clear it out */

      assert(recorded == N_SAMPLES);

      pMsg = (MpBufferMsg*) DmaMsgPool->findFreeMsg();
      if(pMsg == NULL)
         pMsg = new MpBufferMsg(MpBufferMsg::AUD_RECORDED);

      pMsg->setMsgSubType(MpBufferMsg::AUD_RECORDED);
      pMsg->setTag(ob);
      pMsg->setBuf(MpBuf_getSamples(ob));
      pMsg->setLen(MpBuf_getNumSamples(ob));

      if(MpMisc.pMicQ && MpMisc.pMicQ->send(*pMsg, OsTime::NO_WAIT) != OS_SUCCESS)
      {
         OsStatus  res;
         res = MpMisc.pMicQ->receive((OsMsg*&) pFlush, OsTime::NO_WAIT);
         if (OS_SUCCESS == res) {
            MpBuf_delRef(pFlush->getTag());
            pFlush->releaseMsg();
         } else {
            osPrintf("DmaTask: queue was full, now empty (5)!"
               " (res=%d)\n", res);
         }
         if(MpMisc.pMicQ->send(*pMsg, OsTime::NO_WAIT) != OS_SUCCESS)
            MpBuf_delRef(ob);
      }
      if(!pMsg->isMsgReusable())
         delete pMsg;
   }

   osPrintf(" ***********STOP!**********\n");
   return NULL;
}

static void * soundCardWriter(void * arg)
{
   struct timeval start;
   int framesPlayed;

   osPrintf(" **********START SPKR!**********\n");

   gettimeofday(&start, NULL);

   framesPlayed = 0;

   while(dmaOnline)
   {
      MpBufferMsg* pMsg;
      MpBufPtr ob;
      Sample last_buffer[N_SAMPLES] = {0};

      /* write to the card */

      struct timeval now;
      bool playFrame;

      playFrame = 0;
      gettimeofday(&now, NULL);
      if(now.tv_sec > start.tv_sec + 5)
      {
         /* we're really behind (more than 5 seconds) */
         playFrame = 1;
         start = now;
         framesPlayed = 0;
         //osPrintf("soundCardWriter resetting output synchronization\n");
      }
      else
      {
         int delta = timediff(start, now);
         /* avoid overflowing 31 bits by doing a divide by 100 first,
            then dividing by only 10000 at the end */
         int targetFrames = (delta / 100) * SAMPLES_PER_SEC / N_SAMPLES / 10000;
         if(framesPlayed <= targetFrames)
            playFrame = 1;
         if(delta > 1000000)
         {
            start.tv_sec++;
            /* SAMPLES_PER_SEC should be an integer multiple of N_SAMPLES... */
            framesPlayed -= SAMPLES_PER_SEC / N_SAMPLES;
         }
      }

      if(MpMisc.pSpkQ && MpMisc.pSpkQ->receive((OsMsg*&) pMsg, OsTime::NO_WAIT) == OS_SUCCESS)
      {
         ob = (MpBufPtr) pMsg->getTag();
         assert(ob != NULL);
         if(playFrame)
         {
            int played = 0;
            Sample* buffer = MpBuf_getSamples(ob);

            /* copy the buffer for skip protection */
            memcpy(&last_buffer[N_SAMPLES / 2], &buffer[N_SAMPLES / 2], BUFLEN / 2);

            sem_wait(&write_sem);
            while(played < N_SAMPLES)
            {
               int justWritten;
               justWritten = write(soundCard, &buffer[played], BUFLEN - (played * sizeof(Sample)));
               assert(justWritten > 0);
               played += justWritten/sizeof(Sample);
            }
            sem_post(&read_sem);
            assert(played == N_SAMPLES);
            framesPlayed++;
         }
         else
            osPrintf("soundCardWriter dropping sound packet\n");

         MpBuf_delRef(ob);
         pMsg->releaseMsg();
      }
      else if(playFrame)
      {
         int played = 0;
         //osPrintf("soundCardWriter smoothing over audio delay\n");

         /* play half of the last sample backwards, then forward again */
         for(int i = 0; i != N_SAMPLES / 2; i++)
            last_buffer[i] = last_buffer[N_SAMPLES - i - 1];

         sem_wait(&write_sem);
         while(played < N_SAMPLES)
         {
            int justWritten;
            justWritten = write(soundCard, &last_buffer[played], BUFLEN - (played * sizeof(Sample)));
            assert(justWritten > 0);
            played += justWritten/sizeof(Sample);
         }
         sem_post(&read_sem);
         assert(played == N_SAMPLES);
      }
   }

   osPrintf(" ***********STOP!**********\n");
   return NULL;
}

static void startAudioSupport(void)
{
   int res;
   pthread_t thread;

   soundCard = setupSoundCard();

   if(soundCard == -1)
      return;

   /* OsMsgPool setup */
   MpBufferMsg* pMsg = new MpBufferMsg(MpBufferMsg::AUD_RECORDED);
   DmaMsgPool = new OsMsgPool("DmaTask", *(OsMsg*)pMsg,
         40, 60, 100, 5,
         OsMsgPool::SINGLE_CLIENT);

   /* let the read thread go first */
   sem_init(&write_sem, 0, 0);
   sem_init(&read_sem, 0, 1);

   /* Start the reader and writer threads */
   res = pthread_create(&thread, NULL, soundCardReader, NULL);
   assert(res == 0);
   pthread_detach(thread);
   res = pthread_create(&thread, NULL, soundCardWriter, NULL);
   assert(res == 0);
   pthread_detach(thread);
}

static void stopAudioSupport(void)
{
   if (soundCard != -1)
   {
      /* make sure the threads aren't wedged */
      sem_post(&read_sem);
      sem_post(&write_sem);
      close(soundCard);
      soundCard = -1;
      sem_destroy(&read_sem);
      sem_destroy(&write_sem);
   }
}


/* Now the OS-specific sections... */
#ifdef __linux__ /* [ */

#include <sys/types.h>
#include <sys/soundcard.h>

static int setupSoundCard(void)
{
   int res, fd;
   int fragment = 0x00040008; /* magic number, reduces latency (0x0004 dma buffers of 2 ^ 0x0008 = 256 bytes each) */
   int samplesize = BITS_PER_SAMPLE;
   int stereo = 0; /* mono */
   int speed = SAMPLES_PER_SEC;

   fd = open("/dev/dsp", O_RDWR);
   if(fd == -1)
   {
      osPrintf("OSS: could not open /dev/dsp; *** NO SOUND! ***\n");
      return -1;
   }

   res = ioctl(fd, SNDCTL_DSP_SETDUPLEX, 0);
   if(res == -1)
   {
      osPrintf("OSS: could not set full duplex; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }

   res = ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &fragment);
   if(res == -1)
   {
      osPrintf("OSS: could not set fragment size; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }

   res = ioctl(fd, SNDCTL_DSP_SAMPLESIZE, &samplesize);
   if(res == -1 || samplesize != BITS_PER_SAMPLE)
   {
      osPrintf("OSS: could not set sample size; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }
   res = ioctl(fd, SNDCTL_DSP_STEREO, &stereo);
   if(res == -1)
   {
      osPrintf("OSS: could not set single channel audio; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }
   res = ioctl(fd, SNDCTL_DSP_SPEED, &speed);
   if(res == -1 || abs(speed - SAMPLES_PER_SEC) > SAMPLES_PER_SEC / 200) /* allow .5% variance */
   {
      osPrintf("OSS: could not set sample speed; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }

   osPrintf("OSS: /dev/dsp opened OK, %dHz %d-bit %s\n", speed, samplesize, (stereo==1) ? "stereo" : "mono");

   /* Make sure the sound card has the capabilities we need */
   res = AFMT_QUERY;
   ioctl(fd ,SNDCTL_DSP_SETFMT, &res);
   if(res != AFMT_S16_LE)
   {
      osPrintf("OSS: could not set sample format; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }
   ioctl(fd, SNDCTL_DSP_GETCAPS, &res);
   if(!(res & DSP_CAP_DUPLEX))
   {
      osPrintf("OSS: could not set full duplex; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }

   return fd;
}

#elif defined(sun) /* ] [ */

#include <sys/audio.h>

static int setupSoundCard(void)
{
   int res, fd;
   audio_info_t info;

   fd = open("/dev/audio", O_RDWR);
   if(fd == -1)
   {
      osPrintf("SUN: could not open /dev/audio; *** NO SOUND! ***\n");
      return -1;
   }

   res = fcntl(fd, F_SETFL, fcntl(soundCard, F_GETFL) & ~O_NONBLOCK);
   if(res == -1)
   {
      osPrintf("SUN: could not set blocking I/O; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }

   AUDIO_INITINFO(&info);
   info.play.sample_rate = SAMPLES_PER_SEC;
   info.play.channels = 1; /* mono */
   info.play.precision = BITS_PER_SAMPLE;
   info.play.encoding = AUDIO_ENCODING_LINEAR;
   info.record.sample_rate = SAMPLES_PER_SEC;
   info.record.channels = 1; /* mono */
   info.record.precision = BITS_PER_SAMPLE;
   info.record.encoding = AUDIO_ENCODING_LINEAR;

   res = ioctl(soundCard, AUDIO_SETINFO, &info);
   if(res == -1)
   {
      osPrintf("SUN: could not set audio parameters; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }

   return fd;
}

#elif defined(__MACH__) /* ] [ */

/* On OS X, the media library will need to be linked with the CoreAudio and AudioToolbox frameworks. */

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioConverter.h>

#define UPDATE_FREQUENCY 20

static AudioDeviceID CoreAudio_output_id;
static AudioStreamBasicDescription CoreAudio_device_desc, CoreAudio_local_desc;
static AudioConverterRef CoreAudio_output_converter, CoreAudio_input_converter;
static UInt32 CoreAudio_device_io_size;

static int CoreAudio_socket[2] = {-1, -1};

struct CoreAudio_data_desc {
   void * data;
   UInt32 size;
};

static OSStatus CoreAudio_data_proc(AudioConverterRef converter, UInt32 * size, void ** data, void * argument)
{
   struct CoreAudio_data_desc * desc = (struct CoreAudio_data_desc *) argument;
   *size = desc->size;
   *data = desc->data;
   desc->size = 0;
   return kAudioHardwareNoError;
}

static int CoreAudio_shutdown(void);

static OSStatus CoreAudio_io(AudioDeviceID CoreAudio_output_id, const AudioTimeStamp * now, const AudioBufferList * input_data, const AudioTimeStamp * input_time, AudioBufferList * output_data, const AudioTimeStamp * output_time, void * client_data)
{
   /* 16000 bytes is 1 second of 8000 Hz 16-bit mono audio */
   unsigned char buffer[16000 / UPDATE_FREQUENCY];
   struct CoreAudio_data_desc desc;
   UInt32 size;

   /* convert from native to local */
   desc.size = CoreAudio_device_io_size;
   desc.data = input_data->mBuffers[0].mData;
   size = sizeof(buffer);
   AudioConverterFillBuffer(CoreAudio_input_converter, CoreAudio_data_proc, &desc, &size, buffer);
   /* we have to reset the converter or it remembers EOF */
   AudioConverterReset(CoreAudio_input_converter);

   /* do I/O over the audio socket to the rest of the process, emulating a Linux-style /dev/dsp file descriptor */
   if(write(CoreAudio_socket[0], buffer, size) < 0)
      CoreAudio_shutdown();
   memset(buffer, 0, size);
   /* we need "size" bytes of data below, so don't reset it with the return value of read */
   read(CoreAudio_socket[0], buffer, size);

   /* convert from local to native */
   desc.size = size;
   desc.data = buffer;
   size = CoreAudio_device_io_size;
   AudioConverterFillBuffer(CoreAudio_output_converter, CoreAudio_data_proc, &desc, &size, output_data->mBuffers[0].mData);
   /* we have to reset the converter or it remembers EOF */
   AudioConverterReset(CoreAudio_output_converter);

   return kAudioHardwareNoError;
}

static int setupSoundCard(void)
{
   UInt32 parm_size;
   int error;

   if(socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, CoreAudio_socket) < 0)
      goto fail;
   /* avoid getting killed by the audio callback writing to the socket if the client closes it */
   signal(SIGPIPE, SIG_IGN);

   parm_size = sizeof(CoreAudio_output_id);
   error = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &parm_size, &CoreAudio_output_id);
   if(error != kAudioHardwareNoError)
      goto fail_socket;
   if(CoreAudio_output_id == kAudioDeviceUnknown)
      goto fail_socket;

   parm_size = sizeof(CoreAudio_device_desc);
   error = AudioDeviceGetProperty(CoreAudio_output_id, 0, 0, kAudioDevicePropertyStreamFormat, &parm_size, &CoreAudio_device_desc);
   if(error != kAudioHardwareNoError)
      goto fail_socket;

   CoreAudio_local_desc = CoreAudio_device_desc;
   CoreAudio_local_desc.mSampleRate = 8000;
   CoreAudio_local_desc.mFormatID = kAudioFormatLinearPCM;
   CoreAudio_local_desc.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsBigEndian | kLinearPCMFormatFlagIsPacked;
   CoreAudio_local_desc.mBytesPerPacket = 2;
   CoreAudio_local_desc.mFramesPerPacket = 1;
   CoreAudio_local_desc.mBytesPerFrame = 2;
   CoreAudio_local_desc.mChannelsPerFrame = 1;
   CoreAudio_local_desc.mBitsPerChannel = 16;

   error = AudioConverterNew(&CoreAudio_device_desc, &CoreAudio_local_desc, &CoreAudio_input_converter);
   if(error != kAudioHardwareNoError)
      goto fail_socket;
   error = AudioConverterNew(&CoreAudio_local_desc, &CoreAudio_device_desc, &CoreAudio_output_converter);
   if(error != kAudioHardwareNoError)
      goto fail_convert_input;

   parm_size = sizeof(CoreAudio_device_io_size);
   error = AudioDeviceGetProperty(CoreAudio_output_id, 0, 0, kAudioDevicePropertyBufferSize, &parm_size, &CoreAudio_device_io_size);
   if(error != kAudioHardwareNoError)
      goto fail_convert_output;

   CoreAudio_device_io_size = ((int) CoreAudio_device_desc.mSampleRate / UPDATE_FREQUENCY) * CoreAudio_device_desc.mBytesPerFrame;
   error = AudioDeviceSetProperty(CoreAudio_output_id, NULL, 0, 0, kAudioDevicePropertyBufferSize, parm_size, &CoreAudio_device_io_size);
   if(error != kAudioHardwareNoError)
      goto fail_convert_output;

   error = AudioDeviceAddIOProc(CoreAudio_output_id, CoreAudio_io, NULL);
   if(error != kAudioHardwareNoError)
      goto fail_convert_output;

   error = AudioDeviceStart(CoreAudio_output_id, CoreAudio_io);
   if(error != kAudioHardwareNoError)
      goto fail_io_proc;

   return CoreAudio_socket[1];

fail_io_proc:
   AudioDeviceRemoveIOProc(CoreAudio_output_id, CoreAudio_io);
fail_convert_output:
   AudioConverterDispose(CoreAudio_output_converter);
fail_convert_input:
   AudioConverterDispose(CoreAudio_input_converter);
fail_socket:
   close(CoreAudio_socket[0]);
   close(CoreAudio_socket[1]);
fail:
   return -1;
}

static int CoreAudio_shutdown(void)
{
   AudioDeviceStop(CoreAudio_output_id, CoreAudio_io);
   AudioDeviceRemoveIOProc(CoreAudio_output_id, CoreAudio_io);
   AudioConverterDispose(CoreAudio_output_converter);
   AudioConverterDispose(CoreAudio_input_converter);

   /* the other side was closed by the client */
   if (CoreAudio_socket[0] != -1)
   {
      close(CoreAudio_socket[0]);
   }

   return 0;
}

#else /* ] [ */

#warning No POSIX sound driver available; building only the dummy sound driver.

OsStatus dmaStartup(int samplesPerFrame)
{
}

void dmaShutdown(void)
{
}

#endif /* __linux__, sun, __MACH__ ] */

#endif /* _INCLUDE_AUDIO_SUPPORT ] */


#ifdef _JITTER_PROFILE /* [ */
/* determines first the CPU frequency which is then stored in a global
   variable ( cpu_hz) which is needed to get accurate timing information
   through the Pentium RDTSC instruction
*/

unsigned long cpuSpeed(void)
{
   FILE *cpuinfo;
   char entry[128];
   unsigned long speed;


   cpuinfo = fopen("/proc/cpuinfo","r");
   if (cpuinfo == NULL)
   {
      osPrintf("cpuSpeed - Unable to open \"/proc/cpuinfo\"\n");
      return 0;
   }

   while (TRUE)
   {
      // Read /proc/cpuinfo entries until "cpu MHz" line is found.
      if (fgets(entry, 127, cpuinfo) == NULL) break;

      if(!memcmp(entry, "cpu MHz", 7))
      {
         // Entry found, read speed.
         speed = atoi(&entry[10]);
         fclose(cpuinfo);
         osPrintf("cpuSpeed - CPU Speed = %luMHz\n", speed);
         return speed;
      }
   }

   // Unable to find CPU Speed info!
   fclose(cpuinfo);
   osPrintf("cpuSpeed - Unable to determind CPU Speed!\n");
   return 0;
}


// Return the elapsed time in milliseconds since the CPU came out of reset.
// The value is determined by reading the Pentium Realtime Stamp Counter
// and then factoring in the actual speed of the target CPU.
unsigned long long getRDTSC()
{
   unsigned long long rdtsc;
   __asm__ volatile (".byte 0x0f, 0x31" : "=A" (rdtsc));

   return (rdtsc / targetCpuSpeed);
}

#endif /* _JITTER_PROFILE ] */

#endif /* __pingtel_on_posix__ ] */

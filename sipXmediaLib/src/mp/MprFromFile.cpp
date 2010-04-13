//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>
#include <stdio.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsEvent.h"
#include "mp/MpBuf.h"
#include "mp/MprFromFile.h"
#include "mp/MpAudioAbstract.h"
#include "mp/MpAudioFileOpen.h"
#include "mp/MpAudioUtils.h"
#include "mp/MpAudioWaveFileRead.h"
#include "mp/mpau.h"
#include "mp/MpCallFlowGraph.h"
#include "os/OsSysLog.h"
#include "os/OsProtectEventMgr.h"

#include <os/fstream>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
static const unsigned int MAXFILESIZE = 500000;
static const unsigned int MINFILESIZE = 8000;
static const int NO_WAIT = 0;
extern int      samplesPerSecond;
extern int      bitsPerSample;
extern int      samplesPerFrame;

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprFromFile::MprFromFile(const UtlString& rName,
                           int samplesPerFrame, int samplesPerSec)
:  MpResource(rName, 0, 1, 1, 1, samplesPerFrame, samplesPerSec),
   mpFileBuffer(NULL),
   mFileRepeat(FALSE),
   mpNotify(NULL)
{
}

// Destructor
MprFromFile::~MprFromFile()
{
   if(mpFileBuffer) delete mpFileBuffer;
   mpFileBuffer = NULL;
}

/* ============================ MANIPULATORS ============================== */

#define FROM_FILE_READ_BUFFER_SIZE (8000)
#define INIT_BUFFER_LEN (size_t(FROM_FILE_READ_BUFFER_SIZE * 2 * 10))

OsStatus MprFromFile::playBuffer(const char* audioBuffer, unsigned long bufSize,
                                 int type, UtlBoolean repeat, OsProtectedEvent* notify)
{
    OsStatus res = OS_INVALID_ARGUMENT;
    UtlString* buffer;

    char *convertedBuffer;

    buffer = new UtlString();

    if (buffer)
    {
        switch(type)
        {
            case 0 : buffer->append(audioBuffer,bufSize);
            break;

            case 1 : convertedBuffer = new char(bufSize*2);

                     //TODO: actually convert the buffer

                     buffer->append(convertedBuffer,bufSize);

                     delete [] convertedBuffer;
                     break;
        }

        // Tell CpCall that we've copied the data out of the buffer, so it
        // can continue processing.
        if (OS_ALREADY_SIGNALED == notify->signal(0))
        {
           OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
           eventMgr->release(notify);
        }

        // Don't pass the evnet in the PLAY_FILE message.
        // That means that the file-play process can't pass signals
        // back.  But we have already released the OsProtectedEvent.
        MpFlowGraphMsg msg(PLAY_FILE, this, NULL, buffer,
                repeat ? PLAY_REPEAT : PLAY_ONCE, 0);
        res = postMessage(msg);
    }

    return res;
}

// play file w/ file name & repeat option
OsStatus MprFromFile::playFile(const char* audioFileName, UtlBoolean repeat,
   OsNotification* notify)
{
    char* charBuffer;
    FILE* audioFilePtr = NULL;
    OsStatus res = OS_INVALID_ARGUMENT;
    UtlString* buffer = NULL;
    int iTotalChannels = 1;
    unsigned long filesize;
    unsigned long trueFilesize;
    int huhWhat;
    int compressionType = 0;
    int channelsMin = 1, channelsMax = 2, channelsPreferred = 0;
    long rateMin = 8000, rateMax = 44100, ratePreferred = 22050;
    UtlBoolean bDetectedFormatIsOk = TRUE;
    MpAudioAbstract *audioFile = NULL;

    if (!audioFileName)
        return res;

    ifstream inputFile(audioFileName,ios::in|ios::binary);

    if (!inputFile)
    {
        return res;
    }

    //get file size
    inputFile.seekg(0,ios::end);
    filesize = trueFilesize = inputFile.tellg();
    inputFile.seekg(0);

   if (trueFilesize < sizeof(AudioSample))  //we have to have at least one sample to play
   {
      osPrintf("WARNING: %s contains less than one sample to play. Skipping play.\n",
         audioFileName);
      return res;
   }

    if (trueFilesize > MAXFILESIZE)
    {
        osPrintf("playFile('%s') WARNING:\n"
            "    length (%lu) exceeds size limit (%d)\n",
            audioFileName, trueFilesize, MAXFILESIZE);
        filesize = MAXFILESIZE;
    }

    if (trueFilesize < MINFILESIZE)
    {
        osPrintf("playFile('%s') WARNING:\n"
            "    length (%lu) is suspiciously short!\n",
            audioFileName, trueFilesize);
    }


    audioFile = MpOpenFormat(inputFile);

    //if we have an audioFile object, then it must be a known file type
    //otherwise, lets treat it as RAW
    if (audioFile)
    {
        if (audioFile->isOk())
        {
            audioFile->minMaxChannels(&channelsMin,
                                &channelsMax, &channelsPreferred);

            if (channelsMin > channelsMax)
            {
                osPrintf("Couldn't negotiate channels.\n");
                bDetectedFormatIsOk = FALSE;
            }

            audioFile->minMaxSamplingRate(&rateMin,&rateMax,&ratePreferred);
            if (rateMin > rateMax)
            {
                osPrintf("Couldn't negotiate rate.\n");
                bDetectedFormatIsOk = FALSE;
            }
        }
        else
            bDetectedFormatIsOk = FALSE;

        if (bDetectedFormatIsOk)
        {
            iTotalChannels = channelsPreferred;

            compressionType = audioFile->getDecompressionType();
#if 0
            osPrintf("compression = %d\n", compressionType);
#endif
        }
        else
            osPrintf("\nERROR: Could not detect format correctly. Should be AU or WAV or RAW\n");


        // First, figure out which kind of file it is
        if (bDetectedFormatIsOk && audioFile->getAudioFormat() == AUDIO_FORMAT_WAV)
        {
#if 0
            osPrintf("AudioFile: It's WAV file\n");
#endif

            switch(compressionType)
            {
                case MpAudioWaveFileRead::DePcm8Unsigned: //8


                    charBuffer = (char*)malloc(filesize*sizeof(AudioSample));
                    huhWhat = audioFile->getSamples((AudioSample*)charBuffer,
                                           filesize*sizeof(AudioSample));

                    if (huhWhat)
                    {

                        //it's now longer since it's 16 bit now
                        filesize *=sizeof(AudioSample);

                        //CHANGE TO MONO IF NEEDED
                        if (channelsPreferred > 1)
                            filesize = mergeChannels(charBuffer, filesize, iTotalChannels);

                        //RESAMPLE IF NEEDED
                        if (ratePreferred > 8000)
                            filesize = reSample(charBuffer, filesize, ratePreferred, 8000);

                        if (buffer == NULL)
                           buffer = new UtlString();

                        if (buffer)
                        {
                            buffer->append(charBuffer, filesize);
#if 0
                            osPrintf("Buffer length: %d\n", buffer->length());
#endif
                        }
                        free(charBuffer);
                    }
                    else
                    {
                        if (notify) notify->signal(INVALID_SETUP);
                        free(charBuffer);
                    }

                    break;

                case MpAudioWaveFileRead::DePcm16LsbSigned: // 16
                    charBuffer = (char*)malloc(filesize);
                    huhWhat = audioFile->getSamples((AudioSample*)charBuffer,
                                        filesize/sizeof(AudioSample));
                    if (huhWhat)
                    {
                        if (iTotalChannels > 1)
                            filesize = mergeChannels(charBuffer, filesize, iTotalChannels);

                        if (ratePreferred > 8000)
                            filesize = reSample(charBuffer, filesize, ratePreferred, 8000);

                        if (buffer == NULL)
                            buffer = new UtlString();
                        if (buffer)
                        {
                            buffer->append(charBuffer, filesize);
#if 0
                            osPrintf("Buffer length: %d\n", buffer->length());
#endif
                        }
                        free(charBuffer);
                    }
                    else
                    {
                        if (notify) notify->signal(INVALID_SETUP);
                        free(charBuffer);
                    }
                    break;

            }
        }
        else
        if (bDetectedFormatIsOk && audioFile->getAudioFormat() == AUDIO_FORMAT_AU)
        {
#if 0
            osPrintf("AudioFile: It's a AU file\n");
#endif
            switch(compressionType)
            {
                case MpAuRead::DePcm8Unsigned:
                    break; //do nothing for this format

                case MpAuRead::DeG711MuLaw:
                    charBuffer = (char*)malloc(filesize*2);
                    huhWhat = audioFile->getSamples((AudioSample*)charBuffer, filesize);
                    if (huhWhat)
                    {

                        //it's now 16 bit so it's twice as long
                        filesize *= sizeof(AudioSample);

                        if (channelsPreferred > 1)
                            filesize = mergeChannels(charBuffer, filesize, iTotalChannels);

                        if (ratePreferred > 8000)
                            filesize = reSample(charBuffer, filesize, ratePreferred, 8000);

                        if (buffer == NULL)
                            buffer = new UtlString();

                        if (buffer)
                        {
                            buffer->append(charBuffer, filesize);
#if 0
                            osPrintf("Buffer length: %d\n", buffer->length());
#endif
                        }
                        free(charBuffer);
                    }
                    else
                    {
                        if (notify)
                            notify->signal(INVALID_SETUP);
                        free(charBuffer);
                    }
                    break;

                case MpAuRead::DePcm16MsbSigned:
                    charBuffer = (char*)malloc(filesize);
                    huhWhat = audioFile->getSamples((AudioSample*)charBuffer, filesize/2);
                    if (huhWhat)
                    {
                        if (channelsPreferred > 1)
                            filesize = mergeChannels(charBuffer, filesize, iTotalChannels);

                        if (ratePreferred > 8000)
                            filesize = reSample(charBuffer, filesize, ratePreferred, 8000);

                        if (buffer == NULL)
                           buffer = new UtlString();
                        if (buffer)
                        {
                            buffer->append(charBuffer, filesize);
#if 0
                            osPrintf("Buffer length: %d\n", buffer->length());
#endif
                        }

                        free(charBuffer);
                    }
                    else
                    {
                        if (notify) notify->signal(INVALID_SETUP);
                        free(charBuffer);
                    }
                    break;
            }
        }
        else
            OsSysLog::add(FAC_MP, PRI_ERR, "ERROR: Detected audio file is bad.  Must be MONO, 16bit signed wav or u-law au");

        //remove object used to determine rate, compression, etc.
        delete audioFile;
        audioFile = NULL;

    }
    else
    {

#if 0
        osPrintf("AudioFile: raw file\n");
#endif

     // if we cannot determine the format of the audio file,
     // and if the ext of the file is .ulaw, we assume it is a no-header
     // raw file of ulaw audio, 8 bit, 8kHZ.
        if (strstr(audioFileName, ".ulaw"))
        {
            ratePreferred = 8000;
            channelsPreferred = 1;
            audioFile = new MpAuRead(inputFile, 1);
            if (audioFile)
            {
                long size = filesize*sizeof(AudioSample);
                charBuffer = (char*)malloc(size);

                huhWhat = audioFile->getSamples((AudioSample*)charBuffer, filesize);
                if (huhWhat)
                {

                    filesize *= sizeof(AudioSample);

                    if (channelsPreferred > 1)
                        filesize = mergeChannels(charBuffer, filesize, iTotalChannels);

                    if (ratePreferred > 8000)
                        filesize = reSample(charBuffer, filesize, ratePreferred, 8000);

                    if(buffer == NULL)
                        buffer = new UtlString();

                    if (buffer)
                    {
                        buffer->append(charBuffer, filesize);
#if 0
                        osPrintf("Buffer length: %d\n", buffer->length());
#endif
                    }
                    free(charBuffer);
                }
                else
                {
                    if (notify) notify->signal(INVALID_SETUP);
                    free(charBuffer);
                }
            }
        }

        else
        if (0 != (audioFilePtr = fopen(audioFileName, "rb")))
        {
            charBuffer = (char*)malloc(FROM_FILE_READ_BUFFER_SIZE);
            int bytesRead;
            buffer = new UtlString();

            if (buffer)
            {
                buffer->capacity(filesize);

                while(((buffer->length() < filesize) &&
                    ((bytesRead = fread(charBuffer, 1,
                    FROM_FILE_READ_BUFFER_SIZE, audioFilePtr)) > 0)))
                {
                    buffer->append(charBuffer, bytesRead);
                }

#if 0
                osPrintf("Buffer length: %d\n", buffer->length());
#endif
            }

            free(charBuffer);
            fclose(audioFilePtr);
        }
    }


    //create a msg from the buffer
    if (buffer && buffer->length())
    {
        MpFlowGraphMsg msg(PLAY_FILE, this, notify, buffer,
            repeat ? PLAY_REPEAT : PLAY_ONCE, 0);

        //now post the msg (with the audio data) to be played
        res = postMessage(msg);
    }

    return res;
}

// stop file play
OsStatus MprFromFile::stopFile(void)
{
   MpFlowGraphMsg msg(STOP_FILE, this, NULL, NULL, 0, 0);
   return postMessage(msg);
}

// $$$ These enable and disable routines need more thought, as part of
// $$$ the entire notification scheme.

UtlBoolean MprFromFile::enable(void) //$$$
{
   if (mpNotify) {
      mpNotify->signal(PLAYING);
   }
   return MpResource::enable();
}

UtlBoolean MprFromFile::disable(void) //$$$
{
   if (mpNotify) {
      mpNotify->signal(PLAY_STOPPED);
      mpNotify->signal(PLAY_FINISHED);
      mpNotify = NULL;
   }
   return MpResource::disable();
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprFromFile::doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond)
{
   MpBufPtr out = NULL;
   Sample *outbuf;
   int count;
   int bytesLeft;

   if (0 == outBufsSize) return FALSE;
   *outBufs = NULL;
   if (0 == samplesPerFrame) return FALSE;
   if (isEnabled) {
      out = MpBuf_getBuf(MpMisc.UcbPool, samplesPerFrame, 0, MP_FMT_T12);
      assert(NULL != out);
      count = MpBuf_getByteLen(out) / sizeof(Sample);
      count = min(samplesPerFrame, count);
      MpBuf_setNumSamples(out, count);
      MpBuf_setSpeech(out, MP_SPEECH_TONE);
      outbuf = MpBuf_getSamples(out);

      int bytesPerFrame = count * sizeof(Sample);
      if(mpFileBuffer)
      {
          int bufferLength = mpFileBuffer->length();
          int totalBytesRead = 0;
#if 0
          if(mFileBufferIndex == 0) {
             osPrintf("MprFromFile: buffer frameSize: %d\n", bytesPerFrame);
          }
#endif
          if(mFileBufferIndex < bufferLength)
          {
              totalBytesRead = bufferLength - mFileBufferIndex;
              totalBytesRead = min(totalBytesRead, bytesPerFrame);
              memcpy(outbuf, &(mpFileBuffer->data()[mFileBufferIndex]),
                 totalBytesRead);
              mFileBufferIndex += totalBytesRead;

          }
          if(totalBytesRead != bytesPerFrame &&
              mFileBufferIndex < bufferLength)
                osPrintf("MprFromFile: only got %d bytes from buffer\n",
                    totalBytesRead);

          if (mFileRepeat) {
             bytesLeft = 1;
             while((totalBytesRead < bytesPerFrame) && (bytesLeft > 0))
             {
#if 0
                 osPrintf("MprFromFile: repeating buffer frameSize: %d\n",
                    bytesPerFrame);
#endif
                 mFileBufferIndex = 0;
                 bytesLeft = min(bufferLength - mFileBufferIndex,
                    bytesPerFrame - totalBytesRead);
                 memcpy(&outbuf[(totalBytesRead/sizeof(Sample))],
                    &(mpFileBuffer->data()[mFileBufferIndex]), bytesLeft);
                 totalBytesRead += bytesLeft;
                 mFileBufferIndex += bytesLeft;
             }
          } else {
             if (mFileBufferIndex >= bufferLength) {
                bytesLeft = bytesPerFrame - totalBytesRead;
                memset(&outbuf[(totalBytesRead/sizeof(Sample))], 0, bytesLeft);
                MpCallFlowGraph* mpMyFG = (MpCallFlowGraph*) getFlowGraph();
                mpMyFG->stopFile(0);
                disable();
             }
          }
      }
   }
   if (NULL == out) {
      out = *inBufs;
      *inBufs = NULL;
   }
   *outBufs = out;

   return (TRUE);
}

// Handle messages for this resource.

UtlBoolean MprFromFile::handleSetup(MpFlowGraphMsg& rMsg)
{
   if(mpFileBuffer) delete mpFileBuffer;
   if (mpNotify) {
      mpNotify->signal(PLAY_FINISHED);
   }
   mpNotify = (OsNotification*) rMsg.getPtr1();
   mpFileBuffer = (UtlString*) rMsg.getPtr2();
   if(mpFileBuffer) {
#if 0
      osPrintf("File buffer %d bytes long\n", mpFileBuffer->length());
#endif
      mFileBufferIndex = 0;
      mFileRepeat = (rMsg.getInt1() == PLAY_ONCE) ? FALSE : TRUE;
   }
   return TRUE;
}

UtlBoolean MprFromFile::handleStop()
{
   delete mpFileBuffer;
   mpFileBuffer = NULL;
   mFileBufferIndex = 0;
   disable();
   return TRUE;
}

UtlBoolean MprFromFile::handleMessage(MpFlowGraphMsg& rMsg)
{
   switch (rMsg.getMsg()) {
   case PLAY_FILE:
      return handleSetup(rMsg);
      break;

   case STOP_FILE:
      return handleStop();
      break;

   default:
      return MpResource::handleMessage(rMsg);
      break;
   }
   return TRUE;
}

/* ============================ FUNCTIONS ================================= */

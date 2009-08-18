//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/fstream>
#include <os/OsSysLog.h>
#include <mp/MpAudioFileOpen.h>
#include <mp/MpAudioWaveFileRead.h>
#include <mp/MpRawAudioBuffer.h>
#include <mp/MpAudioUtils.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/**
 * Default constructor
 */
MpRawAudioBuffer::MpRawAudioBuffer(const char* pFilePath)
{
   MpAudioAbstract* pAudioAbstract;
   long minRate, maxRate, prefRate;
   int  minChan, maxChan, prefChan;
   int  decompressionType;

   mpAudioBuffer = NULL;
   mAudioBufferLength = 0;

   // Open an IO stream to the audio file
   ifstream sourceFile(pFilePath, ios::in | ios::binary);

   pAudioAbstract = MpOpenFormat(sourceFile);
   if (pAudioAbstract == NULL) {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpRawAudioBuffer::MpRawAudioBuffer(%s) - Failed to load file", pFilePath);
      return;
   }

   // Now validate the format. It must be a WAV file formated as
   // unsigned 16 bit mono at 8KHz
   if (pAudioAbstract->getAudioFormat() != AUDIO_FORMAT_WAV) {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpRawAudioBuffer::MpRawAudioBuffer(%s) Not in WAV format", pFilePath);
      return;
   }

   // Now validate the sampling rate, number of channels and decompression type
   pAudioAbstract->minMaxSamplingRate(&minRate, &maxRate, &prefRate);
   if (prefRate != 8000) {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpRawAudioBuffer::MpRawAudioBuffer(%s) Invalid sampling rate: %d",
                    pFilePath, (int) prefRate);
      return;
   }

   pAudioAbstract->minMaxChannels(&minChan, &maxChan, &prefChan);
   if (prefChan != 1) {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpRawAudioBuffer::MpRawAudioBuffer(%s) Invalid number of channels: %d",
                    pFilePath, prefChan);
      return;
   }

   decompressionType = pAudioAbstract->getDecompressionType();
   if (decompressionType != MpAudioWaveFileRead:: DePcm16LsbSigned) {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpRawAudioBuffer::MpRawAudioBuffer(%s) Data not PCM 16bit Signed", pFilePath);
      return;
   }

   // Determine the audio data size and allocate storage for it
   mAudioBufferLength = pAudioAbstract->getBytesSize();
   mpAudioBuffer = new char[mAudioBufferLength];
   if (mpAudioBuffer == NULL) {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpRawAudioBuffer::MpRawAudioBuffer(%s) - Failed to allocate storage: new char[%d]",
                    pFilePath, (int) mAudioBufferLength);
      mAudioBufferLength = 0;
      return;
   }

   // Finally read in the data
   unsigned long readLen = pAudioAbstract->readBytes(reinterpret_cast<AudioByte*>(mpAudioBuffer), mAudioBufferLength);
   if (readLen != mAudioBufferLength) {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpRawAudioBuffer::MpRawAudioBuffer(%s) Failed to read audio data", pFilePath);
      mAudioBufferLength = 0;
      delete[] mpAudioBuffer;
   }
#ifdef __BIG_ENDIAN__
   // We are running on a big endian processor - raw audio 16-bit samples are
   // in the little endian byte order - convert them to big endian.
   unsigned short *pData = (unsigned short *)mpAudioBuffer;
   for ( unsigned long index = 0; index < ( mAudioBufferLength / 2 ); index++, pData++ )
      *pData = letohs(*pData);
#endif
}

/**
 * Destructor
 */
MpRawAudioBuffer::~MpRawAudioBuffer()
{
   mAudioBufferLength = 0;

   if (mpAudioBuffer)
      delete[] mpAudioBuffer;
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */
char* MpRawAudioBuffer::getAudio(char*& prAudio, unsigned long& rLength)
{
   prAudio  = mpAudioBuffer;
   rLength = mAudioBufferLength;

   return mpAudioBuffer;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

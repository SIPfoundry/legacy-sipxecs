//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef MP_AUDIO_WAVE_FILE_READ_H
#define MP_AUDIO_WAVE_FILE_READ_H

#include "mp/MpAudioAbstract.h"
#include "mp/MpAudioFileDecompress.h"

bool isWaveFile(istream &file);

class MpAudioWaveFileRead: public MpAudioAbstract {
public:
   enum WaveDecompressionType
   {
      DePcm8Unsigned = 1,
      DePcm16LsbSigned,
      DeG711ALaw = 6,
      DeG711MuLaw = 7,
      DeImaAdpcm = 17
   };

/* ============================ CREATORS ================================== */
   MpAudioWaveFileRead(istream & s);
   //: constructor form a stream
   ~MpAudioWaveFileRead();
   //: destructor
/* ============================ MANIPULATORS ============================== */
   size_t readBytes(AudioByte *buffer, size_t numSamples);

   // Access stuff
   size_t getBytesSize();

   int getDecompressionType();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   //: general private functions
   void minMaxSamplingRate(long *min, long *max, long *preferred);
   void minMaxChannels(int *min, int *max, int *preferred);
   size_t getSamples(AudioSample *buffer, size_t numSamples);
   void initializeDecompression();
   void nextChunk(void);
   //: Get next Chunk for chunk management
private:
    //: private stream, ... stuff
   istream & mStream;
   AbstractDecompressor *_decoder; // Current decompressor
   unsigned char *mpformatData; // Contents of fmt chunk
   unsigned long mformatDataLength; // length of fmt chunk

private:
    //: Chunk stuff
   // WAVE chunk stack
   struct {
      unsigned long type; // Type of chunk
      unsigned long size; // Size of chunk
      unsigned long remaining; // Bytes left to read
      bool isContainer;   // true if this is a container
      unsigned long containerType; // type of container
   } _chunk[5];
   int _currentChunk; // top of stack
   unsigned long mFileSize;
};

#endif

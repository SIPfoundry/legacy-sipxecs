//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef AU_H_INCLUDED
#define AU_H_INCLUDED
#include "mp/MpAudioAbstract.h"
#include "mp/MpAudioFileDecompress.h"
#include <iostream>

bool isAuFile(istream &file);

class MpAuRead: public MpAudioAbstract {

private:
   istream & mStream;
   int m_CompressionType;
   AbstractDecompressor *_decoder;
public:
   enum AuDecompressionType
   {
      DeG711MuLaw = 1,
      DePcm8Unsigned,
      DePcm16MsbSigned
   };

   MpAuRead(istream & s, int raw = 0);

   ~MpAuRead() {
      if (_decoder) delete _decoder;
   }
   size_t getSamples(AudioSample *buffer, size_t numSamples) {
      return _decoder->getSamples(buffer,numSamples);
   }
private:
   size_t _dataLength;
public:
   size_t readBytes(AudioByte *buffer, size_t length);
   int getDecompressionType() {return m_CompressionType; }
   size_t getBytesSize();

private:
   bool _headerRead; // true if header has already been read
   int _headerChannels; // channels from header
   int _headerRate; // sampling rate from header
   void ReadHeader(void);
protected:
   void minMaxSamplingRate(long *min, long *max, long *preferred) {
      ReadHeader();
      *min = *max = *preferred = _headerRate;
   }
   void minMaxChannels(int *min, int *max, int *preferred) {
      ReadHeader();
      *min = *max = *preferred = _headerChannels;
   }
};

#endif

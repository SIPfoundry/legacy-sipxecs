//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include "os/OsDefs.h"
#include "mp/MpAudioFileDecompress.h"

/* ==========================CLASS AbstractDecompressor ===================== */
//:Constructor
AbstractDecompressor::AbstractDecompressor(MpAudioAbstract &a)
: _dataSource(a)
{
   // do nothing then
}

//:Destructor
AbstractDecompressor::~AbstractDecompressor()
{
   // do nothing then
}

size_t AbstractDecompressor::readBytes(AudioByte *buff, size_t length)
{
      return _dataSource.readBytes(buff,length);
}

void AbstractDecompressor::minMaxSamplingRate(long *, long *, long *)
{
      osPrintf("minMaxSamplingRate undefined\n");
}

void AbstractDecompressor::minMaxChannels(int *, int *, int *)
{
      osPrintf("minMaxChannels undefined\n");
}

/* ==========================CLASS DecompressPcm8Signed ===================== */
DecompressPcm8Signed::DecompressPcm8Signed(MpAudioAbstract &a)
: AbstractDecompressor(a)
{
      osPrintf("Encoding: 8-bit signed (two's complement) PCM\n");
};

size_t DecompressPcm8Signed::getSamples(AudioSample * buffer,
                                    size_t length) {
   AudioByte *byteBuff =
      reinterpret_cast<AudioByte *>(buffer);
   size_t samplesRead = readBytes(byteBuff,length);
   for(ssize_t i=samplesRead-1; i>=0; i--)
      buffer[i] = static_cast<AudioSample>(byteBuff[i])
                  << ((sizeof(AudioSample)-1)*8);
   return samplesRead;
}

/* ==========================CLASS DecompressPcm8Unsigned ===================== */

DecompressPcm8Unsigned::DecompressPcm8Unsigned(MpAudioAbstract &a)
: AbstractDecompressor(a)
{
      osPrintf("Encoding: 8-bit unsigned (excess-128) PCM\n");
};

size_t DecompressPcm8Unsigned::getSamples(AudioSample * buffer,
                                    size_t length) {
   AudioByte *byteBuff =
      reinterpret_cast<AudioByte *>(buffer);
   size_t samplesRead = readBytes(byteBuff,length);
   for(ssize_t i=samplesRead-1; i>=0; i--)
      buffer[i] = static_cast<AudioSample>(byteBuff[i] ^ 0x80)
                  << ((sizeof(AudioSample)-1)*8);
   return samplesRead;
}

/* ==========================CLASS DecompressPcm16MsbSigned ===================== */
DecompressPcm16MsbSigned::DecompressPcm16MsbSigned(MpAudioAbstract &a)
: AbstractDecompressor(a)
{
      osPrintf("Encoding: 16-bit MSB PCM\n");
};

size_t DecompressPcm16MsbSigned::getSamples(AudioSample *buffer,
                                       size_t length) {
   AudioByte *byteBuff =
      reinterpret_cast<AudioByte *>(buffer);
   size_t read = readBytes(byteBuff,length*2)/2;
   for(ssize_t i=read-1; i>=0; i--) {
      short s = static_cast<AudioSample>(byteBuff[2*i]) << 8;
      s |= static_cast<AudioSample>(byteBuff[2*i+1]) & 255;
      buffer[i] = static_cast<AudioSample>(s)
                  << ((sizeof(AudioSample)-2)*8);
   }
   return read;
}

/* ==========================CLASS DecompressPcm16LsbSigned ===================== */
DecompressPcm16LsbSigned::DecompressPcm16LsbSigned(MpAudioAbstract &a)
: AbstractDecompressor(a)
{
      osPrintf("Encoding: 16-bit LSB PCM\n");
};

size_t DecompressPcm16LsbSigned::getSamples(AudioSample *buffer,
                                       size_t length) {
   AudioByte *byteBuff =
      reinterpret_cast<AudioByte *>(buffer);
   size_t read = readBytes(byteBuff,length*2)/2;
   for(ssize_t i=read-1; i>=0; i--) {
      short s = static_cast<AudioSample>(byteBuff[2*i+1]) << 8;
      s |= static_cast<AudioSample>(byteBuff[2*i]) & 255;
      buffer[i] = static_cast<AudioSample>(s)
                  << ((sizeof(AudioSample)-2)*8);
   }
   return read;
}

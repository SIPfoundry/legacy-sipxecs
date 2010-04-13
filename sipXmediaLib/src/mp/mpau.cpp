//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include "mp/MpAudioFileUtils.h"
#include "mp/mpau.h"
#include "mp/MpAudioAbstract.h"
#include "mp/MpAudioFileDecompress.h"
#include "os/OsDefs.h"

MpAuRead::MpAuRead(istream & s, int raw): MpAudioAbstract(), mStream(s)
{
   osPrintf("File Format: AU\n");
   if (raw)
      _decoder = new DecompressG711MuLaw(*this);
   else
	  _decoder = 0;

  _headerRead = false;
}

bool isAuFile(istream &file)
{
   file.seekg(0);  // Seek to beginning
   int32_t magic = readIntMsb(file,4);
   return (magic == 0x2E736E64); // Should be `.snd'
}

size_t MpAuRead::readBytes(AudioByte *buffer, size_t length)
{
   if (length > _dataLength) { length = _dataLength; }
   mStream.read(reinterpret_cast<char *>(buffer),length);
   size_t lengthRead = mStream.gcount();
   _dataLength -= lengthRead;
   return lengthRead;
}

size_t MpAuRead::getBytesSize()
{
   return _dataLength;
}

void MpAuRead::ReadHeader(void)
{
   if (_headerRead) return;
   _headerRead = true;

   char header[24];
   mStream.read(header,24);

   uint32_t magic = bytesToIntMsb(header+0,4);
   if (magic != 0x2E736E64) { // '.snd'
      osPrintf("Input file is not an AU file.\n");
      return;
   }

   uint32_t headerLength = bytesToIntMsb(header+4,4);
   _dataLength = bytesToIntMsb(header+8,4);
   uint32_t format = bytesToIntMsb(header+12,4);
   _headerRate = bytesToIntMsb(header+16,4);
   _headerChannels = bytesToIntMsb(header+20,4);
   skipBytes(mStream,headerLength - 24); // Junk rest of header

   // Create an appropriate decompression object
   m_CompressionType = format;

   switch(format) {
   case 1: // ITU G.711 mu-Law
      _decoder = new DecompressG711MuLaw(*this);
      break;
   case 2: // 8-bit linear
      _decoder = new DecompressPcm8Unsigned(*this);
      break;
   case 3: // 16-bit linear
      _decoder = new DecompressPcm16MsbSigned(*this);
      break;
   default:
      osPrintf("AU format %d not supported.\n",format);
      m_CompressionType = -1;
      return;
   }

   osPrintf("Sampling Rate: %d\n",_headerRate);
   osPrintf("Channels:      %d\n",_headerChannels);
   osPrintf("\n");
}

#if 0 /* [ */
static void WriteBuffer (ostream &out, AudioSample *buffer, int length)
{
   AudioSample *sampleBuff = buffer;
   AudioByte *byteBuff =
         reinterpret_cast<AudioByte *>(buffer);
   int i = length;
   while (i-->0) {
      Sample sample = *sampleBuff++;
      *byteBuff++ = sample >> 8;
      *byteBuff++ = sample;
   }
   out.write(reinterpret_cast<char *>(buffer),length*2);
}
#endif /* ] */

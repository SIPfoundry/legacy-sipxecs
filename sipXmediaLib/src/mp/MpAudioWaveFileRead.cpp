//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#include "config.h"

#include "mp/MpAudioWaveFileRead.h"
#include "mp/MpAudioFileDecompress.h"
#include "mp/MpAudioFileUtils.h"
#include "os/OsDefs.h"

#define ChunkName(a,b,c,d) (                 \
    ((static_cast<unsigned long>(a)&255)<<24)          \
  + ((static_cast<unsigned long>(b)&255)<<16)          \
  + ((static_cast<unsigned long>(c)&255)<<8)           \
  + ((static_cast<unsigned long>(d)&255)))

#define WAVE_CHUNK_FORMAT ChunkName('R','I','F','F')


bool isWaveFile(istream &file) {
   file.seekg(0);
   uint32_t form = readIntMsb(file,4);
   if (form != WAVE_CHUNK_FORMAT)
      return false; // Not RIFF file
   skipBytes(file,4);  // Skip chunk size
   uint32_t type = readIntMsb(file,4);
   if (type == ChunkName('W','A','V','E'))
      return true;
   return false; // RIFF file, but not WAVE file
}
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
// constructor
MpAudioWaveFileRead::MpAudioWaveFileRead(istream & s): mStream(s) {
//   osPrintf("File Format: Microsoft WAVE\n");
   _decoder = 0;
   mpformatData = 0;
   mformatDataLength = 0;
   mFileSize = 0;
   mbIsOk = true; //assume ok for now

   //find out how big the wave file is...
   s.seekg(0,ios::end);
   mFileSize = s.tellg();
   s.seekg(0,ios::beg);

   _currentChunk = -1; // Empty the stack
   nextChunk();
   // Ensure first chunk is RIFF/WAVE container
   if (  (_currentChunk != 0)
      || (_chunk[0].type != WAVE_CHUNK_FORMAT)
      || (_chunk[0].isContainer != true)
      || (_chunk[0].containerType != ChunkName('W','A','V','E'))
      )
   {
      osPrintf("Outermost chunk in WAVE file isn't RIFF!!");
      mbIsOk = false;
   }
}

// Destructor then
MpAudioWaveFileRead::~MpAudioWaveFileRead() {
   if (_decoder) delete _decoder;

   if(mpformatData) { delete[] mpformatData; }
}

/* ============================ MANIPULATORS ============================== */
// member function: readBytes
size_t MpAudioWaveFileRead::readBytes(AudioByte *buffer, size_t numBytes) {
   while (_chunk[_currentChunk].type != ChunkName('d','a','t','a')) {
      nextChunk();
      if (_currentChunk < 0) {
         osPrintf("I didn't find any sound data!?!?\n");
         mbIsOk = false;
         return 0;
      }
   }
   if (numBytes > _chunk[_currentChunk].remaining)
      numBytes = _chunk[_currentChunk].remaining;
   //mStream.read(reinterpret_cast<char *>(buffer), numBytes);
   mStream.read((char *)buffer, numBytes);
   numBytes = mStream.gcount();
   _chunk[_currentChunk].remaining -= numBytes;
   return numBytes;
}

size_t MpAudioWaveFileRead::getBytesSize()
{
   while (_chunk[_currentChunk].type != ChunkName('d','a','t','a')) {
      nextChunk();
      if (_currentChunk < 0) {
         osPrintf("I didn't find any sound data!?!?\n");
         mbIsOk = false;
         return 0;
      }
   }
   return  _chunk[_currentChunk].remaining;
}

int MpAudioWaveFileRead::getDecompressionType()
{
   int returnValue = 0;

   // Make sure we've read the fmt chunk
   while (!mpformatData)
   {
      nextChunk();
      if (_currentChunk < 0) {
         osPrintf("No `fmt' chunk found?!?!\n");
         mbIsOk = false;
         return 0;
      }
   }

   // Select decompressor based on compression type
   uint32_t type = bytesToIntLsb(mpformatData+0 , 2);

   if (type == 1)
   {  // PCM format
      uint32_t bitsPerSample = bytesToIntLsb(mpformatData+14, 2);
      if (bitsPerSample <= 8) // Wave stores 8-bit data as unsigned
         returnValue = 1;
      else if (bitsPerSample <= 16) // 16 bit data is signed
         returnValue = 2;
   }
   else
      returnValue = (int)type;

   return returnValue;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
// member function: minMaxSamplingRate
void MpAudioWaveFileRead::minMaxSamplingRate(long *min, long *max, long *preferred) {
   initializeDecompression();
   uint32_t samplingRate = bytesToIntLsb(mpformatData+4,4);
   *max = *min = *preferred = samplingRate;
}

// member function: minMaxChannnels
void MpAudioWaveFileRead::minMaxChannels(int *min, int *max, int *preferred) {
   initializeDecompression();
   if (mbIsOk)
   {
        uint32_t channels = bytesToIntLsb(mpformatData+2,2);
        *min = *max = *preferred = channels;
    }
}

// member function: getSamples
size_t MpAudioWaveFileRead::getSamples(AudioSample *buffer, size_t numSamples)
{
   if (!_decoder) initializeDecompression();
   if (!_decoder) return 0; // if it is still NULL, file has unknown format.
   return _decoder->getSamples(buffer,numSamples);
}

// member function: initializeDecompression
void MpAudioWaveFileRead::initializeDecompression()
{
   if (_decoder) return;

   // Make sure we've read the fmt chunk
   while (!mpformatData)
   {
      nextChunk();

      if (_currentChunk < 0) {
         osPrintf("No `fmt' chunk found?!?!\n");
         mbIsOk = false;
      }

      if (!mbIsOk)
          return;  //something is evil... exit now
   }

   // Select decompressor based on compression type
   uint32_t type = bytesToIntLsb(mpformatData+0 , 2);

   if (type == 1)
   {  // PCM format
      uint32_t bitsPerSample = bytesToIntLsb(mpformatData+14, 2);
      if (bitsPerSample <= 8) // Wave stores 8-bit data as unsigned
         _decoder = new DecompressPcm8Unsigned(*this);
      else if (bitsPerSample <= 16) // 16 bit data is signed
         _decoder = new DecompressPcm16LsbSigned(*this);
   }

      // type = 2: MS ADPCM
   if (type == 2)
   {
      osPrintf("I don't support MS ADPCM compression.\n");
      mbIsOk = false;
   }

      // type = 17: IMA ADPCM

      // type = 6: G711ALaw
   if (type == 6)
         _decoder = new DecompressG711ALaw(*this);

      // type = 7: G711MuLaw
   if (type == 7)
         _decoder = new DecompressG711MuLaw(*this);


   if (!_decoder)
   {
      osPrintf("I don't support WAVE compression type %d\n",type);
      mbIsOk = false;
  }

}



// member function: nextChunk
void MpAudioWaveFileRead::nextChunk(void)
{
   if ((_currentChunk >= 0) && (!_chunk[_currentChunk].isContainer))
   {
      unsigned long lastChunkSize = _chunk[_currentChunk].size;
      if (lastChunkSize & 1)
      {
         _chunk[_currentChunk].remaining++;
         lastChunkSize++; // Account for padding in the container update
      }
      skipBytes(mStream,_chunk[_currentChunk].remaining); // Flush the chunk
      _currentChunk--;  // Drop chunk from the stack
      // Sanity check: containing chunk must be container
      if ((_currentChunk < 0) || (!_chunk[_currentChunk].isContainer))
      {
         osPrintf("Chunk contained in non-Container?!?!\n");
         mbIsOk = false;
      }
      // Reduce size of container
      if (_currentChunk >= 0)
      {
         // Sanity check: make sure container is big enough.
         // Also, avoid a really nasty underflow situation.
         if ((lastChunkSize+8) > _chunk[_currentChunk].remaining)
         {
            osPrintf("Error: Chunk is too large to fit in container!?!?\n");
            mbIsOk = false;
            _chunk[_currentChunk].remaining = 0; // container is empty
         }
         else
            _chunk[_currentChunk].remaining -= lastChunkSize + 8;
      }
   }

   // There may be forms that are finished, drop them too
   while (  (_currentChunk >= 0)  // there is a chunk
      &&  (_chunk[_currentChunk].remaining < 8))
   {
      skipBytes(mStream,_chunk[_currentChunk].remaining); // Flush it
      unsigned long lastChunkSize = _chunk[_currentChunk].size;
      _currentChunk--;  // Drop container chunk
      // Sanity check, containing chunk must be container
      if (!_chunk[_currentChunk].isContainer)
      {
         osPrintf("Chunk contained in non-container?!?!\n");
         mbIsOk = false;
      }
      // Reduce size of container
      if (_currentChunk >= 0)
      {
         if ((lastChunkSize+8) > _chunk[_currentChunk].remaining)
         {
            osPrintf("Error in WAVE file: Chunk is too large to fit!?!?\n");
            mbIsOk = false;
            lastChunkSize = _chunk[_currentChunk].remaining;
         }
         _chunk[_currentChunk].remaining -= lastChunkSize + 8;
      }
   }

   // Read the next chunk
   if (mStream.eof())
   {
      _currentChunk = -1; // empty the stack
      return;
   }
   uint32_t type = readIntMsb(mStream,4);
   uint32_t size = readIntLsb(mStream,4);
   if (size > mFileSize || mStream.eof())
   {
      _currentChunk = -1; // empty the stack
      mbIsOk = false;
      return;
   }

   // Put this chunk on the stack
   _currentChunk++;
   _chunk[_currentChunk].type = type;
   _chunk[_currentChunk].size = size;
   _chunk[_currentChunk].remaining = size;
   _chunk[_currentChunk].isContainer = false;
   _chunk[_currentChunk].containerType = 0;

   if ((_currentChunk >= 0) &&
      (_chunk[0].type != WAVE_CHUNK_FORMAT))
   {
      mbIsOk = false;
      _currentChunk = -1;
      return;
   }

   if (type == WAVE_CHUNK_FORMAT)
   {
      _chunk[_currentChunk].isContainer = true;
      // Need to check size of container first.
      _chunk[_currentChunk].containerType = readIntMsb(mStream,4);
      _chunk[_currentChunk].remaining -= 4;
      if (_currentChunk > 0)
      {
         osPrintf("RIFF chunk seen at inner level?!?!\n");
         mbIsOk = false;
      }
      return;
   }
   if (type == ChunkName('f','m','t',' '))
   {
      if (_currentChunk != 1)
      {
         osPrintf("FMT chunk seen at wrong level?!?!\n");
         mbIsOk = false;
      }

	  //if already allocated, then free.
	  if (mpformatData)
		  delete mpformatData;

      mpformatData = new unsigned char[size+2];
      mStream.read(reinterpret_cast<char *>(mpformatData),size);
      mformatDataLength = mStream.gcount();
      _chunk[_currentChunk].remaining = 0;
      return;
   }
   if (type == ChunkName('d','a','t','a'))
   {
      return;
   }

   if ((type & 0xFF000000) == ChunkName('I',0,0,0))
   { // First letter 'I'??
      char *text = new char[size+2];
      mStream.read(text,size);
      long length = mStream.gcount();
      _chunk[_currentChunk].remaining -= length;
      text[length] = 0;
      if (type == ChunkName('I','C','M','T')) // Comment
         osPrintf("Comment: ");
      else if (type == ChunkName('I','C','O','P')) // Copyright notice
         osPrintf("Copyright: ");
      else if (type == ChunkName('I','N','A','M')) // Name of work
         osPrintf("Title: ");
      else if (type == ChunkName('I','A','R','T')) // Name of artist
         osPrintf("Artist: ");
      else
         osPrintf("Text: "); // Other Informational chunk

	  delete [] text;

      return;
   }

   char code[5] = "CODE";
   code[0] = (type>>24)&255;   code[1] = (type>>16)&255;
   code[2] = (type>>8 )&255;   code[3] = (type    )&255;
   osPrintf("Ignoring unrecognized `");
}

//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#define _MPAUDIOUTILS_CPP_

// SYSTEM INCLUDES
#include <stdlib.h>

#ifdef WIN32 /* [ */
#include <io.h>
#include <fcntl.h>
#endif /* WIN32 ] */

#ifdef __pingtel_on_posix__
#include <unistd.h>
#include <fcntl.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsFS.h"
#include "mp/MpTypes.h"
#include "mp/MpAudioUtils.h"
#include "mp/StreamHttpDataSource.h"
#include "mp/MpAudioWaveFileRead.h"

#include <os/fstream>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* MpWaveFileFormat = "RIFF";

// STATIC VARIABLE INITIALIZATIONS

#define MAX_WAVBUF_SIZE 65535

/* ============================ FUNCTIONS ================================= */
void ConvertUnsigned8ToSigned16(unsigned char *in_buffer, Sample *out_buffer, int numBytesToConvert)
{
    for (int loop = 0; loop < numBytesToConvert;loop++)
    {
        *(out_buffer+loop) = static_cast<Sample>(*(in_buffer+loop) ^ 0x80) << 8;
    }
}

int gcd(int a, int b)
{
   int c = b;
   if(b > a)
      return gcd(b, a);
   /* a >= b */
   while(c)
   {
      c = a % b;
      a = b;
      b = c;
   }
   return a;
}

//pass in Size as bytes, and newsize in bytes is returned
int reSample(char *charBuffer,
               int Size, int CurrentSampleRate, int NewSampleRate)
{
   if (CurrentSampleRate > NewSampleRate)
   {
      /* downsampling */
      Sample * buffer = (Sample *) charBuffer;
      int keptSamples = 0, currentSample = 0;
      int rkeptSamples = 0, rcurrentSample = 0;
      int totalSamples = Size / sizeof(Sample);

      int rateGcd = gcd(CurrentSampleRate, NewSampleRate);
      CurrentSampleRate /= rateGcd;
      NewSampleRate /= rateGcd;

      for(; currentSample < totalSamples; currentSample++, rcurrentSample++)
         if (rkeptSamples * CurrentSampleRate <= rcurrentSample * NewSampleRate)
         {
            buffer[rkeptSamples++, keptSamples++] = buffer[currentSample];
            if(rkeptSamples == NewSampleRate && rcurrentSample == CurrentSampleRate)
               rkeptSamples = rcurrentSample = 0;
         }
      Size = keptSamples * sizeof(Sample);
   }
   //should really up-sample here someday...

   // If we are not actually resampling, we just fall through to here...
   return Size;
}

int mergeChannels(char * charBuffer, int Size, int nTotalChannels)
{
   Sample * buffer = (Sample *) charBuffer;

   if(nTotalChannels == 2)
   {
      int targetSamples = Size / (sizeof(Sample) * 2);
      int targetSample = 0, sourceSample = 0;

      for(; targetSample < targetSamples; targetSample++)
      {
         int mergedSample = buffer[sourceSample++];
         mergedSample += buffer[sourceSample++];
         buffer[targetSample] = mergedSample / 2;
      }

      return targetSample * sizeof(Sample);
   }
   /* Test for this afterwards, to optimize 2-channel mixing */
   else if(nTotalChannels == 1)
      return Size;

   int targetSamples = Size / (sizeof(Sample) * nTotalChannels);
   int targetSample = 0, sourceSample = 0;

   for(; targetSample < targetSamples; targetSample++)
   {
      int mergedSample = 0;
      for(int i = 0; i < nTotalChannels; i++)
         mergedSample += buffer[sourceSample++];
      buffer[targetSample] = mergedSample / nTotalChannels;
   }

   return targetSample * sizeof(Sample);
}


OsStatus WriteWaveHdr(OsFile &file)
{
    OsStatus retCode = OS_FAILED;
    char tmpbuf[80];
    uint16_t bitsPerSample = 16;  // 2 byte value for Endian conversion and WAV file

    short sampleSize = sizeof(Sample);
    uint16_t compressionCode = 1; //PCM = 2 byte value for Endian conversion
    short numChannels = 1;
    uint32_t samplesPerSecond = 8000; // 4 byte value for Endian conversion and WAV file
    uint32_t averageSamplePerSec = samplesPerSecond*sampleSize; // 4 byte value
    uint16_t blockAlign = sampleSize*numChannels;               // 2 byte value
    size_t bytesWritten = 0;
    size_t TotalBytesWritten = 0;

    //write RIFF & length
    //8 bytes written
    strcpy(tmpbuf,MpWaveFileFormat);
    uint32_t length = 0;   // 4 byte value for Endian conversion and WAV file

    file.write(tmpbuf, strlen(tmpbuf),bytesWritten);
    TotalBytesWritten += bytesWritten;

    length = htolel(length);
    file.write((char*)&length, sizeof(length),bytesWritten); //filled in on close
    TotalBytesWritten += bytesWritten;

    //write WAVE & length
    //8 bytes written
    strcpy(tmpbuf,"WAVE");

    file.write(tmpbuf, strlen(tmpbuf),bytesWritten);
    TotalBytesWritten += bytesWritten;

    //write fmt & length
    //8 bytes written
    strcpy(tmpbuf,"fmt ");
    length = 16;

    file.write(tmpbuf,strlen(tmpbuf),bytesWritten);
    TotalBytesWritten += bytesWritten;

    length = htolel(length);
    file.write((char*)&length,sizeof(length),bytesWritten); //filled in on close
    TotalBytesWritten += bytesWritten;

    //now write each piece of the format
    //16 bytes written
    compressionCode = htoles(compressionCode);
    file.write((char*)&compressionCode, sizeof(compressionCode),bytesWritten);
    TotalBytesWritten += bytesWritten;

    numChannels = htoles(numChannels);
    file.write((char*)&numChannels, sizeof(numChannels),bytesWritten);
    TotalBytesWritten += bytesWritten;

    samplesPerSecond = htolel(samplesPerSecond);
    file.write((char*)&samplesPerSecond, sizeof(samplesPerSecond),bytesWritten);
    TotalBytesWritten += bytesWritten;

    averageSamplePerSec = htolel(averageSamplePerSec);
    file.write((char*)&averageSamplePerSec, sizeof(averageSamplePerSec),bytesWritten);
    TotalBytesWritten += bytesWritten;

    blockAlign = htoles(blockAlign);
    file.write((char*)&blockAlign, sizeof(blockAlign),bytesWritten);
    TotalBytesWritten += bytesWritten;

    bitsPerSample = htoles(bitsPerSample);
    file.write((char*)&bitsPerSample, sizeof(bitsPerSample),bytesWritten);
    TotalBytesWritten += bytesWritten;


    //write data and length
    strcpy(tmpbuf,"data");
    length = 0;

    file.write(tmpbuf,strlen(tmpbuf),bytesWritten);
    TotalBytesWritten += bytesWritten;

    length = htolel(length);
    file.write((char*)&length,sizeof(length),bytesWritten); //filled in on close
    TotalBytesWritten += bytesWritten;

    //total length at this point should be 44 bytes
    if (TotalBytesWritten == 44)
        retCode = OS_SUCCESS;

    return retCode;

}


OsStatus updateWaveHeaderLengths(OsFile &file)
{
    OsStatus retCode = OS_FAILED;
    size_t bytesWritten = 0;
    //find out how many bytes were written so far
    uint32_t length; // 4 byte value for Endian conversion
    size_t temp_len;
    file.getLength(temp_len);
    length = temp_len;

    //no go back to beg
    file.setPosition(4);

    //and update the RIFF length
    uint32_t rifflength = htolel(length-8); // 4 byte value for WAV file
    file.write((char*)&rifflength,sizeof(rifflength),bytesWritten);
    if (bytesWritten == sizeof(rifflength))
    {

        //now seek to the data length
        file.setPosition(40);

        //this should be the length of just the data
        uint32_t datalength = htolel(length-44); // 4 byte value for WAV file
        file.write((char*)&datalength,sizeof(datalength),bytesWritten);

        if (bytesWritten == sizeof(datalength))
            retCode = OS_SUCCESS;
    }

    return retCode;
}

//works with 16bit samples only. (for now)
OsStatus mergeWaveUrls(UtlString rSourceUrls[], UtlString &rDestFile)
{
    OsStatus retVal = OS_FAILED;
    int index = 0;

    if (OsFileSystem::exists(rDestFile.data()))
        OsFileSystem::remove(rDestFile.data());

    OsFile file(rDestFile.data());
    file.touch();
    if (file.open(OsFile::READ_WRITE) == OS_SUCCESS)
    {
        UtlBoolean bError = FALSE;

        WriteWaveHdr(file);
        while (!bError && rSourceUrls[index] && rSourceUrls[index].length())
        {
            //for now...assume an error until write occurs ok
            bError = TRUE;

            ssize_t bytesRead = 0;
            uint32_t filesize;       // 4 byte value in WAV file
            StreamHttpDataSource reader(rSourceUrls[index].data(),0);
            if (reader.open() == OS_SUCCESS)
            {
                unsigned char chunkId[4];
                if (reader.read((char *)chunkId,4,bytesRead) == OS_SUCCESS)
                {
                    if (memcmp(chunkId,MpWaveFileFormat,4) == 0) //continue if the right format
                    {
                        //now read bytes left
                        if (reader.read((char *)&filesize,sizeof(uint32_t),bytesRead) == OS_SUCCESS)
                        {
                            filesize = letohl(filesize);
                            filesize +=8; //eight bytes we already read

                            if (reader.read((char *)chunkId,4,bytesRead) == OS_SUCCESS) //now read WAVE marker
                            {
                                if (memcmp(chunkId,"WAVE",4) == 0) //continue if RIFF
                                {
                                    if (reader.read((char *)chunkId,4,bytesRead) == OS_SUCCESS) //now read fmt marker
                                    {
                                        if (memcmp(chunkId,"fmt ",4) == 0) //continue if RIFF
                                        {
                                            //read in the fmt length (most likely 16)
                                            int32_t fmtlength;
                                            if (reader.read((char *)&fmtlength,sizeof(int32_t),bytesRead) == OS_SUCCESS)
                                            {
                                                fmtlength = letohl(fmtlength);
                                                if (bytesRead == 4)
                                                {
                                                    char fmtbuffer[100];
                                                    reader.read(fmtbuffer,fmtlength,bytesRead);
                                                }

                                                //now read in the next check
                                                if (reader.read((char *)chunkId,4,bytesRead) == OS_SUCCESS) //now read fmt marker
                                                {
                                                    bError = FALSE;

                                                    while (!bError && memcmp(chunkId,"data",4) == 0)
                                                    {
                                                        //for now...assume an error until write occurs ok
                                                        bError = TRUE;

                                                        ssize_t datalength;
                                                        if (reader.read((char *)&datalength,sizeof(int32_t),bytesRead) == OS_SUCCESS)
                                                        {
                                                            datalength = letohl(datalength);
                                                            if (bytesRead == sizeof(int32_t))
                                                            {
                                                                unsigned char *charBuffer = (unsigned char*)malloc(datalength);
                                                                size_t bytesWritten;

                                                                if (reader.read((char *)charBuffer,datalength,bytesRead) == OS_SUCCESS)
                                                                {
                                                                    if (datalength == bytesRead)
                                                                    {
                                                                        file.write(charBuffer,bytesRead,bytesWritten);

                                                                        if ((ssize_t)bytesWritten != bytesRead)
                                                                        {
                                                                           bError = TRUE;
                                                                        }
                                                                        else
                                                                        {
                                                                           bError =  FALSE;
                                                                        }
                                                                    }
                                                                }

                                                                free(charBuffer);
                                                            }

                                                        }
                                                        memset(chunkId,0,sizeof(chunkId));
                                                        reader.read((char *)chunkId,4,bytesRead);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }              //  BIG CONDITIONAL AREA HUH?
                            }
                        }
                    }
                    reader.close();
                }
            }

            index++;  //move to next file
        }

        OsStatus updateStat = updateWaveHeaderLengths(file);
        if (!bError)
        {
            retVal = updateStat;
        }

        file.close();
    }

    return retVal;
}

//works with 16bit samples only. (for now)
OsStatus mergeWaveFiles(UtlString rSourceFiles[], UtlString &rDestFile)
{
    OsStatus retVal = OS_FAILED;
    int index = 0;

    if (OsFileSystem::exists(rDestFile.data()))
        OsFileSystem::remove(rDestFile.data());

    OsFile file(rDestFile.data());
    file.touch();
    if (file.open(OsFile::READ_WRITE) == OS_SUCCESS)
    {
        UtlBoolean bError = FALSE;
        int lastCompressionType = -1;
        WriteWaveHdr(file);
        while (!bError && rSourceFiles[index] && rSourceFiles[index].length())
        {
            ifstream inputFile(rSourceFiles[index].data(),ios::in|ios::binary);
            if (inputFile)
            {
                inputFile.seekg(0,ios::end);
                long filesize = inputFile.tellg();
                inputFile.seekg(0);

                MpAudioWaveFileRead reader(inputFile);
                int compressionType = reader.getDecompressionType();

                if (lastCompressionType == -1)
                    lastCompressionType = compressionType;

                if (lastCompressionType == compressionType)
                {
                    unsigned char *charBuffer = (unsigned char*)malloc(filesize);
                    if (charBuffer)
                    {

                        size_t TotalBytesRead = 0;
                        size_t bytesRead = 0;
                        do
                        {
                            bytesRead = reader.readBytes(charBuffer+TotalBytesRead, 65535);
                            TotalBytesRead += bytesRead;
                        } while (bytesRead > 0);

                        if (TotalBytesRead > 0)
                        {
                            size_t bytesWritten;
                            file.write(charBuffer,TotalBytesRead,bytesWritten);

                            if (bytesWritten != TotalBytesRead)
                                bError = TRUE;

                        }
                        free(charBuffer);
                    }
                    else
                        bError = TRUE;

                }
                else
                    bError = TRUE;
            }
            else
                bError = TRUE;

            index++;  //move to next file
        }

        OsStatus updateStat = updateWaveHeaderLengths(file);
        if (!bError)
        {
            retVal = updateStat;
        }

        file.close();
    }

    return retVal;
}

/* The number of bits required by each value */
static unsigned char numBits[] = {
   0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
   6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
};

/* Mu-Law conversions */
static bool muLawDecodeTableInitialized = false;
static Sample muLawDecodeTable[256];
static bool aLawDecodeTableInitialized = false;
static Sample aLawDecodeTable[256];

// Constructor initializes the decoding table
void InitG711Tables()
{
   if (!muLawDecodeTableInitialized) {
      muLawDecodeTableInitialized = true;
      for(int i=0;i<256;i++)
         muLawDecodeTable[i] = MuLawDecode2(i);
   }

   if (!aLawDecodeTableInitialized) {
      aLawDecodeTableInitialized = true;
      for(int i=0;i<256;i++)
         aLawDecodeTable[i] = ALawDecode2(i);
   }
}

size_t DecompressG711MuLaw(Sample *buffer,size_t length)
{
   unsigned char *byteBuff =
      reinterpret_cast<unsigned char *>(buffer);

   for(long i=length-1; i>=0; i--)
   {
      buffer[i] = muLawDecodeTable[byteBuff[i]];
   }
   return length;
}

unsigned char MuLawEncode2(Sample s)
{
   unsigned char sign = (s<0)?0:0x80; // Save the sign
   if (s<0) s=-s; // make sample positive
   int32_t adjusted = static_cast<int32_t>(s) << (16-sizeof(Sample)*8);
   adjusted += 128L+4L;
   if (adjusted > 32767) adjusted = 32767;
   unsigned char exponent = numBits[(adjusted>>7)&0xFF] - 1;
   unsigned char mantissa = (adjusted >> (exponent + 3)) & 0xF;
   return ~(sign | (exponent << 4) | mantissa);
}

Sample MuLawDecode2(unsigned char ulaw)
{
   ulaw = ~ulaw;
   unsigned char exponent = (ulaw >> 4) & 0x7;
   unsigned char mantissa = (ulaw & 0xF) + 16;
   uint32_t adjusted = (mantissa << (exponent + 3)) - 128 - 4;
   return (ulaw & 0x80)? adjusted : -adjusted;
}


size_t DecompressG711ALaw(Sample *buffer, size_t length)
{
   unsigned char *byteBuff =
      reinterpret_cast<unsigned char *>(buffer);

   for(long i=length-1; i>=0; i--)
      buffer[i] = aLawDecodeTable[ byteBuff[i] ];
   return length;
}

unsigned char ALawEncode2(Sample s)
{
   unsigned char sign = (s<0)?0:0x80; // Save the sign
   if (s<0) s=-s; // make sample positive
   signed long adjusted = static_cast<long>(s)+8L; // Round it
   if (adjusted > 32767) adjusted = 32767; // Clip it
   unsigned char exponent = numBits[(adjusted>>8)&0x7F];
   unsigned char mantissa = (adjusted >> (exponent + 4)) & 0xF;
   return sign | (((exponent << 4) | mantissa) ^ 0x55);
}

Sample ALawDecode2(unsigned char alaw)
{
   alaw ^= 0x55;
   unsigned char exponent = (alaw >> 4) & 0x7;
   unsigned char mantissa = (alaw & 0xF) + (exponent?16:0);
   uint32_t adjusted = (mantissa << (exponent + 4));
   return (alaw & 0x80)? -adjusted : adjusted;
}

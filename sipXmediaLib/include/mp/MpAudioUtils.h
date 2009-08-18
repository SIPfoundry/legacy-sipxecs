//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _MPAUDIOUTILS_H_
#define _MPAUDIOUTILS_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "utl/UtlString.h"
#include "mp/MpTypes.h"

//Host to little endian and little endian to host conversion macros used
//for handling of wave files (RIFF).
#ifdef __BIG_ENDIAN__
#define letohs( s )                     ((((s) & 0xFF00) >> 8) | \
                                         (((s) & 0xFF) << 8))
#define letohl( l )                     ((((l) & 0xFF000000) >> 24) | \
                                         (((l) & 0xFF0000) >> 8) | \
                                         (((l) & 0xFF00) << 8) | \
                                         (((l) & 0xFF) << 24))
#define htoles( s )                     (letohs(s))
#define htolel( l )                     (letohl(l))
#else
#define letohs( s )                     (s)
#define letohl( l )                     (l)
#define htoles( s )                     (s)
#define htolel( l )                     (l)
#endif

void ConvertUnsigned8ToSigned16(unsigned char *in_buffer, Sample *out_buffer, int numBytesToConvert);

extern const char* MpWaveFileFormat;

//returns the GCD of a and b
//don't pass it negative numbers or (0, 0)
int gcd(int a, int b);

//downsamples from current rate to new rate
//doesn't upsample yet
int reSample(char * charBuffer, int numBytes, int currentSampleRate, int newSampleRate);

//merges two or more channels into one
//takes size in bytes as input.  returns new size in bytes
int mergeChannels(char * charBuffer, int Size, int nTotalChannels);

//works with 16bit wavs only.  (for now)
OsStatus mergeWaveUrls(UtlString rSourceUrls[], UtlString &rDestFile);

//works with 16bit wavs only.  (for now)
OsStatus mergeWaveFiles(UtlString rSourceFiles[], UtlString &rDestFile);

//routines for compressing & decompressing aLaw and uLaw
void InitG711Tables();
size_t DecompressG711MuLaw(Sample *buffer,size_t length);
size_t DecompressG711ALaw(Sample *buffer, size_t length);
unsigned char ALawEncode2(Sample s);
unsigned char MuLawEncode2(Sample s);
Sample MuLawDecode2(unsigned char ulaw);
Sample ALawDecode2(unsigned char alaw);

#endif // _MPAUDIOUTILS_H_

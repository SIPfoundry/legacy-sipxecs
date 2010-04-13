//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef MP_AUDIO_FILE_DECOMPRESS_H
#define MP_AUDIO_FILE_DECOMPRESS_H

// SYSTEM INCLUDES
#include <os/iostream>
// APPLICATION INCLUDES
#include "mp/MpAudioAbstract.h"

/* =========================CLASS AbstractDecompressor=========================== */
class AbstractDecompressor {
protected:
   MpAudioAbstract &_dataSource;  // The object to get raw bytes from
   size_t readBytes(AudioByte *buff, size_t length);
public:
   AbstractDecompressor(MpAudioAbstract &a);
   virtual ~AbstractDecompressor();
   virtual size_t getSamples(AudioSample *, size_t) = 0;
   virtual void minMaxSamplingRate(long *, long *, long *);
   virtual void minMaxChannels(int *, int *, int *);
};

/* ==========================CLASS DecompressPcm8Signed========================= */
//:Class DecompressPcm8Signed
class DecompressPcm8Signed: public AbstractDecompressor {
public:
   DecompressPcm8Signed(MpAudioAbstract &a);
   //:Constructor
   size_t getSamples(AudioSample * buffer, size_t length);
   //:getSamples
};

/* ==========================CLASS DecompressPcm8Unsigned======================= */
class DecompressPcm8Unsigned: public AbstractDecompressor
// class DecompressPcm8Unsigned
{
public:
   DecompressPcm8Unsigned(MpAudioAbstract &a);
   //:constructor
   size_t getSamples(AudioSample * buffer, size_t length);
   //: getSamples
};

/* ==================== CLASS DecompressPcm16MsbSigned=========================== */
// class DecompressPcm16MsbSigned
class DecompressPcm16MsbSigned: public AbstractDecompressor {
public:
   DecompressPcm16MsbSigned(MpAudioAbstract &a);
   // constructor
   size_t getSamples(AudioSample *buffer, size_t length);
   // getSamples
};

/* ==========================CLASS DecompressPcm16LsbSigned======================= */
//:DecompressPcm16LsbSigned
class DecompressPcm16LsbSigned: public AbstractDecompressor {
public:

   DecompressPcm16LsbSigned(MpAudioAbstract &a);
   //: Constructor

   size_t getSamples(AudioSample *buffer, size_t length);
};

/* ==========================CLASS DecompressG711MuLaw======================= */
//:DecompressG711MuLaw
class DecompressG711MuLaw: public AbstractDecompressor {
public:
   DecompressG711MuLaw(MpAudioAbstract &a);
   size_t getSamples(AudioSample *buffer, size_t length);
};

AudioSample MuLawDecode(AudioByte);
AudioByte MuLawEncode(AudioSample);

/* ==========================CLASS DecompressG711ALaw======================= */
//:DecompressG711ALaw

class DecompressG711ALaw: public AbstractDecompressor {
private:
   static AudioSample *_decodeTable;
public:
   DecompressG711ALaw(MpAudioAbstract &a);
   size_t getSamples(AudioSample *buffer, size_t length);
};

AudioSample ALawDecode(AudioByte);
AudioByte ALawEncode(AudioSample);

#endif

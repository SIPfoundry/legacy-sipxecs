//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpCodecInfo_h_
#define _MpCodecInfo_h_

// SYSTEM INCLUDES
#include "net/SdpCodec.h"

// APPLICATION INCLUDES

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Static information describing a codec
class MpCodecInfo
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MpCodecInfo(SdpCodec::SdpCodecTypes codecType,
               UtlString    codecVersion,
               UtlBoolean   usesNetEq,
               unsigned    samplingRate,
               unsigned    numBitsPerSample,
               unsigned    numChannels,
               unsigned    interleaveBlockSize,
               unsigned    bitRate,
               unsigned    minPacketBits,
               unsigned    avgPacketBits,
               unsigned    maxPacketBits,
               unsigned    numSamplesPerFrame,
               UtlBoolean   signalingCodec = FALSE,
               UtlBoolean   doesVadCng = FALSE);
     //:Constructor
     //!param: codecType - codec type
     //!param: codecVersion - string identifying the codec version
     //!param: usesNetEq - boolean indicating whether the codec uses GIPS NetEq
     //!param: samplingRate - sampling rate for the PCM data expected by the codec
     //!param: numBitsPerSample - number of PCM samples per input frame for this codec
     //!param: numChannels - number of channels supported by the codec
     //!param: interleaveBlockSize - size of the interleave block (in samples)
     //!param: bitRate - bit rate for this codec (in bits per second)
     //!param: minPacketBits - minimum number of bits in an encoded frame
     //!param: avgPacketBits - average number of bits in an encoded frame
     //!param: maxPacketBits - maximum number of bits in an encoded frame

   MpCodecInfo(const MpCodecInfo& rMpCodecInfo);
     //:Copy constructor

   virtual
   ~MpCodecInfo();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   SdpCodec::SdpCodecTypes getCodecType(void) const;
     //:Returns the codec type

   UtlString getCodecVersion(void) const;
     //:Returns a string identifying the codec version

   unsigned getSamplingRate(void) const;
     //:Returns the sampling rate for the PCM data expected by the codec

   unsigned getNumBitsPerSample(void) const;
     //:Returns the PCM data sample size (in bits)

   unsigned getNumSamplesPerFrame(void) const;
     //:Returns the number of PCM samples per input frame for this codec

   unsigned getNumChannels(void) const;
     //:Returns the number of channels supported by the codec

   unsigned getInterleaveBlockSize(void) const;
     //:Returns the size of the interleave block (in samples)
     // This value is not meaningful if the number of channels for the
     // codec is equal to 1.

   unsigned getBitRate(void) const;
     //:Returns the bit rate for this codec (in bits per second)
     // If the codec is variable rate, then the average expected bit rate
     // should be returned.

   unsigned getMinPacketBits(void) const;
     //:Returns the minimum number of bits in an encoded frame

   unsigned getAvgPacketBits(void) const;
     //:Returns the average number of bits in an encoded frame

   unsigned getMaxPacketBits(void) const;
     //:Returns the maximum number of bits in an encoded frame

/* ============================ INQUIRY =================================== */

   UtlBoolean usesNetEq(void) const;
     //:Returns TRUE if codec uses GIPS NetEq; otherwise returns FALSE

   UtlBoolean isSignalingCodec(void) const;
     //:Returns TRUE if codec is used for signaling; otherwise returns FALSE

   UtlBoolean doesVadCng(void) const;
     //:Returns TRUE if codec does its own VAD and CNG; otherwise returns FALSE

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SdpCodec::SdpCodecTypes mCodecType;
   UtlString    mCodecVersion;
   UtlBoolean   mUsesNetEq;
   unsigned    mSamplingRate;
   unsigned    mNumBitsPerSample;
   unsigned    mNumSamplesPerFrame;
   unsigned    mNumChannels;
   unsigned    mInterleaveBlockSize;
   unsigned    mBitRate;
   unsigned    mMinPacketBits;
   unsigned    mAvgPacketBits;
   unsigned    mMaxPacketBits;
   UtlBoolean   mIsSignalingCodec;
   UtlBoolean   mDoesVadCng;

   MpCodecInfo& operator=(const MpCodecInfo& rhs);
     //:Assignment operator
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpCodecInfo_h_

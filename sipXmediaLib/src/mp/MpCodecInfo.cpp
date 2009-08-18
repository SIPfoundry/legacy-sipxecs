//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#include "mp/MpCodecInfo.h"

MpCodecInfo::MpCodecInfo(SdpCodec::SdpCodecTypes codecType,
                           UtlString       codecVersion,
                           UtlBoolean      usesNetEq,
                           unsigned int   samplingRate,
                           unsigned int   numBitsPerSample,
                           unsigned int   numChannels,
                           unsigned int   interleaveBlockSize,
                           unsigned int   bitRate,
                           unsigned int   minPacketBits,
                           unsigned int   avgPacketBits,
                           unsigned int   maxPacketBits,
                           unsigned int   numSamplesPerFrame,
                           UtlBoolean      signalingCodec,
                           UtlBoolean      doesVadCng)
 : mCodecType(codecType),
   mCodecVersion(codecVersion),
   mUsesNetEq(usesNetEq),
   mSamplingRate(samplingRate),
   mNumBitsPerSample(numBitsPerSample),
   mNumSamplesPerFrame( numSamplesPerFrame ),
   mNumChannels(numChannels),
   mInterleaveBlockSize(interleaveBlockSize),
   mBitRate(bitRate),
   mMinPacketBits(minPacketBits),
   mAvgPacketBits(avgPacketBits),
   mMaxPacketBits(maxPacketBits),
   mIsSignalingCodec(signalingCodec),
   mDoesVadCng(doesVadCng)
{
}

//:Copy constructor
MpCodecInfo::MpCodecInfo(const MpCodecInfo& rMpCodecInfo)
{
   mCodecType=rMpCodecInfo.mCodecType;
   mCodecVersion=rMpCodecInfo.mCodecVersion;
   mUsesNetEq=rMpCodecInfo.mUsesNetEq;
   mSamplingRate=rMpCodecInfo.mSamplingRate;
   mNumBitsPerSample=rMpCodecInfo.mNumBitsPerSample;
   mNumSamplesPerFrame=rMpCodecInfo.mNumSamplesPerFrame;
   mNumChannels=rMpCodecInfo.mNumChannels;
   mInterleaveBlockSize=rMpCodecInfo.mInterleaveBlockSize;
   mBitRate=rMpCodecInfo.mBitRate;
   mMinPacketBits=rMpCodecInfo.mMinPacketBits;
   mAvgPacketBits=rMpCodecInfo.mAvgPacketBits;
   mMaxPacketBits=rMpCodecInfo.mMaxPacketBits;
   mDoesVadCng=rMpCodecInfo.mDoesVadCng;
}

MpCodecInfo::~MpCodecInfo(){}
//:Destructor

/* ============================ MANIPULATORS ============================== */

// $$$ MS VC++ does not like this.  I don't need it.
//MpCodecInfo& MpCodecInfo::operator=(const MpCodecInfo& rhs){}
//:Assignment operator

/* ============================ ACCESSORS ================================= */

SdpCodec::SdpCodecTypes MpCodecInfo::getCodecType(void) const
{
//Returns the codec type
   return (mCodecType);
}

UtlString MpCodecInfo::getCodecVersion(void) const
{
//Returns a string identifying the codec version
   return(mCodecVersion);
}

unsigned MpCodecInfo::getSamplingRate(void) const
{
//Returns the sampling rate for the PCM data expected by the codec
   return(mSamplingRate);
}

unsigned MpCodecInfo::getNumBitsPerSample(void) const
{
//Returns the PCM data sample size (in bits)
   return(mNumBitsPerSample);
}

unsigned MpCodecInfo::getNumSamplesPerFrame(void) const
{
//Returns the number of PCM samples per input frame for this codec
   return(mNumSamplesPerFrame);
}

unsigned MpCodecInfo::getNumChannels(void) const
{
//Returns the number of channels supported by the codec
   return(mNumChannels);
}

unsigned MpCodecInfo::getInterleaveBlockSize(void) const
{
//Returns the size of the interleave block (in samples)
// This value is not meaningful if the number of channels for the
// codec is equal to 1.
   return(mInterleaveBlockSize);
}

unsigned MpCodecInfo::getBitRate(void) const
{
//Returns the bit rate for this codec (in bits per second)
// If the codec is variable rate, then the average expected bit rate
// should be returned.
   return(mBitRate);
}

unsigned MpCodecInfo::getMinPacketBits(void) const
{
//Returns the minimum number of bits in an encoded frame
   return(mMinPacketBits);
}

unsigned MpCodecInfo::getAvgPacketBits(void) const
{
//Returns the average number of bits in an encoded frame
   return(mAvgPacketBits);
}

unsigned MpCodecInfo::getMaxPacketBits(void) const
{
//Returns the maximum number of bits in an encoded frame
   return(mMaxPacketBits);
}


/* ============================ INQUIRY =================================== */

UtlBoolean MpCodecInfo::usesNetEq(void) const
{
//Returns TRUE if codec uses GIPS NetEq; otherwise returns FALSE
   return(mUsesNetEq);
}


UtlBoolean MpCodecInfo::isSignalingCodec (void) const
{
//Returns TRUE if codec is used for signaling; otherwise returns FALSE
   return(mIsSignalingCodec);
}


UtlBoolean MpCodecInfo::doesVadCng (void) const
{
//Returns TRUE if codec does its own VAD and CNG; otherwise returns FALSE
   return(mDoesVadCng);
}

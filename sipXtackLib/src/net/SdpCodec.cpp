//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/SdpCodec.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SdpCodec::SdpCodec(enum SdpCodecTypes sdpCodecType,
                   int payloadFormat,
                   const char* mimeType,
                   const char* mimeSubtype,
                   int sampleRate,
                   int preferredPacketLength,
                   int numChannels,
                   const char* formatSpecificData,
                   const int CPUCost,
                   const int BWCost,
                   const int videoFormat,
                   const int videoFmtp)
{
   setValue(sdpCodecType);
   mCodecPayloadFormat = payloadFormat;
   mSampleRate = sampleRate;
   mPacketLength = preferredPacketLength;
   mNumChannels = numChannels;
   mMimeSubtype.append(mimeSubtype ? mimeSubtype : "");
   mMimeSubtype.toLower();
   mMimeType = mimeType ? mimeType : "audio";
   mMimeType.toLower();
   mFormatSpecificData.append(formatSpecificData ? formatSpecificData : "");
   mCPUCost = CPUCost;
   mBWCost = BWCost;
   mVideoFormat = videoFormat;
   mVideoFmtp = videoFmtp;
}

// Copy constructor
SdpCodec::SdpCodec(const SdpCodec& rSdpCodec)
{
    setValue(rSdpCodec.getValue());
    mCodecPayloadFormat = rSdpCodec.mCodecPayloadFormat;
    mSampleRate = rSdpCodec.mSampleRate;
    mPacketLength = rSdpCodec.mPacketLength;
    mNumChannels = rSdpCodec.mNumChannels;
    mMimeType = rSdpCodec.mMimeType;
    mMimeSubtype = rSdpCodec.mMimeSubtype;
    mFormatSpecificData = rSdpCodec.mFormatSpecificData;
    mCPUCost  = rSdpCodec.mCPUCost;
    mBWCost  = rSdpCodec.mBWCost;
    mVideoFormat = rSdpCodec.mVideoFormat;
    mVideoFmtp = rSdpCodec.mVideoFmtp;
}

// Destructor
SdpCodec::~SdpCodec()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SdpCodec&
SdpCodec::operator=(const SdpCodec& rhs)
{

   if (this == &rhs)            // handle the assignment to self case
      return *this;
    setValue(rhs.getValue());
    mCodecPayloadFormat = rhs.mCodecPayloadFormat;
    mSampleRate = rhs.mSampleRate;
    mPacketLength = rhs.mPacketLength;
    mNumChannels = rhs.mNumChannels;
    mMimeType = rhs.mMimeType;
    mMimeSubtype = rhs.mMimeSubtype;
    mFormatSpecificData = rhs.mFormatSpecificData;
    mCPUCost  = rhs.mCPUCost;
    mBWCost = rhs.mBWCost;
    mVideoFormat = rhs.mVideoFormat;
    mVideoFmtp = rhs.mVideoFmtp;

   return *this;
}

/* ============================ ACCESSORS ================================= */

SdpCodec::SdpCodecTypes SdpCodec::getCodecType() const
{
        return((enum SdpCodecTypes) getValue());
}

int SdpCodec::getCodecPayloadFormat() const
{
    return(mCodecPayloadFormat);
}

void SdpCodec::setCodecPayloadFormat(int formatId)
{
    mCodecPayloadFormat = formatId;
}

void SdpCodec::getSdpFmtpField(UtlString& formatSpecificData) const
{
    formatSpecificData = mFormatSpecificData;
}

void SdpCodec::getMediaType(UtlString& mimeType) const
{
    mimeType = mMimeType;
}

void SdpCodec::getEncodingName(UtlString& mimeSubtype) const
{
    mimeSubtype = mMimeSubtype;
}

int SdpCodec::getSampleRate() const // samples per second
{
    return(mSampleRate);
}

int SdpCodec::getVideoFormat() const // samples per second
{
    return(mVideoFormat);
}

int SdpCodec::getPacketLength() const //micro seconds
{
    return(mPacketLength);
}

int SdpCodec::getNumChannels() const
{
    return(mNumChannels);
}

void SdpCodec::setVideoFmtp(const int videoFmtp)
{
    mVideoFmtp = videoFmtp;
}

int SdpCodec::getVideoFmtp() const
{
    return mVideoFmtp;
}

void SdpCodec::toString(UtlString& sdpCodecContents) const
{
    char stringBuffer[256];
    sprintf(stringBuffer,"SdpCodec:\n\
codecId: %" PRIdPTR "\n\
payloadId: %d\n\
mimeType: \'%s/%s\'\n\
sampleRate: %d\n\
packetLength: %d\n\
numChannels: %d\n\
formatData: \'%s\'\n",
            getValue(), mCodecPayloadFormat,
            mMimeType.data(), mMimeSubtype.data(),
            mSampleRate, mPacketLength, mNumChannels,
            mFormatSpecificData.data());
    sdpCodecContents = stringBuffer;
}

// Get the CPU cost for this codec.
int SdpCodec::getCPUCost() const
{
   return mCPUCost;
}

// Get the bandwidth cost for this codec.
int SdpCodec::getBWCost() const
{
   return mBWCost;
}

/* ============================ INQUIRY =================================== */

UtlBoolean SdpCodec::isSameDefinition(SdpCodec& codec) const
{
    return(mSampleRate == codec.mSampleRate &&
           mNumChannels == codec.mNumChannels &&
           mMimeType.compareTo(codec.mMimeType, UtlString::ignoreCase) == 0 &&
           mMimeSubtype.compareTo(codec.mMimeSubtype, UtlString::ignoreCase) == 0);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

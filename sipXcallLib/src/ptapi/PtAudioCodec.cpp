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
#include <assert.h>

// APPLICATION INCLUDES
#include "ptapi/PtAudioCodec.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
//:TODO: find the channels definition
PtAudioCodec::PtAudioCodec(PtRtpAudioCodecType codecType)
{
        mCodecType = codecType;
        switch (mCodecType)
        {
        case UNKNOWN_CODEC:
                mAudioEncodingMethod = UNKNOWN_ENCODING;
                mSampleSize = 0;
                mSampleRate = 0;
                mNumberOfChannels = 0;
                break;
        case MU_LAW_8B_8K:
                mAudioEncodingMethod = MU_LAW;
                mSampleSize = 8;
                mSampleRate = 8000;
                mNumberOfChannels = 1;
                break;
        case A_LAW_8B_8K:
                mAudioEncodingMethod = A_LAW;
                mSampleSize = 8;
                mSampleRate = 8000;
                mNumberOfChannels = 1;
                break;
        case LINEAR_16B_8K:
                mAudioEncodingMethod = LINEAR;
                mSampleSize = 16;
                mSampleRate = 8000;
                mNumberOfChannels = 1;
                break;
        }
}

// Copy constructor
PtAudioCodec::PtAudioCodec(const PtAudioCodec& rPtAudioCodec)
{
        mCodecType = rPtAudioCodec.mCodecType;
        mAudioEncodingMethod = rPtAudioCodec.mAudioEncodingMethod;
        mSampleSize = rPtAudioCodec.mSampleSize;
        mSampleRate = rPtAudioCodec.mSampleRate;
        mNumberOfChannels = rPtAudioCodec.mNumberOfChannels;
}

// Destructor
PtAudioCodec::~PtAudioCodec()
{
        // doing nothing right now.
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtAudioCodec&
PtAudioCodec::operator=(const PtAudioCodec& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        mCodecType = rhs.mCodecType;
        mAudioEncodingMethod = rhs.mAudioEncodingMethod;
        mSampleSize = rhs.mSampleSize;
        mSampleRate = rhs.mSampleRate;
        mNumberOfChannels = rhs.mNumberOfChannels;

   return *this;
}

/* ============================ ACCESSORS ================================= */
// Get the codec type
PtAudioCodec::PtRtpAudioCodecType PtAudioCodec::getRtpCodecType() const
{
        return mCodecType;
}

// Get the encoding method.
PtAudioCodec::PtRtpAudioEncodingMethod PtAudioCodec::getRtpEncodingMethod() const
{
        return mAudioEncodingMethod;
}

// Get the sample size for this codec
int PtAudioCodec::getSampleSize() const
{
        return mSampleSize;
}

// Get the sample rate
int PtAudioCodec::getSampleRate() const
{
        return mSampleRate;
}

// Get the number of channels supported for this codec
int PtAudioCodec::getNumChannels() const
{
        return mNumberOfChannels;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

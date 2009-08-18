//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef HAVE_GIPS /* [ */

#include "assert.h"
// APPLICATION INCLUDES
#include "mp/MpeSipxPcma.h"
#include "mp/JB/JB_API.h"

const MpCodecInfo MpeSipxPcma::smCodecInfo(
         SdpCodec::SDP_CODEC_PCMA, JB_API_VERSION, true,
         8000, 8, 1, 160, 64000, 1280, 1280, 1280, 160);

MpeSipxPcma::MpeSipxPcma(int payloadType)
   : MpEncoderBase(payloadType, &smCodecInfo)
{

}

MpeSipxPcma::~MpeSipxPcma()
{
   freeEncode();
}

OsStatus MpeSipxPcma::initEncode(void)
{
   return OS_SUCCESS;
}

OsStatus MpeSipxPcma::freeEncode(void)
{
   return OS_SUCCESS;
}

OsStatus MpeSipxPcma::encode(const Sample* pAudioSamples,
                             const int numSamples,
                             int& rSamplesConsumed,
                             unsigned char* pCodeBuf,
                             const int bytesLeft,
                             int& rSizeInBytes,
                             UtlBoolean& sendNow,
                             MpBufSpeech& rAudioCategory)
{
   JB_ret res;
   JB_size size;

   res = G711A_Encoder(numSamples, (Sample*)pAudioSamples, pCodeBuf, &size);
   rSizeInBytes = size;
   rAudioCategory = MP_SPEECH_UNKNOWN;
   sendNow = FALSE;
   rSamplesConsumed = numSamples;

   return OS_SUCCESS;
}
#endif /* HAVE_GIPS ] */

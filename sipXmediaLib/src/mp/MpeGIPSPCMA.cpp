//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifdef HAVE_GIPS /* [ */

#include "assert.h"
// APPLICATION INCLUDES
#include "mp/MpeGIPSPCMA.h"
#include "mp/GIPS/GIPS_API.h"

const MpCodecInfo MpeGIPSPCMA::smCodecInfo(
         SdpCodec::SDP_CODEC_GIPS_PCMA, GIPS_API_VERSION, true,
         8000, 8, 1, 160, 64000, 1280, 1280, 1280, 160);

MpeGIPSPCMA::MpeGIPSPCMA(int payloadType)
   : MpEncoderBase(payloadType, &smCodecInfo) // ,
      // pEncoderState(NULL)
{

}

MpeGIPSPCMA::~MpeGIPSPCMA()
{
   freeEncode();
}

OsStatus MpeGIPSPCMA::initEncode(void)
{
   return OS_SUCCESS;
}

OsStatus MpeGIPSPCMA::freeEncode(void)
{
   return OS_SUCCESS;
}

OsStatus MpeGIPSPCMA::encode(const short* pAudioSamples,
                             const int numSamples,
                             int& rSamplesConsumed,
                             unsigned char* pCodeBuf,
                             const int bytesLeft,
                             int& rSizeInBytes,
                             UtlBoolean& sendNow,
                             MpBufSpeech& rAudioCategory)
{
   int res;
   short size;
   // res = EG711_GIPS_10MS16B_Encoder(pEncoderState,
                             // (short*) pAudioSamples, pCodeBuf, &size);
   res = G711A_GIPS_10MS16B_Encoder(numSamples,
                             (short*) pAudioSamples, pCodeBuf, &size);
   rSizeInBytes = size;
   rAudioCategory = MP_SPEECH_UNKNOWN;
   // sendNow = (0 != rSizeInBytes);
   sendNow = FALSE;
   rSamplesConsumed = numSamples;

   return OS_SUCCESS;
}
#endif /* HAVE_GIPS ] */

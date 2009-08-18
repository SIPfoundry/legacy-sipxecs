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
#include "mp/MpeGIPSPCMU.h"
#include "mp/GIPS/GIPS_API.h"

const MpCodecInfo MpeGIPSPCMU::smCodecInfo(
         SdpCodec::SDP_CODEC_GIPS_PCMU, GIPS_API_VERSION, true,
         8000, 8, 1, 160, 64000, 1280, 1280, 1280, 160);

MpeGIPSPCMU::MpeGIPSPCMU(int payloadType)
   : MpEncoderBase(payloadType, &smCodecInfo) // ,
      // pEncoderState(NULL)
{

}

MpeGIPSPCMU::~MpeGIPSPCMU()
{
   freeEncode();
}

OsStatus MpeGIPSPCMU::initEncode(void)
{
   return OS_SUCCESS;
}

OsStatus MpeGIPSPCMU::freeEncode(void)
{
   return OS_SUCCESS;
}

OsStatus MpeGIPSPCMU::encode(const short* pAudioSamples,
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
   res = G711U_GIPS_10MS16B_Encoder(numSamples,
                             (short*) pAudioSamples, pCodeBuf, &size);
   rSizeInBytes = size;
   rAudioCategory = MP_SPEECH_UNKNOWN;
   sendNow = FALSE;
   rSamplesConsumed = numSamples;

   return OS_SUCCESS;
}
#endif /* HAVE_GIPS ] */

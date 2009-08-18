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
#include "mp/MpeGIPSiPCMA.h"
#include "mp/GIPS/GIPS_API.h"

const MpCodecInfo MpeGIPSiPCMA::smCodecInfo(
         SdpCodec::SDP_CODEC_GIPS_IPCMA, GIPS_API_VERSION, true,
         8000,0, 1, 160, 64000, 176, 1280, 2400, 160);

MpeGIPSiPCMA::MpeGIPSiPCMA(int payloadType)
   : MpEncoderBase(payloadType, &smCodecInfo),
     pEncoderState(NULL)
{

}

MpeGIPSiPCMA::~MpeGIPSiPCMA()
{
   freeEncode();
}

OsStatus MpeGIPSiPCMA::initEncode(void)
{
   int res = 0;

   //Allocate memory for encoder instance
   if (NULL == pEncoderState) {
      res += EG711A_GIPS_10MS16B_create(&pEncoderState);
   }

   //Initialize encoder
   res += EG711A_GIPS_10MS16B_InitEncoder(pEncoderState, 0, 160);

   return ((0==res) ? OS_SUCCESS : OS_NO_MEMORY);
}

OsStatus MpeGIPSiPCMA::freeEncode(void)
{
   int res;
   OsStatus ret = OS_DELETED;

   if (pEncoderState!=NULL) {
      res = EG711A_GIPS_10MS16B_free(pEncoderState);
      pEncoderState = NULL;
      ret = OS_SUCCESS;
   }
   return ret;
}

OsStatus MpeGIPSiPCMA::encode(const short* pAudioSamples,
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

   res = EG711A_GIPS_10MS16B_Encoder(pEncoderState,
                             (short*) pAudioSamples, pCodeBuf, &size);
   rSizeInBytes = size;
   rAudioCategory = MP_SPEECH_UNKNOWN;
   sendNow = (0 != rSizeInBytes);
   rSamplesConsumed = numSamples;

   return OS_SUCCESS;
}
#endif /* HAVE_GIPS ] */

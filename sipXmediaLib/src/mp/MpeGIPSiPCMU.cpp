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
#include "mp/MpeGIPSiPCMU.h"
#include "mp/GIPS/GIPS_API.h"

const MpCodecInfo MpeGIPSiPCMU::smCodecInfo(
         SdpCodec::SDP_CODEC_GIPS_IPCMU, GIPS_API_VERSION, true,
         8000,0, 1, 160, 64000, 176, 1280, 2400, 160);

MpeGIPSiPCMU::MpeGIPSiPCMU(int payloadType)
   : MpEncoderBase(payloadType, &smCodecInfo),
     pEncoderState(NULL)
{

}

MpeGIPSiPCMU::~MpeGIPSiPCMU()
{
   freeEncode();
}

OsStatus MpeGIPSiPCMU::initEncode(void)
{
   int res = 0;

   //Allocate memory for encoder instance
   if (NULL == pEncoderState) {
      res += EG711U_GIPS_10MS16B_create(&pEncoderState);
   }

   //Initialize encoder
   res += EG711U_GIPS_10MS16B_InitEncoder(pEncoderState, 0, 160);

   return ((0==res) ? OS_SUCCESS : OS_NO_MEMORY);
}

OsStatus MpeGIPSiPCMU::freeEncode(void)
{
   int res = 0;
   // osPrintf(" MpeGIPSiPCMU::freeEncode(): t=%d, pt=%d, dt=0x%p\n",
   //    smCodecInfo.getCodecType(), getPayloadType(), pEncoderState);
   if(pEncoderState!=NULL){
      res = EG711U_GIPS_10MS16B_free(pEncoderState);
      pEncoderState=NULL;
      return(OS_SUCCESS);
   } else {
      return(OS_DELETED);
   }
}

OsStatus MpeGIPSiPCMU::encode(const short* pAudioSamples,
                              const int numSamples,
                              int& rSamplesConsumed,
                              unsigned char* pCodeBuf,
                              const int bytesLeft,
                              int& rSizeInBytes,
                              UtlBoolean& sendNow,
                              MpBufSpeech& rAudioCategory)
{
   int res = 0;
   short size = 0;

   res = EG711U_GIPS_10MS16B_Encoder(pEncoderState,
                 (short*) pAudioSamples, pCodeBuf, &size);
   rSizeInBytes = size;
   rAudioCategory = MP_SPEECH_UNKNOWN;
   sendNow = (0 != rSizeInBytes);
   rSamplesConsumed = numSamples;

   return OS_SUCCESS;
}
#endif /* HAVE_GIPS ] */

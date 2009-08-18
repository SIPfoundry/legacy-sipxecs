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
#include "mp/MpeGIPSiPCMWB.h"
#include "mp/GIPS/GIPS_API.h"
const MpCodecInfo MpeGIPSiPCMWB::smCodecInfo(
         SdpCodec::SDP_CODEC_GIPS_IPCMWB, GIPS_API_VERSION, true,
         16000, 0, 1, 160, 75000, 176, 1280, 4800, 320);

MpeGIPSiPCMWB::MpeGIPSiPCMWB(int payloadType)
   : MpEncoderBase(payloadType, &smCodecInfo),
     pEncoderState(NULL)
{
   assert(FALSE); // we seem to be missing the library components...
}

MpeGIPSiPCMWB::~MpeGIPSiPCMWB()
{
   freeEncode();
}

OsStatus MpeGIPSiPCMWB::initEncode(void)
{
#ifdef NOT_YET /* [ */
   int res = 0;

   //Allocate memory for encoder instance
   if (NULL == pEncoderState)
      res += IPCMWB_GIPS_10MS16B_create(&pEncoderState);

   //Initialize encoder
   res += IPCMWB_GIPS_10MS16B_InitEncoder(pEncoderState);

   return ((0==res) ? OS_SUCCESS : OS_NO_MEMORY);
#endif /* NOT_YET ] */
   return OS_NOT_YET_IMPLEMENTED;
}

OsStatus MpeGIPSiPCMWB::freeEncode(void)
{
#ifdef NOT_YET /* [ */
   int res;
   OsStatus ret = OS_DELETED;

   if (pEncoderState!=NULL) {
      res = IPCMWB_GIPS_10MS16B_free(pEncoderState);
      pEncoderState = NULL;
      ret = OS_SUCCESS;
   }
   return ret;
#endif /* NOT_YET ] */
   return OS_NOT_YET_IMPLEMENTED;
}

OsStatus MpeGIPSiPCMWB::encode(const short* pAudioSamples,
                               const int numSamples,
                               int& rSamplesConsumed,
                               unsigned char* pCodeBuf,
                               const int bytesLeft,
                               int& rSizeInBytes,
                               UtlBoolean& sendNow,
                               MpBufSpeech& rAudioCategory)
{
#ifdef NOT_YET /* [ */
   int res;
   short size;

   res = IPCMWB_GIPS_10MS16B_Encoder(pEncoderState,
                             (short*) pAudioSamples, pCodeBuf, &size);
   rSizeInBytes = size;
   rAudioCategory = MP_SPEECH_UNKNOWN;
   sendNow = (0 != rSizeInBytes);
   rSamplesConsumed = numSamples;

   return OS_SUCCESS;
#endif /* NOT_YET ] */
   return OS_NOT_YET_IMPLEMENTED;
}
#endif /* HAVE_GIPS ] */

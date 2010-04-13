//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifdef HAVE_GIPS /* [ */

#ifndef __pingtel_on_posix__ /* [ */

#include "assert.h"
// APPLICATION INCLUDES
#include "mp/MpeGIPSiLBC.h"
#include "mp/GIPS/GIPS_API.h"

const MpCodecInfo MpeGIPSiLBC::smCodecInfo(
         SdpCodec::SDP_CODEC_GIPS_ILBC, GIPS_API_VERSION, true,
         8000, 0, 1, 240,
         13334, 50*8, 50*8, 50*8, 240);

MpeGIPSiLBC::MpeGIPSiLBC(int payloadType)
   : MpEncoderBase(payloadType, &smCodecInfo),
     mNumSamples(0),
     pEncoderState(NULL)
{

}

MpeGIPSiLBC::~MpeGIPSiLBC()
{
   freeEncode();
}

OsStatus MpeGIPSiLBC::initEncode(void)
{
   int res = 0;

   //Allocate memory for encoder instance
   if (NULL == pEncoderState) {
      res+= ILBCENC_GIPS_10MS16B_create(&pEncoderState);
   }

   //Initialize encoder
   res += ILBC_GIPS_10MS16B_InitEncoder(pEncoderState);

   mNumSamples = 0;

   return ((0==res) ? OS_SUCCESS : OS_NO_MEMORY);
}

OsStatus MpeGIPSiLBC::freeEncode(void)
{
   int res = 0;
   if(pEncoderState!=NULL){
      res = ILBCENC_GIPS_10MS16B_free(pEncoderState);
      pEncoderState=NULL;
      return(OS_SUCCESS);
   } else {
      return(OS_DELETED);
   }
}

OsStatus MpeGIPSiLBC::encode(const short* pAudioSamples,
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

   assert(ILBC_SAMPLES_PER_FRAME == numSamples);

   memcpy(mBuf+mNumSamples, pAudioSamples, numSamples*sizeof(Sample));
   mNumSamples += numSamples;

   if ((ILBC_SAMPLES_PER_FRAME * ILBC_FRAMES_PER_PACKET) == mNumSamples) {
      res = ILBC_GIPS_10MS16B_Encoder(pEncoderState, mBuf, pCodeBuf, &size);
      mNumSamples = 0;
   }
   rSizeInBytes = size;
   rAudioCategory = MP_SPEECH_UNKNOWN;
   sendNow = (0 != rSizeInBytes);
   rSamplesConsumed = numSamples;

   return OS_SUCCESS;
}

#endif /* __pingtel_on_posix__ ] */
#endif /* HAVE_GIPS ] */

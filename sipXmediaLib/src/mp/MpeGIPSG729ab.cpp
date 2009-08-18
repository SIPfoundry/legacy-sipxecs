//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifdef HAVE_G729 /* [ */

#ifdef HAVE_GIPS /* [ */

//////////////////////////////////////////////////////////////////////////////
//  G.729 enabling controls.  Currently only on VxWorks and Windows
//////////////////////////////////////////////////////////////////////////////

#undef PLATFORM_SUPPORTS_G729

#ifdef HAVE_G729
#define PLATFORM_SUPPORTS_G729
#endif

//////////////////////////////////////////////////////////////////////////////

#define READY_FOR_ANNEX_B

#include "assert.h"
#include "stdlib.h"
// APPLICATION INCLUDES
#include "mp/MpeGIPSG729ab.h"

#include "mp/iG729/_ippdefs.h"
#include "mp/iG729/ippdefs.h"
#include "mp/iG729/_p729.h"

//////////////////////////////////////////////////////////////////////////////
// Define stub replacements for the G729 API routines, then undef them
// on platforms where we support G729
//////////////////////////////////////////////////////////////////////////////

#define EncoderInit_G729A_16s(a) assert("EncoderInit_G729A_16s()" == NULL)
#define Encoder_G729A_16s(a, b, c) assert("Encoder_G729A_16s()" == NULL)
#define EncoderInit_G729B_16s(a) assert("EncoderInit_G729B_16s()" == NULL)
#define Encoder_G729B_16s(a, b, c, d) assert("Encoder_G729B_16s()" == NULL)

#ifdef PLATFORM_SUPPORTS_G729 /* [ */
#undef EncoderInit_G729A_16s
#undef Encoder_G729A_16s
#undef EncoderInit_G729B_16s
#undef Encoder_G729B_16s
#endif /* PLATFORM_SUPPORTS_G729 ] */

//////////////////////////////////////////////////////////////////////////////

#define FRAMES_PER_PACKET 2
const MpCodecInfo MpeGIPSG729ab::smCodecInfo(
         SdpCodec::SDP_CODEC_G729AB, "1.0", true,
         8000, 1, 1, FRAMES_PER_PACKET*80, 8000,
         FRAMES_PER_PACKET*80, FRAMES_PER_PACKET*80,
         FRAMES_PER_PACKET*80, FRAMES_PER_PACKET*80, FALSE, TRUE);


MpeGIPSG729ab::MpeGIPSG729ab(int payloadType)
   : MpEncoderBase(payloadType, &smCodecInfo),
     useVad(TRUE),
     mLastSID0(0),
     mLastSID1(0),
     mpEncoderState(NULL)
{
}

MpeGIPSG729ab::~MpeGIPSG729ab()
{
   freeEncode();
}

IppG729EncoderStruct* g729CreateCoder()
{
   return (IppG729EncoderStruct*) malloc(sizeof(IppG729EncoderStruct));
}

OsStatus MpeGIPSG729ab::initEncode(void)
{
   freeEncode();
   mpEncoderState = g729CreateCoder();
   EncoderInit_G729B_16s(mpEncoderState);
   return (NULL == mpEncoderState) ? OS_NO_MEMORY : OS_SUCCESS;
}

OsStatus MpeGIPSG729ab::freeEncode(void)
{
   if (NULL != mpEncoderState) {
      free(mpEncoderState);
      mpEncoderState = NULL;
   }
   return OS_SUCCESS;
}

#ifdef READY_FOR_ANNEX_B /* [ */
static int actuallyDoVad = 1;
static int actuallySendDummySID = 1;
#endif /* READY_FOR_ANNEX_B ] */

OsStatus MpeGIPSG729ab::encode(const short* pAudioSamples,
                             const int numSamples,
                             int& rSamplesConsumed,
                             unsigned char* pCodeBuf,
                             const int bytesLeft,
                             int& rSizeInBytes,
                             UtlBoolean& sendNow,
                             MpBufSpeech& rAudioCategory)
#ifdef READY_FOR_ANNEX_B /* [ */
{
   short Serial[82];
   short *pSer;
   int code;
   unsigned char packedBits;
   int n;
   int codeBits;
   int codeBytes;
   unsigned char* pSaveBuf = pCodeBuf;

   assert(80 == numSamples);
   assert(10 <= bytesLeft);
   pSer = Serial;

   if (useVad) {

      pSer[16] = 0; // SID is only 15 bits
      Encoder_G729B_16s(pAudioSamples, pSer, actuallyDoVad, mpEncoderState);
      codeBits = *pSer++;  // get bit count, skip to first data bit

   } else {

      Encoder_G729A_16s(pAudioSamples, pSer, mpEncoderState);
      codeBits = 80;
   }

   codeBytes = (codeBits + 7) / 8; // Round for the 15 bit case
   memset(pCodeBuf, 0, codeBytes);
   for (code=0; code<(codeBytes); code++) {
      packedBits = 0;
      for (n=0; n<8; n++) {
         packedBits = (packedBits << 1) + ((*pSer++) & 1);
      }
      *pCodeBuf++ = packedBits;
   }

#undef SEND_LAST_SID_FOR_UNTRANSMITTED_FRAMES
#define SEND_LAST_SID_FOR_UNTRANSMITTED_FRAMES
#ifdef SEND_LAST_SID_FOR_UNTRANSMITTED_FRAMES /* [ */
   switch (codeBytes) {
   case 0:
      if (actuallySendDummySID) {
         *pCodeBuf++ = mLastSID0;
         *pCodeBuf++ = mLastSID1 | 1;
         codeBytes = 2;
      }
      break;
   case 2:
      mLastSID0 = pSaveBuf[0];
      mLastSID1 = pSaveBuf[1];
      break;
   default:
      break;
   }
#endif /* SEND_LAST_SID_FOR_UNTRANSMITTED_FRAMES ] */

   sendNow = (2 == codeBytes);

#define FORCE_PAYLOAD_LENGTH_TO_MULTIPLE_OF_4
#undef FORCE_PAYLOAD_LENGTH_TO_MULTIPLE_OF_4
#ifdef FORCE_PAYLOAD_LENGTH_TO_MULTIPLE_OF_4 /* [ */
   if (2 == codeBytes) { // Hack so that SMS will recognize these packets
      *pCodeBuf++ = pSaveBuf[0];
      *pCodeBuf++ = pSaveBuf[1] | 1;
      codeBytes = 4;
   }
#endif /* FORCE_PAYLOAD_LENGTH_TO_MULTIPLE_OF_4 ] */

   rSizeInBytes = codeBytes;
   rAudioCategory = MP_SPEECH_ACTIVE; //Force all G.729A frames to be sent
   rSamplesConsumed = numSamples;

   return OS_SUCCESS;
}
#else /* READY_FOR_ANNEX_B ] [ else: Annex A only */
{
   short Serial[82];
   short *pSer = Serial+2;
   int code;
   unsigned char packedBits;
   int n;

   assert(80 == numSamples);
   assert(10 <= bytesLeft);

   Encoder_G729A_16s(pAudioSamples, pSer, mpEncoderState);

   memset(pCodeBuf, 0, 10);
   for (code=0; code<(80/8); code++) {
      packedBits = 0;
      for (n=0; n<8; n++) {
         packedBits = (packedBits << 1) + ((*pSer++) & 1);
      }
      *pCodeBuf++ = packedBits;
   }

   rSizeInBytes = 80/8;
   rAudioCategory = MP_SPEECH_ACTIVE; //Force all G.729A frames to be sent
   sendNow = FALSE;
   rSamplesConsumed = numSamples;

   return OS_SUCCESS;
}
#endif /* READY_FOR_ANNEX_B ] */

UtlBoolean MpeGIPSG729ab::setVad(UtlBoolean enableVad)
{
   UtlBoolean save = useVad;
   useVad = enableVad;
   return save;
}
#endif /* HAVE_GIPS ] */
#endif /* HAVE_G729 ] */

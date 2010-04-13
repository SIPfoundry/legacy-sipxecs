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
#include "string.h"

#include "mp/JB/JB_API.h"
#include "mp/MpJitterBuffer.h"
#include "mp/MpSipxDecoders.h"
#include "mp/NetInTask.h" // for definition of RTP packet

static int debugCount = 0;

/* ============================ CREATORS ================================== */

MpJitterBuffer::MpJitterBuffer(void)
{
   int i;

   for (i=0; i<JbPayloadMapSize; i++) payloadMap[i] = NULL;
   JbQWait = JbLatencyInit;
   JbQCount = 0;
   JbQIn = 0;
   JbQOut = 0;

   debugCount = 0;
}

//:Destructor
MpJitterBuffer::~MpJitterBuffer()
{
}

/* ============================ MANIPULATORS ============================== */

int MpJitterBuffer::ReceivePacket(JB_uchar* RTPpacket, JB_size RTPlength, JB_ulong TS)
{
   int numSamples = 0;
   unsigned char* pRtpData = NULL;
   struct rtpHeader* pHdr = (struct rtpHeader*) RTPpacket;
   int cc;
   int payloadType;
   int overhead;

   payloadType = (pHdr->mpt) & 0x7f;
   cc = (pHdr->vpxcc) & 0x0f;

   overhead = sizeof(struct rtpHeader) + (cc*sizeof(int));
   switch (payloadType) {
   case 0: // G.711 u-Law
   case 8: // G.711 a-Law
      numSamples = RTPlength - overhead;;
      pRtpData = RTPpacket + overhead;
      break;
   default:
      break;
   }

   if (0 == numSamples)
   {
      return 0;
   }

   if (numSamples != 160)
   {
      if (debugCount++ < 10)
      {
	        printf("RTPlength=%d, cc=%d, payloadType=%d\n", RTPlength,
                    cc, payloadType);
      }
   }

   if (JbQWait > 0)
   {
      JbQWait--;
   }

   if (JbQueueSize == JbQCount)
   {
      // discard some data...
      JbQOut = JbQIn + numSamples;
      JbQCount -= numSamples;
   }

   switch (payloadType)
   {
   case 0: // G.711 u-Law
      G711U_Decoder(numSamples, pRtpData, JbQ+JbQIn);
      break;
   case 8: // G.711 a-Law
      G711A_Decoder(numSamples, pRtpData, JbQ+JbQIn);
      break;
   default:
      break;
   }

   JbQCount += numSamples;
   JbQIn += numSamples;

   if (JbQIn >= JbQueueSize)
   {
       JbQIn -= JbQueueSize;
   }
   return 0;
}

int MpJitterBuffer::GetSamples(Sample *voiceSamples, JB_size *pLength)
{
    int numSamples = 80;

    if (JbQCount == 0)
    {
        JbQWait = JbLatencyInit; // No data, prime the buffer (again).
        memset((char*) voiceSamples, 0x00, 80 * sizeof(Sample));
    }
    else
    {
        memcpy(voiceSamples, JbQ+JbQOut, numSamples * sizeof(Sample));

        JbQCount -= numSamples;
        JbQOut += numSamples;
        if (JbQOut >= JbQueueSize)
        {
            JbQOut -= JbQueueSize;
        }
    }

    *pLength = numSamples;
    return 0;
}

int MpJitterBuffer::SetCodepoint(const JB_char* codec, JB_size sampleRate,
   JB_code codepoint)
{
   return 0;
}

/* ===================== Jitter Buffer API Functions ====================== */

JB_ret JB_initCodepoint(JB_inst *JB_inst,
                              const JB_char* codec,
                              JB_size sampleRate,
                              JB_code codepoint)
{
   return JB_inst->SetCodepoint(codec, sampleRate, codepoint);
}

JB_ret JB_RecIn(JB_inst *JB_inst,
                      JB_uchar* RTPpacket,
                      JB_size RTPlength,
                      JB_ulong timeStamp)
{
   return JB_inst->ReceivePacket(RTPpacket, RTPlength, timeStamp);
}

JB_ret JB_RecOut(JB_inst *JB_inst,
                      Sample *voiceSamples,
                      JB_size *pLength)
{
   return JB_inst->GetSamples(voiceSamples, pLength);
}

JB_ret JB_create(JB_inst **pJB)
{
   *pJB = new MpJitterBuffer();
   return 0;
}

JB_ret JB_init(JB_inst *pJB, int fs)
{
   return 0;
}

JB_ret JB_free(JB_inst *pJB)
{
   delete pJB;
   return 0;
}
#endif /* NOT(HAVE_GIPS) ] */

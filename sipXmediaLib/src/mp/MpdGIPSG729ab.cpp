//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifdef HAVE_GIPS /* [ */

// If GIPS is used, but G729 is not, it seems we need to
// stub out the G729 callbacks
#ifndef HAVE_G729

#include "assert.h"
#include "stdlib.h"
#include "mp/GIPS/G729Interface.h"
#include "mp/GIPS/GIPS_API.h"
#include "mp/GIPS/gips_typedefs.h"

// The gips library has call backs for G729 that need to be stubbed out
GIPS_Word16 G729FIX_GIPS_decoderinit(G729_decinst_t *G729dec_inst)
{
   assert("G729 not included" == NULL);
   return(0);
}

GIPS_Word16 G729FIX_GIPS_decodePLC(G729_decinst_t *G729dec_inst,
                                   GIPS_Word16 *decoded,
                                   GIPS_Word16 noOfLostFrames)
{
   assert("G729 not included" == NULL);
   return(0);
}

GIPS_Word16 G729FIX_GIPS_decode(G729_decinst_t *G729dec_inst,
                           GIPS_Word16 *encoded,
                           GIPS_Word16 len,
                           GIPS_Word16 *decoded,
                           GIPS_Word16 *speechType)
{
   assert("G729 not included" == NULL);
   return(0);
}

#else

//////////////////////////////////////////////////////////////////////////////
//  G.729 enabling controls.  Currently only on VxWorks and Windows
//////////////////////////////////////////////////////////////////////////////

#undef PLATFORM_SUPPORTS_G729

#ifdef HAVE_G729
#define PLATFORM_SUPPORTS_G729
#endif

//////////////////////////////////////////////////////////////////////////////

#include "assert.h"
#include "stdlib.h"
// APPLICATION INCLUDES
#include "mp/MpdGIPSG729ab.h"
#include "mp/MprDejitter.h"
#include "mp/MpConnection.h"
#include "mp/GIPS/G729Interface.h"
#include "mp/GIPS/GIPS_API.h"

#ifdef PLATFORM_SUPPORTS_G729 /* [ */
#include "mp/iG729/_ippdefs.h"
#include "mp/iG729/ippdefs.h"
#include "mp/iG729/_p729.h"
#endif /* PLATFORM_SUPPORTS_G729 ] */

//////////////////////////////////////////////////////////////////////////////
// Define stub replacements for the G729 API routines, then undef them
// on platforms where we support G729
//////////////////////////////////////////////////////////////////////////////

#define DecoderInit_G729B_16s(a) assert("DecoderInit_G729B_16s" == NULL)
#define Decoder_G729B_16s(a, b, c, d) assert("Decoder_G729B_16s" == NULL)
#define Decoder_G729B_b_16s(a, b, c, d, e) assert("Decoder_G729B_b_16s" == NULL)

#ifdef PLATFORM_SUPPORTS_G729 /* [ */
#undef DecoderInit_G729B_16s
#undef Decoder_G729B_b_16s
#endif /* PLATFORM_SUPPORTS_G729 ] */

#include "os/OsTask.h"

#define FRAMES_PER_PACKET 2
const MpCodecInfo MpdGIPSG729ab::smCodecInfo(
         SdpCodec::SDP_CODEC_G729AB, "1.0", true,
         8000, 1, 1, FRAMES_PER_PACKET*80, 8000,
         FRAMES_PER_PACKET*80, FRAMES_PER_PACKET*80,
         FRAMES_PER_PACKET*80, FRAMES_PER_PACKET*80);

static int DebugCount = 0;

MpdGIPSG729ab::MpdGIPSG729ab(int payloadType)
   : MpDecoderBase(payloadType, &smCodecInfo),
     mpDecoderState(NULL)
{
   osPrintf("MpdGIPSG729ab::MpdGIPSG729ab(%d)\n", payloadType);
}

MpdGIPSG729ab::~MpdGIPSG729ab()
{
   freeDecode();
}

OsStatus MpdGIPSG729ab::initDecode(MpConnection* pConnection)
{
   int res;

   //Get NetEq pointer
   mpJBState = pConnection->getJBinst();

   // Set the payload number for NetEq
   res = NETEQ_GIPS_10MS16B_initCodepoint(mpJBState,
                                          "G729", 8000, getPayloadType());

   //Attach the decoder to NetEq instance
   res += NETEQG729_GIPS_10MS16B_init(mpJBState, (G729_decinst*) this);
   osPrintf("NETEQG729_GIPS_10MS16B_init(0x%p, 0x%p) returned %d\n",
      mpJBState,  this, res);

   osPrintf("MpdGIPSG729ab::initDecode: payloadType=%d\n", getPayloadType());
   return ((0==res) ? OS_SUCCESS : OS_NO_MEMORY);
}

OsStatus MpdGIPSG729ab::freeDecode(void)
{
   if (NULL != mpDecoderState) {
      free(mpDecoderState);
      mpDecoderState = NULL;
   }
   return OS_SUCCESS;
}

IppsG729DecoderStruct* g729CreateDecode()
{
   return (IppsG729DecoderStruct*) malloc(sizeof(IppsG729DecoderStruct));
}

IppsG729DecoderStruct* MpdGIPSG729ab::getDecoder()
{
   return mpDecoderState;
}

OsStatus MpdGIPSG729ab::createDecoder()
{
   freeDecode();
   mpDecoderState = g729CreateDecode();
   DecoderInit_G729B_16s(mpDecoderState);
   return OS_SUCCESS;
}

int MpdGIPSG729ab::decodeIn(MpBufPtr pPacket)
{
   int thisLen;

   thisLen = MpBuf_getContentLen(pPacket);
   if (4 == ((thisLen - 12) % 10)) {
      thisLen -= 2;
   }
   return thisLen;
}

//////////////////////////////  FUNCTIONS ///////////////////////////////////

static int G729FixCountD = 0;
static int G729FixCountP = 0;

/****************************************************************************
 * G729FIX_GIPS_decoderinit(...)
 *
 * This function initializes a G729 instance
 *
 * Input:
 *      - G729_decinst_t    : G729 instance, i.e. the user that should receive
 *                            be initialized
 *
 * Return value             :  0 - Ok
 *                            -1 - Error
 */

GIPS_Word16 G729FIX_GIPS_decoderinit(G729_decinst_t *G729dec_inst)
{
   MpdGIPSG729ab* pMe = (MpdGIPSG729ab*) G729dec_inst;

   osPrintf("G729FIX_GIPS_decoderinit(0x%X)\n", G729dec_inst);
   DebugCount = 0;
   G729FixCountD = 0;
   G729FixCountP = 0;
   pMe->createDecoder();
   return 0;
}
/***************************************************************************/

/****************************************************************************
 * G729FIX_GIPS_decode(...)
 *
 * This function decodes a packet with G729 frame(s). Output speech length
 * will be a multiple of 80 samples (80*frames/package).
 *
 * Input:
 *      - G729dec_inst      : G729 instance, i.e. the user that should decode
 *                            a packet
 *      - encoded           : Encoded G729 frame(s)
 *      - len               : Bytes in encoded vector
 *
 * Output:
 *      - decoded           : The decoded vector
 *
 * Return value             : >0 - Samples in decoded vector
 *                            -1 - Error
 */

#define ANNEXA_BITS 80
#define ANNEXA_SAMPLES 80
GIPS_Word16 G729FIX_GIPS_decode(G729_decinst_t *G729dec_inst,
                           GIPS_Word16 *encoded,
                           GIPS_Word16 len,
                           GIPS_Word16 *decoded,
                           GIPS_Word16 *speechType)
{
   MpdGIPSG729ab* pMe = (MpdGIPSG729ab*) G729dec_inst;
   int serial_size;
   int nS = 0;
   unsigned char* pC = (unsigned char*) encoded;

   if (2 == len) {
      if (1 & pC[1]) len = 0;
   }

   *speechType = G729_GIPS_SPEECH;

   if (NULL == pMe->getDecoder()) {
      // This may be overkill now that the original problem has been fixed...
      osPrintf(
      "G729FIX_GIPS_decode(0x%p, %d): attempt to use stale instance!\n",
         pMe, len);
      if (0 == len) nS = ANNEXA_SAMPLES;
      while (len > 0) {
         nS += ANNEXA_SAMPLES;
         len -= ANNEXA_SAMPLES/8;
      }
      memset(decoded, 0, (sizeof(GIPS_Word16) * nS));
      return nS;
   }

   if (10 > G729FixCountD) {
      osPrintf("G729FIX_GIPS_decode(0x%p), len=%d\n", G729dec_inst, len);
   }

   if (0 == len) {
      pC = (unsigned char*) 0xfffffff0,
      nS = ANNEXA_SAMPLES;
      Decoder_G729B_b_16s(pC, decoded, 0, pMe->getDecoder(), 0);
      *speechType = G729_GIPS_CNG;
   }

   while (len > 0) {
      serial_size = len * 8;
      if (serial_size > ANNEXA_BITS) serial_size = ANNEXA_BITS;
      if (2 == len) *speechType = G729_GIPS_CNG;

      Decoder_G729B_b_16s(pC, decoded + nS, serial_size, pMe->getDecoder(), 0);
      nS += ANNEXA_SAMPLES;
      len -= serial_size/8;
      pC  += serial_size/8;
   }
   G729FixCountD++;
   return nS;
}

/****************************************************************************
 * G729FIX_GIPS_decodePLC(...)
 *
 * This function conducts PLC for G729 frame(s). Output speech length
 * will be a multiple of 80 samples.
 *
 * Input:
 *      - G729dec_inst      : G729 instance, i.e. the user that should perform
 *                            a PLC
 *      - noOfLostFrames    : Number of PLC frames to produce
 *
 * Output:
 *      - decoded           : The "decoded" vector
 *
 * Return value             : >0 - Samples in decoded PLC vector
 *                            -1 - Error
 */

GIPS_Word16 G729FIX_GIPS_decodePLC(G729_decinst_t *G729dec_inst,
                                   GIPS_Word16 *decoded,
                                   GIPS_Word16 noOfLostFrames)
{
   int nS = ANNEXA_SAMPLES * noOfLostFrames;

   if (10 > G729FixCountP) {
      osPrintf("G729FIX_GIPS_decodePLC(0x%p, 0x%p, %d)\n",
         G729dec_inst, decoded, noOfLostFrames);
   }
   G729FixCountP++;
   memset((void*) decoded, 0, nS * sizeof(short));
   return nS;
}

#if 0 /* [ */
extern "C"
GIPS_Word16 G729FIX_GIPS_GetVersion(char *versionStr, GIPS_Word16 len)
{
   static const char* OurG729Version = "G.729 20021220";
   int ourLen = strlen(OurG729Version);
   int ret = 0;

   if (len < ourLen) {
      ourLen = len;
      ret = -1;
   }
   memcpy(versionStr, OurG729Version, ourLen);
   return ret;
}
#endif /* ] */

#endif /* HAVE_G729 */
#endif /* HAVE_GIPS ] */

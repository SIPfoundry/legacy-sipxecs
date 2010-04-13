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
#include "mp/MpConnection.h"
#include "mp/MpdGIPSiLBC.h"
#include "mp/GIPS/GIPS_API.h"
#include "mp/MprDejitter.h"

const MpCodecInfo MpdGIPSiLBC::smCodecInfo(
         SdpCodec::SDP_CODEC_GIPS_ILBC, GIPS_API_VERSION, true,
         8000, 0, 1, 240,
         13334, 50*8, 50*8, 50*8, 240);

MpdGIPSiLBC::MpdGIPSiLBC(int payloadType)
   : MpDecoderBase(payloadType, &smCodecInfo),
     pDecoderState(NULL)
{
   osPrintf("MpdGIPSiLBC::MpdGIPSiLBC(%d)\n", payloadType);
}

MpdGIPSiLBC::~MpdGIPSiLBC()
{
   freeDecode();
}

OsStatus MpdGIPSiLBC::initDecode(MpConnection* pConnection)
{
   int res = 0;

   //Get NetEq pointer
   mpJBState = pConnection->getJBinst();
   assert(NULL != mpJBState);

   //Allocate memory, only once though
   if (NULL == pDecoderState) {
      res += ILBCDEC_GIPS_10MS16B_create(&pDecoderState);
   }

   // Set the payload number for NetEq
   NETEQ_GIPS_10MS16B_initCodepoint(mpJBState,
                                        "ILBC", 8000, getPayloadType());

   //Attach the decoder to NetEq instance
   res += NETEQILBC_GIPS_10MS16B_init(mpJBState, pDecoderState);

   osPrintf("MpdGIPSiLBC::initDecode: payloadType=%d\n", getPayloadType());
   return ((0==res) ? OS_SUCCESS : OS_NO_MEMORY);
}

OsStatus MpdGIPSiLBC::freeDecode(void)
{
   int res;
   OsStatus ret = OS_DELETED;

   if (NULL != pDecoderState) {
      res = ILBCDEC_GIPS_10MS16B_free(pDecoderState);
      pDecoderState = NULL;
      ret = OS_SUCCESS;
   }
   return ret;
}

#endif /* __pingtel_on_posix__ ] */
#endif /* HAVE_GIPS ] */

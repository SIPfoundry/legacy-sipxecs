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
#include "mp/MpConnection.h"
#include "mp/MpdGIPSiPCMWB.h"
#include "mp/GIPS/GIPS_API.h"
const MpCodecInfo MpdGIPSiPCMWB::smCodecInfo(
         SdpCodec::SDP_CODEC_GIPS_IPCMWB, GIPS_API_VERSION, true,
         16000, 0, 1, 160, 75000, 176, 1280, 4480, 320);
MpdGIPSiPCMWB::MpdGIPSiPCMWB(int payloadType)
   : MpDecoderBase(payloadType, &smCodecInfo),
     pDecoderState(NULL)
{
   assert(FALSE); // we seem to be missing the library components...
}

MpdGIPSiPCMWB::~MpdGIPSiPCMWB()
{
   freeDecode();
}

OsStatus MpdGIPSiPCMWB::initDecode(MpConnection* pConnection)
{
#ifdef NOT_YET /* [ */
   int res = 0;

   //Get NetEq pointer
   mpJBState = pConnection->getJBinst();

   //Allocate memory, only once though
   if(pDecoderState==NULL)
      res += IPCMWB_GIPS_10MS16B_create(&pDecoderState);

   // Set the payload number for NetEq
   NETEQ_GIPS_10MS16B_initCodepoint(mpJBState,
                         "IPCMWB", 16000, getPayloadType());

   //Attach the decoder to NetEq instance
   res += NETEQIPCMWB_GIPS_10MS16B_init(mpJBState,pDecoderState);

   return ((0==res) ? OS_SUCCESS : OS_NO_MEMORY);
#endif /* NOT_YET ] */
   return OS_NOT_YET_IMPLEMENTED;
}

OsStatus MpdGIPSiPCMWB::freeDecode(void)
{
#ifdef NOT_YET /* [ */
   int res;
   OsStatus ret = OS_DELETED;

   if (NULL != pDecoderState) {
      res = IPCMWB_GIPS_10MS16B_free(pDecoderState);
      pDecoderState = NULL;
      ret = OS_SUCCESS;
   }
   return ret;
#endif /* NOT_YET ] */
   return OS_NOT_YET_IMPLEMENTED;
}
#endif /* HAVE_GIPS ] */

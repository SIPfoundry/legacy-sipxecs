//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#if 1 /* [ */
#ifndef HAVE_GIPS /* [ */

// APPLICATION INCLUDES
#include "mp/MpConnection.h"
#include "mp/MpdSipxPcma.h"
#include "mp/JB/JB_API.h"
#include "mp/MprDejitter.h"

const MpCodecInfo MpdSipxPcma::smCodecInfo(
         SdpCodec::SDP_CODEC_PCMA, JB_API_VERSION, true,
         8000, 8, 1, 160, 64000, 1280, 1280, 1280, 160);

MpdSipxPcma::MpdSipxPcma(int payloadType)
   : MpDecoderBase(payloadType, &smCodecInfo)
{
}

MpdSipxPcma::~MpdSipxPcma()
{
   freeDecode();
}

OsStatus MpdSipxPcma::initDecode(MpConnection* pConnection)
{
   //Get JB pointer
   pJBState = pConnection->getJBinst();

   // Set the payload number for JB
   JB_initCodepoint(pJBState, "PCMA", 8000, getPayloadType());

   return OS_SUCCESS;
}

OsStatus MpdSipxPcma::freeDecode(void)
{
   return OS_SUCCESS;
}
#endif /* HAVE_GIPS ] */
#endif /* ] */

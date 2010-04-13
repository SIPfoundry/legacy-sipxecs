//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include "mp/MpEncoderBase.h"

/* ============================ CREATORS ================================== */

// Constructor
// Returns a new encoder object.
// param: payloadType - (in) RTP payload type associated with this encoder
MpEncoderBase::MpEncoderBase(int payloadType, const MpCodecInfo *pInfo) :
   mpCodecInfo(pInfo),
   mPayloadType(payloadType)
{
}

// Destructor
MpEncoderBase::~MpEncoderBase()
{
}

/* ============================ ACCESSORS ================================= */

// Get static information about the encoder
// Returns a pointer to an <i>MpCodecInfo</i> object that provides
// static information about the encoder.
const MpCodecInfo* MpEncoderBase::getInfo(void) const
{
   return(mpCodecInfo);
}

//:Returns the RTP payload type associated with this encoder.
int MpEncoderBase::getPayloadType(void)
{
   return(mPayloadType);
}

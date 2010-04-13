//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifdef HAVE_GIPS /* [ */

// If OLD_GIPS is defined, then we check that G.711 packets are an exact
// multiple of 10 milliseconds (80 samples), to avoid a bug in the older
// version of the GIPS libraries.  Currently, the only platform that is
// using the newer version of the libraries is the TCAS8 (XScale) hard phone.
#define OLD_GIPS
#ifdef CPU_XSCALE /* [ */
#undef  OLD_GIPS
#endif /* CPU_XSCALE ] */

#ifndef _MpdGIPSPCMU_h_
#define _MpdGIPSPCMU_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpDecoderBase.h"
#include "mp/GIPS/gips_typedefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

//:Derived class for GIPS PCMU decoder.
class MpdGIPSPCMU: public MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   MpdGIPSPCMU(int payloadType);
     //:Constructor
     // Returns a new decoder object.
     //!param: payloadType - (in) RTP payload type associated with this decoder

   virtual ~MpdGIPSPCMU(void);
     //:Destructor

   virtual OsStatus initDecode(MpConnection* pConnection);
     //:Initializes a codec data structure for use as a decoder
     //!param: pConnection - (in) Pointer to the MpConnection container
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_NO_MEMORY - Memory allocation failure

   virtual OsStatus freeDecode(void);
     //:Frees all memory allocated to the decoder by <i>initDecode</i>
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_DELETED - Object has already been deleted

/* ============================ MANIPULATORS ============================== */

#ifdef OLD_GIPS /* [ */
   virtual int decodeIn(MpBufPtr pPacket);
     //:Receive a packet of RTP data
     //!param: pPacket - (in) Pointer to a media buffer
     //!retcode: length of packet to hand to jitter buffer, 0 means don't.
#endif /* OLD_GIPS ] */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const MpCodecInfo smCodecInfo;  // static information about the codec
   JB_inst* mpJBState;
};

#endif  // _MpdGIPSPCMU_h_
#endif /* HAVE_GIPS ] */

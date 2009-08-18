//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifdef HAVE_GIPS /* [ */
#ifdef HAVE_G729

#ifndef _MpdGIPSG729ab_h_
#define _MpdGIPSG729ab_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpDecoderBase.h"
#include "mp/GIPS/gips_typedefs.h"

#include "mp/iG729/_ippdefs.h"
#include "mp/iG729/ippdefs.h"
#include "mp/iG729/_p729.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

//:Derived class for G.729 decoder.
class MpdGIPSG729ab: public MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   MpdGIPSG729ab(int payloadType);
     //:Constructor
     // Returns a new decoder object.
     //!param: payloadType - (in) RTP payload type associated with this decoder

   virtual ~MpdGIPSG729ab(void);
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

   virtual int decodeIn(MpBufPtr pPacket);
     //:Receive a packet of RTP data
     //!param: pPacket - (in) Pointer to a media buffer
     //!retcode: length of packet to hand to jitter buffer, 0 means don't.

   virtual OsStatus createDecoder(void);

/* ============================ ACCESSORS ================================= */

   virtual IppsG729DecoderStruct* getDecoder(void);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const MpCodecInfo smCodecInfo;  // static information about the codec
   JB_inst* mpJBState;
   IppsG729DecoderStruct* mpDecoderState;
};

#endif  // _MpdGIPSG729ab_h_
#endif /* HAVE_G729 */
#endif /* HAVE_GIPS ] */

//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _MpDecoderBase_h_
#define _MpDecoderBase_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "mp/MpCodecInfo.h"
#include "mp/MpBuf.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MpConnection;
class OsNotification;
class MprRecorder;

//:Base class for all media processing decoders.
class MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   friend class MprDecode;

/* ============================ CREATORS ================================== */

   MpDecoderBase(int payloadType, const MpCodecInfo* pInfo);
     //:Constructor
     // Returns a new decoder object.
     //!param: payloadType - (in) RTP payload type associated with this decoder
     //!param: pInfo - (in) pointer to derived class' static const MpCodecInfo

   virtual
   ~MpDecoderBase();
     //:Destructor

   virtual OsStatus initDecode(MpConnection* pConnection)=0;
     //:Initializes a codec data structure for use as a decoder
     //!param: pConnection - (in) Pointer to the MpConnection container
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_NO_MEMORY - Memory allocation failure

   virtual OsStatus freeDecode(void)=0;
     //:Frees all memory allocated to the decoder by <i>initDecode</i>
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_DELETED - Object has already been deleted

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   virtual const MpCodecInfo* getInfo(void) const;
     //:Get static information about the decoder
     // Returns a pointer to an <i>MpCodecInfo</i> object that provides
     // static information about the decoder.

   virtual int getPayloadType(void);
     //:Returns the RTP payload type associated with this decoder.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

   virtual int decodeIn(MpBufPtr pPacket);
     //:Receive a packet of RTP data
     //!param: pPacket - (in) Pointer to a media buffer
     //!retcode: length of packet to hand to jitter buffer, 0 means don't.

   virtual UtlBoolean handleSetDtmfNotify(OsNotification* pNotify);
     //:Handle the FLOWGRAPH_SET_DTMF_NOTIFY message.

   virtual UtlBoolean setDtmfTerm(MprRecorder *pRecorder);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MpDecoderBase(const MpDecoderBase& rMpDecoderBase);
     //:Copy constructor

   MpDecoderBase& operator=(const MpDecoderBase& rhs);
     //:Assignment operator

   const MpCodecInfo* mpCodecInfo;
   int mPayloadType;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpDecoderBase_h_

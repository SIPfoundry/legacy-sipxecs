//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpdPtAVT_h_
#define _MpdPtAVT_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpDecoderBase.h"
#include "mp/JB/JB_API.h"

// FORWARD DECLARATIONS

class OsNotification;
class MprRecorder;

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

//:Derived class for Pingtel AVT/Tone decoder.
class MpdPtAVT: public MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   MpdPtAVT(int payloadType);
     //:Constructor
     // Returns a new decoder object.
     //!param: payloadType - (in) RTP payload type associated with this decoder

   virtual ~MpdPtAVT(void);
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

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   virtual UtlBoolean handleSetDtmfNotify(OsNotification* n);
     //:Handle the FLOWGRAPH_SET_DTMF_NOTIFY message.
     // Returns TRUE

   virtual UtlBoolean setDtmfTerm(MprRecorder* pRecorder);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const MpCodecInfo smCodecInfo;  // static information about the codec
   JB_inst* mpJBState;

   int mCurrentToneKey;       // The key ID
   unsigned int mPrevToneSignature;    // The timestamp for last KEYUP event
   unsigned int mCurrentToneSignature; // The starting timestamp
   unsigned int mToneDuration;         // last reported duration
   OsNotification* mpNotify;  // Object to signal on key-down/key-up events
   MprRecorder* mpRecorder;

   void signalKeyDown(MpBufPtr pPacket);
   void signalKeyUp(MpBufPtr pPacket);
};

#endif  // _MpdPtAVT_h_

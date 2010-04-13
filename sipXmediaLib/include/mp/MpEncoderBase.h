//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _MpEncoderBase_h_
#define _MpEncoderBase_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "mp/MpBuf.h"
#include "mp/MpCodecInfo.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MpConnection;

//:Base class for all media processing encoders.
class MpEncoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MpEncoderBase(int payloadType, const MpCodecInfo* pInfo);
     //:Constructor
     // Returns a new encoder object.
     //!param: payloadType - (in) RTP payload type associated with this encoder

   virtual
   ~MpEncoderBase();
     //:Destructor

   virtual OsStatus initEncode(void)=0;
     //:Initializes a codec data structure for use as an encoder
     //!param: pConnection - (in) Pointer to the MpConnection container
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_NO_MEMORY - Memory allocation failure

   virtual OsStatus freeEncode(void)=0;
     //:Frees all memory allocated to the encoder by <i>initEncode</i>
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_DELETED - Object has already been deleted

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus encode(const Sample* pAudioSamples,
                           const int numSamples,
                           int& rSamplesConsumed,
                           unsigned char* pCodeBuf,
                           const int bytesLeft,
                           int& rSizeInBytes,
                           UtlBoolean& sendNow,
                           MpBufSpeech& rAudioCategory) = 0;
     //:Encode audio samples
     // Processes the array of audio samples.  If sufficient samples to encode
     // a frame are now available, the encoded data will be written to the
     // <i>pCodeBuf</i> array.  The number of bytes written to the
     // <i>pCodeBuf</i> array is returned in <i>rSizeInBytes</i>.
     //!param: pAudioSamples - (in) Pointer to array of PCM samples
     //!param: numSamples - (in) number of samples at pAudioSamples
     //!param: rSamplesConsumed - (out) Number of samples encoded
     //!param: pCodeBuf - (out) Pointer to array for encoded data
     //!param: bytesLeft - (in) number of bytes available at pCodeBuf
     //!param: rSizeInBytes - (out) Number of bytes written to the <i>pCodeBuf</i> array
     //!param: sendNow - (out) if true, the packet is complete, send it.
     //!param: rAudioCategory - (out) Audio type (e.g., unknown, silence, comfort noise)
     //!retcode: OS_SUCCESS - Success

/* ============================ ACCESSORS ================================= */

   virtual const MpCodecInfo* getInfo(void) const;
     //:Get static information about the encoder
     // Returns a pointer to an <i>MpCodecInfo</i> object that provides
     // static information about the encoder.

   virtual int getPayloadType(void);
     //:Returns the RTP payload type associated with this encoder.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   const MpCodecInfo* mpCodecInfo;
   int mPayloadType;

   MpEncoderBase(const MpEncoderBase& rMpEncoderBase);
     //:Copy constructor

   MpEncoderBase& operator=(const MpEncoderBase& rhs);
     //:Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpEncoderBase_h_

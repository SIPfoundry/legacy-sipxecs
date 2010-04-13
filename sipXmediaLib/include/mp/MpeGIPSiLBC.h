//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifdef HAVE_GIPS /* [ */

#ifndef _MpeGIPSiLBC_h_
#define _MpeGIPSiLBC_h_

// APPLICATION INCLUDES
#include "mp/MpEncoderBase.h"
#include "mp/GIPS/gips_typedefs.h"

#define ILBC_SAMPLES_PER_FRAME 80
#define ILBC_FRAMES_PER_PACKET 3
#define ILBC_SAMPLES_PER_PACKET (ILBC_SAMPLES_PER_FRAME*ILBC_FRAMES_PER_PACKET)

//:Derived class for GIPS iLBC encoder.
class MpeGIPSiLBC: public MpEncoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   MpeGIPSiLBC(int payloadType);
     //:Constructor
     // Returns a new decoder object.
     //!param: payloadType - (in) RTP payload type associated with this decoder

   virtual ~MpeGIPSiLBC(void);
     //:Destructor

   virtual OsStatus initEncode(void);
     //:Initializes a codec data structure for use as an encoder
     //!param: pConnection - (in) Pointer to the MpConnection container
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_NO_MEMORY - Memory allocation failure

   virtual OsStatus freeEncode(void);
     //:Frees all memory allocated to the encoder by <i>initEncode</i>
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_DELETED - Object has already been deleted

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus encode(const short* pAudioSamples,
                   const int numSamples,
                   int& rSamplesConsumed,
                   unsigned char* pCodeBuf,
                   const int bytesLeft,
                   int& rSizeInBytes,
                   UtlBoolean& sendNow,
                   MpBufSpeech& rAudioCategory);
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

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const MpCodecInfo smCodecInfo;  // static information about the codec
   ILBC_encinst *pEncoderState;

   int    mNumSamples;
   Sample mBuf[ILBC_SAMPLES_PER_PACKET];
};

#endif  // _MpeGIPSiLBC_h_
#endif /* HAVE_GIPS ] */

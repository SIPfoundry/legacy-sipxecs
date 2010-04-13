//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtAudioCodec_h_
#define _PtAudioCodec_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Class containing an RTP encoding type definintion
// This class is used to contain the definition of an RTP encoding type.
// The RTP encoding type is used during call setup to negotiate and come
// to an agreement as to what RTP encoding will be used by the parties of
// a call.  This class encapsulates the RTP encoding type and definitions
// for parameters such as sample size, sample rate, number of channels and
// encoding method.
// <BR>
// The currentl implementation does not allow free form definition of all of
// the RTP encoding parameters.  A fixed set of combinations may be specified
// using the <I>PtRtpAudioCodecType</I> enumeration.

class PtAudioCodec
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    enum PtRtpAudioCodecType
    {
        UNKNOWN_CODEC,
        MU_LAW_8B_8K,
        A_LAW_8B_8K,
        LINEAR_16B_8K
    };
    //: Predefined RTP encoding parameters
    //! enumcode: UNKNOWN_CODEC - undefined encoding parameters.
    //! enumcode: MU_LAW_8B_8K - mu-law encoding, 8 bit samples, 8 kilohertz sample rate, one channel
    //! enumcode: A_LAW_8B_8K - a-law encoding, 8 bit samples, 8 kilohertz sample rate, one channel
    //! enumcode: LINEAR_16B_8K - PCM/linear encoding, 16 bit samples, 8 kilohertz sample rate, one channel

    enum PtRtpAudioEncodingMethod
    {
        UNKNOWN_ENCODING,
        MU_LAW,
        A_LAW,
        LINEAR
    };
    //: RTP encoding methods
    //! enumcode: UNKNOWN_ENCODING - undefined encoding method.
    //! enumcode: MU_LAW - mu-law encoding
    //! enumcode: A_LAW - a-law encoding
    //! enumcode: LINEAR - PCM/linear encoding

/* ============================ CREATORS ================================== */

   PtAudioCodec(PtRtpAudioCodecType codecType = UNKNOWN_CODEC);
     //:Default constructor

   PtAudioCodec(const PtAudioCodec& rPtAudioCodec);
     //:Copy constructor

   virtual
   ~PtAudioCodec();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtAudioCodec& operator=(const PtAudioCodec& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

   PtRtpAudioCodecType getRtpCodecType() const;
 //  int getRtpCodecType() const;
   //: Get the codec type
   //! returns: The codec type enumeration.

   PtRtpAudioEncodingMethod getRtpEncodingMethod() const;
   //: Get the encoding method for this codec
   //! returns: the enumeration for the encoding method for this codec.

   int getSampleSize() const;
   //: Get the sample size for this codec
   //! returns: The number of bits per sample

   int getSampleRate() const;
   //: Get the sample rate for this codec
   //! returns: The number of samples per second.

   int getNumChannels() const;
   //: Get the number of channels supported for this codec
   //! returns: The number of channels.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    PtRtpAudioCodecType mCodecType;
    PtRtpAudioEncodingMethod mAudioEncodingMethod;
    int mSampleSize;
    int mSampleRate;
    int mNumberOfChannels;


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtAudioCodec_h_

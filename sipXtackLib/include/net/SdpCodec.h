//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SdpCodec_h_
#define _SdpCodec_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlInt.h"
#include "utl/UtlString.h"


// DEFINES

// Mime major types
#define MIME_TYPE_AUDIO "audio"
#define MIME_TYPE_VIDEO "video"

// Mime Sub types
#define MIME_SUBTYPE_PCMU "pcmu"
#define MIME_SUBTYPE_PCMA "pcma"
#define MIME_SUBTYPE_G729A "G729"
#define MIME_SUBTYPE_G729AB "G729"
#define MIME_SUBTYPE_G729ACISCO7960 "G729a"
#define MIME_SUBTYPE_G723 "G723"
#define MIME_SUBTYPE_DTMF_TONES "telephone-event"
#define MIME_SUBTYPE_IPCMU "EG711U"
#define MIME_SUBTYPE_IPCMA "EG711A"
#define MIME_SUBTYPE_IPCMWB "IPCMWB"
#define MIME_SUBTYPE_ILBC "ILBC"
#define MIME_SUBTYPE_ISAC "ISAC"
#define MIME_SUBTYPE_GSM "GSM"
#define MIME_SUBTYPE_VP71 "VP71"
#define MIME_SUBTYPE_IYUV "IYUV"
#define MIME_SUBTYPE_I420 "I420"
#define MIME_SUBTYPE_RGB24 "RGB24"

// Bandwidth requirements for SDP Codecs
#define SDP_CODEC_BANDWIDTH_VARIABLE 0
#define SDP_CODEC_BANDWIDTH_LOW      1
#define SDP_CODEC_BANDWIDTH_NORMAL   2
#define SDP_CODEC_BANDWIDTH_HIGH     3

// Video formats - must be bitmap values
#define SDP_VIDEO_FORMAT_SQCIF       0x0001
#define SDP_VIDEO_FORMAT_QCIF        0x0002
#define SDP_VIDEO_FORMAT_CIF         0x0004

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Container for SDP/RTP codec specification
// This class holds the SDP definition of a codec
// Included in this information is: sample rate,
// number of channels, the mapping from an internal
// pingtel codec id to the public SDP format and RTP
// payload type id.
//
// This is the base class.  Specific codec types
// should implement sub classes which define the
// codec specific parameters.  All specific codec
// types MUST be registered with the SdpCodecFactory
// to be useable.  Generally codecs are constructed
// ONLY by the SdpCodecFactory.

// The method that we have been using on
// SdpCodec::getCodecType() retrieved
// the static codec type/id.  I have defined
// an enum in SdpCodec which contains the
// current values as well as some additional
// ones.  The idea is that these are private,
// internally assigned ids to the codecs we
// support.

// I have added a new method
// SdpCodec::getCodecPayloadFormat()
// which returns the RTP payload id to be
// used in RTP and the SDP.  For static
// codec id the returned value for both of these
// methods would typically be the same.
// However for the dynamic codecs they
// will mostly be different.

// The intent is that eventually we will
// support a factory which will allow
// registration of new codec types.

class SdpCodec : public UtlInt
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    //: Unique identifier used for each supported codec
    // Note it is possible that the format id/type used
    // in the SDP "m" field and RTP header is different
    // than these internally used ids.
    enum SdpCodecTypes
    {
        SDP_CODEC_UNKNOWN = -1,
        SDP_CODEC_PCMU = 0, // G.711 mu-law
        SDP_CODEC_GSM = 3,
        SDP_CODEC_G723 = 4,
        SDP_CODEC_PCMA = 8, // G.711 a-law
        SDP_CODEC_L16_STEREO = 10, // PCM 16 bit/sample 44100 samples/sec.
        SDP_CODEC_L16_MONO = 11, // PCM 16 bit/sample 44100 samples/sec.
        SDP_CODEC_G729 = 18, // G.729, with or without Annexes A or B
        SDP_CODEC_MAXIMUM_STATIC_CODEC = 95,
        SDP_CODEC_DEFAULT_TONES_CODEC = 101,
        SDP_CODEC_MAXIMUM_DYNAMIC_CODEC = 127,
        SDP_CODEC_TONES = 128, // AVT/DTMF Tones, RFC 2833
        SDP_CODEC_G729A = 129,
        SDP_CODEC_G7221 = 130, // Siren
        SDP_CODEC_G7231 = 131,
        SDP_CODEC_L16_8K = 132, // Mono PCM 16 bit/sample 8000 samples/sec.
        SDP_CODEC_G729AB = 133,
        SDP_CODEC_G729ACISCO7960 = 134,

         // Range for 3rd party add in codec types
        SDP_CODEC_3RD_PARTY_START = 256,
        SDP_CODEC_GIPS_PCMA  = 257,
        SDP_CODEC_GIPS_PCMU  = 258,
        SDP_CODEC_GIPS_IPCMA = 259,
        SDP_CODEC_GIPS_IPCMU = 260,
        SDP_CODEC_GIPS_IPCMWB = 261,
        SDP_CODEC_GIPS_ILBC = 262,
        SDP_CODEC_GIPS_ISAC = 263,
        SDP_CODEC_VP71_CIF = 264,
        SDP_CODEC_VP71_QCIF = 265,
        SDP_CODEC_VP71_SQCIF = 266,
        SDP_CODEC_IYUV_CIF = 267,
        SDP_CODEC_IYUV_QCIF = 268,
        SDP_CODEC_IYUV_SQCIF = 269,
        SDP_CODEC_I420_CIF = 270,
        SDP_CODEC_I420_QCIF = 271,
        SDP_CODEC_I420_SQCIF = 272,
        SDP_CODEC_RGB24_CIF = 273,
        SDP_CODEC_RGB24_QCIF = 274,
        SDP_CODEC_RGB24_SQCIF = 275,
        SDP_CODEC_3RD_PARTY_END = 511
    };


    //:Identifies the relative CPU cost for a SDP Codec.
    enum SdpCodecCPUCost
    {
       SDP_CODEC_CPU_LOW = 0,
       SDP_CODEC_CPU_HIGH = 1,
    };


/* ============================ CREATORS ================================== */

   SdpCodec(enum SdpCodecTypes sdpCodecType = SDP_CODEC_UNKNOWN,
           int payloadFormat = -1,
           const char* mimeType = "audio",
           const char* mimeSubtype = "",
           int sampleRate = 8000, // samples per second
           int preferredPacketLength = 20000, // micro seconds
           int numChannels = 1,
           const char* formatSpecificData = "",
           const int CPUCost = SDP_CODEC_CPU_LOW,
           const int BWCost = SDP_CODEC_BANDWIDTH_NORMAL,
           const int videoFormat = SDP_VIDEO_FORMAT_QCIF,
           const int videoFmtp = 0);
     //:Default constructor

   SdpCodec(const SdpCodec& rSdpCodec);
     //:Copy constructor

   virtual
   ~SdpCodec();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   SdpCodec& operator=(const SdpCodec& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

   enum SdpCodecTypes getCodecType() const;
   //: Get the internal/Pingtel codec type id
   // Note it is possible that the format id/type used
    // in the SDP "m" field and RTP header is different
    // than these internally used ids.

   int getCodecPayloadFormat() const;
   //: Get the SDP/RTP payload id to be used for this codec
   // This is the id used in the SDP "m" format sub-field
   // and RTP header.

   void setCodecPayloadFormat(int formatId);
   //: Set the SDP/RTP payload id to be used for this codec

   virtual void getSdpFmtpField(UtlString& formatSpecificData) const;
   //: Get the format specific parameters for the SDP
   // This is what goes in the SDP "a" field in the
   // format: "a=fmtp <payloadFormat> <formatSpecificData>

   void getMediaType(UtlString& mimeMajorType) const;
   //: Get the media type for the codec
   // This is the mime major type (i.e. video, audio, etc)

   void getEncodingName(UtlString& mimeSubtype) const;
   //: MimeSubtype used as encoding name
   // This is the encoding name used in the SDP
   // "a=rtpmap: <payloadFormat> <mimeSubtype/sampleRate[/numChannels]"
   // field.

   int getSampleRate() const;
   //: Get the number of samples per second

   int getVideoFormat() const;
   //: Return the video format

   int getNumChannels() const;
   //: Get the number of channels

   int getPacketLength() const;
   //: Get the preferred packet size in micro seconds
   // Get the preferred (not mandated) packet
   // size.  This measure is in microseconds and
   // is independent of whether this is frame or
   // sample based codec

   void toString(UtlString& sdpCodecContents) const;
   //: Get a string dump of this codecs definition

   int getCPUCost() const;
   //:Get the CPU cost for this codec.
   //!returns SDP_CODEC_CPU_HIGH or SDP_CODEC_CPU_LOW

   int getBWCost() const;
   //:Get the bandwidth cost for this codec.
   //!returns SDP_CODEC_BANDWIDTH_HIGH, SDP_CODEC_BANDWIDTH_NORMAL or SDP_CODEC_BANDWIDTH_LOW

   int getVideoFmtp() const;
   //:Get the video format bitmap

   void setVideoFmtp(const int videoFmtp);
   //:Set the video format bitmap

/* ============================ INQUIRY =================================== */

   UtlBoolean isSameDefinition(SdpCodec& codec) const;
   //: Returns TRUE if this codec is the same definition as the given codec
   // That is the encoding type and its characteristics, not the payload type.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

        //enum SdpCodecTypes mCodecType; // Internal id
    int mCodecPayloadFormat; // The id which appears in SDP & RTP
    UtlString mFormatSpecificData; // a=fmtp parameter
    UtlString mMimeType; // audio, video, etc.
    UtlString mMimeSubtype; // a=rtpmap mime subtype value
    int mSampleRate; // samples per second
    int mNumChannels;
    int mPacketLength; // micro seconds
    int mCPUCost; // relative cost of a SDP codec
    int mBWCost;
    int mVideoFormat;
    int mVideoFmtp;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SdpCodec_h_

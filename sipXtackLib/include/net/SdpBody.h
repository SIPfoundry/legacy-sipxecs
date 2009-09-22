//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _SdpBody_h_
#define _SdpBody_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlSListIterator.h"

#include <net/HttpBody.h>
#include <net/NameValuePair.h>
#include <net/SdpCodec.h>

// DEFINES
// Crypto suites
#define AES_CM_128_HMAC_SHA1_80   1
#define AES_CM_128_HAMC_SHA1_32   2
#define F8_128_HMAC_SHA1_80       3
// Protection level
#define SRTP_ENCRYPTION           0x0001
#define SRTP_AUTHENTICATION       0x0002
#define SRTP_SEND                 0x0004
#define SRTP_RECEIVE              0x0008
#define SRTP_OFF                  0
#define SRTP_ON                   (SRTP_ENCRYPTION|SRTP_AUTHENTICATION|SRTP_SEND|SRTP_RECEIVE)
#define SRTP_SECURITY_MASK        (SRTP_ENCRYPTION|SRTP_AUTHENTICATION)
// Key length
#define SRTP_KEY_LENGTH          30

// sipX-specific SDP attributes
#define NTAP_PROPRIETARY_SDP_ATTRIB     ("x-sipx-ntap")

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define SDP_CONTENT_TYPE "application/sdp"
// STRUCTS
// TYPEDEFS
typedef struct SdpSrtpParameters
{
    int cipherType;
    int securityLevel;
    unsigned char masterKey[SRTP_KEY_LENGTH+1];
} SdpSrtpParameters;

/** Enum values to represent SDP directionality attributes.
 *  The values are constructed so that boolean operations on them
 *  behave as expected, with SdpDirectionalityInactive being 0.
 */
typedef
   enum
{
   sdpDirectionalityInactive = 0,
   sdpDirectionalitySendOnly = 1,
   sdpDirectionalityRecvOnly = 2,
   sdpDirectionalitySendRecv = 3
} SdpDirectionality;

// FORWARD DECLARATIONS
class SdpCodecFactory;

/// Container for MIME type application/sdp.
/**
 * This body type gets constructed when a HttpBody is a single
 * part MIME or multipart MIME which has a part of type
 * application/sdp.  This object has methods to manipulate,
 * iterate and construct SDP fields and media stream definitions
 *
 * In the descriptions below:
 *
 * * A 'valid field string' is any sequence of UTF-8 encoded ISO 10646 characters
 *   except 0x00 (Nul), 0x0A (LF), and 0x0D (CR).
 *   For other character sets, see RFC 2327.
 *
 * * An 'IP address' is in the canonical text format:
 *   For IP4 this is of the format:
 *   nnn.nnn.nnn.nnn where nnn is 0 to 255
 *
 * * The 'network type' and 'address type' are strings as defined
 *   in rfc 2327; should probably be "IN" and "IP4" respectively for now.
 *
 */
class SdpBody : public HttpBody
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   /// The a= values that represent the directionality values.
   static const char* sdpDirectionalityStrings[4];

   /// Calcuate the "reverse" of a given directionality.
   //  I.e., sdpDirectionalityReverse[sdpDirectionalitySendOnly] ==
   //  sdpDirectionalityRecvOnly.
   static const SdpDirectionality sdpDirectionalityReverse[4];

/**
 * @name ====================== Constructors and Destructors
 * @{
 */
   /// Construct from an existing SDP message value, or create an empty body.
   SdpBody(const char* bytes = NULL, ///< NULL creates an empty body
           ssize_t byteCount = -1        ///< -1 means treat bytes as null terminated
           );

   /// Copy constructor
   SdpBody(const SdpBody& rSdpBody);

   virtual SdpBody* copy() const;

   /// Destructor
   virtual
      ~SdpBody();

///@}

/**
 * @name ====================== Message Serialization Interfaces
 *
 * @{
 */

   /// Get the string length this body would be if serialized
   virtual ssize_t getLength() const;

   /// Get the serialized string representation of this SDP message.
   virtual void getBytes(const char** bytes, ///< buffer space where SDP is written, null terminated
                         ssize_t* length      ///< number of bytes written (not including the null terminator)
                         ) const;

   /// Get the serialized string representation of this SDP message.
   virtual void getBytes(UtlString* bytes, ///< message output
                         ssize_t* length    ///< number of bytes in message
                         ) const;

///@}

/**
 * @name ====================== Header Setting Interfaces
 *
 * These methods set the standard header fields (the ones not in a media set)
 *
 * @{
 */

   void setStandardHeaderFields(const char* sessionName,  ///< any valid field string
                                const char* emailAddress, ///< email address per rfc 2327
                                const char* phoneNumber,  ///< phone number address per rfc 2327
                                const char* originatorAddress ///< IP address
                                );


   void setSessionNameField(const char* sessionName);

   void setOriginator(const char* userId,/**< user id of caller or "-" if host does not support
                                          *   the idea of user logon */
                      int sessionId,     ///< unique id for this call session
                      int sessionVersion,/**< number to get incremented each time
                                          *   the SDP data gets modified for this call session */
                      const char* address///< IP address
                      );

   void setEmailAddressField(const char* emailAddress);

   void setPhoneNumberField(const char* phoneNumber);

   // Append a 't' field after any existing times, converting from epoch->ntp time.
   void addEpochTime(unsigned long epochStartTime,
                     unsigned long epochEndTime = 0
                     );

   // Append a 't' field after any existing times.
   void addNtpTime(unsigned long ntpStartTime,
                   unsigned long ntpEndTime = 0
                   );

///@}

/**
 * @name ====================== Media Setting Interfaces
 *
 * These interfaces are used to build up media descriptions
 *
 * @{
 */

   /// Create a set of media codec and address entries
   void addAudioCodecs(const char* rtpAddress,
                       int rtpPort,
                       int rtcpPort,
                       int videoRtpPort,
                       int videoRtcpPort,
                       int numRtpCodecs,
                       SdpCodec* rtpCodecs[],
                       SdpSrtpParameters& srtpParams
                       );

   /// Create a response to a set of media codec and address entries.
   void addAudioCodecs(const char* rtpAddress,
                       int rtpAudioPort,
                       int rtcpAudioPort,
                       int rtpVideoPort,
                       int rtcpVideoPort,
                       int numRtpCodecs,
                       SdpCodec* rtpCodecs[],
                       SdpSrtpParameters& srtpParams,
                       const SdpBody* sdpRequest ///< Sdp we are responding to
                       );
   /**<
    * This method is for building a SdpBody which is in response
    * to a SdpBody send from the other side
    */


   /// Create a new media set for SDP message.
   void addMediaData(const char* mediaType, ///< "audio", "video", "application", "data", "control"
                     int portNumber,        ///< TCP or UDP poart number for the media stream
                     int portPairCount,     ///< the number of PAIRS of ports to be used for the media stream.
                     const char* mediaTransportType, ///< i.e. "RTP/AVP"
                     int numPayloadTypes,   ///< entries in the payloadType parameter
                     int payloadType[]      /**< format specifier specific to context
                                             * of mediaTransportType.
                                             * (i.e. for TRP/AVP u-law this is 0, a-law is 14).
                                             * see RFC 1890 for the Payload Type numbers
                                             */
                     );

   /// Adds an attribute to the SDP body
   void addAttribute(const char* attributeName,      ///< name of attribute to add
                     const char* pAttributeValue = 0 ///< optional attribute value.
                          );

   void addCodecParameters(int numRtpCodecs,
                           SdpCodec* rtpCodecs[],
                           const char* szMimeType = "audio"
                           );

   /// Set address.
   void addAddressData(const char* ipAddress /**< for IP4 this is of the format:
                                              * nnn.nnn.nnn.nnn where nnn is 0 to 255 */
                       );
    /**<
     * Set address for SDP message header or specific media set if called
     * after addAddressData.
     */

   void addAddressData(const char* networkType, ///< network type - should be "IN"
                       const char* addressType, ///< address type - should be "IP4"
                       const char* ipAddress    ///< IP address
                       );

   void addRtpmap(int payloadType,
                  const char* mimeSubtype,
                  int sampleRate,
                  int numChannels
                  );

   void addSrtpCryptoField(SdpSrtpParameters& params);

   void addFormatParameters(int payloadType,
                            const char* formatParameters
                            );

    /**
     * Set the candidate attribute per draft-ietf-mmusic-ice-04
     */
    void addCandidateAttribute(const char* id,
                               double qValue,
                               const char* userFrag,
                               const char* password,
                               const char* unicastIp,
                               int unicastPort,
                               const char* candidateIp,
                               int candidatePort) ;

///@}

 /**
  * @name ====================== SDP Modification Interfaces
  *
  * These interfaces are used to add to or modify an existing SDP body
  *
  * @{
  */
    /// Adds an attribute to an existing media description
    bool insertMediaAttribute(int mediaIndex,                 ///< which media description set to modify.  -1 means that attributee will be added to session description
                              const char* attributeName,      ///< name of attribute to add
                              const char* pAttributeValue = 0 ///< optional attribute value.
                               );

    /// Removes an attribute from an existing media description. If the attribute cannot be found
    /// in the designated media description, the routine will try to remove the attribute  from
    /// the session description section.
    bool removeMediaAttribute(int mediaIndex,                 ///< which media description set to modify.  -1 means that attributee will be added to session description
                              const char* attributeName       ///< name of attribute to add
                               );

    /// Set IP address for the indicated media stream.  If a media address (c= line in SDP body)
    /// cannot be found in the designated media description, a c= line with the new address is
    // added to it.  Specifying a mediaIndex of -1 will affect the Session Description section
    bool modifyMediaAddress(int mediaIndex, ///< which media description set to modify.  use -1 of Session Description section
                            const char* pNewAddress  ///< new address information
                               );

    /// Set the port number for the indicated media stream.
    bool modifyMediaPort(int mediaIndex, ///< which media description set to modify
                         int newPort     ///< new port value
                               );

 ///@}

/**
 * @name ====================== Field Reading Interfaces
 *
 * Use these interfaces to get field values rather than the generic interfaces below.
 *
 * @{
 */

   //! Get the number of media description sets
   int getMediaSetCount() const;

   /// Get the index to the next media set of the given type.
   int findMediaType(const char* mediaType,  ///< the media type to search for
                     int startMediaIndex = 0 ///< start searching from here
                     ) const;
   /**<
    * The default is to start from the begining.  The resulting
    * index may be the start index if it is of the given type.
    *
    * @return the media index if it exists, -1 if it does not exist.
    */

   /// Read a full media line.
   UtlBoolean getMediaData(int mediaIndex, ///< the index of the media to read (the nth m line)
                           UtlString* mediaType,
                           int* mediaPort,
                           int* mediaPortPairs,
                           UtlString* mediaTransportType,
                           int maxPayloadTypes,
                           int* numPayloadTypes,
                           int payloadTypes[],
                           SdpDirectionality* directionality) const;

   /// Read whether the media network type is IP4 or IP6.
   UtlBoolean getMediaNetworkType(int mediaIndex, ///< which media description set to read
                                  UtlString* networkType) const;

   /// Get IP address for the indicated media stream.
   UtlBoolean getMediaAddress(int mediaIndex, ///< which media description set to read
                              UtlString* address) const;

   /// Get the media type for the indicated  media stream.
   UtlBoolean getMediaType(int mediaIndex, ///< which media description set to read
                           UtlString* mediaType ///< audio, video, application, etc.
                           ) const;

   /// Get the media direction attribute for the inidicated media stream
   UtlBoolean getMediaDirection(int mediaIndex,
                                SdpDirectionality* directionality) const;

   /// Get the port number for the indicated media stream.
   UtlBoolean getMediaPort(int mediaIndex, ///< which media description set to read
                           int* port) const;

   // Get the rtcp port number of the indicated media stream.
   UtlBoolean getMediaRtcpPort(int mediaIndex, ///< which media description set to read
                               int* port) const ;

   /// Get the number of port pairs in media stream.
   UtlBoolean getMediaPortCount(int mediaIndex, ///< which media description set to read
                                int* numPorts) const;
   /**<
    * Stream pairs start with the port number given by getMediaPort
    * and are incremented from there to derive the additional
    * port numbers
    */

   /// Get the transport protocol for the indicated media stream.
   UtlBoolean getMediaProtocol(int mediaIndex, ///< which media description set to read
                               UtlString* transportProtocol
                               ) const;

   /// Get the payload types for the indicated media stream.
   UtlBoolean getMediaPayloadType(int mediaIndex, ///< which media description set to read
                                  int maxTypes,   ///< size of the payloadTypes array
                                  int* numTypes,  ///< number of entries returned in payloadTypes
                                  int payloadTypes[] ///< array of integer payload types
                                  ) const;

   /// Get the attribute the indicated media stream.
   /// @return TRUE is the attribute is found in either the specified media description or the
   ///         session descrpition section.
   UtlBoolean getMediaAttribute(int mediaIndex,                 ///< which media description set to read
                                const UtlString& attributeName, ///< name of attribute to search for
                                UtlString* pAttributeValue = 0  ///< will receive attrib's value if found and not a simple flag
                                ) const;

   /// Media field accessor utility.
   UtlBoolean getMediaSubfield(int mediaIndex,
                               int subfieldIndex,
                               UtlString* subField
                               ) const;

   /// Get the subfields of the rtpmap field.
   UtlBoolean getPayloadRtpMap(int payloadType,        ///< which rtp map to read
                               UtlString& mimeSubtype, ///< the codec name (mime subtype)
                               int& sampleRate,        ///< the number of samples/sec. (-1 if not set)
                               int& numChannels        ///< the number of channels (-1 if not set)
                               ) const;

   // Get the fmtp parameter
   UtlBoolean getPayloadFormat(int payloadType,
                               UtlString& fmtp,
                               int& videoFmtp) const;

   // Get the crypto field for SRTP
   UtlBoolean getSrtpCryptoField(int mediaIndex,                  ///< mediaIndex of crypto field
                                 int index,                       ///< Index inside of media type
                                 SdpSrtpParameters& params) const;

   /**<
    * Find the "a" record containing an rtpmap for the given
    * payload type id, parse it and return the parameters for it.
    */


   /// Find the send and receive codecs from the rtpCodecs array which are compatible with this SdpBody..
   void getBestAudioCodecs(int numRtpCodecs,        // this version does not seem to be used 04/2009
                           SdpCodec rtpCodecs[],
                           UtlString* rtpAddress,
                           int* rtpPort,
                           int* sendCodecIndex,
                           int* receiveCodecIndex) const;
   ///< It is assumed that the best matches are first in the body



   /// Find the send and receive codecs from the rtpCodecs array which are compatible with this SdpBody.
   void getBestAudioCodecs(SdpCodecFactory& localRtpCodecs, // version used by SipConnection class
                           int& numCodecsInCommon,
                           SdpCodec**& codecsInCommonArray,
                           UtlString& rtpAddress,
                           int& rtpPort,
                           int& rtcpPort,
                           int& videoRtpPort,
                           int& videoRtcpPort) const;
   ///< It is assumed that the best are matches are first in the body.


   // Get the list of codecs in common between this SDP and a codec factory.
   void getCodecsInCommon(/** codec payload types in this SDP to consider
                           *  for inclusion */
                          int audioPayloadIdCount,
                          int videoPayloadCount,
                          int audioPayloadTypes[],
                          int videoPayloadTypes[],
                          /// codec factory to compare this SDP with
                          SdpCodecFactory& codecFactory,
                          /// output
                          int& numCodecsInCommon,
                          SdpCodec* codecs[]) const;

   // Find common encryption suites
   void getEncryptionInCommon(SdpSrtpParameters& audioParams,
                              SdpSrtpParameters& videoParams,
                              SdpSrtpParameters& commonAudioParms,
                              SdpSrtpParameters& commonVideoParms);

    /**
     * Get the candidate attribute per draft-ietf-mmusic-ice-04
     */
    UtlBoolean getCandidateAttribute(int index,
                                     UtlString& rId,
                                     double& rQvalue,
                                     UtlString& rUserFrag,
                                     UtlString& rPassword,
                                     UtlString& rUnicastIp,
                                     int& rUnicastPort,
                                     UtlString& rCandidateIp,
                                     int& rCandidatePort) const ;

///@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
   friend class SdpBodyTest;

   /**
    * @name ====================== Generic SDP field accessors
    *
    * It is better to use the more specific accessor methods above for the field type you need
    * than to use these generic interfaces.  These may be deprecated in the future.
    *
    * @{
    */

   /// Return the number of fields in the body.
   int getFieldCount() const;

   /// Return the name and value of the nth field in the body.
   UtlBoolean getValue(int fieldIndex,
                       UtlString* name,
                       UtlString* value) const;
   /**<
    * It is better to use a more specific accessor method for the field type you need
    * than to iterate over all fields using this interface.
    *
    * @return FALSE if there are not enough fields
    */

   /// Set a specific field.
   void setValue(const char* name,       ///< field name to set
                 const char* value       ///< value for the named field
                 );

///@}

   /// Parse an existing string into the internal representation.
   void parseBody(const char* bytes = NULL, ///< NULL creates an empty body
                  ssize_t byteCount = -1        ///< -1 means treat bytes as null terminated
                  );


/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   /// Get the position of the first of a set of headers, checking in order.
   ssize_t findFirstOf( const char* headers /**< null terminated set of single-character header names */);
   /**<
    * @returns The index of the first of the headers found, or UTL_NOT_FOUND if none are present.
    *
    * Example:
    * @code
    *
    *    ssize_t timeLocation = findFirstOf("zkam");
    *    addValue("t", value.data(), timeLocation);
    *
    * @endcode
    */

   /// Is the named field optional according to RFC 2327? true==optional.
   bool isOptionalField(const char* name) const;

   /// Add the given name/value pair to the body at the specified position.
   void addValue(const char* name,
                 const char* value = NULL,
                 ssize_t fieldIndex = UTL_NOT_FOUND /**< UTL_NOT_FOUND == at the end */
                 );

   UtlSList* sdpFields;

   /// Position to the field instance.
   static NameValuePair* positionFieldInstance(int fieldInstanceIndex, ///< field instance of interest starting a zero
                                               UtlSListIterator* iter, /**< an iterator on sipFields. The search will
                                                                        *  start at the begining.*/
                                               const char* fieldName
                                               );
   /**<
    * Positions the given iterator to the to the nth instance of fieldName.
    * @return the field name value pair if it exists leaving the iterator positioned there.
    */

   static NameValuePair* findFieldNameBefore(UtlSListIterator* iter,
                                             const char* targetFieldName,
                                             const char* beforeFieldName);

   /**<
    * Modifies a specific subfield on a given SDP line value.
    */
   bool modifySdpSubfieldValue( UtlString& sdpLineValue,
                                int subFieldToModifyIndex,
                                const UtlString& subFiledReplacement );

   /**<
    * Returns the reference of the first SDP attribute (a= line) matching
    * the supplied name for the specified media description.  If the attribute
    * is not found in the media descrption, the session description session
    * is then searched for a match.
    */
   NameValuePair* getRefToMediaAttributeByName( int mediaIndex,
                                                const char* pAttributeName ) const;


   //! Disabled assignment operator
   SdpBody& operator=(const SdpBody& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SdpBody_h_

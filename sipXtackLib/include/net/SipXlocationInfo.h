/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */

#ifndef _SIP_X_LOCATION_INFO_H_
#define _SIP_X_LOCATION_INFO_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/UtlBool.h"
#include "net/SipMessage.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipXlocationInfoTest;

/// Maintain SipXSignedHeader information
/**
 * This class encapsulates SipXSignedHeader information. SipXSignedHeader is used by the
 * sipXproxy to inform sipXregistrar of which location should be associated with an INVITE request
 * based on the location of the REFER request originator. This is useful in call transfer
 * scenarios, where the transfered call should be bound to the location of the call transfer
 * initiator, and directed through the gateway associated with the initiator location.
 *
 * SipXSignedHeader information is carried in X-Sipx-Location-Info header of the following format:
 *
 *  X-Sipx-Location-Info: "<" <identity>;location=<location>;signature=<signature-hash> ">"
 * where:
 * - <identity> is a user identity in addr-spec format as a SIP-URI.
 * - <location> is location of which the user belongs
 * - <signature-hash> is  MD5(<identity><location><secret>)
 *
 * The value of X-Sipx-Location-Info header is signed using MD5. The signature is calculated
 * over the content of the header value and a unique secret, known only to sipXecs components
 * in a given installation. This should prevent (or minimize) the replay attacks on the
 * system making it relatively difficult to spoof the X-Sipx-Location-Info header.
 *
 * To access the SipXSignedHeader information in a message, construct a SipXSignedHeader object
 * passing the SipMessage to the SipXSignedHeader constructor. Use setInfo to set location info,
 * use getIdentity and getLocation method to get location info. Use remove method to remove the
 * info from message.
 *
 * @nosubgrouping
 */
class SipXSignedHeader
{
  public:

  /// Default Constructor
  SipXSignedHeader(const UtlString&  identity, const UtlString& headerName);

  // ================================================================
  /** @name            Decoding Operations
   *
   * These methods access the location info in a message.
   */
  ///@{

  /// Constructor which decodes SipXSignedHeader from a received message.
  SipXSignedHeader(const SipMessage& message,  ///< message to scan for an location info header
      const UtlString& headerName);
  /**<
   * The message may or may not contain SipXSignedHeader information. If location information is
   * present but signature does not match subsequent calls to get information will return false
   * until the new info is set via a call to setInfo
   */

  /// Extract location saved in the SipXSignedHeader.
  bool getParam(const UtlString&  paramName, UtlString& paramValue) const;
  /**<
   * Returns encapsulated location
   * @returns true if the SipXSignedHeader information is valid, false if not.
   * @param location is null if return was false
   */

  /// Extract identity saved in the SipXSignedHeader.
  bool getIdentity(UtlString&  identity) const;
  /**<
   * Returns encapsulated identity in the format user@domain
   * @returns true if the SipXSignedHeader information is valid, false if not.
   * @param identity is null if return was false; if true, it is in the format user@domain
   */


  ///@}

  // ================================================================
  /** @name            Encoding Operations
   *
   * These methods manipulate the information encoded into a request.
   */
  ///@{

  /// Stores location info
  void setParam(const UtlString&  paramName, const UtlString& paramValue);
  /**<
   * Establish a new value for the identity and location to be included
   * when the SipXSignedHeader information is generated.
   * @param identity is in the format user@domain
   */

  /// Remove location info header from a message.
  static void remove(SipMessage& request, const UtlString& headerName);

  /// Encode location information into a URL
  bool encodeUri(Url& uri           ///< target URI to get encoded location information
      );
  /**<
   * Encodes new information, identity and location into a URI based
   * on the stored info.
   *
   * @returns true if the SipXSignedHeader information is valid, false if not.
   *
   * @Note  Existing location info in the URI is removed
   */

  /// Initialize the secret value used to sign SipXSignedHeader information.
  static void setSecret(const char* secret /**< a random value used as input to sign the
                              *  header info.  This should be chosen such that it:
                              * - is hard for an attacker to guess (includes at
                              *  least 32 bits of cryptographicaly random data)
                              * - is the same for all components in a given installation
                              *  this is to sure components can generate location info and
                              *  can validate authenticity of the information.
                              */
          );
  /**<
   * This must be called once at initialization time,
   * before any SipXSignedHeader objects are created.
   *
   * It may be called after that, but doing so with a
   * new value will invalidate any outstanding headers.
   */
  ///@}

  /// destructor
  ~SipXSignedHeader();

  /// Encodes user location info: identity and location
  bool encode(UtlString& headerValue ///< encoded header value
      );
  /**<
   * Encodes the user location info, identity and location, into a string value
   * including the signature
   */

  protected:

  friend class SipXSignedHeaderTest; // to allow unit tests for protected members

  /// Check the signature, extract the identity and location
  bool decodeHeader(const UtlString& headerValue ///<  headerValue value to decode
      );
  /**<
   * Decodes the identity and location from the header, validates authenticity of
   * the information, sets validity flag accordingly
   *
   * @returns true if the location was correctly signed and successfully parsed
   */

  /// Check the signature, extract the identity and location
  bool decode(const SipMessage& message   ///<  message to take header from.
      );
  /**<
   * Searches for an location info header and, if found, decodes the identity and
   * location from the header, validates authenticity of the information, sets
   * validity flag accordingly
   *
   * @returns true if the location was correctly signed and successfully parsed
   */

  private:
  static const char* SignatureUrlParamName;

  UtlString  _identity;  /// identity of the user
  UtlString  _headerName;
  bool     _isValid;
  Url     _encodedUrl;

  static UtlString _sSignatureSecret;

  // @cond INCLUDENOCOPY
  // There is no copy constructor.
  SipXSignedHeader(const SipXSignedHeader& nocopyconstructor);

  // There is no assignment operator.
  SipXSignedHeader& operator=(const SipXSignedHeader& noassignmentoperator);
  // @endcond
};

#endif // _SIP_X_LOCATION_INFO_H_

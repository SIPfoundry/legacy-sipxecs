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

// SYSTEM INCLUDES
#include "os/OsSysLog.h"
#include "net/NetMd5Codec.h"

// APPLICATION INCLUDES
#include "net/SipXlocationInfo.h"

// CONSTANTS
const char* SipXlocationInfo::SignatureUrlParamName = "signature";
const char* SipXlocationInfo::LocationUrlParamName = "location";
const char* SipXlocationInfo::HeaderName = "X-SipX-Location-Info";

// STATIC VARIABLES
UtlString  SipXlocationInfo::_sSignatureSecret;

/*****************************************************************
 * SipXlocationInfo Encoding
 *
 *  X-Sipx-Location-Info: "<" <identity>;location=<location>;signature=<signature-hash> ">"
 * where:
 * - <identity> is a user identity in addr-spec format as a SIP-URI.
 * - <location> is location of which the user belongs
 * - <signature-hash> is  MD5(<identity><location><secret>)
 */

/// Default Constructor
SipXlocationInfo::SipXlocationInfo()
  : _isValid(false)
{
}

/// destructor
SipXlocationInfo::~SipXlocationInfo()
{
}

/// Decode the location from a message by searching for SipXlocationInfo
SipXlocationInfo::SipXlocationInfo(const SipMessage& message)
  : _isValid(false)
{
  decode(message);
}

/// Extract location saved in the SipXlocationInfo.
bool SipXlocationInfo::getLocation(UtlString& location) const
{
  location.remove(0);
  if (_isValid)
  {
    location = _location;
  }

  return _isValid;
}

/// Extract identity saved in the SipXlocationInfo.
bool SipXlocationInfo::getIdentity(UtlString&  identity) const
{
  identity.remove(0);
  if (_isValid)
  {
    identity = _identity;
  }

  return _isValid;
}

/// Stores the new value of identity and location
void SipXlocationInfo::setInfo(const UtlString&  identity, const UtlString&  location)
{
  _identity = identity;
  _location = location;

  // Reset the validity flag
  _isValid = true;
}

/// Remove location info from a message.
void SipXlocationInfo::remove(SipMessage& message)
{
  int headerCount = message.getCountHeaderFields(HeaderName);

  if (headerCount > 0)
  {
    OsSysLog::add(FAC_SIP, PRI_WARNING,
        "SipXlocationInfo::remove"
        ": '%d' occurrances of %s",
        headerCount, HeaderName);

    for (int i = headerCount - 1; i >= 0 ; i--)
    {
      message.removeHeader(HeaderName, i);
    }
  }
}

/// Encode location info into a URL
bool SipXlocationInfo::encodeUri(Url& uri)
{
  // Don't proceed if the encapsulated info is invalid
  if (!_isValid)
  {
    OsSysLog::add(FAC_SIP, PRI_CRIT,
        "SipXlocationInfo::encodeUri encapsulated SipXlocationInfo is invalid");
  }
  else
  {
    // make sure no existing location info header in the URI
    uri.removeHeaderParameter(SipXlocationInfo::HeaderName);


    UtlString value;
    encode(value);
    uri.setHeaderParameter(HeaderName, value.data());

    OsSysLog::add(FAC_SIP, PRI_DEBUG,
        "SipXlocationInfo::encodeUri encoded URI '%s'",
        uri.toString().data());
  }

  return _isValid;
}

/// Encodes location info
void SipXlocationInfo::encode(UtlString& headerValue)
{
  //<signature-hash> is  MD5(<identity><location><secret>)
   NetMd5Codec signature;
   signature.hash(_identity);
   signature.hash(_location);
   signature.hash(_sSignatureSecret);

   UtlString signatureStr;
   signature.appendHashValue(signatureStr);

   Url encodedUrl(_identity);
   encodedUrl.setScheme(Url::SipUrlScheme);
   encodedUrl.setUrlParameter(LocationUrlParamName, _location.data());
   encodedUrl.setUrlParameter(SignatureUrlParamName, signatureStr.data());

   encodedUrl.toString(headerValue);

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
       "SipXlocationInfo::encode location info '%s'", headerValue.data());
}


/// Check the signature and parse the location info
bool SipXlocationInfo::decodeHeader(const UtlString& headerValue)
{
  OsSysLog::add(FAC_SIP, PRI_DEBUG,
      "SipXlocationInfo::decode parse '%s'", headerValue.data());

  _identity.remove(0);
  _location.remove(0);
  _isValid = false;

  bool decodeError = false; // false if the info was correctly signed and successfully parsed
  UtlString decodedIdentity;
  UtlString actualSignature;

  Url encodedUrl(headerValue, Url::NameAddr);
  if (Url::SipUrlScheme == encodedUrl.getScheme())
  {
    UtlString decodedIdentity;
    // Only proceed if the URL parsing succeeded
    // Extract the identity
    encodedUrl.getIdentity(_identity);

    // Extract location parameter
    if (encodedUrl.getUrlParameter(LocationUrlParamName, _location))
    {
      // Extract signature parameter
      if (!encodedUrl.getUrlParameter(SignatureUrlParamName, actualSignature))
      {
        decodeError = true;
        OsSysLog::add(FAC_SIP, PRI_WARNING,
            "SipXlocationInfo::decode '%s' missing '%s' param",
            headerValue.data(), SignatureUrlParamName);
      }
    }
    else
    {
      decodeError = true;
      OsSysLog::add(FAC_SIP, PRI_WARNING,
          "SipXlocationInfo::decode '%s' missing '%s' param",
          headerValue.data(), LocationUrlParamName);
    }
  }
  else
  {
    decodeError = true;
    OsSysLog::add(FAC_SIP, PRI_WARNING,
        "SipXlocationInfo::decode '%s' URL parsing failed",
        headerValue.data());
  }

  // validate signature hash
  if (!decodeError)
  {
    //<signature-hash> is  MD5(<identity><location><secret>)
    NetMd5Codec validSignatureHash;
    validSignatureHash.hash(_identity);
    validSignatureHash.hash(_location);
    validSignatureHash.hash(_sSignatureSecret);

    UtlString validSignature;
    validSignatureHash.appendHashValue(validSignature);

    if (validSignature.compareTo(actualSignature) == 0)
    {
      // the signature checks out
      _isValid = TRUE;
    }
    else
    {
      decodeError = true;
      OsSysLog::add(FAC_SIP, PRI_WARNING,
          "SipXlocationInfo::decode '%s' invalid signature '%s' != '%s'",
          headerValue.data(), actualSignature.data(), validSignature.data());
    }
  }

  if (decodeError)
  {
    _identity.remove(0);
    _location.remove(0);
  }

  return _isValid;
}

/// Check the signature and parse the identity contained in specified header name
bool SipXlocationInfo::decode(const SipMessage& message)
{
  bool foundHeader = false;

  int headerCount = message.getCountHeaderFields(HeaderName);
  if (1 == headerCount)
  {
    foundHeader = decodeHeader(message.getHeaderValue(0, HeaderName));
  }
  else if (headerCount>1)
  {
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
        "SipXlocationInfo::decode: '%d' occurrences of %s",
        headerCount, HeaderName);
    foundHeader = decodeHeader(message.getHeaderValue(headerCount - 1, HeaderName));
  }

  if (foundHeader)
  {
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
        "SipXlocationInfo::decode: found %s, identity '%s', location '%s'",
        HeaderName, _identity.data(), _location.data());
  }

  return foundHeader;
}

/// Initialize the secret value used to sign hashes.
void SipXlocationInfo::setSecret(const char* secret /**< a random value used as input to sign the
                                    * state value.  This should be chosen such that it:
                                    * - is hard for an attacker to guess
                                    * - ideally, is the same in replicated authproxies
                                    *  (this is not important yet, since we don't use
                                    *  SRV names in the Record-Route targets).
                                    */
                      )
{
  /*
   * This must be called once at initialization time,
   * before any SipXlocationInfo objects are created.
   *
   * It may be called after that, but doing so with a
   * new value will invalidate any outstanding identities.
   */
  if (!_sSignatureSecret.isNull() && _sSignatureSecret.compareTo(secret))
  {
    OsSysLog::add(FAC_SIP,PRI_NOTICE,
        "SipXlocationInfo::setSecret called more than once;\n"
        " previously signed state will now fail signature checks");
  }
  _sSignatureSecret.remove(0);
  _sSignatureSecret.append(secret);
}

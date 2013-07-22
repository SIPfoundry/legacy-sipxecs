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
#include "os/OsLogger.h"
#include "net/NetMd5Codec.h"

// APPLICATION INCLUDES
#include "net/SipXlocationInfo.h"

// CONSTANTS
const char* SipXSignedHeader::SignatureUrlParamName = "signature";

// STATIC VARIABLES
UtlString  SipXSignedHeader::_sSignatureSecret;

/*****************************************************************
 * SipXSignedHeader Encoding
 *
 *  X-Sipx-Location-Info: "<" <identity>;location=<location>;signature=<signature-hash> ">"
 * where:
 * - <identity> is a user identity in addr-spec format as a SIP-URI.
 * - <location> is location of which the user belongs
 * - <signature-hash> is  MD5(<identity><location><secret>)
 */

/// Default Constructor
SipXSignedHeader::SipXSignedHeader(const UtlString&  identity, const UtlString& headerName)
  : _identity(identity),
    _headerName(headerName),
    _isValid(false),
    _encodedUrl(_identity)
{
    _encodedUrl.setScheme(Url::SipUrlScheme);

    if (!_identity.isNull() && !_headerName.isNull())
    {
      _isValid = true;
    }
}

/// destructor
SipXSignedHeader::~SipXSignedHeader()
{
}

/// Decode the location from a message by searching for SipXSignedHeader
SipXSignedHeader::SipXSignedHeader(const SipMessage& message, const UtlString& headerName)
  : _headerName(headerName),
    _isValid(false)
{
  decode(message);
}

/// Extract location saved in the SipXSignedHeader.
bool SipXSignedHeader::getParam(const UtlString&  paramName, UtlString& paramValue) const
{
  paramValue.remove(0);
  if (_isValid)
  {
    return _encodedUrl.getUrlParameter(paramName, paramValue);
  }

  return _isValid;
}


/// Extract identity saved in the SipXSignedHeader.
bool SipXSignedHeader::getIdentity(UtlString&  identity) const
{
  identity.remove(0);
  if (_isValid)
  {
    identity = _identity;
  }

  return _isValid;
}

/// Stores the new value of identity and location
void SipXSignedHeader::setParam(const UtlString&  paramName, const UtlString&  paramValue)
{
  if (_isValid)
  {
    _encodedUrl.setUrlParameter(paramName, paramValue.data());
  }
}


/// Remove location info from a message.
void SipXSignedHeader::remove(SipMessage& message, const UtlString& headerName)
{
  int headerCount = message.getCountHeaderFields(headerName);

  if (headerCount > 0)
  {
    Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
        "SipXSignedHeader::remove"
        ": '%d' occurrences of %s",
        headerCount, headerName.data());

    for (int i = headerCount - 1; i >= 0 ; i--)
    {
      message.removeHeader(headerName, i);
    }
  }
}

/// Encode location info into a URL
bool SipXSignedHeader::encodeUri(Url& uri)
{
  // Don't proceed if the encapsulated info is invalid
  if (!_isValid)
  {
    Os::Logger::instance().log(FAC_SIP, PRI_CRIT,
        "SipXSignedHeader::encode encapsulated SipXSignedHeader is invalid");
  }
  else
  {
    // make sure no existing location info header in the URI
    uri.removeHeaderParameter(_headerName);


    UtlString value;
    encode(value);
    uri.setHeaderParameter(_headerName, value.data());

    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
        "SipXSignedHeader::encodeUri encoded URI '%s'",
        uri.toString().data());
  }

  return _isValid;
}

/// Encodes location info
bool SipXSignedHeader::encode(UtlString& headerValue)
{
  // Don't proceed if the encapsulated info is invalid
  if (!_isValid)
  {
    Os::Logger::instance().log(FAC_SIP, PRI_CRIT,
        "SipXSignedHeader::encode encapsulated SipXSignedHeader is invalid");
  }
  else
  {
    //<signature-hash> is  MD5(<identity><location><secret>)
     NetMd5Codec signature;
     UtlString body;
     _encodedUrl.toString(body);
     signature.hash(body);
     Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
         "SipXSignedHeader::decode WILL DO SIG FOR '%s'", body.data());
     signature.hash(_sSignatureSecret);

     UtlString signatureStr;
     signature.appendHashValue(signatureStr);

     _encodedUrl.setUrlParameter(SignatureUrlParamName, signatureStr.data());

     _encodedUrl.toString(headerValue);

     Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
         "SipXSignedHeader::encode location info '%s'", headerValue.data());
  }

  return _isValid;
}


/// Check the signature and parse the location info
bool SipXSignedHeader::decodeHeader(const UtlString& headerValue)
{
  Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
      "SipXSignedHeader::decode parse '%s'", headerValue.data());

  _identity.remove(0);
  _isValid = false;

  bool decodeError = false; // false if the info was correctly signed and successfully parsed
  UtlString decodedIdentity;
  UtlString actualSignature;

  bool ret=false;
  ret = _encodedUrl.fromString(headerValue, Url::NameAddr);
  Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
         "SipXSignedHeader::decode first parsing '%d'", ret);

  if (Url::SipUrlScheme == _encodedUrl.getScheme())
  {
    UtlString decodedIdentity;
    // Only proceed if the URL parsing succeeded
    // Extract the identity
    _encodedUrl.getIdentity(_identity);

    if (_encodedUrl.getUrlParameter(SignatureUrlParamName, actualSignature))
    {
      _encodedUrl.removeUrlParameter(SignatureUrlParamName);

    }
    else
    {
      decodeError = true;
      Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
          "SipXSignedHeader::decode '%s' missing '%s' param",
          headerValue.data(), SignatureUrlParamName);
    }
  }
  else
  {
    decodeError = true;
    Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
        "SipXSignedHeader::decode '%s' URL parsing failed",
        headerValue.data());
  }

  // validate signature hash
  if (!decodeError)
  {
    //<signature-hash> is  MD5(<identity><location><secret>)
    NetMd5Codec validSignatureHash;
    UtlString body;
    _encodedUrl.toString(body);
    Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
        "SipXSignedHeader::decode WILL CHECK SIG FOR '%s'", body.data());
    validSignatureHash.hash(body);
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
      Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
          "SipXSignedHeader::decode '%s' invalid signature '%s' != '%s'",
          headerValue.data(), actualSignature.data(), validSignature.data());
    }
  }

  if (decodeError)
  {
    _identity.remove(0);
    _encodedUrl.reset();
  }

  return _isValid;
}

/// Check the signature and parse the identity contained in specified header name
bool SipXSignedHeader::decode(const SipMessage& message)
{
  bool foundHeader = false;

  int headerCount = message.getCountHeaderFields(_headerName);
  if (1 == headerCount)
  {
    foundHeader = decodeHeader(message.getHeaderValue(0, _headerName));
  }
  else if (headerCount>1)
  {
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
        "SipXSignedHeader::decode: '%d' occurrences of %s",
        headerCount, _headerName.data());
    foundHeader = decodeHeader(message.getHeaderValue(headerCount - 1, _headerName));
  }

  if (foundHeader)
  {
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
        "SipXSignedHeader::decode: found %s, identity '%s'",
        _headerName.data(), _identity.data());
  }

  return foundHeader;
}

/// Initialize the secret value used to sign hashes.
void SipXSignedHeader::setSecret(const char* secret /**< a random value used as input to sign the
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
   * before any SipXSignedHeader objects are created.
   *
   * It may be called after that, but doing so with a
   * new value will invalidate any outstanding identities.
   */
  if (!_sSignatureSecret.isNull() && _sSignatureSecret.compareTo(secret))
  {
    Os::Logger::instance().log(FAC_SIP,PRI_NOTICE,
        "SipXSignedHeader::setSecret called more than once;\n"
        " previously signed state will now fail signature checks");
  }
  _sSignatureSecret.remove(0);
  _sSignatureSecret.append(secret);
}

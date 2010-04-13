//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsSysLog.h"
#include "utl/UtlRegex.h"
#include "net/NetMd5Codec.h"

// APPLICATION INCLUDES
#include "net/SipXauthIdentity.h"

// CONSTANTS
const char* SipXauthIdentity::SignatureFieldSeparator = ":";
const char* SipXauthIdentity::SignatureUrlParamName = "signature";
SipXauthIdentity::HeaderName SipXauthIdentity::AuthIdentityHeaderName = SIP_SIPX_AUTHIDENTITY;
SipXauthIdentity::HeaderName SipXauthIdentity::PAssertedIdentityHeaderName = "P-Asserted-Identity";

// STATIC VARIABLES
UtlString   SipXauthIdentity::sSignatureSecret;
OsTime      SipXauthIdentity::sSignatureValidityInterval;

/*****************************************************************
 * SipXauthIdentity Encoding
 *
 *   X-Sipx-Authidentity: "<" <identity>;signature=<timestamp><separator><signature-hash> ">"
 * where:
 * - <identity> is a user identity in addr-spec format as a SIP-URI.
 * - <separator> is
 *   - ":" if the identity is bound to the dialog
 *   - "::" if the identity is not bound to the dialog
 * - <timestamp> is epoch seconds as hex without "0x" prefix indicating the
 *   time the signature was generated.
 * - <signature-hash> is
 *   - if bound to the dialog: MD5(<timestamp><secret><from-tag><call-id><identity>)
 *   - if not:                 MD5(<timestamp><secret><identity>)
 */
const RegEx SignatureRegEx("([0-9A-F]+):(:?)("MD5_REGEX")");

// method to convert a hexadecimal string value to an unsigned long
bool SipXauthIdentity::from_string(unsigned long & value, const UtlString& s)
{
   char* end;
   value = strtoul(s.data(), &end, 16);
   return (end - s.data() == (int)s.length());
}

/// Default Constructor
SipXauthIdentity::SipXauthIdentity()
  : mIsValidIdentity(FALSE)
{
}

/// destructor
SipXauthIdentity::~SipXauthIdentity()
{
}

/// Decode the identity from a message.
SipXauthIdentity::SipXauthIdentity(const SipMessage& message,
                                   const HeaderName headerName,
                                   DialogRule bindRule
                                   )
  : mIsValidIdentity(FALSE)
{
   UtlString callId;
   UtlString fromTag;
   Url fromUrl;
   message.getCallIdField(&callId);
   message.getFromUrl(fromUrl);
   fromUrl.getFieldParameter("tag", fromTag);

   decode(headerName, message, callId, fromTag, bindRule);
}

/// Decode the identity from a message by searching for SipXauthIdentity then P-Asserted-Identity
SipXauthIdentity::SipXauthIdentity( const SipMessage& message,
                                    UtlString& matchedHeaderName,
                                    bool bSipXauthIdentityTakesPrecedence,
                                    DialogRule bindRule )
  : mIsValidIdentity(FALSE)
{
   UtlString callId;
   UtlString fromTag;
   Url fromUrl;
   message.getCallIdField(&callId);
   message.getFromUrl(fromUrl);
   fromUrl.getFieldParameter("tag", fromTag);
   matchedHeaderName.remove(0);
   HeaderName firstHeaderToTest;
   HeaderName secondHeaderToTest;

   if( bSipXauthIdentityTakesPrecedence == true )
   {
      firstHeaderToTest  = AuthIdentityHeaderName;
      secondHeaderToTest = PAssertedIdentityHeaderName;
   }
   else
   {
      firstHeaderToTest  = PAssertedIdentityHeaderName;
      secondHeaderToTest = AuthIdentityHeaderName;
   }

   if( decode(firstHeaderToTest, message, callId, fromTag, bindRule) )
   {
      matchedHeaderName = firstHeaderToTest;
   }
   else if( decode(secondHeaderToTest, message, callId, fromTag, bindRule) )
   {
      matchedHeaderName = secondHeaderToTest;
   }
}


/// Extract identity saved in the SipXauthIdentity.
bool SipXauthIdentity::getIdentity(UtlString&  identityValue) const
{
   if (mIsValidIdentity)
   {
      identityValue = mIdentity;
   }
   else
   {
      identityValue.remove(0);
   }

   return mIsValidIdentity;
}

/// Stores the new value of identity
void SipXauthIdentity::setIdentity(const UtlString&  identityValue)
{
   // Reset the validity flag
   mIsValidIdentity = TRUE;
   mIdentity = identityValue;
}

/// Remove identity info from a message.
void SipXauthIdentity::remove(SipMessage & message, HeaderName headerName)
{
   int idHeaderCount = message.getCountHeaderFields(headerName);
   if (idHeaderCount>0)
   {
      UtlString rUri;
      message.getRequestUri(&rUri);
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipXauthIdentity::remove"
                    ": '%d' occurrances of %s in request to '%s'",
                    idHeaderCount, headerName, rUri.data());
      for (int i = idHeaderCount - 1;i>=0;i--)
      {
         message.removeHeader(headerName, i);
      }
   }
}


/// Normalize identity info in a message.
void SipXauthIdentity::normalize(SipMessage & message,  HeaderName headerName)
{
   int idHeaderCount = message.getCountHeaderFields(headerName);
   if (idHeaderCount>1)
   {
      UtlString rUri;
      message.getRequestUri(&rUri);
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipXauthIdentity::remove"
                    ": '%d' occurrances of SipXauthIdentity in request to '%s'",
                    idHeaderCount, rUri.data());
      // Remove all BUT the last header
      for (int i = idHeaderCount - 2;i>=0;i--)
      {
         //message.removeHeader(SipXauthIdentity::AuthIdentityHeaderName, i);
         message.removeHeader(headerName, i);
      }
   }
}


/// Encode identity info into a URL
bool SipXauthIdentity::encodeUri(Url              & uri,
                                 const SipMessage & request,
                                 const OsDateTime * timestamp)
{
   // Don't proceed if the encapsulated identity is invalid
   if (!mIsValidIdentity)
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "SipXauthIdentity::encodeUri[bound]: encapsulated SipXauthIdentity is invalid");
   }
   else
   {
      // make sure no existing identity in the URI
      uri.removeHeaderParameter(SipXauthIdentity::AuthIdentityHeaderName);
      // set Call-Id and from-tag for the signature calculation
      UtlString callId;
      UtlString fromTag;
      Url fromUrl;
      request.getCallIdField(&callId);
      request.getFromUrl(fromUrl);
      fromUrl.getFieldParameter("tag", fromTag);

      OsDateTime now;
      OsDateTime::getCurTime(now);
      if (NULL==timestamp)
      {
         timestamp = &now;
      }

      UtlString value;
      encode(value, callId, fromTag, *timestamp);
      uri.setHeaderParameter(SipXauthIdentity::AuthIdentityHeaderName, value.data());

      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipXauthIdentity::encodeUri[bound] encoded URI '%s'",
                    uri.toString().data()
                    );
   }

   return mIsValidIdentity;
}

/// Encode dialog-independent identity into a URL
bool SipXauthIdentity::encodeUri(Url             & uri,     ///< target URI to get encoded identity
                                 const OsDateTime* timestamp///< timestamp for generated identity
               )
{
   // Don't proceed if the encapsulated identity is invalid
   if (!mIsValidIdentity)
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "SipXauthIdentity::encodeUri[unbound]: "
                    "encapsulated SipXauthIdentity is invalid");
   }
   else
   {
      // make sure no existing identity in the URI
      uri.removeHeaderParameter(SipXauthIdentity::AuthIdentityHeaderName);

      OsDateTime now;
      OsDateTime::getCurTime(now);
      if (NULL==timestamp)
      {
         timestamp = &now;
      }

      UtlString value;
      UtlString callId;
      UtlString fromTag;
      encode(value, callId, fromTag, *timestamp, allowUnbound);
      uri.setHeaderParameter(SipXauthIdentity::AuthIdentityHeaderName, value.data());

      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipXauthIdentity::encodeUri[unbound] "
                    "encoded URI '%s'",
                    uri.toString().data()
                    );
   }

   return mIsValidIdentity;
}

/// Encode identity info into a URL
bool SipXauthIdentity::encodeUri(Url              & uri,
                                 const char*    pCallId,
                                 const Url      fromUrl,
                                 const OsDateTime * timestamp)
{
   // Don't proceed if the encapsulated identity is invalid
   if (!mIsValidIdentity)
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "SipXauthIdentity::encodeUri[no msg]: "
                    "encapsulated SipXauthIdentity is invalid");
   }
   else
   {
      // make sure no existing identity in the URI
      uri.removeHeaderParameter(SipXauthIdentity::PAssertedIdentityHeaderName);
      // set Call-Id and from-tag for the signature calculation
      UtlString callId(pCallId);
      UtlString fromTag;
      fromUrl.getFieldParameter("tag", fromTag);

      OsDateTime now;
      OsDateTime::getCurTime(now);
      if (NULL==timestamp)
      {
         timestamp = &now;
      }

      UtlString value;
      encode(value, callId, fromTag, *timestamp);
      uri.setHeaderParameter(SipXauthIdentity::PAssertedIdentityHeaderName, value.data());

      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipXauthIdentity::encodeUri[o msg] "
                    "encoded URI '%s'",
                    uri.toString().data()
                    );
   }

   return mIsValidIdentity;
}



/// Add identity info to a message.
bool SipXauthIdentity::insert(SipMessage & message,
                              HeaderName headerName,
                              const OsDateTime * timestamp)
{
   // Don't proceed if the encapsulated identity is invalid
   if (!mIsValidIdentity)
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "SipXauthIdentity::insert: "
                    "encapsulated SipXauthIdentity is invalid");
   }
   else
   {
      // make sure no existing identity in the message
      remove(message, headerName);

      // set Call-Id and from-tag for the signature calculation
      UtlString callId;
      UtlString fromTag;
      Url fromUrl;
      message.getCallIdField(&callId);
      message.getFromUrl(fromUrl);
      fromUrl.getFieldParameter("tag", fromTag);

      OsDateTime now;
      OsDateTime::getCurTime(now);
      if (NULL==timestamp)
      {
         timestamp = &now;
      }

      UtlString value;
      encode(value, callId, fromTag, *timestamp);

      // Insert displayName if it is an P-Asserted-Identity header.
      if (headerName == SipXauthIdentity::PAssertedIdentityHeaderName)
      {
          UtlString displayName;
          fromUrl.getDisplayName(displayName);
          value.prepend(displayName.data());
      }

      message.addHeaderField(headerName, value.data());
   }

   return mIsValidIdentity;
}

/// Encodes identity info
void SipXauthIdentity::encode(UtlString        & identityValue,
                              const UtlString  & callId,
                              const UtlString  & fromTag,
                              const OsDateTime & timestamp,
                              DialogRule       bindRule
                              )
{

   // calculate timestamp
   OsTime osTime;
   timestamp.cvtToTimeSinceEpoch(osTime);
   long seconds = osTime.seconds();
   char stamp[65];
   sprintf(stamp, "%lX", seconds);
   UtlString strSignature(stamp);

   strSignature.append(SignatureFieldSeparator);

   // signature-hash=MD5(<timestamp><secret><from-tag><call-id><identity>)
   NetMd5Codec signatureHash;
   signatureHash.hash(stamp);
   signatureHash.hash(sSignatureSecret);
   if (requireDialogBinding == bindRule)
   {
      signatureHash.hash(fromTag);
      signatureHash.hash(callId);
   }
   else
   {
      strSignature.append(SignatureFieldSeparator);
   }
   signatureHash.hash(mIdentity);

   UtlString strSignatureHash;
   signatureHash.appendHashValue(strSignature);

   Url encodedUrl(mIdentity);
   encodedUrl.setScheme(Url::SipUrlScheme);
   encodedUrl.setUrlParameter(SignatureUrlParamName, strSignature.data());

   encodedUrl.toString(identityValue);

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipXauthIdentity::encode "
                 "identity '%s'",
                 identityValue.data()
                 );
}


/// Check the signature and parse the identity
bool SipXauthIdentity::decode(const UtlString& identityValue,
                              const UtlString& callId,
                              const UtlString& fromTag,
                              DialogRule       bindRule
                              )
{
  /**
   * See SipXauthIdentity Encoding comment at the top of the file
   */
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipXauthIdentity::decode "
                 "parse '%s'",
                 identityValue.data()
                 );
   mIdentity.remove(0);
   mIsValidIdentity = FALSE;

   bool decodeError = false; // false iff the identity was correctly signed and successfully parsed
   UtlString decodedIdentity;
   unsigned long epochTimestamp = 0;
   UtlString timestamp;
   UtlString actualSignatureHash;
   bool isBound = false;

   Url encodedUrl(identityValue, Url::NameAddr);
   if (Url::SipUrlScheme == encodedUrl.getScheme())
   {
      // Only proceed if the URL parsing succeeded
      // Extract the identity
      encodedUrl.getIdentity(decodedIdentity);

      // Extract signature parameter
      UtlString signatureParamValue;
      if (encodedUrl.getUrlParameter(SignatureUrlParamName, signatureParamValue))
      {
         // only proceed if signature parameter was found
         RegEx signatureRegEx(SignatureRegEx);

         if (signatureRegEx.Search(signatureParamValue))
         {
            UtlString secondSeparator;

            isBound = (   signatureRegEx.MatchString(&secondSeparator,2)
                       && secondSeparator.isNull()); // there is only one ':' separator

            if (   (requireDialogBinding == bindRule) // must be bound
                && ! isBound
                )
            {
               decodeError = true;
               OsSysLog::add(FAC_SIP, PRI_WARNING,
                             "SipXauthIdentity::decode "
                             "'%s' is an unbound identity",
                             identityValue.data()
                             );
            }
            else
            {
               // extract timestamp
               if (   !signatureRegEx.MatchString(&timestamp,1)
                   || !from_string(epochTimestamp, timestamp)
                   )
               {
                  decodeError = true;
                  OsSysLog::add(FAC_SIP, PRI_WARNING,
                                "SipXauthIdentity::decode "
                                "'%s' invalid timestamp",
                                identityValue.data()
                                );
               }
               // extract signature
               else if (!signatureRegEx.MatchString(&actualSignatureHash,3))
               {
                  decodeError = true;
                  OsSysLog::add(FAC_SIP, PRI_WARNING,
                                "SipXauthIdentity::decode '%s' missing hash",
                                identityValue.data()
                                );
               }
            }
         }
         else
         {
            decodeError = true;
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipXauthIdentity::decode "
                          "'%s' invalid signature format",
                          identityValue.data()
                          );
         }
      }
      else
      {
         decodeError = true;
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "SipXauthIdentity::decode "
                       "'%s' missing '%s' param",
                       identityValue.data(), SignatureUrlParamName
                       );
      }
   }
   else
   {
      decodeError = true;
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipXauthIdentity::decode "
                    "'%s' URL parsing failed",
                     identityValue.data()
                    );
   }

   // validate timestamp
   if (!decodeError && !sSignatureValidityInterval.isNoWait())
   {
      // timestamp validity check
      if (epochTimestamp + sSignatureValidityInterval.seconds() < OsDateTime::getSecsSinceEpoch())
      {
         decodeError = true;
         OsDateTime generateDate(OsTime(epochTimestamp,0));
         UtlString generateTimeString;
         generateDate.getIsoTimeStringZ(generateTimeString);
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "SipXauthIdentity::decode(%s)"
                       " timestamp '%lX' from '%s' too old with interval of '%d' seconds",
                       identityValue.data(), epochTimestamp, generateTimeString.data(),
                       sSignatureValidityInterval.seconds()
                       );
      }
   }

   // validate signature hash
   if (!decodeError)
   {
      UtlString validSignature;

      // signature-hash=MD5(<timestamp><secret><from-tag><call-id><identity>)
      NetMd5Codec signatureHash;
      signatureHash.hash(timestamp);
      signatureHash.hash(sSignatureSecret);
      if (isBound)
      {
         signatureHash.hash(fromTag);
         signatureHash.hash(callId);
      }
      signatureHash.hash(decodedIdentity);
      signatureHash.appendHashValue(validSignature);

      if (validSignature.compareTo(actualSignatureHash) == 0)
      {
         // the signature checks out
         mIdentity = decodedIdentity;
         mIsValidIdentity = TRUE;
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "SipXauthIdentity::decode "
                       "'%s' invalid signature '%s' != '%s'",
                       identityValue.data(), actualSignatureHash.data(), validSignature.data()
                       );
      }
   }

   return mIsValidIdentity;
}

/// Check the signature and parse the identity contained in specified header name
bool SipXauthIdentity::decode(const UtlString& headerName,
                              const SipMessage& message,
                              const UtlString& callId,
                              const UtlString& fromTag,
                              DialogRule bindRule )
{
   bool foundIdentityHeader = false;
   UtlString rUri;
   message.getRequestUri(&rUri);

   int idHeaderCount = message.getCountHeaderFields(headerName);
   if (1==idHeaderCount)
   {
      foundIdentityHeader =
         decode(message.getHeaderValue(0, headerName),
                callId, fromTag, bindRule);
   }
   else if (idHeaderCount>1)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipXauthIdentity::decode:"
                    " '%d' occurrences of %s in request to '%s'",
                    idHeaderCount, headerName.data(), rUri.data());
      foundIdentityHeader =
         decode(message.getHeaderValue(idHeaderCount-1, headerName),
                callId, fromTag, bindRule);
   }

   if (foundIdentityHeader)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipXauthIdentity::decode:"
                    " found %s '%s' in request to '%s'",
                    headerName.data(), mIdentity.data(), rUri.data());
   }
   return foundIdentityHeader;
}


/// Initialize the signature validity interval
void SipXauthIdentity::setSignatureValidityInterval(const OsTime & interval)
{
   sSignatureValidityInterval = interval;
}


/// Initialize the secret value used to sign hashes.
void SipXauthIdentity::setSecret(const char* secret /**< a random value used as input to sign the
                                                     * state value.  This should be chosen such that it:
                                                     * - is hard for an attacker to guess
                                                     * - ideally, is the same in replicated authproxies
                                                     *   (this is not important yet, since we don't use
                                                     *   SRV names in the Record-Route targets).
                                                     */
                                 )
{
   /*
    * This must be called once at initialization time,
    * before any SipXauthIdentity objects are created.
    *
    * It may be called after that, but doing so with a
    * new value will invalidate any outstanding identities.
    */
   if (!sSignatureSecret.isNull() && sSignatureSecret.compareTo(secret))
   {
      OsSysLog::add(FAC_SIP,PRI_NOTICE,
                    "SipXauthIdentity::setSecret called more than once;\n"
                    " previously signed state will now fail signature checks"
                    );
   }
   sSignatureSecret.remove(0);
   sSignatureSecret.append(secret);
}

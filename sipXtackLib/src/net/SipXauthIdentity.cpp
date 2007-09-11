// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsSysLog.h"
#include "utl/UtlTokenizer.h"
#include "net/NetMd5Codec.h"

// APPLICATION INCLUDES
#include "net/SipXauthIdentity.h"

// CONSTANTS
const char* SipXauthIdentity::SignatureFieldSeparator = ":";
const char* SipXauthIdentity::SignatureUrlParamName = "signature";
const char* SipXauthIdentity::AuthIdentityHeaderName = "X-Sipx-Authidentity";

// STATIC VARIABLES
UtlString   SipXauthIdentity::sSignatureSecret;
OsTime      SipXauthIdentity::sSignatureValidityInterval;

// method to convert a hexadecimal string value to an unsigned long
bool SipXauthIdentity::from_string(unsigned long & value, const UtlString& s)
{
   char* end;
   value = strtoul(s.data(), &end, 16);
   return (end - s.data() == (int)s.length());
}

/// Default Constructor
SipXauthIdentity::SipXauthIdentity()
  : mIsValidIdentity(TRUE)
{
}

/// destructor
SipXauthIdentity::~SipXauthIdentity()
{
}

/// Decode the identity from a message.
SipXauthIdentity::SipXauthIdentity(const SipMessage& message)
  : mIsValidIdentity(TRUE)
{
   UtlString callId;
   UtlString fromTag;
   Url fromUrl;
   message.getCallIdField(&callId);
   message.getFromUrl(fromUrl);
   fromUrl.getFieldParameter("tag", fromTag);
   UtlString rUri;
   message.getRequestUri(&rUri);

   bool foundIdentityHeader = false;

   int idHeaderCount = message.getCountHeaderFields(SipXauthIdentity::AuthIdentityHeaderName);
   if (1==idHeaderCount)
   {
      foundIdentityHeader =
         decode(message.getHeaderValue(0, SipXauthIdentity::AuthIdentityHeaderName),
                callId, fromTag);
   }
   else if (idHeaderCount>1)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipXauthIdentity::SipXauthIdentity: '%d' occurrences of SipXauthIdentity in request to '%s'",
                    idHeaderCount, rUri.data());
      foundIdentityHeader = decode(message.getHeaderValue(idHeaderCount - 1,
                                                          SipXauthIdentity::AuthIdentityHeaderName),
                                   callId,
                                   fromTag);
   }

   if (foundIdentityHeader)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipXauthIdentity::SipXauthIdentity: found SipXauthIdentity '%s' in request to '%s'",
                    mIdentity.data(), rUri.data());
   }
}

/// Extract identity saved in the SipXauthIdentity.
bool SipXauthIdentity::getIdentity(UtlString&  identityValue) const
{
   if (mIsValidIdentity)
   {
      identityValue = mIdentity;
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
void SipXauthIdentity::remove(SipMessage & message)
{
   int idHeaderCount = message.getCountHeaderFields(SipXauthIdentity::AuthIdentityHeaderName);
   if (idHeaderCount>0)
   {
      UtlString rUri;
      message.getRequestUri(&rUri);
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipXauthIdentity::remove: '%d' occuances of SipXauthIdentity in request to '%s'",
                    idHeaderCount, rUri.data());
      for (int i = idHeaderCount - 1;i>=0;i--)
      {
         message.removeHeader(SipXauthIdentity::AuthIdentityHeaderName, i);
      }
   }
}


/// Normalize identity info in a message.
void SipXauthIdentity::normalize(SipMessage & message)
{
   int idHeaderCount = message.getCountHeaderFields(SipXauthIdentity::AuthIdentityHeaderName);
   if (idHeaderCount>1)
   {
      UtlString rUri;
      message.getRequestUri(&rUri);
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipXauthIdentity::remove: '%d' occuances of SipXauthIdentity in request to '%s'",
                    idHeaderCount, rUri.data());
      // Remove all BUT the last header
      for (int i = idHeaderCount - 2;i>=0;i--)
      {
         message.removeHeader(SipXauthIdentity::AuthIdentityHeaderName, i);
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
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipXauthIdentity::encodeUri: encapsulated SipXauthIdentity is invalid");
      return FALSE;
   }
   
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
                 "SipXauthIdentity::encodeUri encoded URI '%s'",
                 uri.toString().data()
                 );

   return TRUE;
}

/// Add identity info to a message.
bool SipXauthIdentity::insert(SipMessage & message, const OsDateTime * timestamp)
{
   // Don't proceed if the encapsulated identity is invalid
   if (!mIsValidIdentity)
   {
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipXauthIdentity::insert: encapsulated SipXauthIdentity is invalid");
      return FALSE;
   }
   
   // make sure no existing identity in the message
   remove(message);

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
   message.addHeaderField(SipXauthIdentity::AuthIdentityHeaderName, value.data());

   return TRUE;
}

/// Encodes identity info
void SipXauthIdentity::encode(UtlString        & identityValue,
                              const UtlString  & callId,
                              const UtlString  & fromTag,
                              const OsDateTime & timestamp)
{
  /**
   *     X-Sipx-Authidentity: < <identity>;signature=<timestamp>:<signature-hash> >
   * where:
   * - "identity" is a user identity in addr-spec format as a SIP-URI.
   * - "timestamp" is epoch seconds as hex without "0x" prefix indicating the 
   *   time the signature was generated.
   * - "signature-hash"=MD5(<timestamp><secret><from-tag><call-id><identity>)
   */

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
   signatureHash.hash(fromTag);
   signatureHash.hash(callId);
   signatureHash.hash(mIdentity);

   UtlString strSignatureHash;
   signatureHash.appendHashValue(strSignature);

   Url encodedUrl(mIdentity);
   encodedUrl.setScheme(Url::SipUrlScheme);
   encodedUrl.setUrlParameter(SignatureUrlParamName, strSignature.data());

   encodedUrl.toString(identityValue);

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipXauthIdentity::encode identity '%s'",
                 identityValue.data()
                 );
}


/// Check the signature and parse the identity
bool SipXauthIdentity::decode(const UtlString& identityValue,
                              const UtlString& callId,
                              const UtlString& fromTag)
{
  /**
   *     X-Sipx-Authidentity: < <identity>;signature=<timestamp>:<signature-hash> >
   * where:
   * - "identity" is a user identity in addr-spec format as a SIP-URI.
   * - "timestamp" is epoch seconds as hex without "0x" prefix indicating the 
   *   time the signature was generated.
   * - "signature-hash"=MD5(<timestamp><secret><from-tag><call-id><identity>)
   */

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipXauthIdentity::decode parse '%s'",
                 identityValue.data()
                 );
   mIdentity = "";
   mIsValidIdentity = FALSE;

   bool decodeError = false; // false iff the identity was correctly signed and successfully parsed
   UtlString decodedIdentity;
   unsigned long epochTimestamp = 0;
   UtlString timestamp;
   UtlString actualSignatureHash;

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
         UtlTokenizer tokenizer(signatureParamValue);
         if (tokenizer.next(timestamp, SignatureFieldSeparator))
         {
            // extract timestamp 
            if (!from_string(epochTimestamp, timestamp))
            {
               decodeError = true;
               OsSysLog::add(FAC_SIP, PRI_WARNING,
                             "SipXauthIdentity::decode failed to decode '%s' - invalid timestamp",
                             identityValue.data()
                             );
            }
            else if (!tokenizer.next(actualSignatureHash, SignatureFieldSeparator))
            {
               decodeError = true;
               OsSysLog::add(FAC_SIP, PRI_WARNING,
                             "SipXauthIdentity::decode failed to decode '%s' - missing hash",
                             identityValue.data()
                             );
            }
         }
         else
         {
            decodeError = true;
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipXauthIdentity::decode failed to decode '%s' - missing timestamp",
                          identityValue.data()
                          );
         }
      }
      else
      {
         decodeError = true;
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "SipXauthIdentity::decode failed to decode '%s' - missing '%s' param",
                       identityValue.data(), SignatureUrlParamName
                       );
      }
   }
   else
   {
      decodeError = true;
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipXauthIdentity::decode failed to decode '%s' - URL parsing failed",
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
                       "SipXauthIdentity::decode(%s) timestamp '%lX' from '%s' too old with interval of '%lu' seconds",
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
      signatureHash.hash(fromTag);
      signatureHash.hash(callId);
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
         decodeError = true;
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "SipXauthIdentity::decode of '%s' failed - invalid signature", identityValue.data()
                       );
      }
   }
   
   return !decodeError;
}


/// Initialize the signature validity interval
void SipXauthIdentity::setSignatureValidityInterval(const OsTime & interval)
{
   sSignatureValidityInterval = interval;
}


/// Initialize the secret value used to sign hashes.
void SipXauthIdentity::setSecret(const char* secret /**< a null terminated string used as input to sign the
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
                    " value changed from '%s' to '%s'\n"
                    " previously signed state will now fail signature checks",
                    sSignatureSecret.data(), secret
                    );
   }
   sSignatureSecret.remove(0);
   sSignatureSecret.append(secret);
}


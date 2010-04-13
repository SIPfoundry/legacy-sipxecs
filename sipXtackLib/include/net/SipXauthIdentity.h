//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _SIPXAUTHIDENTITY_H_
#define _SIPXAUTHIDENTITY_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/UtlBool.h"
#include "net/SipMessage.h"
#include "os/OsDateTime.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipXauthIdentityTest;

/// Maintain sipXauthIdentity information
/**
 * This class encapsulates SipXauthIdentity and P-Asserted-Identity information. SipXauthIdentity
 * is used by upstream proxies to instruct sipXauthproxy that a request needs to be routed based
 * on the permisssions of an identity different from that of the request originator.
 * P-Asserted-Identity is to communicate the identity of the subscribers as described in RFC3325.
 * Given the lack of TLS support for many SIP elements deployed today, to ensure the security and
 * prevent from forgery, replay, and falsification, P-Asserted-Identity is also encapsulated in the
 * same manner as SipXauthIdentity.
 *
 * SipXauthIdentity can be used for a variety of purposes, but at least initially it is used to
 * allow privileged users to forward their calls to destinations that require permissions.
 *
 * SipXauthIdentity information is carried in X-Sipx-Authidentity header of the follwoing format:
 *
 *     X-Sipx-Authidentity: <identity;signature=<timestamp>:<signature-hash>>
 *
 * P-Asserted-Identity information is carried in P-Asserted-Identity header of the following format:
 *
 *     P-Asserted-Identity: 'displayName' <identity;signature=<timestamp>:<signature-hash>>
 * where
 * - "displayName" is a displayName of the subscriber.
 * - "identity" is a user identity in addr-spec format as a SIP-URI.
 * - "timestamp" is epoch seconds as hex without "0x" prefix indicating the
 *   time the signature was generated.
 * - "signature-hash" is MD5(<timestamp><secret><from-tag><call-id><identity>)
 *
 * The value of X-Sipx-Authidentity and P-Asserted-Identity headers are signed using MD5. The
 * signature is calculated over the content of the header value, signature timestamp, data from
 * the SIP message and a unique secret, known only to sipXecs components in a given installation.
 * This should prevent (or minimize) the replay attacks on the system making it
 * relatively difficuilt to spoof the X-Sipx-Authidentity and P-Asserted-Identity headers.
 *
 * Signature includes a timestamp as epoch seconds indicating when the signature was calculated.
 * Signature validation fails if the signature is older then a configurable mout of time.
 * Timestamp validation can be disabled and it is disabled by default untill non-zero duration
 * interval is specified via a call to setSignatureValidityInterval.
 *
 * To access the sipXauthIdentity or P-Asserted-Identity information in a message, construct a
 * SipXauthIdentity object passing the SipMessage and header type( sipXauthIdentity or
 * P-Asserted-Identity to the SipXauthIdentity constructor, Use getIdentity/setIdentity method
 * to get and set identity. Use getIdentity/setIdentity method to get and set identity.Use remove
 * and insert methods to remove and put the identity info from/into the message.
 *
 * @nosubgrouping
 */
class SipXauthIdentity
{
  public:

   typedef const char* HeaderName;

   // Currently, valid HeaderName are AuthIdentityHeaderName or PAssertedIdentityHeaderName
   // references outside the class will need to be qualified by the class name.
   static HeaderName AuthIdentityHeaderName;
   static HeaderName PAssertedIdentityHeaderName;

   /// Default Constructor
   SipXauthIdentity();
   /**<
    * no identity is assumed at this point
    */

   /// Control whether a given identity is generated or acceptable without dialog signature.
   typedef enum
   {
      requireDialogBinding, ///< the id must be bound to the dialog (callid & from tag) of the msg.
      allowUnbound          ///< the id is allowed to not include the dialog binding.
   } DialogRule;

   // ================================================================
   /** @name                  Decoding Operations
    *
    * These methods access the identity in a message.
    */
   ///@{

   /// Constructor which decodes SipXauthIdentity or P-Asserted-Identity from a received message.
   /// The headerName parameter dictates which of SipXauthIdentity or P-Asserted-Identity will
   /// be decoded.
   SipXauthIdentity(const SipMessage& message,   ///< message to scan for an identity header
                    const HeaderName headerName, /**<headerName for the identity,
                                                  * either SipXauthIdentity or P-Asserted-Identity
                                                  */
                    DialogRule bindRule = requireDialogBinding
                    );

   /// Constructor which decodes SipXauthIdentity or P-Asserted-Identity from a received message.
   /// In this version of the constructor, the headerName is not supplied.  As such, the routine
   /// will look for SipXauthIdentity and P-Asserted-Identity headers.
   /// If either are found to contain a valid signature, the name of the matching header will
   /// be returned in the supplied matchedHeaderName.
   /// If neither are found, an empty string will be returned.
   /// The order in which the routine will look for SipXauthIdentity and P-Asserted-Identity headers
   /// is dictated by the bSipXauthIdentityTakesPrecedence flag.
   SipXauthIdentity(const SipMessage& message,     ///< message to scan for an identity header
                    UtlString& matchedHeaderName, ///< will receive name of header carrying the authenticated identity
                    bool bSipXauthIdentityTakesPrecedence, ///< true  = will look for SipXauthIdentity first;
                                                           ///< false = will look for P-Asserted-Identity first.
                    DialogRule bindRule = requireDialogBinding
                    );

   /**<
    * The message may or may not contain SipXauthIdentity information. If identity information is
    * present but signature does not match subsequent calls to getIdentity return false
    * until the new identity is set via a call to setIdentity
    */

   /// Extract identity saved in the SipXauthIdentity.
   bool getIdentity(UtlString&  identityValue) const;
   /**<
    * Returns encapsulated identity in the format user@domain
    * @returns true if the SipXauthIdentity information is valid, false if not.
    * @param identityValue is null if return was false; if true, it is in the format user@domain
    */

   ///@}

   // ================================================================
   /** @name                  Encoding Operations
    *
    * These methods manipulate the identity encoded into a request.
    */
   ///@{

   /// Stores the new value of identity
   void setIdentity(const UtlString&  identityValue);
   /**<
    * Establish a new value for the identity to be included
    * when the SipXauthIdentity information is generated.
    * @param identityValue is in the format user@domain
    */

   /// Remove identity info from a message.
   static void remove(SipMessage & request, HeaderName headerName);
   /**<
    * Remove identity information from the SIP request
    */

   /// Normalize identity info in a message.
   static void normalize(SipMessage & request, HeaderName headerName);
   /**<
    * Remove all but most recent identity information from the SIP request
    * If the request contains info on multiple identities, remove all but
    * most recent identity
    */

   /// Add identity info to a message.
   bool insert(SipMessage       & request,         ///< message to add identity info to
               HeaderName      headerName,         /**<headerName for the identity,
                                                    * either SipXauthIdentity or P-Asserted-Identity
                                                    */
               const OsDateTime * timestamp = NULL  ///< timestamp for generated identity
               );
   /**<
    * Generates new identity information based on the stored identity
    * and add it into the SipMessage. If the timestamp parameter is NULL
    * use current time as a timestamp, otherwise use provided timestamp
    *
    * @returns true if the SipXauthIdentity information is valid, false if not.
    *
    * @Note  Existing identity info in the message is removed
    */

   /// Encode identity for a specific dialog into a URL
   bool encodeUri(Url              & uri,                ///< target URI to get encoded identity
                  const SipMessage & request,            ///< original message
                  const OsDateTime * timestamp = NULL    ///< timestamp for generated identity
                  );
   /**<
    * Encodes new identity information into a URI based on the stored identity
    * and dialog information in a SipMessage.
    *
    * If the timestamp parameter is NULL, use current time as a timestamp,
    * otherwise use provided timestamp.
    *
    * This method can be used by a SIP proxy when retargeting a SIP request.
    * Instead of encoding the identity info into the request it is encoded
    * into a URI, which can then be used to retarget the original request
    *
    * @returns true if the SipXauthIdentity information is valid, false if not.
    *
    * @Note  Existing identity info in the message is removed
    */

   /// Encode dialog-independent identity into a URL
   bool encodeUri(Url              & uri,                ///< target URI to get encoded identity
                  const OsDateTime * timestamp = NULL    ///< timestamp for generated identity
                  );
   /**<
    * Encodes new identity information into a URI based only on the stored identity.
    *
    * If the timestamp parameter is NULL, use current time as a timestamp,
    * otherwise use provided timestamp.
    *
    * This method should not be used if it is possible to use the stronger form
    * that binds the identity to the dialog of a specific message.
    *
    * Instead of encoding the identity info into the request as a header, it is encoded
    * into a URI.
    *
    * @returns true if the SipXauthIdentity information is valid, false if not.
    *
    * @Note  Existing identity info in the URI is removed
    */

   bool encodeUri(Url              & uri,                ///< target URI to get encoded identity
                  const char*    pCallId,
                  const Url      fromUrl,
                  const OsDateTime * timestamp = NULL);    ///< timestamp for generated identity

   /// Initialize the signature validity interval
   static void setSignatureValidityInterval(const OsTime & interval);
   /**<
    * Signature includes a timestamp as epoch seconds indicating when the signature was calculated.
    * This method sets the signature validity interval such that signature validation fails if the
    * signature is older then the interval duration. Timestamp validation can be disabled by specifying
    * zero duration interval
    *
    * @param interval for signature validity
    */

   /// Initialize the secret value used to sign SipXauthIdentity information.
   static void setSecret(const char* secret /**< a random value used as input to sign the
                                             *   identity value.  This should be chosen such that it:
                                             * - is hard for an attacker to guess (includes at
                                             *   least 32 bits of cryptographicaly random data)
                                             * - is the same for all components in a given installation
                                             *   this is to sure components can generate identity info and
                                             *   SipAuthProxy can validate authenticity of the information.
                                             */
                         );
   /**<
    * This must be called once at initialization time,
    * before any SipXauthIdentity objects are created.
    *
    * It may be called after that, but doing so with a
    * new value will invalidate any outstanding identities.
    */
   ///@}

   /// destructor
   ~SipXauthIdentity();

  protected:

   friend class SipXauthIdentityTest; // to allow unit tests for protected members

   static bool from_string(unsigned long & value, const UtlString& s);

   /// Encodes identity info
   void encode(UtlString        & identityValue, ///< identity value to decode
               const UtlString  & callId,        ///< request Call-ID
               const UtlString  & fromTag,       ///< request From-tag
               const OsDateTime & timestamp,     ///< timestamp of the signature generation
               DialogRule bindRule = requireDialogBinding
               );
   /**<
    * Encodes the identity info into a string value including the signature
    */

   /// Check the signature, parse the identity info and extract the identity
   bool decode(const UtlString& headerValue, ///<  headerValue value to decode
               const UtlString& callId,      ///<  callId request Call-ID
               const UtlString& fromTag,     ///<  fromTag request From-tag
               DialogRule bindRule = requireDialogBinding
               );
   /**<
    * Decodes the identity in the identity header, validates authenticity
    * of the valid, sets validity flag accordingly
    *
    * @returns true iff the identity was correctly signed and successfully parsed
    */

   /// Check the signature, parse the identity info and extract the identity
   bool decode(const UtlString& headerName,  ///<  name of header containing the authenticated identity
               const SipMessage& message,    ///<  message to take identity header from.
               const UtlString& callId,      ///<  callId request Call-ID
               const UtlString& fromTag,     ///<  fromTag request From-tag
               DialogRule bindRule = requireDialogBinding
               );
   /**<
    * Searches for an authenticated identity in a header matching the supplied name then
    * if found, decodes the identity in the header, validates authenticity
    * of the valid, sets validity flag accordingly
    *
    * @returns true iff the identity was correctly signed and successfully parsed
    */

  private:
   static const char* SignatureFieldSeparator;
   static const char* SignatureUrlParamName;

   UtlString  mIdentity;
   bool       mIsValidIdentity;

   static OsTime    sSignatureValidityInterval;
   static UtlString sSignatureSecret;

   // @cond INCLUDENOCOPY
   // There is no copy constructor.
   SipXauthIdentity(const SipXauthIdentity& nocopyconstructor);

   // There is no assignment operator.
   SipXauthIdentity& operator=(const SipXauthIdentity& noassignmentoperator);
   // @endcond
};

#endif // _SIPXAUTHIDENTITY_H_

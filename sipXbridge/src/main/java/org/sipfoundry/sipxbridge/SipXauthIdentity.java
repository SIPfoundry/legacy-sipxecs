/*
 *  Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.math.BigInteger;
import java.security.MessageDigest;
import javax.sip.header.Header;
import javax.sip.message.Request;

import org.apache.commons.codec.binary.Base64;
import org.apache.log4j.Logger;

/** Maintain sipXauthIdentity information
 * This class encapsulates SipXauthIdentity and P-Asserted-Identity information. SipXauthIdentity
 * is used by upstream proxies to instruct sipXauthproxy that a request needs to be routed based
 * on the permissions of an identity different from that of the request originator.
 * P-Asserted-Identity is to communicate the identity of the subscribers as described in RFC3325.
 * Given the lack of TLS support for many SIP elements deployed today, to ensure the security and
 * prevent from forgery, replay, and falsification, P-Asserted-Identity is also encapsulated in the
 * same manner as SipXauthIdentity.
 *
 * SipXauthIdentity can be used for a variety of purposes, but at least initially it is used to
 * allow privileged users to forward their calls to destinations that require permissions.
 *
 * SipXauthIdentity information is carried in X-Sipx-Authidentity header of the following format:
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
 * relatively difficult to spoof the X-Sipx-Authidentity and P-Asserted-Identity headers.
 *
 * Signature includes a timestamp as epoch seconds indicating when the signature was calculated.
 *
 * Adapted from sipXtack/net/SipXauthIdentity.cpp
 *
 */

public class SipXauthIdentity {

    public static String AuthIdentityHeaderName = "X-sipX-Authidentity";
    public static String PAssertedIdentityHeaderName = "P-Asserted-Identity";
    private static String SignatureFieldSeparator = ":";

    private static String sSignatureSecret;
    private static byte[] sSignatureSecretDecoded;

    private static Logger logger = Logger.getLogger(SipXauthIdentity.class);

    public SipXauthIdentity()
    {
    }

    /**
     * Add signed identity header to a message.
     * Generates new identity information and adds a header into the message.
     * Uses current time as a timestamp.
     *
     * @param identity identity to be encoded in header
     * @param message message to add identity info to
     * @param headerName either SipXauthIdentity or P-Asserted-Identity
     * @returns true if the identity is valid, false if not.
     *
     * @Note  Existing identity info in the message is removed
     */
    static public Boolean insertIdentity(String identity, Request message, String headerName) {
        // Don't proceed if the provided identity is invalid
        if (identity == null || identity.isEmpty()) {
            logger.error("SipXauthIdentity.insertIdentity:  identity is invalid");
            return false;
        }

        if ( logger.isDebugEnabled() ) logger.debug("SipXauthIdentity.insert '" + identity + "' into " + headerName);

        // make sure there is no existing identity in the message
        message.removeHeader(headerName);

        // set Call-Id and from-tag for the signature calculation
        String callId = SipUtilities.getCallId(message);
        String fromTag = SipUtilities.getFromTag(message);
        long timestamp = System.currentTimeMillis();
        String value = encodeIdentity(identity, callId, fromTag, timestamp);

        // Insert displayName if it is an P-Asserted-Identity header.
        // TODO (no immediate need for signed PAI headers)
        /* if (headerName == SipXauthIdentity.PAssertedIdentityHeaderName) {
         *     String displayName;
         *     fromUrl.getDisplayName(displayName);
         *     value.prepend(displayName);
         * }
         */

        try {
            Header header = ProtocolObjects.headerFactory.createHeader(headerName, value);
            message.addHeader(header);
        } catch (Exception ex) {
            throw new SipXbridgeException("Unexpected exception ", ex);
        }

        return true;
    }

  /// Encodes identity info
    static private String encodeIdentity(String identity, String callId, String fromTag, long timestamp) {
        // calculate timestamp
        Long seconds = timestamp / 1000;
        String stamp = Long.toHexString(seconds).toUpperCase();
        String strSignature = stamp + SignatureFieldSeparator;

        // signature-hash=MD5(<timestamp><secret><from-tag><call-id><identity>)
        try {
            MessageDigest digest = MessageDigest.getInstance("MD5");
            digest.update(stamp.getBytes());
            digest.update(sSignatureSecretDecoded);
            digest.update(fromTag.getBytes());
            digest.update(callId.getBytes());
            digest.update(identity.getBytes());
            // create md5 hash of token
            byte md5hash[] = digest.digest();
            BigInteger number = new BigInteger(1, md5hash);
            strSignature += number.toString(16);
        } catch (Exception ex) {
            logger.error("Failed to generate signature", ex);
        }

        String encodedUrl = "<sip:" + identity + ";signature=" + strSignature + ">";
        return encodedUrl;
    }

    /** Initialize the secret value used to sign SipXauthIdentity information.
     *
     * This must be called once at initialization time,
     * before any SipXauthIdentity objects are created.
     *
     * It may be called after that, but doing so with a
     * new value will invalidate any outstanding identities.
     *
     * @param secret a random value used as input to sign the identity value
     */
    public static void setSecret(String secret) {
        if (sSignatureSecret != null && sSignatureSecret != secret)
        {
           logger.info("SipXauthIdentity::setSecret called more than once; " +
                         " previously signed state will now fail signature checks"
                         );
        }
        sSignatureSecret = new String(secret);
        sSignatureSecretDecoded = Base64.decodeBase64(secret.getBytes());
    }
}

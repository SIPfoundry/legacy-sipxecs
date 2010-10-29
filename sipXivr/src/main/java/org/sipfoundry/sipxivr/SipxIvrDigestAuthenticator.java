package org.sipfoundry.sipxivr;

import java.io.IOException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.Principal;

import org.apache.log4j.Logger;
import org.mortbay.http.DigestAuthenticator;
import org.mortbay.http.HttpFields;
import org.mortbay.http.HttpRequest;
import org.mortbay.http.HttpResponse;
import org.mortbay.http.SecurityConstraint;
import org.mortbay.http.UserRealm;
import org.mortbay.util.Credential;
import org.mortbay.util.QuotedStringTokenizer;

public class SipxIvrDigestAuthenticator extends DigestAuthenticator {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    @Override
    public Principal authenticate(UserRealm realm, String pathInContext, HttpRequest request, HttpResponse response)
            throws IOException {
        // Get the user if we can
        boolean stale = false;
        Principal user = null;
        String credentials = request.getField(HttpFields.__Authorization);

        if (credentials != null) {
            QuotedStringTokenizer tokenizer = new QuotedStringTokenizer(credentials, "=, ", true, false);
            Digest digest = new Digest(request.getMethod());
            String last = null;
            String name = null;

            loop: while (tokenizer.hasMoreTokens()) {
                String tok = tokenizer.nextToken();
                char c = (tok.length() == 1) ? tok.charAt(0) : '\0';

                switch (c) {
                case '=':
                    name = last;
                    last = tok;
                    break;
                case ',':
                    name = null;
                case ' ':
                    break;

                default:
                    last = tok;
                    if (name != null) {
                        if ("username".equalsIgnoreCase(name))
                            digest.username = tok;
                        else if ("realm".equalsIgnoreCase(name))
                            digest.realm = tok;
                        else if ("nonce".equalsIgnoreCase(name))
                            digest.nonce = tok;
                        else if ("nc".equalsIgnoreCase(name))
                            digest.nc = tok;
                        else if ("cnonce".equalsIgnoreCase(name))
                            digest.cnonce = tok;
                        else if ("qop".equalsIgnoreCase(name))
                            digest.qop = tok;
                        else if ("uri".equalsIgnoreCase(name))
                            digest.uri = tok;
                        else if ("response".equalsIgnoreCase(name))
                            digest.response = tok;
                        break;
                    }
                }
            }

            int n = checkNonce(digest.nonce, request);
            if (n > 0)
                user = realm.authenticate(digest.username, digest, request);
            else if (n == 0)
                stale = true;

            if (user == null)
                LOG.warn("AUTH FAILURE: user " + digest.username);
            else {
                request.setAuthType(SecurityConstraint.__DIGEST_AUTH);
                request.setAuthUser(digest.username);
                request.setUserPrincipal(user);
            }
        }

        // Challenge if we have no user
        if (user == null && response != null)
            sendChallenge(realm, request, response, stale);

        return user;
    }

    private static class Digest extends Credential {
        String method = null;
        String username = null;
        String realm = null;
        String nonce = null;
        String nc = null;
        String cnonce = null;
        String qop = null;
        String uri = null;
        String response = null;

        /* ------------------------------------------------------------ */
        Digest(String m) {
            method = m;
        }

        /* ------------------------------------------------------------ */
        public boolean check(Object credentials) {
            String pintoken = (credentials instanceof String) ? (String) credentials : credentials.toString();

            String A2 = null;
            if (qop == null || qop.trim().length() == 0 || qop.trim().equalsIgnoreCase("auth")) {
                A2 = method + ":" + uri;
            } else {
                A2 = method + ":" + uri + ":" + realm;
            }

            String expectedValue = null;

            if (cnonce != null && qop != null && nc != null
                    && (qop.equalsIgnoreCase("auth") || qop.equalsIgnoreCase("auth-int")))

            {
                expectedValue = KD(pintoken, nonce + ":" + nc + ":" + cnonce + ":" + qop + ":" + H(A2));

            } else {
                expectedValue = KD(pintoken, nonce + ":" + H(A2));
            }
            if (expectedValue.equals(response)) {
                return true;
            }

            return false;
        }

        public String toString() {
            return username + "," + response;
        }

    }

    public static String H(String data) {
        try {
            MessageDigest digest = MessageDigest.getInstance("MD5");

            return toHexString(digest.digest(data.getBytes()));
        } catch (NoSuchAlgorithmException ex) {
            // shouldn't happen
            throw new RuntimeException("Failed to instantiate an MD5 algorithm", ex);
        }
    }

    public static String toHexString(byte b[]) {
        int pos = 0;
        char[] c = new char[b.length * 2];
        for (int i = 0; i < b.length; i++) {
            c[pos++] = toHex[(b[i] >> 4) & 0x0F];
            c[pos++] = toHex[b[i] & 0x0f];
        }
        return new String(c);
    }

    private static final char[] toHex = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    private static String KD(String secret, String data) {
        return H(secret + ":" + data);
    }
}

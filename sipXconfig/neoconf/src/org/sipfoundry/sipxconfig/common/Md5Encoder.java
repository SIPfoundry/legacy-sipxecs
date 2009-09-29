/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $$
 */

package org.sipfoundry.sipxconfig.common;

import org.apache.commons.codec.digest.DigestUtils;

/**
 * Helper method for creating passtokens
 */
public final class Md5Encoder {
    /** MD5 message digest length */
    public static final int LEN = 32;

    private Md5Encoder() {
        // do not instantiate
    }

    /**
     * Computes the digest without DNS domain name
     */
    public static final String digestPassword(String user, String realm, String password) {
        String full = user + ':' + realm + ':' + password;
        String digest = DigestUtils.md5Hex(full);
        return digest;
    }

    /**
     * Computes the digest with DNS domain name - "old way" left for compatibility In future we
     * may allow user to choose this method or "no DNS" method
     *
     * @deprecated use version that does not require credentials
     */
    @Deprecated
    public static final String digestPassword(String user, String domain, String realm,
            String password) {
        String full = user + '@' + domain + ':' + realm + ':' + password;
        return DigestUtils.md5Hex(full);
    }
}

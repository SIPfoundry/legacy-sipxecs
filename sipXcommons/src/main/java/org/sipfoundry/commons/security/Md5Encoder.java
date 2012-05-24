/**
 *
 *
 * Copyright (c) 2010 / 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.commons.security;

import org.apache.commons.codec.digest.DigestUtils;

public final class Md5Encoder {
    /** MD5 message digest length */
    public static final int LEN = 32;

    private Md5Encoder() {
        // do not instantiate
    }

    /**
     * Computes the digest without realm domain name
     */
    public static final String digestEncryptPassword(String user, String password) {
        String full = user + ':' + password;
        String digest = DigestUtils.md5Hex(full);
        return digest;
    }

    /**
     * Computes the digest with realm in accordance with RFC. Note realm == domain in practice
     * so storing this means your password validation will fail if domain name changes. 
     */
    public static final String digestEncryptPassword(String user, String realm, String password) {
        String full = user + ':' + realm + ':' + password;
        String digest = DigestUtils.md5Hex(full);
        return digest;
    }

    /**
     * We are not going to use a one-way password encryption - if at some point we decide to do encryption,
     * we change this method, instead of changing code all over the places
     * @param password
     * @return
     */
    public static final String getEncodedPassword(String password) {
        return password;
    }

    /**
     * Computes the digest with DNS domain name - "old way" left for compatibility In future we
     * may allow user to choose this method or "no DNS" method
     *
     * @deprecated use version that does not require credentials
     */
    @Deprecated
    public static final String digestEncryptPassword(String user, String domain, String realm,
            String password) {
        String full = user + '@' + domain + ':' + realm + ':' + password;
        return DigestUtils.md5Hex(full);
    }
}

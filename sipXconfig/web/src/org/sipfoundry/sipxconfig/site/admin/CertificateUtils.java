/**
 *
 *
 * Copyright (c) 2010 / 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.site.admin;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.StringReader;
import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.interfaces.RSAPrivateKey;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.security.spec.PKCS8EncodedKeySpec;

import org.apache.commons.lang.StringUtils;
import org.bouncycastle.util.encoders.Base64;

/**
 * Reusable certificate related functions
 */
public final class CertificateUtils {

    private CertificateUtils() {
        // disable instantiation
    }

    /**
     * Returns the modulus bit size of the function used to encrypt this key
     *
     * @param key The key in PEM format
     * @return The size of the modulus
     * @throws IOException
     * @throws NoSuchAlgorithmException
     * @throws InvalidKeySpecException
     */
    public static int getEncryptionStrength(String key) throws IOException, NoSuchAlgorithmException,
            InvalidKeySpecException {
        if (StringUtils.isBlank(key)) {
            return -1;
        }

        StringBuilder strippedCert = new StringBuilder();
        BufferedReader br = new BufferedReader(new StringReader(key));
        String empty = "";
        String line;
        while ((line = br.readLine()) != null) {
            line = line.trim();
            // strip start and end lines
            if (!line.startsWith("--")) {
                // remove EOL
                strippedCert.append(line.replaceAll("\n", empty).replaceAll("\r", empty));
            }
        }

        byte[] pkBytes = Base64.decode(strippedCert.toString());

        KeyFactory keyFactory = KeyFactory.getInstance("RSA");
        KeySpec ks = new PKCS8EncodedKeySpec(pkBytes);
        RSAPrivateKey pk = (RSAPrivateKey) keyFactory.generatePrivate(ks);

        return pk.getModulus().bitLength();
    }
}

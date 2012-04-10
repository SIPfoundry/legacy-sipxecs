/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import javax.sip.header.CallIdHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.ToHeader;
import javax.sip.message.Message;
import javax.sip.message.Request;
import javax.sip.message.Response;

public class Util {
    /**
     * to hex converter
     */
    private static final char[] toHex = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    /**
     * Converts b[] to hex string.
     * 
     * @param b the bte array to convert
     * @return a Hex representation of b.
     */
    public static String toHexString(byte b[]) {
        int pos = 0;
        char[] c = new char[b.length * 2];
        for (int i = 0; i < b.length; i++) {
            c[pos++] = toHex[(b[i] >> 4) & 0x0F];
            c[pos++] = toHex[b[i] & 0x0f];
        }
        return new String(c);
    }

    /**
     * Defined in rfc 2617 as H(data) = MD5(data);
     * 
     * @param data data
     * @return MD5(data)
     */
    public static String H(String data) {
        try {
            MessageDigest digest = MessageDigest.getInstance("MD5");

            return toHexString(digest.digest(data.getBytes()));
        } catch (NoSuchAlgorithmException ex) {
            // shouldn't happen
            throw new RuntimeException("Failed to instantiate an MD5 algorithm", ex);
        }
    }

 

    

}

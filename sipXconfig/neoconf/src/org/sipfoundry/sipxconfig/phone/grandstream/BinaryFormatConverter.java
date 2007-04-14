/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.apache.commons.codec.DecoderException;
import org.apache.commons.codec.binary.Hex;

public final class BinaryFormatConverter {
    private static final byte[] HEADER = new byte[] {
        0x00, 0x00, 0x00, 0x44, 0x58, 0x1B
    };
    private static final byte[] CRLF = new byte[] {
        0x0d, 0x0a
    };

    private BinaryFormatConverter() {
    }

    @SuppressWarnings("unused")
    public static void convert(String macAddress, InputStream in_, OutputStream out)
        throws IOException {
        BufferedOutputStream bout = new BufferedOutputStream(out);
        bout.write(HEADER);
        try {
            bout.write(Hex.decodeHex(macAddress.toCharArray()));
        } catch (DecoderException e) {
            throw new RuntimeException(e);
        }
        bout.write(CRLF);
        bout.write(CRLF);
        bout.flush();
        bout.close();
    }
}

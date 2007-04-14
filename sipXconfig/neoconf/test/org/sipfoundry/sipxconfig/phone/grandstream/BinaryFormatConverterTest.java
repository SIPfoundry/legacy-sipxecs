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

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

import junit.framework.TestCase;

import org.apache.commons.codec.binary.Hex;
import org.apache.commons.io.IOUtils;

public class BinaryFormatConverterTest extends TestCase {

    public void testConvert() throws IOException {
        byte[] expectedBytes = IOUtils.toByteArray(getClass().getResourceAsStream("config.bin"));
        ByteArrayOutputStream actualStream = new ByteArrayOutputStream();
        InputStream in = getClass().getResourceAsStream("config.txt");
        BinaryFormatConverter.convert("00000000", in, actualStream);
        byte[] actualBytes = actualStream.toByteArray();
        String actual = new String(Hex.encodeHex(actualBytes));
        String expected = new String(Hex.encodeHex(expectedBytes));
        assertEquals(expected, actual);
    }
}

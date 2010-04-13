/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.device;

import java.io.OutputStream;
import java.io.Reader;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;

public class MemoryProfileLocationTest extends TestCase {
    public void testGetOutput() throws Exception {
        MemoryProfileLocation location = new MemoryProfileLocation();

        OutputStream output = location.getOutput("abc.txt");
        for (char c = 'a'; c < 'e'; c++) {
            output.write(c);
        }
        location.closeOutput(output);
        assertEquals("abcd", location.toString());

        Reader reader = location.getReader();
        assertEquals("abcd", IOUtils.toString(reader));
    }

    public void testGetOutputMany() throws Exception {
        MemoryProfileLocation location = new MemoryProfileLocation();

        OutputStream output1 = location.getOutput("abc.txt");
        for (char c = 'a'; c < 'e'; c++) {
            output1.write(c);
        }
        OutputStream output2 = location.getOutput("123.txt");
        for (char c = '1'; c < '5'; c++) {
            output2.write(c);
        }

        location.closeOutput(output1);
        location.closeOutput(output2);

        assertEquals("abcd", location.toString("abc.txt"));
        Reader reader = location.getReader("abc.txt");
        assertEquals("abcd", IOUtils.toString(reader));

        assertEquals("1234", location.toString("123.txt"));
        reader = location.getReader("123.txt");
        assertEquals("1234", IOUtils.toString(reader));
    }
}

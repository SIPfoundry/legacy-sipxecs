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

import junit.framework.TestCase;

public class GrandstreamProfileWriterTest extends TestCase {

    public void testWriteIpAddress() {
        ByteArrayOutputStream actual = new ByteArrayOutputStream();
        GrandstreamProfileWriter wtr = new GrandstreamProfileWriter(null);
        wtr.setOutputStream(actual);
        wtr.writeIpAddress("bird", null);
        assertEquals("bird = " + GrandstreamProfileWriter.LF, actual.toString());
    }

}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.device;

import java.io.ByteArrayOutputStream;

import junit.framework.TestCase;

public class OutputStreamProfileLocationTest extends TestCase {

    public void testGetOutput() {
        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        OutputStreamProfileLocation location = new OutputStreamProfileLocation(stream);
        assertSame(stream, location.getOutput(""));
    }
}

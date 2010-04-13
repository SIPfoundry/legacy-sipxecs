/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Locale;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.forwarding.Ring;

/**
 * ForkQueueValueTest
 */
public class ForkQueueValueTest extends TestCase {

    public void testSerial() throws Exception {
        ForkQueueValue value = new ForkQueueValue(3);
        assertEquals("q=0.95",value.getSerial());
        assertEquals("q=0.9",value.getSerial());
        assertEquals("q=0.85",value.getSerial());
    }

    public void testMixed() throws Exception {
        ForkQueueValue value = new ForkQueueValue(3);
        assertEquals("q=1.0",value.getParallel());
        assertEquals("q=1.0",value.getParallel());
        assertEquals("q=0.95",value.getSerial());
        assertEquals("q=0.95",value.getParallel());
        assertEquals("q=0.9",value.getSerial());
        assertEquals("q=0.85",value.getSerial());
    }

    public void testLastBeforeNext() throws Exception {
        ForkQueueValue value = new ForkQueueValue(3);
        assertEquals("q=1.0",value.getParallel());
        assertEquals("q=1.0",value.getParallel());
        assertEquals("q=0.95",value.getSerial());
        assertEquals("q=0.95",value.getParallel());
        assertEquals("q=0.9",value.getSerial());
        assertEquals("q=0.9",value.getParallel());
    }

    public void testGetValue() {
        ForkQueueValue value = new ForkQueueValue(3);
        assertEquals("q=1.0",value.getValue(Ring.Type.IMMEDIATE));
        assertEquals("q=1.0",value.getValue(Ring.Type.IMMEDIATE));
        assertEquals("q=0.95",value.getValue(Ring.Type.DELAYED));
        assertEquals("q=0.95",value.getValue(Ring.Type.IMMEDIATE));
        assertEquals("q=0.9",value.getValue(Ring.Type.DELAYED));
        assertEquals("q=0.85",value.getValue(Ring.Type.DELAYED));
    }

    public void testInternational() {
        Locale localeDef = Locale.getDefault();
        Locale.setDefault(new Locale("pl"));
        ForkQueueValue value = new ForkQueueValue(3);
        assertEquals("q=1.0",value.getValue(Ring.Type.IMMEDIATE));
        assertEquals("q=1.0",value.getValue(Ring.Type.IMMEDIATE));
        assertEquals("q=0.95",value.getValue(Ring.Type.DELAYED));
        Locale.setDefault(localeDef);
    }
}

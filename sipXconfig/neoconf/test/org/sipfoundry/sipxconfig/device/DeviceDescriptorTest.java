/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.device;

import junit.framework.TestCase;

public class DeviceDescriptorTest extends TestCase {
    public void testCleanSerialNumber() {
        DeviceDescriptor dd = new DeviceDescriptor() {
        };

        assertEquals("123456789012", dd.cleanSerialNumber("123456789012"));
        assertEquals("123456789012", dd.cleanSerialNumber("1234 5678 9012"));
        assertEquals("123456789012", dd.cleanSerialNumber("12:34:56:78:90:12"));
        assertEquals("aabbccddeeff", dd.cleanSerialNumber("AABBCCDDEEFF"));
        assertEquals("totallybogus", dd.cleanSerialNumber("totallybogus"));
        assertNull(dd.cleanSerialNumber(null));
    }

    public void testGetModelDir() {
        DeviceDescriptor dd = new DeviceDescriptor() {
        };

        assertNull(dd.getModelDir());

        dd.setBeanId("bean");
        assertEquals("bean", dd.getModelDir());
        assertEquals("bean", dd.getBeanId());

        dd.setModelDir("dir");
        assertEquals("dir", dd.getModelDir());
        assertEquals("bean", dd.getBeanId());
    }
}

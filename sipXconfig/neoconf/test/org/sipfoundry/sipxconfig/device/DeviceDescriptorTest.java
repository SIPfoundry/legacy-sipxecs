/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.device;

import java.util.Collections;
import java.util.Set;

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

    public void testGetDefinitions() {
        DeviceDescriptor dd = new DeviceDescriptor() {
        };

        dd.setModelId("volvo");
        dd.setSupportedFeatures(Collections.singleton("4x4"));

        Set<String> definitions = dd.getDefinitions(null);
        assertNotNull(definitions);
        assertTrue(definitions.contains("volvo"));
        assertTrue(definitions.contains("4x4"));
        assertFalse(definitions.contains("s80"));
        assertFalse(definitions.contains("leather"));
    }

    public void testGetDefinitionsWithVersion() {
        DeviceDescriptor dd = new DeviceDescriptor() {
        };

        dd.setModelId("volvo");
        dd.setSupportedFeatures(Collections.singleton("4x4"));

        DeviceVersion version = new DeviceVersion("volvo", "s80");
        version.setSupportedFeatures(Collections.singleton("leather"));

        Set<String> definitions = dd.getDefinitions(version);
        assertNotNull(definitions);
        assertTrue(definitions.contains("volvo"));
        assertTrue(definitions.contains("4x4"));
        assertTrue(definitions.contains("s80"));
        assertTrue(definitions.contains("leather"));
    }

    public void testValidateSerialNumber() {
        DeviceDescriptor dd = new DeviceDescriptor() {
        };

        assertFalse(dd.isSerialNumberValid("55"));
        assertTrue(dd.isSerialNumberValid("554433221100"));

        dd.setSerialNumberPattern("\\d{1,3}");
        assertTrue(dd.isSerialNumberValid("55"));
        assertFalse(dd.isSerialNumberValid("554433221100"));

        dd.setSerialNumberPattern("");
        assertTrue(dd.isSerialNumberValid("554433221100"));
        assertTrue(dd.isSerialNumberValid("55"));
    }

    public void testGetSerialNumber() {
        DeviceDescriptor dd = new DeviceDescriptor() {
        };

        assertTrue(dd.getHasSerialNumber());

        dd.setSerialNumberPattern(null);
        assertEquals("", dd.getSerialNumberPattern());

        assertFalse(dd.getHasSerialNumber());
    }
}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting.type;

import junit.framework.TestCase;

public class StringSettingTest extends TestCase {

    public void testConvertToTypedValue() {
        SettingType type = new StringSetting();
        assertEquals("", type.convertToTypedValue(""));
        assertEquals("bongo", type.convertToTypedValue("bongo"));
        assertNull(type.convertToTypedValue(null));
    }

    public void testConvertToStringValue() {
        SettingType type = new StringSetting();
        assertEquals("bongo", type.convertToStringValue("bongo"));
        assertNull("Only null is null", type.convertToStringValue(null));
        assertEquals("", type.convertToStringValue(""));
        assertEquals("\t ", type.convertToStringValue("\t "));
        assertNull(type.convertToStringValue(null));
    }

    public void testGetDefaultValueForPassword() {
        StringSetting type = new StringSetting();
        type.setPassword(true);
        assertNull(type.getLabel("foo"));
    }
}

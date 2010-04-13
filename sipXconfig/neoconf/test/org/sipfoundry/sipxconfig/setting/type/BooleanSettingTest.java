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

public class BooleanSettingTest extends TestCase {

    public void testConvertToTypedValue() {
        SettingType type = new BooleanSetting();
        assertTrue(((Boolean) type.convertToTypedValue("1")).booleanValue());
        assertFalse(((Boolean) type.convertToTypedValue("0")).booleanValue());
        assertNull(type.convertToTypedValue(null));
        assertFalse(((Boolean) type.convertToTypedValue("xyz")).booleanValue());
    }

    public void testConvertToStringValue() {
        BooleanSetting type = new BooleanSetting();
        type.setTrueValue("enabled");
        type.setFalseValue("disabled");
        assertEquals("enabled", type.convertToStringValue(Boolean.TRUE));
        assertEquals("disabled", type.convertToStringValue(Boolean.FALSE));
        assertNull(type.convertToStringValue(null));
    }

    public void testGetLabel() {
        BooleanSetting type = new BooleanSetting();
        assertEquals("true.value", type.getResourceLabel("1"));
        assertEquals("false.value", type.getResourceLabel("0"));
        assertNull(type.getResourceLabel(null));
    }
}

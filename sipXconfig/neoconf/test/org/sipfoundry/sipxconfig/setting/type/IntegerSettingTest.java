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

public class IntegerSettingTest extends TestCase {

    public void testConvertToTypedValue() {
        SettingType type = new IntegerSetting();
        assertEquals(new Integer(51), type.convertToTypedValue("51"));
        assertNull(type.convertToTypedValue("kuku"));
    }

    public void testConvertToStringValue() {
        SettingType type = new IntegerSetting();
        assertEquals("52", type.convertToStringValue(new Integer(52)));
        assertNull(type.convertToStringValue(null));
    }
}

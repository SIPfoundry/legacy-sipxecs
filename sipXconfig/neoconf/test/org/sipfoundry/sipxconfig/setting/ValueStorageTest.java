/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import junit.framework.TestCase;

public class ValueStorageTest extends TestCase {

    public void testSettingValue() {
        ValueStorage vs = new ValueStorage();
        String svalue = "red";
        Setting setting = new SettingImpl("parrot");
        vs.setSettingValue(setting, new SettingValueImpl(svalue), null);
        assertEquals(svalue, vs.getSettingValue(setting).getValue());
    }

    public void testNullValue() {
        ValueStorage vs = new ValueStorage();
        Setting setting = new SettingImpl("parrot");
        vs.setSettingValue(setting, new SettingValueImpl(null), null);
        assertNull(vs.getSettingValue(setting).getValue());
    }
}

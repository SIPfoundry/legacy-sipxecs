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

import org.sipfoundry.sipxconfig.setting.SettingImpl;

import junit.framework.TestCase;

public class EnumSettingTest extends TestCase {

    private EnumSetting m_intEnum;
    private EnumSetting m_stringEnum;

    protected void setUp() throws Exception {
        m_intEnum = new EnumSetting();
        m_intEnum.addEnum("1", "jeden");
        m_intEnum.addEnum("2", "dwa");
        m_intEnum.addEnum("3", "trzy");

        m_stringEnum = new EnumSetting();
        m_stringEnum.addEnum("one", "jeden");
        m_stringEnum.addEnum("two", "dwa");
        m_stringEnum.addEnum("three", "trzy");
    }

    public void testConvertToTypedValue() {
        assertEquals(new Integer(1), m_intEnum.convertToTypedValue("1"));
        assertNull(m_intEnum.convertToTypedValue("bongo"));
        assertNull(m_intEnum.convertToTypedValue(null));

        assertEquals("one", m_stringEnum.convertToTypedValue("one"));
        assertNull(m_stringEnum.convertToTypedValue("bongo"));
        assertNull(m_stringEnum.convertToTypedValue(null));
    }

    public void testConvertToStringValue() {
        assertEquals("2", m_intEnum.convertToStringValue(new Integer(2)));
        assertNull(m_intEnum.convertToStringValue(new Integer(5)));
        assertNull(m_intEnum.convertToStringValue(null));

        assertEquals("two", m_stringEnum.convertToStringValue("two"));
        assertNull(m_stringEnum.convertToStringValue("five"));
        assertNull(m_stringEnum.convertToStringValue(null));
    }

    public void testGet() {
        SettingImpl setting = new SettingImpl();
        setting.setName("abc");
        assertEquals("abc.label.xyz", m_stringEnum.getLabelKey(setting, "xyz"));

        m_stringEnum.setId("enum3");
        assertEquals("type.enum3.xyz", m_stringEnum.getLabelKey(setting, "xyz"));

        m_stringEnum.setLabelKeyPrefix("enum3.labels");
        assertEquals("enum3.labels.xyz", m_stringEnum.getLabelKey(setting, "xyz"));
    }
}

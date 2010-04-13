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

import static java.util.Arrays.asList;
import java.util.List;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.setting.SettingImpl;

public class MultiEnumSettingTest extends TestCase {

    private MultiEnumSetting m_intEnum;
    private MultiEnumSetting m_stringEnum;

    @Override
    protected void setUp() throws Exception {
        m_intEnum = new MultiEnumSetting();
        m_intEnum.addEnum("1", "jeden");
        m_intEnum.addEnum("2", "dwa");
        m_intEnum.addEnum("3", "trzy");

        m_stringEnum = new MultiEnumSetting();
        m_stringEnum.addEnum("one", "jeden");
        m_stringEnum.addEnum("two", "dwa");
        m_stringEnum.addEnum("three", "trzy");
    }

    public void testConvertToTypedValueInt() {
        List tv = (List) m_intEnum.convertToTypedValue("1|bongo|3");
        assertEquals(2, tv.size());
        assertEquals(1, tv.get(0));
        assertEquals(3, tv.get(1));
        tv = (List) m_intEnum.convertToTypedValue("3");
        assertEquals(1, tv.size());
        assertEquals(3, tv.get(0));
        tv = (List) m_intEnum.convertToTypedValue("bongo");
        assertTrue(tv.isEmpty());
        assertNull(m_intEnum.convertToTypedValue(null));
    }

    public void testConvertToTypedValueStr() {
        List tv = (List) m_stringEnum.convertToTypedValue("one|bongo|three");
        assertEquals(2, tv.size());
        assertEquals("one", tv.get(0));
        assertEquals("three", tv.get(1));
        tv = (List) m_stringEnum.convertToTypedValue("three");
        assertEquals(1, tv.size());
        assertEquals("three", tv.get(0));
        tv = (List) m_stringEnum.convertToTypedValue("bongo");
        assertTrue(tv.isEmpty());
        assertNull(m_stringEnum.convertToTypedValue(null));
    }

    public void testConvertToStringValueInt() {
        assertEquals("2", m_intEnum.convertToStringValue(2));
        assertEquals("2", m_intEnum.convertToStringValue(asList(2)));
        assertEquals("2|1", m_intEnum.convertToStringValue(asList(2, 1)));
        assertEquals("2|1", m_intEnum.convertToStringValue(asList(5, 2, 1)));
        assertNull(m_intEnum.convertToStringValue(asList(5)));
        assertNull(m_intEnum.convertToStringValue(null));
    }

    public void testConvertToStringValueStr() {
        assertEquals("two", m_stringEnum.convertToStringValue("two"));
        assertEquals("two", m_stringEnum.convertToStringValue(asList("two")));
        assertEquals("two|one", m_stringEnum.convertToStringValue(asList("two", "one")));
        assertEquals("two|one", m_stringEnum.convertToStringValue(asList("five", "two", "one")));
        assertNull(m_stringEnum.convertToStringValue(asList("five")));
        assertNull(m_stringEnum.convertToStringValue(null));
    }

    public void testSeparator() {
        m_intEnum.setSeparator(" ");
        List tv = (List) m_intEnum.convertToTypedValue("1 bongo 3");
        assertEquals(2, tv.size());
        assertEquals(1, tv.get(0));
        assertEquals(3, tv.get(1));
        assertEquals("2 1", m_intEnum.convertToStringValue(asList(5, 2, 1)));
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

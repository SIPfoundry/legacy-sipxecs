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

import java.util.Arrays;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class BeanWithGroupsTest extends TestCase {
    private BeanWithGroups m_bean;

    protected void setUp() throws Exception {
        m_bean = new BirdWithGroups();
    }

    static class BirdWithGroups extends BeanWithGroups {
        protected Setting loadSettings() {
            return TestHelper.loadSettings(BeanWithGroupsTest.class, "birds.xml");
        }
    }

    public void testGetSettingValueOneGroup() {
        Group g1 = new Group();
        Setting s = m_bean.getSettings().getSetting("towhee/rufous-sided");
        assertNull(m_bean.getSettingValue("towhee/rufous-sided"));

        g1.setSettingValue(s, new SettingValueImpl("10"), null);
        m_bean.addGroup(g1);

        assertEquals("10", m_bean.getSettingValue("towhee/rufous-sided"));

        m_bean.removeGroup(g1);
        g1.setSettingValue(s, new SettingValueImpl("10"), null);
    }

    public void testGetSettingValueMultipleGroup() {
        Group g1 = new Group();
        Setting s = m_bean.getSettings().getSetting("towhee/rufous-sided");
        g1.setSettingValue(s, new SettingValueImpl("10"), null);
        m_bean.addGroup(g1);

        assertEquals("10", m_bean.getSettingValue("towhee/rufous-sided"));

        Group g2 = new Group();
        g2.setWeight(1);
        g2.setSettingValue(s, new SettingValueImpl("20"), null);
        m_bean.addGroup(g2);

        assertEquals("20", m_bean.getSettingValue("towhee/rufous-sided"));

        // changing g2 influences setting value
        g2.setSettingValue(s, new SettingValueImpl("15"), null);
        assertEquals("15", m_bean.getSettingValue("towhee/rufous-sided"));

        // changing g1 does not...
        g1.setSettingValue(s, new SettingValueImpl("16"), null);
        assertEquals("15", m_bean.getSettingValue("towhee/rufous-sided"));

        // until we remove g2
        m_bean.removeGroup(g2);
        assertEquals("16", m_bean.getSettingValue("towhee/rufous-sided"));

        // if you set value on the bean groups do not matter
        m_bean.setSettingValue("towhee/rufous-sided", "4");
        assertEquals("4", m_bean.getSettingValue("towhee/rufous-sided"));

        m_bean.addGroup(g2);
        assertEquals("4", m_bean.getSettingValue("towhee/rufous-sided"));

        // and groups setting value does not change
        Setting settingTrs = m_bean.getSettings().getSetting("towhee/rufous-sided");
        assertEquals("16", g1.getSettingValue(settingTrs).getValue());
    }

    public void testGetGroupsNames() {
        Group[] groups = new Group[2];
        groups[0] = new Group();
        groups[0].setName("g1");
        groups[1] = new Group();
        groups[1].setName("g2");

        assertEquals("", m_bean.getGroupsNames());
        m_bean.setGroupsAsList(Arrays.asList(groups));
        assertEquals("g1 g2", m_bean.getGroupsNames());
    }
}

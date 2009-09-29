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

import java.util.Formatter;

import junit.framework.TestCase;

public class SettingArrayTest extends TestCase {

    private SettingSet m_set;
    private SettingArray m_settingArray;

    protected void setUp() {
        Setting b = new SettingImpl("b");
        Setting a = new SettingSet("a");
        a.addSetting(b);

        SettingSet bongo = new SettingSet("bongo");
        bongo.addSetting(a);

        m_settingArray = new SettingArray("kuku", 3);
        m_settingArray.addSetting(bongo);

        m_set = new SettingSet();
        m_set.addSetting(m_settingArray);
    }

    public void testGetSetting() {
        assertSame(m_settingArray, m_set.getSetting("kuku"));
        Setting a = m_set.getSetting("kuku/bongo[0]/a");
        assertNotNull(a);
        assertEquals("a", a.getName());
        assertEquals("kuku/bongo[0]/a", a.getPath());

        Setting b = m_set.getSetting("kuku/bongo[1]/a/b");
        assertNotNull(a);
        assertEquals("b", b.getName());
        assertEquals("kuku/bongo[1]/a/b", b.getPath());
    }

    public void testAcceptVisitor() {
        final StringBuffer names = new StringBuffer();

        SettingVisitor dummyVisitor = new AbstractSettingVisitor() {
            private Formatter m_formatter = new Formatter(names);

            public void visitSetting(Setting setting) {
                m_formatter.format("/%s", setting.getName());
            }

            public boolean visitSettingGroup(SettingSet group) {
                if (group.getIndex() >= 0) {
                    m_formatter.format("/%s[%d]", group.getName(), group.getIndex());
                } else {
                    m_formatter.format("/%s", group.getName());
                }
                return true;
            }
        };

        m_set.acceptVisitor(dummyVisitor);

        assertEquals("//bongo[0]/a/b/bongo[1]/a/b/bongo[2]/a/b", names.toString());
    }

    public void testCopy() {
        Setting copy = m_set.copy();
        String pathA = "kuku/bongo[0]/a";
        Setting settingA = m_set.getSetting(pathA);
        Setting copyA = copy.getSetting(pathA);
        assertEquals(settingA.getPath(), copyA.getPath());
        assertNotSame(settingA, copyA);
    }
}

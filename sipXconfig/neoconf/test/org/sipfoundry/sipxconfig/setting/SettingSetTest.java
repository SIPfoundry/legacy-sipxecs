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

import org.easymock.EasyMock;
import org.easymock.IMocksControl;

public class SettingSetTest extends TestCase {
    private SettingSet m_set;
    private Setting[] m_settings;
    private SettingSet m_settingA;
    private SettingImpl m_settingB;

    protected void setUp() {
        m_set = new SettingSet();

        SettingSet bongo = new SettingSet("bongo");

        m_settings = new Setting[] {
            new SettingImpl("kuku"), bongo, new SettingImpl("kuku1")
        };
        for (Setting setting : m_settings) {
            m_set.addSetting(setting);
        }

        m_settingA = new SettingSet("a");
        m_settingB = new SettingImpl("b");
        bongo.addSetting(m_settingA);
        m_settingA.addSetting(m_settingB);
    }

    public void testGetDefaultSetting() {
        assertEquals("kuku", m_set.getDefaultSetting(Setting.class).getName());
        assertEquals("bongo", m_set.getDefaultSetting(SettingSet.class).getName());
    }

    public void testVisitSettingGroup() {
        IMocksControl settingVisitorControl = EasyMock.createControl();
        SettingVisitor settingVisitor = settingVisitorControl.createMock(SettingVisitor.class);
        settingVisitor.visitSettingGroup(m_set);
        settingVisitorControl.andReturn(false);
        settingVisitorControl.replay();
        m_set.acceptVisitor(settingVisitor);
        settingVisitorControl.verify();

        settingVisitorControl.reset();
        settingVisitor.visitSettingGroup(m_set);
        settingVisitorControl.andReturn(true);
        settingVisitor.visitSetting(m_settings[0]);
        settingVisitor.visitSettingGroup((SettingSet) m_settings[1]);
        settingVisitorControl.andReturn(true);

        settingVisitor.visitSettingGroup(m_settingA);
        settingVisitorControl.andReturn(false);

        settingVisitor.visitSetting(m_settings[2]);
        settingVisitorControl.replay();
        m_set.acceptVisitor(settingVisitor);
        settingVisitorControl.verify();
    }

    public void testGetSettingEmptyPath() {
        assertSame(m_set, m_set.getSetting(""));
        assertSame(m_set, m_set.getSetting(null));
    }

    public void testGetSetting() {
        Setting kuku = m_set.getSetting("kuku");
        assertEquals("kuku", kuku.getName());

        String path = kuku.getParent().getPath();
        assertSame(m_set, m_set.getSetting(path));
    }

    public void testGetSettingPath() {
        Setting a = m_set.getSetting("bongo/a");
        assertSame(m_settingA, a);
        Setting b = m_set.getSetting("bongo/a/b");
        assertSame(m_settingB, b);
    }
}

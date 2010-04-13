/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.setting;

import java.util.Collection;

import junit.framework.TestCase;

import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class SettingsFieldsetTest extends TestCase {

    private SettingsFieldset m_fieldset;

    protected void setUp() throws Exception {
        Creator instantiator = new Creator();
        m_fieldset = (SettingsFieldset) instantiator.newInstance(SettingsFieldset.class);
        m_fieldset.setSettings(new SettingSet("x"));
    }

    public void testRender() throws Exception {
        IMocksControl control = EasyMock.createNiceControl();
        Setting setting = control.createMock(Setting.class);
        setting.getParent();
        control.andReturn(null).atLeastOnce();
        setting.isAdvanced();
        control.andReturn(true);
        setting.isAdvanced();
        control.andReturn(false);
        control.replay();

        m_fieldset.getSettings().addSetting(setting);

        m_fieldset.setShowAdvanced(true);
        m_fieldset.setCurrentSetting(setting);
        assertTrue(m_fieldset.getRenderSetting());
        assertTrue(m_fieldset.getRenderSetting());

        m_fieldset.setShowAdvanced(false);
        assertFalse(m_fieldset.getRenderSetting());
        assertTrue(m_fieldset.getRenderSetting());

        control.verify();
    }

    public void testRenderHidden() throws Exception {
        SettingSet set = new SettingSet();
        SettingImpl toggle = new SettingImpl("toggle");

        set.addSetting(new SettingImpl("a"));
        set.addSetting(toggle);
        set.addSetting(new SettingImpl("c"));

        toggle.setHidden(true);
        m_fieldset.setSettings(set);
        m_fieldset.prepareForRender(null);

        Collection<Setting> flat = m_fieldset.getFlattenedSettings();
        assertEquals(3, flat.size());
        assertFalse(flat.contains(toggle));
        assertTrue(flat.contains(set));

        toggle.setHidden(false);
        m_fieldset.setFlattenedSettings(null);
        m_fieldset.prepareForRender(null);

        flat = m_fieldset.getFlattenedSettings();
        assertEquals(4, flat.size());
        assertTrue(flat.contains(toggle));
        assertTrue(flat.contains(set));
    }

    public void testRenderToggle() throws Exception {
        SettingSet set = new SettingSet();

        SettingImpl toggle = new SettingImpl("toggle");

        set.addSetting(new SettingImpl("a"));
        set.addSetting(toggle);
        set.addSetting(new SettingImpl("c"));

        SettingsIron iron = new SettingsIron();
        set.acceptVisitor(iron);
        assertFalse(iron.isAdvanced());

        toggle.setAdvanced(true);
        iron = new SettingsIron();
        set.acceptVisitor(iron);
        assertTrue(iron.isAdvanced());
    }
}

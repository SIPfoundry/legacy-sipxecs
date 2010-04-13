/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.site.setting;

import java.util.List;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class SettingsIronTest extends TestCase {
    public void testGetFlat() {
        SettingSet g1 = new SettingSet();
        g1.setName("g1");

        SettingImpl s1 = new SettingImpl("s1");
        SettingImpl s2 = new SettingImpl("s2");

        g1.addSetting(s1);
        g1.addSetting(s2);

        SettingsIron iron = new SettingsIron();

        g1.acceptVisitor(iron);
        List<Setting> flat = iron.getFlat();
        assertEquals(3, flat.size());
        assertEquals("g1", flat.get(0).getName());
        assertEquals("s1", flat.get(1).getName());
        assertEquals("s2", flat.get(2).getName());
    }

    public void testGetHidden() {
        SettingSet g1 = new SettingSet();
        g1.setName("g1");

        SettingImpl s1 = new SettingImpl("s1");
        s1.setHidden(true);
        SettingImpl s2 = new SettingImpl("s2");

        g1.addSetting(s1);
        g1.addSetting(s2);

        SettingsIron iron = new SettingsIron();
        iron.setSettingsToHide(null);

        g1.acceptVisitor(iron);
        List<Setting> flat = iron.getFlat();
        assertEquals(2, flat.size());
        assertSame("g1", flat.get(0).getName());
        assertSame("s2", flat.get(1).getName());
    }

    public void testGetFlatExtraHidden() {
        SettingSet g1 = new SettingSet();
        g1.setName("g1");

        SettingImpl s1 = new SettingImpl("s1");
        SettingImpl s2 = new SettingImpl("s2");

        g1.addSetting(s1);
        g1.addSetting(s2);

        SettingsIron iron = new SettingsIron();
        iron.setSettingsToHide("s1, s2");

        g1.acceptVisitor(iron);
        List<Setting> flat = iron.getFlat();
        assertEquals(1, flat.size());
        assertSame("g1", flat.get(0).getName());
    }
}

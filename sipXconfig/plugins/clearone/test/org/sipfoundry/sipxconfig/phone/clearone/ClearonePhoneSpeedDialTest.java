/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.clearone;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.setting.SettingValue;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

import junit.framework.TestCase;

public class ClearonePhoneSpeedDialTest extends TestCase {
    private final static String[][] DATA = {
        {
            "bongo", "1234"
        }, {
            "kuku", "2345"
        }

    };
    private ClearonePhoneSpeedDial m_speedDial;
    private SettingImpl m_setting;

    protected void setUp() throws Exception {
        List<Button> buttons = new ArrayList<Button>();
        for (int i = 0; i < DATA.length; i++) {
            Button button = new Button();
            button.setLabel(DATA[i][0]);
            button.setNumber(DATA[i][1]);
            buttons.add(button);
        }
        SpeedDial sd = new SpeedDial();
        sd.setButtons(buttons);

        m_speedDial = new ClearonePhoneSpeedDial(sd);

        m_setting = new SettingImpl();

        SettingSet set = new SettingSet();
        set.setName("speed_dial");

        set.addSetting(m_setting);

        SettingSet root = new SettingSet();
        root.addSetting(set);
    }

    public void testGetSettingValue() {
        m_setting.setName("name");

        SettingValue value = m_speedDial.getSettingValue(m_setting);
        assertNull(value);

        m_setting.setName("speed_dial_0");
        value = m_speedDial.getSettingValue(m_setting);
        assertNotNull(value);
        assertEquals("1234", value.getValue());

        m_setting.setName("speed_dial_1");
        value = m_speedDial.getSettingValue(m_setting);
        assertNotNull(value);
        assertEquals("2345", value.getValue());

        m_setting.setName("speed_dial_2");
        value = m_speedDial.getSettingValue(m_setting);
        assertNull(value);
    }
}

/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.fail2ban;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallSettings;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class Fail2banConfigTest {
    private Fail2banConfig m_config;
    private Fail2banSettings m_settings;
    private StringWriter m_actual;
    
    @Before
    public void setUp() {
        m_config = new Fail2banConfig();
        m_settings = new Fail2banSettings();
        m_settings.setModelFilesContext(TestHelper.getModelFilesContext());
        m_actual = new StringWriter();
    }

    @Test
    public void security() throws IOException {
        FirewallManager firewall = EasyMock.createMock(FirewallManager.class);
        FirewallSettings firewallSetting = new FirewallSettings();
        firewallSetting.setModelFilesContext(TestHelper.getModelFilesContext());
        firewall.getSettings();
        EasyMock.expectLastCall().andReturn(firewallSetting);
        EasyMock.replay(firewall);
        m_settings.setFirewallManager(firewall);

        m_settings.setSettingTypedValue("config/ignoreip", "127.0.0.1");
        m_settings.setSettingTypedValue("config/bantime", 300);
        m_config.writeConfig(m_actual, m_settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-security.yaml"));
        assertEquals(expected, m_actual.toString());
    }
}

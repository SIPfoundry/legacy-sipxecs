/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Setting;

public class ConfigAgentConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxConfigAgentService configAgentService = new SipxConfigAgentService();
        configAgentService.setBeanName(SipxConfigAgentService.BEAN_ID);
        configAgentService.setSettings(TestHelper.loadSettings("sipxconfigagent/configagent.xml"));

        Setting portSetting = configAgentService.getSettings().getSetting("configagent-config/CONFIG_SERVER_AGENT_PORT");
        portSetting.setValue("9901");

        SipxServiceManager serviceManager = TestHelper.getMockSipxServiceManager(true, configAgentService);

        ConfigAgentConfiguration out = new ConfigAgentConfiguration();
        out.setTemplate("sipxconfigagent/config-agent.properties.vm");
        out.setSipxServiceManager(serviceManager);

        assertCorrectFileGeneration(out, "expected-config-agent.properties");
    }
}

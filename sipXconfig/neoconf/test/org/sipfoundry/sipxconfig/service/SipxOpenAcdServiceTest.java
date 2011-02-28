/*
 *
 *
 * Copyright (C) 2011 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.service;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import java.util.ArrayList;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.LoggingManager;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxOpenAcdServiceTest extends SipxServiceTestBase {

    public void testOpenAcdLogLevels() {
        SipxOpenAcdService openAcdService = new SipxOpenAcdService();
        initCommonAttributes(openAcdService);
        Setting settings = TestHelper.loadSettings("openacd/sipxopenacd.xml");
        openAcdService.setSettings(settings);

        Setting appSettings = openAcdService.getSettings().getSetting("openacd-config");
        setLogLevel(appSettings, "DEBUG");
        assertEquals("debug", openAcdService.getOpenAcdLogLevel());

        setLogLevel(appSettings, "INFO");
        assertEquals("info", openAcdService.getOpenAcdLogLevel());

        setLogLevel(appSettings, "NOTICE");
        assertEquals("notice", openAcdService.getOpenAcdLogLevel());

        setLogLevel(appSettings, "WARNING");
        assertEquals("warning", openAcdService.getOpenAcdLogLevel());

        setLogLevel(appSettings, "ERR");
        assertEquals("error", openAcdService.getOpenAcdLogLevel());

        setLogLevel(appSettings, "CRIT");
        assertEquals("critical", openAcdService.getOpenAcdLogLevel());

        setLogLevel(appSettings, "ALERT");
        assertEquals("alert", openAcdService.getOpenAcdLogLevel());

        setLogLevel(appSettings, "EMERG");
        assertEquals("emergency", openAcdService.getOpenAcdLogLevel());
    }

    public void testManageLogLevels() {
        SipxOpenAcdService openAcdService = new SipxOpenAcdService();
        initCommonAttributes(openAcdService);
        Setting settings = TestHelper.loadSettings("openacd/sipxopenacd.xml");
        openAcdService.setSettings(settings);

        Setting appSettings = openAcdService.getSettings().getSetting("openacd-config");
        setLogLevel(appSettings, "CRIT");

        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.storeService(openAcdService, false);

        LoggingManager loggingManager = createMock(LoggingManager.class);
        loggingManager.getEntitiesToProcess();
        expectLastCall().andReturn(new ArrayList<LoggingEntity>()).atLeastOnce();

        replay(loggingManager, sipxServiceManager);
        openAcdService.setSipxServiceManager(sipxServiceManager);
        openAcdService.setLoggingManager(loggingManager);

        openAcdService.setLogLevel("DEBUG");
        verify(loggingManager, sipxServiceManager);
    }

    private void setLogLevel(Setting appSettings, String level) {
        appSettings.getSetting("SIPX_OPENACD_LOG_LEVEL").setValue(level);
    }

}

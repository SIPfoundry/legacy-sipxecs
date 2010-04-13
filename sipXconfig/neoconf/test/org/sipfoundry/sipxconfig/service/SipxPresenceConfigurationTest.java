/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.setting.Setting;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxPresenceConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxPresenceService presenceService = new SipxPresenceService();
        initCommonAttributes(presenceService);
        Setting settings = TestHelper.loadSettings("sipxpresence/sipxpresence.xml");
        presenceService.setSettings(settings);

        Setting presenceSettings = presenceService.getSettings().getSetting("presence-config");
        presenceSettings.getSetting("SIP_PRESENCE_LOG_LEVEL").setValue("CRIT");
        presenceSettings.getSetting("SIP_PRESENCE_SIGN_IN_CODE").setValue("*76");
        presenceSettings.getSetting("SIP_PRESENCE_SIGN_OUT_CODE").setValue("*77");
        presenceSettings.getSetting("PRESENCE_SERVER_SIP_PORT").setValue("5131");
        presenceSettings.getSetting("PRESENCE_SERVER_HTTP_PORT").setValue("8112");

        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxPresenceService.BEAN_ID);
        expectLastCall().andReturn(presenceService).atLeastOnce();
        replay(sipxServiceManager);

        List<AcdServer> acdServers = new ArrayList<AcdServer>();
        AcdServer acdServer = new AcdServer();
        acdServers.add(acdServer);
        AcdContext acdContext  = createMock(AcdContext.class);
        acdContext.getServers();
        expectLastCall().andReturn(acdServers).atLeastOnce();
        replay(acdContext);

        SipxPresenceConfiguration out = new SipxPresenceConfiguration();
        out.setSipxServiceManager(sipxServiceManager);
        out.setAcdContext(acdContext);
        out.setTemplate("sipxpresence/sipxpresence-config.vm");

        assertCorrectFileGeneration(out, "expected-presence-config");

        verify(sipxServiceManager);
        verify(acdContext);
    }
}

/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Setting;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxRlsConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxRlsService service = new SipxRlsService();
        service.setRlsPort("5140");
        Setting settings = TestHelper.loadSettings("sipxrls/sipxrls.xml");
        service.setSettings(settings);
        initCommonAttributes(service);

        Setting parkSettings = service.getSettings().getSetting("rls-config");
        parkSettings.getSetting("SIP_RLS_LOG_LEVEL").setValue("WARN");

        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxRlsService.BEAN_ID);
        expectLastCall().andReturn(service).atLeastOnce();
        replay(sipxServiceManager);

        SipxRlsConfiguration config = new SipxRlsConfiguration();
        config.setTemplate("sipxrls/sipxrls-config.vm");
        config.setSipxServiceManager(sipxServiceManager);

        assertCorrectFileGeneration(config, "expected-rls-config");

        verify(sipxServiceManager);
    }
}

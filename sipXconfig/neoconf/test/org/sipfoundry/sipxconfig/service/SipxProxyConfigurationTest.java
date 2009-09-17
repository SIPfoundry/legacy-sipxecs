/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.setting.Setting;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

public class SipxProxyConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxProxyConfiguration out = new SipxProxyConfiguration();
        out.setTemplate("sipxproxy/sipXproxy-config.vm");

        SipxCallResolverService callResolverService = new SipxCallResolverService();
        Setting callResolverSettings = TestHelper.loadSettings("sipxcallresolver/sipxcallresolver.xml");
        callResolverSettings.getSetting("callresolver").getSetting("CALLRESOLVER_CALL_STATE_DB").setValue("DISABLE");
        callResolverService.setSettings(callResolverSettings);

        SipxProxyService proxyService = new SipxProxyService();
        initCommonAttributes(proxyService);
        Setting settings = TestHelper.loadSettings("sipxproxy/sipxproxy.xml");
        proxyService.setSettings(settings);

        Setting proxySettings = proxyService.getSettings().getSetting("proxy-configuration");
        proxySettings.getSetting("SIPX_PROXY_DEFAULT_EXPIRES").setValue("35");
        proxySettings.getSetting("SIPX_PROXY_DEFAULT_SERIAL_EXPIRES").setValue("170");
        proxySettings.getSetting("SIPX_PROXY_LOG_LEVEL").setValue("CRIT");
        proxySettings.getSetting("SIPX_PROXY_DIALOG_SUBSCRIBE_AUTHENTICATION").setValue("dialog");
        proxySettings.getSetting("SIP_PORT").setValue("5064");
        proxySettings.getSetting("TLS_SIP_PORT").setValue("5065");

        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxCallResolverService.BEAN_ID);
        expectLastCall().andReturn(callResolverService).atLeastOnce();
        sipxServiceManager.getServiceByBeanId(SipxProxyService.BEAN_ID);
        expectLastCall().andReturn(proxyService).atLeastOnce();
        replay(sipxServiceManager);

        out.setSipxServiceManager(sipxServiceManager);

        assertCorrectFileGeneration(out, "expected-proxy-config");
    }

    @Override
    protected Location createDefaultLocation() {
        Location location = super.createDefaultLocation();
        location.setAddress("192.168.1.2");
        location.setFqdn("sipx1.example.org");
        return location;
    }
}

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

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxProxyConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxProxyConfiguration out = new SipxProxyConfiguration();
        out.setVelocityEngine(TestHelper.getVelocityEngine());
        out.setTemplate("sipxproxy/sipXproxy-config.vm");

        SipxCallResolverService callResolverService = new SipxCallResolverService();
        Setting callResolverSettings = TestHelper
                .loadSettings("sipxcallresolver/sipxcallresolver.xml");
        callResolverSettings.getSetting("callresolver").getSetting("CALLRESOLVER_CALL_STATE_DB")
                .setValue("DISABLE");
        callResolverService.setSettings(callResolverSettings);

        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxCallResolverService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(callResolverService).atLeastOnce();
        EasyMock.replay(sipxServiceManager);
        out.setSipxServiceManager(sipxServiceManager);

        SipxProxyService proxyService = new SipxProxyService();
        initCommonAttributes(proxyService);
        Setting settings = TestHelper.loadSettings("sipxproxy/sipxproxy.xml");
        proxyService.setSettings(settings);

        Setting proxySettings = proxyService.getSettings().getSetting("proxy-configuration");
        proxySettings.getSetting("SIPX_PROXY_DEFAULT_EXPIRES").setValue("35");
        proxySettings.getSetting("SIPX_PROXY_DEFAULT_SERIAL_EXPIRES").setValue("170");
        proxySettings.getSetting("SIPX_PROXY_LOG_LEVEL").setValue("CRIT");

        proxyService.setSecureSipPort("5061");
        proxyService.setSipSrvOrHostport("sipsrv.example.org");

        proxyService.setCallResolverCallStateDb("CALL_RESOLVER_DB");

        out.generate(proxyService);

        assertCorrectFileGeneration(out, "expected-proxy-config");

    }
}

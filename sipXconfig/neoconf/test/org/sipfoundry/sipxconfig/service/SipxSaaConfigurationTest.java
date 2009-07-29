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

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class SipxSaaConfigurationTest extends SipxServiceTestBase {
    public void testWrite() throws Exception {
        SipxSaaService saaService = new SipxSaaService();
        saaService.setBeanId(SipxSaaService.BEAN_ID);
        saaService.setModelDir("sipxsaa");
        saaService.setModelName("sipxsaa.xml");
        saaService.setModelFilesContext(TestHelper.getModelFilesContext());
        saaService.setTcpPort(9910);
        saaService.setUdpPort(9911);

        DomainManager domainManager = TestUtil.getMockDomainManager();
        domainManager.getAuthorizationRealm();
        EasyMock.expectLastCall().andReturn("example.org");
        EasyMock.replay(domainManager);
        saaService.setDomainManager(domainManager);

        Setting logSetting = saaService.getSettings().getSetting("logging/SIP_SAA_LOG_LEVEL");
        logSetting.setValue("CRIT");

        SipxSaaConfiguration out = new SipxSaaConfiguration();

        SipxServiceManager sipxServiceManager = TestUtil.getMockSipxServiceManager(true, saaService);
        out.setSipxServiceManager(sipxServiceManager);

        out.setTemplate("sipxsaa/sipxsaa-config.vm");

        assertCorrectFileGeneration(out, "expected-sipxsaa-config");
    }
}

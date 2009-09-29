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

import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import org.sipfoundry.sipxconfig.test.TestUtil;

public class StatusPluginConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxService statusService = new SipxStatusService();
        initCommonAttributes(statusService);
        statusService.setBeanName(SipxStatusService.BEAN_ID);

        SipxIvrService ivrService = new SipxIvrService();
        ivrService.setModelDir("sipxivr");
        ivrService.setModelName("sipxivr.xml");
        initCommonAttributes(ivrService);
        ivrService.setBeanName(SipxIvrService.BEAN_ID);

        SipxServiceManager sipxServiceManager = TestUtil.getMockSipxServiceManager(false, statusService,
                ivrService);
        replay(sipxServiceManager);

        StatusPluginConfiguration out = new StatusPluginConfiguration();
        out.setSipxServiceManager(sipxServiceManager);

        out.setTemplate("sipxstatus/status-plugin.vm");

        assertCorrectFileGeneration(out, "expected-status-plugin-config");
        verify(sipxServiceManager);
    }
}

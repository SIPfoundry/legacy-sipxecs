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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class StatusPluginConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxStatusService statusService = new SipxStatusService();
        initCommonAttributes(statusService);

        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxStatusService.BEAN_ID);
        expectLastCall().andReturn(statusService).atLeastOnce();
        replay(sipxServiceManager);

        StatusPluginConfiguration out = new StatusPluginConfiguration();
        out.setSipxServiceManager(sipxServiceManager);

        out.setTemplate("sipxstatus/status-plugin.vm");

        assertCorrectFileGeneration(out, "expected-status-plugin-config");
        verify(sipxServiceManager);
    }
}

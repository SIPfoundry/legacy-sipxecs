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

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.setting.Setting;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

public class SipxStatusConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxStatusService statusService = new SipxStatusService();
        initCommonAttributes(statusService);
        statusService.setSettings(TestHelper.loadSettings("sipxstatus/sipxstatus.xml"));
        statusService.setHttpsPort(8101);

        Setting statusConfigSettings = statusService.getSettings().getSetting("status-config");
        statusConfigSettings.getSetting("SIP_STATUS_LOG_LEVEL").setValue("CRIT");

        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxStatusService.BEAN_ID);
        expectLastCall().andReturn(statusService).atLeastOnce();
        replay(sipxServiceManager);

        SipxStatusConfiguration out = new SipxStatusConfiguration();
        out.setSipxServiceManager(sipxServiceManager);
        out.setTemplate("sipxstatus/status-config.vm");

        assertCorrectFileGeneration(out, "expected-status-config");
    }

    @Override
    protected Location createDefaultLocation() {
        Location location = super.createDefaultLocation();
        location.setAddress("192.168.1.2");
        return location;
    }
}

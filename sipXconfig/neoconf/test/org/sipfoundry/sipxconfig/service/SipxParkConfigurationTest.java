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
import static org.easymock.EasyMock.verify;


public class SipxParkConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxParkService parkService = new SipxParkService();
        initCommonAttributes(parkService);
        parkService.setParkServerSipPort("9999");
        Setting settings = TestHelper.loadSettings("sipxpark/sipxpark.xml");
        parkService.setSettings(settings);

        Setting parkSettings = parkService.getSettings().getSetting("park-config");
        parkSettings.getSetting("SIP_PARK_LOG_LEVEL").setValue("CRIT");

        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxParkService.BEAN_ID);
        expectLastCall().andReturn(parkService).atLeastOnce();
        replay(sipxServiceManager);

        SipxParkConfiguration out = new SipxParkConfiguration();
        out.setSipxServiceManager(sipxServiceManager);
        out.setTemplate("sipxpark/sipxpark-config.vm");

        assertCorrectFileGeneration(out, "expected-park-config");

        verify(sipxServiceManager);
    }

    @Override
    protected Location createDefaultLocation() {
        Location location = new Location();
        location.setName("localLocation");
        location.setFqdn("mysystem.realm.example.org");
        location.setAddress("192.168.1.2");
        return location;
    }
}

/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Arrays;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import static org.sipfoundry.sipxconfig.test.TestUtil.*;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxCallResolverConfigurationTest extends SipxServiceTestBase {
    private SipxCallResolverService m_callResolverService;
    private SipxServiceManager m_sipxServiceManager;
    private SipxCallResolverAgentService m_callResolverAgentService;

    @Override
    protected void setUp() throws Exception {
        m_callResolverAgentService = new SipxCallResolverAgentService();
        m_callResolverAgentService.setBeanId(SipxCallResolverAgentService.BEAN_ID);
        initCommonAttributes(m_callResolverAgentService);

        m_callResolverService = new SipxCallResolverService();
        m_callResolverService.setBeanId(SipxCallResolverService.BEAN_ID);
        initCommonAttributes(m_callResolverService);
        Setting settings = TestHelper.loadSettings("sipxcallresolver/sipxcallresolver.xml");
        m_callResolverService.setSettings(settings);

        Setting callresolverSettings = m_callResolverService.getSettings().getSetting("callresolver");
        callresolverSettings.getSetting("SIP_CALLRESOLVER_PURGE").setValue("DISABLE");
        callresolverSettings.getSetting("SIP_CALLRESOLVER_PURGE_AGE_CDR").setValue("40");
        callresolverSettings.getSetting("SIP_CALLRESOLVER_PURGE_AGE_CSE").setValue("10");
        callresolverSettings.getSetting("SIP_CALLRESOLVER_LOG_LEVEL").setValue("CRIT");

        m_callResolverService.setAgentPort(8090);

        m_sipxServiceManager = getMockSipxServiceManager(true, m_callResolverAgentService, m_callResolverService);
    }

    public void testWriteNoHA() throws Exception {
        Location primary = new Location();
        primary.setPrimary(true);
        primary.setFqdn("localhost");
        primary.setAddress("192.168.1.1");
        primary.setName("primary");

        LocationsManager locationManager = createMock(LocationsManager.class);

        locationManager.getPrimaryLocation();
        expectLastCall().andReturn(primary).anyTimes();
        locationManager.getLocationsForService(m_callResolverAgentService);
        expectLastCall().andReturn(Arrays.asList()).anyTimes();

        replay(locationManager);

        m_callResolverService.setLocationManager(locationManager);

        SipxCallResolverConfiguration out = new SipxCallResolverConfiguration();
        out.setSipxServiceManager(m_sipxServiceManager);
        out.setLocationsManager(locationManager);
        out.setTemplate("sipxcallresolver/callresolver-config.vm");

        assertCorrectFileGeneration(out, "expected-callresolver-config-no-ha");

        verify(locationManager);
    }

    public void testWrite() throws Exception {
        Location primary = new Location();
        primary.setPrimary(true);
        primary.setFqdn("localhost");
        primary.setAddress("192.168.1.1");
        primary.setName("primary");
        Location secondary = new Location();
        secondary.setPrimary(false);
        secondary.setFqdn("remote.exampl.com");
        secondary.setAddress("192.168.1.2");
        secondary.setName("remote");

        LocationsManager locationManager = createMock(LocationsManager.class);

        locationManager.getPrimaryLocation();
        expectLastCall().andReturn(primary).anyTimes();
        locationManager.getLocationsForService(m_callResolverAgentService);
        expectLastCall().andReturn(Arrays.asList(secondary)).anyTimes();

        m_callResolverService.setLocationManager(locationManager);

        replay(locationManager);

        SipxCallResolverConfiguration out = new SipxCallResolverConfiguration();
        out.setSipxServiceManager(m_sipxServiceManager);
        out.setLocationsManager(locationManager);
        out.setTemplate("sipxcallresolver/callresolver-config.vm");

        assertCorrectFileGeneration(out, "expected-callresolver-config");

        verify(locationManager);
    }
}

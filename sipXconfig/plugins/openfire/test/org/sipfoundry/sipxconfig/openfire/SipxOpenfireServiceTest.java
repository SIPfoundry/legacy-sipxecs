/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.openfire;

import static org.easymock.EasyMock.expectLastCall;

import java.util.Arrays;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class SipxOpenfireServiceTest extends TestCase {
    private SipxOpenfireService m_service;
    private LocationsManager m_locationsManager;
    DomainManager m_domainManager;

    @Override
    public void setUp() {
        m_locationsManager = EasyMock.createMock(LocationsManager.class);
        m_domainManager = EasyMock.createMock(DomainManager.class);

        Location location = new Location();
        location.setName("locationTest");
        location.setFqdn("locationTest");
        location.setAddress("192.168.1.1");
        location.setPrimary(true);
        m_service = new SipxOpenfireService();
        m_locationsManager.getLocationsForService(m_service);
        expectLastCall().andReturn(Arrays.asList(location)).anyTimes();
        EasyMock.replay(m_locationsManager);

        Domain domain = new Domain();
        domain.setName("domain.example");
        m_domainManager.getDomain();
        expectLastCall().andReturn(domain).anyTimes();
        EasyMock.replay(m_domainManager);
        m_service.setDomainManager(m_domainManager);
        m_service.setLocationsManager(m_locationsManager);
        m_service.setModelDir("openfire");
        m_service.setModelName("openfire.xml");
        m_service.initialize();
        m_service.setModelFilesContext(TestHelper.getModelFilesContext());
    }

    public void testSettings() {
        assertEquals("locationTest", m_service.getServerAddress());
        assertEquals(9094, m_service.getPort());

        m_service.setSettingValue(SipxOpenfireService.XML_RPC_PORT_SETTING, "8081");
        m_service.setSettingValue(SipxOpenfireService.HOST_SETTING, "locationTestChanged");

        assertEquals(8081, m_service.getPort());
        assertEquals("locationTestChanged", m_service.getServerAddress());
        assertEquals("NOTICE", m_service.getLogLevel());
    }
}

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
import static org.easymock.EasyMock.replay;

import java.util.Arrays;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

import junit.framework.TestCase;

public class SipxOpenfireContextImplTest extends TestCase {
    private SipxOpenfireService m_service;
    private LocationsManager m_locationsManager;
    DomainManager m_domainManager;

    @Override
    public void setUp() throws Exception {
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

        m_service.setLocationsManager(m_locationsManager);
        m_service.setModelDir("openfire");
        m_service.setModelName("openfire.xml");
        m_service.setLogDir("LogDirTest");
        m_service.setDomainManager(m_domainManager);

        m_service.initialize();
        m_service.setModelFilesContext(TestHelper.getModelFilesContext());
        m_service.setSettingValue(SipxOpenfireService.XML_RPC_PORT_SETTING, "9095");
    }

    public void testOpenfireServerUrl() throws Exception {
        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxOpenfireService.BEAN_ID);
        expectLastCall().andReturn(m_service).atLeastOnce();
        replay(sipxServiceManager);
        SipxOpenfireContextImpl openfireContext = new SipxOpenfireContextImpl();
        openfireContext.setSipxServiceManager(sipxServiceManager);
        assertEquals("http://locationTest:9095/plugins/sipx-openfire/user", openfireContext.getOpenfireServerUrl());
    }

    public void testNoOpenfireServerUrl() throws Exception {
        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxOpenfireService.BEAN_ID);
        expectLastCall().andReturn(null).atLeastOnce();
        replay(sipxServiceManager);
        SipxOpenfireContextImpl openfireContext = new SipxOpenfireContextImpl();
        openfireContext.setSipxServiceManager(sipxServiceManager);
        try {
            openfireContext.getOpenfireServerUrl();
            assertTrue(false);
        } catch (Exception ex) {
            // nothing to do
        }
    }

}

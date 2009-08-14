/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.openfire;

import java.util.Arrays;

import org.sipfoundry.sipxconfig.common.CoreContext;

import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;

import org.sipfoundry.sipxconfig.common.User;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.SipxServiceTestBase;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxOpenfireConfigurationTest extends SipxServiceTestBase {
    private SipxOpenfireService m_service;
    private LocationsManager m_locationsManager;
    SipxServiceManager m_sipxServiceManager;
    DomainManager m_domainManager;
    private CoreContext m_coreContext;

    @Override
    public void setUp() throws Exception {
        m_locationsManager = createMock(LocationsManager.class);
        m_domainManager = createMock(DomainManager.class);

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
        replay(m_domainManager);

        m_service.setLocationsManager(m_locationsManager);
        m_service.setModelDir("openfire");
        m_service.setModelName("openfire.xml");
        m_service.setLogDir("LogDirTest");
        m_service.setDomainManager(m_domainManager);

        m_service.initialize();
        m_service.setModelFilesContext(TestHelper.getModelFilesContext());
        m_service.setSettingValue(SipxOpenfireService.XML_RPC_PORT_SETTING, "9095");

        m_sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        m_sipxServiceManager.getServiceByBeanId(SipxOpenfireService.BEAN_ID);
        expectLastCall().andReturn(m_service).atLeastOnce();

        User xmppuser = new User();
        xmppuser.setSipPassword("1234");
        xmppuser.setUserName(SpecialUserType.XMPP_SERVER.getUserName());

        m_coreContext = createMock(CoreContext.class);
        m_coreContext.getSpecialUser(SpecialUserType.XMPP_SERVER);
        expectLastCall().andReturn(xmppuser);

        replay(m_coreContext, m_sipxServiceManager);
    }

    public void testWrite() throws Exception {

        SipxOpenfireConfiguration config = new SipxOpenfireConfiguration();
        config.setTemplate("openfire/sipxopenfire.vm");
        config.setSipxServiceManager(m_sipxServiceManager);
        config.setCoreContext(m_coreContext);
        assertCorrectFileGeneration(config, "expected-sipxopenfire-config");

        verify(m_sipxServiceManager);
    }
}

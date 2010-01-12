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

import java.util.ArrayList;
import java.util.List;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxRegistrarConfigurationTest extends SipxServiceTestBase {

    private SipxRegistrarService m_registrarService;
    private SipxProxyService m_proxyService;
    private SipxServiceManager m_sipxServiceManager;
    private SipxParkService m_parkService;
    private SipxRegistrarConfiguration m_out;

    public void setUp() {
        m_registrarService = new SipxRegistrarService();
        m_registrarService.setBeanName(SipxRegistrarService.BEAN_ID);
        m_registrarService.setModelDir("sipxregistrar");
        m_registrarService.setModelName("sipxregistrar.xml");
        m_registrarService.setModelFilesContext(TestHelper.getModelFilesContext());
        initCommonAttributes(m_registrarService);

        m_proxyService = new SipxProxyService();
        m_proxyService.setBeanName(SipxProxyService.BEAN_ID);
        m_proxyService.setModelDir("sipxproxy");
        m_proxyService.setModelName("sipxproxy.xml");
        m_proxyService.setModelFilesContext(TestHelper.getModelFilesContext());
        m_proxyService.setSipPort("5060");

        Domain domain = new Domain();
        domain.addAlias("another.example.org");
        domain.setName("example.org");
        DomainManager domainManager = createMock(DomainManager.class);
        expect(domainManager.getDomain()).andReturn(domain).anyTimes();
        expect(domainManager.getAuthorizationRealm()).andReturn("realm.example.org").anyTimes();
        m_registrarService.setDomainManager(domainManager);

        replay(domainManager);

        setSettingValuesForGroup(m_registrarService, "userparam", new String[] {
            "SIP_REDIRECT.090-USERPARAM.STRIP_ALL"
        }, new String[] {
            "Y"
        });
        setSettingValuesForGroup(m_registrarService, "logging", new String[] {
            "SIP_REGISTRAR_LOG_LEVEL"
        }, new String[] {
            "WARNING"
        });
        setSettingValuesForGroup(m_registrarService, "call-pick-up", new String[] {
            "SIP_REDIRECT.100-PICKUP.DIRECTED_CALL_PICKUP_CODE", "SIP_REDIRECT.100-PICKUP.CALL_RETRIEVE_CODE",
            "SIP_REDIRECT.100-PICKUP.CALL_PICKUP_WAIT"
        }, new String[] {
            "*42", "*43", "15.0"
        });
        setSettingValuesForGroup(m_registrarService, "isn", new String[] {
            "SIP_REDIRECT.150-ISN.BASE_DOMAIN", "SIP_REDIRECT.150-ISN.PREFIX"
        }, new String[] {
            "myisndomain.org", null
        });
        setSettingValuesForGroup(m_registrarService, "enum", new String[] {
            "SIP_REDIRECT.160-ENUM.BASE_DOMAIN", "SIP_REDIRECT.160-ENUM.DIAL_PREFIX",
            "SIP_REDIRECT.160-ENUM.ADD_PREFIX", "SIP_REDIRECT.160-ENUM.PREFIX_PLUS"
        }, new String[] {
            "myenumdomain.org", null, "*66", "Y"
        });

        m_registrarService.setProxyServerSipHostport("proxy.example.org");
        m_registrarService.setSipPort("5070");
        m_registrarService.setRegistrarEventSipPort("5075");

        m_parkService = new SipxParkService();
        Location parkLocation = new Location();
        parkLocation.setAddress("192.168.1.5");
        List<Location> locations = new ArrayList<Location>();
        locations.add(parkLocation);
        LocationsManager locationsManager = EasyMock.createNiceMock(LocationsManager.class);
        locationsManager.getLocationsForService(m_parkService);
        EasyMock.expectLastCall().andReturn(locations).anyTimes();

        m_parkService.setLocationsManager(locationsManager);
        m_parkService.setBeanName(SipxParkService.BEAN_ID);
        m_parkService.setParkServerSipPort("9909");

        m_sipxServiceManager = TestUtil.getMockSipxServiceManager(false, m_registrarService,
                m_proxyService, m_parkService);
        expect(m_sipxServiceManager.getServiceParam("openfire-host")).andReturn("192.168.1.10").anyTimes();
        expect(m_sipxServiceManager.getServiceParam("openfire-xml-rpc-port")).andReturn(49094).anyTimes();

        m_out = new SipxRegistrarConfiguration();

        Location primaryLocation = TestUtil.createDefaultLocation();
        Location otherRegistrarLocation = new Location();
        otherRegistrarLocation.setName("Other registrar");
        otherRegistrarLocation.setFqdn("other-registrar.example.org");
        otherRegistrarLocation.addService(new LocationSpecificService(m_registrarService));
        Location otherMediaServerLocation = new Location();
        otherMediaServerLocation.setName("Other media server");
        otherMediaServerLocation.setFqdn("other-media-server.example.org");
        otherMediaServerLocation.addService(new LocationSpecificService(new SipxIvrService()));

        locationsManager.getPrimaryLocation();
        EasyMock.expectLastCall().andReturn(primaryLocation).anyTimes();
        locationsManager.getLocations();
        EasyMock.expectLastCall().andReturn(new Location[] {
            primaryLocation, otherRegistrarLocation, otherMediaServerLocation
        }).anyTimes();
        EasyMock.replay(locationsManager);
        m_out.setLocationsManager(locationsManager);

        m_out.setSipxServiceManager(m_sipxServiceManager);
        m_out.setTemplate("sipxregistrar/registrar-config.vm");
    }

    public void testWriteWithoutOpenfire() throws Exception {
        expect(m_sipxServiceManager.isServiceInstalled("sipxOpenfireService")).andReturn(false);
        replay(m_sipxServiceManager);

        assertCorrectFileGeneration(m_out, "expected-registrar-config-without-openfire");
        verify(m_sipxServiceManager);

    }

    public void testWriteWithOpenfire() throws Exception {
        expect(m_sipxServiceManager.isServiceInstalled("sipxOpenfireService")).andReturn(true);
        replay(m_sipxServiceManager);

        assertCorrectFileGeneration(m_out, "expected-registrar-config-with-openfire");
        verify(m_sipxServiceManager);

    }

    private void setSettingValuesForGroup(SipxRegistrarService registrarService, String group,
            String[] settingNames, String[] values) {
        for (int i = 0; i < settingNames.length; i++) {
            registrarService.getSettings().getSetting(group).getSetting(settingNames[i]).setValue(values[i]);
        }
    }
}

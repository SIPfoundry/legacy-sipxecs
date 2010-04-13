/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import static java.util.Arrays.asList;

import org.apache.commons.collections.CollectionUtils;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.SipxParkService;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceBundle;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

import junit.framework.TestCase;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class LocationTest extends TestCase {
    public void testGetProcessMonitorUrl() {
        Location out = new Location();
        out.setFqdn("localhost");
        assertEquals("https://localhost:8092/RPC2", out.getProcessMonitorUrl());
        Location out1 = new Location();
        out1.setFqdn("mysystem.europe.pmd.com");
        assertEquals("https://mysystem.europe.pmd.com:8092/RPC2", out1.getProcessMonitorUrl());
    }

    @SuppressWarnings("deprecation")
    public void testParseAddress() {
        // it tests setUrl - which is deprecated, because it's only used when migrating from
        // topology.xml
        Location out = new Location();
        out.setUrl("https://localhost:8091/cgi-bin/replication/replication.cgi");
        assertEquals("localhost", out.getFqdn());
        assertNull(out.getAddress());

        Location out1 = new Location();
        out1.setUrl("https://192.168.1.10:8091/cgi-bin/replication/replication.cgi");
        assertEquals("192.168.1.10", out1.getFqdn());
        assertNull(out.getAddress());
    }

    public void testSetServiceDefinitions() {
        Location out = new Location();
        Collection<SipxService> sipxServices = new ArrayList<SipxService>();
        SipxService proxyService = new SipxProxyService();
        proxyService.setBeanName(SipxProxyService.BEAN_ID);
        sipxServices.add(proxyService);
        SipxService registrarService = new SipxRegistrarService();
        registrarService.setBeanName(SipxRegistrarService.BEAN_ID);
        sipxServices.add(registrarService);

        out.setServiceDefinitions(sipxServices);

        Collection<LocationSpecificService> servicesFromOut = out.getServices();
        assertNotNull(servicesFromOut);
        assertEquals(2, servicesFromOut.size());
    }

    public void testSetServices() {
        Location out = new Location();
        Collection<LocationSpecificService> services = new ArrayList<LocationSpecificService>();
        SipxService proxyService = new SipxProxyService();
        LocationSpecificService proxyLss = new LocationSpecificService();
        proxyLss.setSipxService(proxyService);
        services.add(proxyLss);
        SipxService registrarService = new SipxRegistrarService();
        LocationSpecificService registrarLss = new LocationSpecificService();
        registrarLss.setSipxService(registrarService);
        services.add(registrarLss);

        out.setServices(services);

        Collection<LocationSpecificService> servicesFromOut = out.getServices();
        assertNotNull(servicesFromOut);
        assertEquals(2, servicesFromOut.size());
    }

    public void testAddService() {
        Location out = new Location();
        Collection<SipxService> sipxServices = new ArrayList<SipxService>();
        SipxService proxyService = new SipxProxyService();
        proxyService.setBeanName(SipxProxyService.BEAN_ID);
        sipxServices.add(proxyService);
        SipxService registrarService = new SipxRegistrarService();
        registrarService.setBeanName(SipxRegistrarService.BEAN_ID);
        sipxServices.add(registrarService);

        out.setServiceDefinitions(sipxServices);

        SipxService parkService = new SipxParkService();
        parkService.setBeanName(SipxParkService.BEAN_ID);
        out.addService(parkService);

        Collection<LocationSpecificService> servicesFromOut = out.getServices();
        assertNotNull(servicesFromOut);
        assertEquals(3, servicesFromOut.size());
    }

    public void testRemoveService() {
        Location out = new Location();
        Collection<SipxService> sipxServices = new ArrayList<SipxService>();
        SipxService proxyService = new SipxProxyService();
        sipxServices.add(proxyService);
        proxyService.setBeanName(SipxProxyService.BEAN_ID);
        SipxService registrarService = new SipxRegistrarService();
        registrarService.setBeanName(SipxRegistrarService.BEAN_ID);
        sipxServices.add(registrarService);

        out.setServiceDefinitions(sipxServices);

        LocationSpecificService proxyLss = new LocationSpecificService();
        proxyLss.setSipxService(proxyService);
        out.removeService(proxyLss);

        Collection<LocationSpecificService> servicesFromOut = out.getServices();
        assertNotNull(servicesFromOut);
        assertEquals(1, servicesFromOut.size());
    }

    public void testRemoveServices() {
        Location out = new Location();

        SipxService proxyService = new SipxProxyService();
        proxyService.setBeanName(SipxProxyService.BEAN_ID);
        SipxService registrarService = new SipxRegistrarService();
        registrarService.setBeanName(SipxRegistrarService.BEAN_ID);
        SipxService parkService = new SipxParkService();
        parkService.setBeanName(SipxParkService.BEAN_ID);

        // should be possible to remove services on a new location
        out.removeServices(asList(proxyService));

        out.setServiceDefinitions(asList(proxyService, registrarService, parkService));
        out.removeServices(asList(proxyService, parkService));

        Collection<LocationSpecificService> servicesFromOut = out.getServices();
        assertNotNull(servicesFromOut);
        assertEquals(1, servicesFromOut.size());
        assertEquals(registrarService, servicesFromOut.iterator().next().getSipxService());
    }

    public void testGetSipxServices() {
        Location out = new Location();
        assertTrue(out.getSipxServices().isEmpty());

        SipxService proxyService = new SipxProxyService();
        proxyService.setBeanName(SipxProxyService.BEAN_ID);
        SipxService registrarService = new SipxRegistrarService();
        registrarService.setBeanName(SipxRegistrarService.BEAN_ID);

        out.setServiceDefinitions(asList(proxyService, registrarService));
        assertEquals(2, out.getServices().size());

        Collection<SipxService> sipxServices = out.getSipxServices();
        assertEquals(2, sipxServices.size());
        assertTrue(sipxServices.contains(registrarService));
        assertTrue(sipxServices.contains(proxyService));
    }

    public void testGetHostname() {
        Location out = new Location();
        out.setFqdn("sipx.example.org");

        assertEquals("sipx", out.getHostname());
    }

    public void testRemoveServiceByBeanId() {
        Location out = new Location();
        Collection<SipxService> sipxServices = new ArrayList<SipxService>();
        SipxService proxyService = new SipxProxyService();
        proxyService.setBeanName("sipxProxyService");
        sipxServices.add(proxyService);
        SipxService registrarService = new SipxRegistrarService();
        registrarService.setBeanName("sipxRegistrarService");
        sipxServices.add(registrarService);

        out.setServiceDefinitions(sipxServices);

        out.removeServiceByBeanId("sipxProxyService");

        Collection<LocationSpecificService> servicesFromOut = out.getServices();
        assertNotNull(servicesFromOut);
        assertEquals(1, servicesFromOut.size());

        assertNotSame(proxyService, servicesFromOut.iterator().next().getSipxService());
    }

    public void testGetService() {
        Location out = new Location();
        Collection<SipxService> sipxServices = new ArrayList<SipxService>();
        SipxService proxyService = new SipxProxyService();
        proxyService.setBeanName("sipxProxyService");
        sipxServices.add(proxyService);
        SipxService registrarService = new SipxRegistrarService();
        registrarService.setBeanName("sipxRegistrarService");
        sipxServices.add(registrarService);

        out.setServiceDefinitions(sipxServices);

        assertSame(proxyService, out.getService("sipxProxyService").getSipxService());
        assertSame(registrarService, out.getService("sipxRegistrarService").getSipxService());
        assertNull(out.getService("invalidBeanId"));
    }

    public void testGetInstallableBundles() {
        SipxServiceBundle unrestricted = new SipxServiceBundle("unrestricted");
        SipxServiceBundle primaryOnly = new SipxServiceBundle("primaryOnly");
        primaryOnly.setOnlyPrimary(true);
        SipxServiceBundle remoteOnly = new SipxServiceBundle("remoteOnly");
        remoteOnly.setOnlyRemote(true);

        Location location = new Location();
        assertTrue(location.getInstallableBundles(null).isEmpty());

        Collection<SipxServiceBundle> remoteInstallable = location.getInstallableBundles(asList(unrestricted,
                primaryOnly, remoteOnly));
        assertEquals(2, remoteInstallable.size());
        assertFalse(remoteInstallable.contains(primaryOnly));
        assertTrue(remoteInstallable.contains(unrestricted));
        assertTrue(remoteInstallable.contains(remoteOnly));

        location.setPrimary(true);
        Collection<SipxServiceBundle> primaryInstallable = location.getInstallableBundles(asList(unrestricted,
                primaryOnly, remoteOnly));
        assertEquals(2, primaryInstallable.size());
        assertFalse(primaryInstallable.contains(remoteOnly));
        assertTrue(primaryInstallable.contains(unrestricted));
        assertTrue(primaryInstallable.contains(primaryOnly));
    }

    public void testIsBundleInstalled() {
        Location location = new Location();
        assertFalse(location.isBundleInstalled(null));
        assertFalse(location.isBundleInstalled("b1"));

        location.setInstalledBundles(asList("b1", "b2"));
        assertFalse(location.isBundleInstalled(null));
        assertTrue(location.isBundleInstalled("b1"));
        assertFalse(location.isBundleInstalled("b3"));
    }

    public void testInitBundlesExistingLocation() {
        Location location = new Location();
        location.setInstalledBundles(asList("b1", "b2"));
        location.initBundles(null);
        List<String> installedBundles = location.getInstalledBundles();
        assertEquals(2, installedBundles.size());
        assertTrue(installedBundles.contains("b1"));
        assertTrue(installedBundles.contains("b2"));
    }

    public void testInitBundlesNewLocation() {
        SipxServiceBundle b1 = new SipxServiceBundle("b1");
        b1.setBeanName("b1");
        b1.setAutoEnable(true);
        SipxServiceBundle b2 = new SipxServiceBundle("b2");
        b2.setBeanName("b2");
        b2.setAutoEnable(false);
        Collection<SipxServiceBundle> bundles = asList(b1, b2);

        Location location = new Location();

        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.getBundleDefinitions();
        expectLastCall().andReturn(bundles);
        sipxServiceManager.getBundlesForLocation(location);
        expectLastCall().andReturn(bundles);
        sipxServiceManager.getServiceDefinitions(bundles);
        expectLastCall().andReturn(CollectionUtils.EMPTY_COLLECTION);

        replay(sipxServiceManager);

        location.setPrimary(true);
        location.setServices(CollectionUtils.EMPTY_COLLECTION);
        location.initBundles(sipxServiceManager);

        List<String> installedBundles = location.getInstalledBundles();
        assertEquals(1, installedBundles.size());
        assertTrue(installedBundles.contains("b1"));
        assertFalse(installedBundles.contains("b2"));

        verify(sipxServiceManager);
    }

    public void testNotifyOnAddRemoveService() {
        Location out = new Location();
        Collection<SipxService> sipxServices = new ArrayList<SipxService>();

        MockSipxService service = new MockSipxService();
        service.setBeanName("MockService1");
        sipxServices.add(service);

        out.setServiceDefinitions(sipxServices);
        assertEquals("init", service.getTestString());

        out.removeServiceByBeanId("MockService1");
        assertEquals("destroy", service.getTestString());

        out.addServices(sipxServices);
        assertEquals("init", service.getTestString());

        out.removeServices(sipxServices);
        assertEquals("destroy", service.getTestString());

        out.addService(new LocationSpecificService(service));
        assertEquals("init", service.getTestString());

        out.removeService(new LocationSpecificService(service));
        assertEquals("destroy", service.getTestString());

        out.addService(service);
        assertEquals("init", service.getTestString());

    }

    private class MockSipxService extends SipxService {
        private String m_test = "dummy";

        @Override
        public void onInit() {
            m_test = "init";
        }

        @Override
        public void onDestroy() {
            m_test = "destroy";
        }

        public String getTestString() {
            return m_test;
        }
    }
}

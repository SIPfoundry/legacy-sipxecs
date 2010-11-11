/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.admin.logging.AuditLogContextImpl;
import org.sipfoundry.sipxconfig.service.SipxAcdService;
import org.sipfoundry.sipxconfig.service.SipxConfigService;
import org.sipfoundry.sipxconfig.service.SipxParkService;
import org.sipfoundry.sipxconfig.service.SipxPresenceService;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceBundle;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.test.TestUtil;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.createStrictMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.sipfoundry.sipxconfig.TestHelper.asArray;
import static org.sipfoundry.sipxconfig.TestHelper.asArrayElems;
import static org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status.Disabled;
import static org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status.Failed;
import static org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status.Running;
import static org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status.Starting;
import static org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status.Undefined;
import static org.sipfoundry.sipxconfig.test.TestUtil.getMockSipxServiceManager;

public class SipxProcessContextImplTest extends TestCase {
    private SipxProcessContextImpl m_processContextImpl;
    private LocationsManager m_locationsManager;
    private SipxServiceManager m_sipxServiceManager;
    private Set<SipxServiceBundle> m_bundleSet = new HashSet<SipxServiceBundle>();
    SipxRegistrarService m_registrarService;
    SipxParkService m_parkService;
    SipxService m_proxyService;
    SipxPresenceService m_presenceService;
    SipxAcdService m_acdService;

    private final static SipxService[] PROCESSES = new SipxService[] {
        new SipxRegistrarService(), new SipxParkService()
    };

    private final static SipxService[] PROCESSES_2 = new SipxService[] {
        new SipxConfigService(), new SipxParkService()
    };

    private final static SipxService[] PROCESSES_3 = new SipxService[] {
        new SipxRegistrarService()
    };

    private final static SipxService[] START_PROCESSLIST = new SipxService[] {
        new SipxPresenceService(),
    };

    @Override
    protected void setUp() throws Exception {
        m_locationsManager = createNiceMock(LocationsManager.class);
        Location location = TestUtil.createDefaultLocation();
        SipxServiceBundle bundle = new SipxServiceBundle("primarySipRouter");
        bundle.setBeanName(bundle.getName());
        m_bundleSet = new HashSet<SipxServiceBundle>();
        m_bundleSet.add(bundle);
        m_registrarService = new SipxRegistrarService();
        m_registrarService.setProcessName("SIPRegistrar");
        m_registrarService.setBundles(m_bundleSet);
        m_registrarService.setBeanName(SipxRegistrarService.BEAN_ID);

        m_proxyService = new SipxProxyService();
        m_proxyService.setProcessName("SIPXProxy");
        m_proxyService.setBeanName(SipxProxyService.BEAN_ID);
        m_proxyService.setBundles(m_bundleSet);

        m_parkService = new SipxParkService();
        m_parkService.setBeanName(SipxParkService.BEAN_ID);
        m_parkService.setProcessName("MediaServer");
        m_parkService.setBundles(m_bundleSet);

        m_presenceService = new SipxPresenceService();
        m_presenceService.setBeanName(SipxPresenceService.BEAN_ID);
        m_presenceService.setProcessName("PresenceServer");
        m_presenceService.setBundles(m_bundleSet);

        m_acdService = new SipxAcdService();
        m_acdService.setBeanName(SipxAcdService.BEAN_ID);
        m_acdService.setProcessName("ACDServer");

        location.setServiceDefinitions(Arrays.asList(m_registrarService, m_proxyService, m_parkService));
        ArrayList<String> installedBundles = new ArrayList<String>();
        installedBundles.add("primarySipRouter");
        location.setInstalledBundles(installedBundles);
        location.setRegistered(true);

        Location location2 = new Location();
        location2.setFqdn("LocationSecondary");
        SipxPresenceService presenceService = new SipxPresenceService();
        presenceService.setBeanName(SipxPresenceService.BEAN_ID);
        presenceService.setProcessName("PresenceServer");
        location2.setServiceDefinitions(Arrays.asList(presenceService));
        location2.setRegistered(true);

        m_locationsManager.getLocations();
        expectLastCall().andReturn(new Location[] {
            location, location2
        }).anyTimes();
        m_locationsManager.getPrimaryLocation();
        expectLastCall().andReturn(location).anyTimes();
        replay(m_locationsManager);

        m_sipxServiceManager = createNiceMock(SipxServiceManager.class);
        m_sipxServiceManager.getServiceByBeanId(SipxConfigService.BEAN_ID);
        expectLastCall().andReturn(PROCESSES_2[0]);
        replay(m_sipxServiceManager);

        m_processContextImpl = new SipxProcessContextImpl();
        m_processContextImpl.setLocationsManager(m_locationsManager);
        m_processContextImpl.setSipxServiceManager(m_sipxServiceManager);
        m_processContextImpl.setAuditLogContext(new AuditLogContextImpl());
    }

    public void testGetStatus() {
        Location location = m_locationsManager.getLocations()[0];

        Map<String, String> result = new LinkedHashMap<String, String>();
        result.put("SIPRegistrar", "Starting");
        result.put("MediaServer", "Running");
        result.put("PresenceServer", "Disabled");
        result.put("SIPXProxy", "Failed");
        result.put("ACDServer", "Undefined");

        SipxServiceManager serviceManager = getMockSipxServiceManager(false, m_registrarService, m_parkService,
                m_presenceService, m_proxyService, m_acdService);

        ProcessManagerApi api = createMock(ProcessManagerApi.class);
        api.getStateAll("sipx.example.org");
        expectLastCall().andReturn(result);

        ApiProvider provider = createMock(ApiProvider.class);
        provider.getApi(location.getProcessMonitorUrl());
        expectLastCall().andReturn(api);

        m_processContextImpl.setProcessManagerApiProvider(provider);
        m_processContextImpl.setSipxServiceManager(serviceManager);
        // mark services for restart
        m_processContextImpl.markServicesForRestart(Arrays.asList(m_registrarService, m_parkService,
                m_presenceService));
        m_processContextImpl.markServicesForReload(Arrays.asList(m_registrarService));
        replay(provider, api, serviceManager);

        ServiceStatus[] resultServiceStatus = m_processContextImpl.getStatus(location, false);

        assertEquals(result.size(), resultServiceStatus.length);
        assertEquals(Starting, resultServiceStatus[0].getStatus());
        assertEquals(SipxRegistrarService.BEAN_ID, resultServiceStatus[0].getServiceBeanId());
        assertTrue(resultServiceStatus[0].isNeedsRestart());
        assertTrue(resultServiceStatus[0].isNeedsReload());

        assertEquals(Running, resultServiceStatus[1].getStatus());
        assertEquals(SipxParkService.BEAN_ID, resultServiceStatus[1].getServiceBeanId());
        assertTrue(resultServiceStatus[1].isNeedsRestart());

        assertEquals(Disabled, resultServiceStatus[2].getStatus());
        assertEquals(SipxPresenceService.BEAN_ID, resultServiceStatus[2].getServiceBeanId());
        assertFalse(resultServiceStatus[2].isNeedsRestart());

        assertEquals(Failed, resultServiceStatus[3].getStatus());
        assertEquals(SipxProxyService.BEAN_ID, resultServiceStatus[3].getServiceBeanId());
        assertFalse(resultServiceStatus[3].isNeedsRestart());

        assertEquals(Undefined, resultServiceStatus[4].getStatus());
        assertEquals(SipxAcdService.BEAN_ID, resultServiceStatus[4].getServiceBeanId());
        assertFalse(resultServiceStatus[4].isNeedsRestart());

        verify(provider, api, serviceManager);
    }

    public void testGetStatusSingle() {
        Location location = m_locationsManager.getLocations()[0];

        Map<String, String> result = new LinkedHashMap<String, String>();
        result.put("SIPRegistrar", "Starting");
        result.put("MediaServer", "Running");

        SipxServiceManager serviceManager = getMockSipxServiceManager(false, m_registrarService, m_parkService,
                m_proxyService);

        ProcessManagerApi api = createMock(ProcessManagerApi.class);
        api.getStateAll("sipx.example.org");
        expectLastCall().andReturn(result).atLeastOnce();

        ApiProvider provider = createMock(ApiProvider.class);
        provider.getApi(location.getProcessMonitorUrl());
        expectLastCall().andReturn(api).atLeastOnce();

        m_processContextImpl.setProcessManagerApiProvider(provider);
        m_processContextImpl.setSipxServiceManager(serviceManager);
        // mark services for restart
        m_processContextImpl.markServicesForRestart(Arrays.asList(m_registrarService, m_parkService,
                m_presenceService));
        replay(provider, api, serviceManager);

        assertEquals(ServiceStatus.Status.Running, m_processContextImpl.getStatus(location, m_parkService));
        assertEquals(ServiceStatus.Status.Starting, m_processContextImpl.getStatus(location, m_registrarService));
        assertEquals(ServiceStatus.Status.Undefined, m_processContextImpl.getStatus(location, m_proxyService));

        verify(provider, api, serviceManager);
    }

    public void testGetStatusForServices() {
        Location location = m_locationsManager.getLocations()[0];

        Map<String, String> result = new LinkedHashMap<String, String>();
        result.put("SIPRegistrar", "Starting");
        result.put("MediaServer", "Starting");
        result.put("PresenceServer", "Stopped");
        result.put("SIPXProxy", "Failed");
        result.put("ACDServer", "Unknown");

        SipxServiceManager serviceManager = getMockSipxServiceManager(false, m_registrarService, m_parkService,
                m_presenceService, m_proxyService, m_acdService);

        ProcessManagerApi api = createMock(ProcessManagerApi.class);
        api.getStateAll("sipx.example.org");
        expectLastCall().andReturn(result);

        ApiProvider provider = createMock(ApiProvider.class);
        provider.getApi(location.getProcessMonitorUrl());
        expectLastCall().andReturn(api);

        replay(provider, api, serviceManager);

        m_processContextImpl.setProcessManagerApiProvider(provider);
        m_processContextImpl.setSipxServiceManager(serviceManager);

        ServiceStatus[] resultServiceStatus = m_processContextImpl.getStatus(location, true);

        assertEquals(3, resultServiceStatus.length);
        assertEquals(SipxRegistrarService.BEAN_ID, resultServiceStatus[0].getServiceBeanId());
        assertEquals(Starting, resultServiceStatus[0].getStatus());
        assertEquals(SipxParkService.BEAN_ID, resultServiceStatus[1].getServiceBeanId());
        assertEquals(Starting, resultServiceStatus[1].getStatus());
        assertEquals(SipxProxyService.BEAN_ID, resultServiceStatus[2].getServiceBeanId());
        assertEquals(Failed, resultServiceStatus[2].getStatus());

        verify(provider, api, serviceManager);
    }

    public void testManageServicesSingleLocation() {
        Location location = m_locationsManager.getLocations()[0];

        ProcessManagerApi api = createStrictMock(ProcessManagerApi.class);
        api.stop(host(), asArray(PROCESSES[0].getProcessName(), PROCESSES[1].getProcessName()), block());
        expectLastCall().andReturn(null);
        api.start(host(), asArray(START_PROCESSLIST[0].getProcessName()), block());
        expectLastCall().andReturn(null);
        api.restart(host(), asArray(PROCESSES[0].getProcessName(), PROCESSES[1].getProcessName()), block());
        expectLastCall().andReturn(null);

        ApiProvider provider = createMock(ApiProvider.class);
        provider.getApi(location.getProcessMonitorUrl());
        expectLastCall().andReturn(api).times(3);

        m_processContextImpl.setProcessManagerApiProvider(provider);
        replay(provider, api);

        m_processContextImpl.manageServices(location, Arrays.asList(PROCESSES), Command.STOP);
        m_processContextImpl.manageServices(location, Arrays.asList(START_PROCESSLIST), Command.START);
        m_processContextImpl.manageServices(location, Arrays.asList(PROCESSES), Command.RESTART);
        verify(provider, api);
    }

    public void testRestartServicesMapLocation() {
        Location location1 = new Location();
        location1.setUniqueId();
        location1.setName("location1");
        location1.setFqdn("location1Fqdn");
        location1.setRegistered(true);

        Location location2 = new Location();
        location2.setUniqueId();
        location2.setName("location2");
        location2.setFqdn("location2Fqdn");
        location2.setRegistered(true);

        ProcessManagerApi api = createStrictMock(ProcessManagerApi.class);

        api.restart(host(), asArray(PROCESSES_3[0].getProcessName()), block());
        expectLastCall().andReturn(null);
        api.restart(host(), asArray(PROCESSES_2[0].getProcessName(), PROCESSES_2[1].getProcessName()), block());
        expectLastCall().andReturn(null);

        ApiProvider provider = createMock(ApiProvider.class);
        provider.getApi(location2.getProcessMonitorUrl());
        expectLastCall().andReturn(api);
        provider.getApi(location1.getProcessMonitorUrl());
        expectLastCall().andReturn(api);

        m_processContextImpl.setProcessManagerApiProvider(provider);
        replay(provider, api);

        Map<Location, List<SipxService>> servicesMap = new HashMap<Location, List<SipxService>>();
        servicesMap.put(location1, Arrays.asList(PROCESSES_2));
        servicesMap.put(location2, Arrays.asList(PROCESSES_3));
        m_processContextImpl.manageServices(servicesMap, Command.RESTART);
        verify(provider, api);
    }

    public void testManageServices() {
        ProcessManagerApi api = createStrictMock(ProcessManagerApi.class);
        api.stop(host(), asArray(PROCESSES[0].getProcessName(), PROCESSES[1].getProcessName()), block());
        expectLastCall().andReturn(null).times(m_locationsManager.getLocations().length);
        ApiProvider provider = createMock(ApiProvider.class);
        for (Location location : m_locationsManager.getLocations()) {
            provider.getApi(location.getProcessMonitorUrl());
            expectLastCall().andReturn(api);
        }

        m_processContextImpl.setProcessManagerApiProvider(provider);
        replay(provider, api);

        m_processContextImpl.manageServices(Arrays.asList(PROCESSES), Command.STOP);
        verify(provider, api);
    }

    public void testEnforceRole() {
        Map<String, String> result = new LinkedHashMap<String, String>();
        result.put("SIPRegistrar", "Running");
        result.put("SIPXProxy", "Running");
        result.put("PresenceServer", "Disabled");
        result.put("ACDServer", "Undefined");

        SipxServiceManager serviceManager = getMockSipxServiceManager(true, m_registrarService, m_parkService,
                m_presenceService, m_proxyService, m_acdService);

        ProcessManagerApi api = createMock(ProcessManagerApi.class);
        api.getStateAll("sipx.example.org");
        expectLastCall().andReturn(result);

        Location location = m_locationsManager.getLocations()[0];

        ApiProvider provider = createMock(ApiProvider.class);
        provider.getApi(location.getProcessMonitorUrl());
        expectLastCall().andReturn(api).atLeastOnce();

        replay(provider, api);

        m_processContextImpl.setProcessManagerApiProvider(provider);
        m_processContextImpl.setSipxServiceManager(serviceManager);

        location.setServiceDefinitions(Arrays.asList(m_acdService, m_proxyService));
        LocationStatus locationStatus = m_processContextImpl.getLocationStatus(location);
        Collection<SipxService> toBeStarted = locationStatus.getToBeStarted();
        assertEquals(1, toBeStarted.size());
        assertEquals(m_acdService, toBeStarted.iterator().next());

        Collection<SipxService> toBeStopped = locationStatus.getToBeStopped();
        assertEquals(1, toBeStopped.size());
        assertEquals(m_registrarService, toBeStopped.iterator().next());

        verify(provider, api, serviceManager);
    }

    public void testRestartMarked() {
        Location location = m_locationsManager.getLocations()[0];
        SipxServiceManager serviceManager = getMockSipxServiceManager(true, m_registrarService, m_parkService,
                m_presenceService, m_proxyService);

        ProcessManagerApi api = createMock(ProcessManagerApi.class);
        api.restart(host(), asArrayElems("MediaServer", "SIPXProxy"), block());
        expectLastCall().andReturn(null).once();

        ApiProvider provider = createMock(ApiProvider.class);
        provider.getApi(location.getProcessMonitorUrl());
        expectLastCall().andReturn(api).atLeastOnce();

        replay(provider, api);

        m_processContextImpl.setProcessManagerApiProvider(provider);
        m_processContextImpl.setSipxServiceManager(serviceManager);

        // no service marked
        m_processContextImpl.restartMarkedServices(location);
        m_processContextImpl.markServicesForRestart(Arrays.asList(m_proxyService, m_parkService));
        // different location
        m_processContextImpl.restartMarkedServices(m_locationsManager.getLocations()[1]);
        // this is is only call that should result in restarting services
        m_processContextImpl.restartMarkedServices(location);

        verify(provider, api);
    }

    public void testReloadMarked() {
        Location location = m_locationsManager.getLocations()[0];
        SipxServiceManager serviceManager = getMockSipxServiceManager(true, m_registrarService, m_parkService,
                m_presenceService, m_proxyService);

        ProcessManagerApi api = createMock(ProcessManagerApi.class);

        ApiProvider provider = createMock(ApiProvider.class);
        provider.getApi(location.getProcessMonitorUrl());
        expectLastCall().andReturn(api).atLeastOnce();

        replay(provider, api);

        m_processContextImpl.setProcessManagerApiProvider(provider);
        m_processContextImpl.setSipxServiceManager(serviceManager);

        // no service marked
        m_processContextImpl.reloadMarkedServices(location);
        m_processContextImpl.markServicesForReload(Arrays.asList(m_proxyService, m_parkService));
        // different location
        m_processContextImpl.reloadMarkedServices(m_locationsManager.getLocations()[1]);
        // this is is only call that should result in reloading services
        m_processContextImpl.reloadMarkedServices(location);

        verify(provider, api);
    }

    public void testIgnore() {
        Location location = m_locationsManager.getLocations()[0];
        SipxServiceManager serviceManager = getMockSipxServiceManager(true, m_registrarService, m_parkService,
                m_presenceService, m_proxyService);

        ProcessManagerApi api = createMock(ProcessManagerApi.class);
        api.restart(host(), asArrayElems("MediaServer", "SIPXProxy"), block());
        expectLastCall().andReturn(null).once();

        ApiProvider provider = createMock(ApiProvider.class);
        provider.getApi(location.getProcessMonitorUrl());
        expectLastCall().andReturn(api).atLeastOnce();

        replay(provider, api);

        m_processContextImpl.setProcessManagerApiProvider(provider);
        m_processContextImpl.setSipxServiceManager(serviceManager);

        // no service marked
        m_processContextImpl.restartMarkedServices(location);
        // mark two services for restart
        m_processContextImpl.markServicesForRestart(Arrays.asList(m_proxyService, m_parkService));
        // unmark one service
        Collection<RestartNeededService> restartNeededServices = m_processContextImpl.getRestartNeededServices();
        assertEquals(2, restartNeededServices.size());
        RestartNeededService[] servicesArray = new RestartNeededService[0];
        servicesArray = restartNeededServices.toArray(servicesArray);
        List<RestartNeededService> newServiceList = new ArrayList<RestartNeededService>();
        newServiceList.add(servicesArray[1]);
        m_processContextImpl.unmarkServicesToRestart(newServiceList);
        // check for only one service remaining to be restarted
        restartNeededServices = m_processContextImpl.getRestartNeededServices();
        assertEquals(1, restartNeededServices.size());
    }

    public void testNotifyOnManageService() {
        Location location = m_locationsManager.getLocations()[0];
        ApiProvider<ProcessManagerApi> apiProvider = createMock(ApiProvider.class);
        ProcessManagerApi api = createStrictMock(ProcessManagerApi.class);
        api.stop(host(), asArray("service1", "service2"), block());
        expectLastCall().andReturn(null);
        api.start(host(), asArray("service1", "service2"), block());
        expectLastCall().andReturn(null);
        api.restart(host(), asArray("service1", "service2"), block());
        expectLastCall().andReturn(null);

        apiProvider.getApi(location.getProcessMonitorUrl());
        expectLastCall().andReturn(api).times(3);

        m_processContextImpl.setProcessManagerApiProvider(apiProvider);
        replay(apiProvider, api);

        Collection<SipxService> services = new ArrayList<SipxService>();
        MockSipxService service1 = new MockSipxService();
        service1.setProcessName("service1");

        MockSipxService service2 = new MockSipxService();
        service2.setProcessName("service2");

        services.add(service1);
        services.add(service2);
        m_processContextImpl.manageServices(location, services, Command.STOP);

        assertEquals("stop", service1.getTestString());
        assertEquals("stop", service2.getTestString());

        m_processContextImpl.manageServices(location, services, Command.START);
        assertEquals("start", service1.getTestString());
        assertEquals("start", service2.getTestString());

        m_processContextImpl.manageServices(location, services, Command.RESTART);
        assertEquals("restart", service1.getTestString());
        assertEquals("restart", service2.getTestString());

    }

    private class MockSipxService extends SipxService {
        private String m_test = "dummy";

        public MockSipxService() {
        }

        @Override
        public void onStart() {
            m_test = "start";
        }

        @Override
        public void onStop() {
            m_test = "stop";
        }

        @Override
        public void onRestart() {
            m_test = "restart";
        }

        public String getTestString() {
            return m_test;
        }
    }

    static String host() {
        return eq("sipx.example.org");
    }

    static boolean block() {
        return eq(true);
    }
}

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
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.service.SipxAcdService;
import org.sipfoundry.sipxconfig.service.SipxMediaService;
import org.sipfoundry.sipxconfig.service.SipxPresenceService;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceBundle;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.test.TestUtil;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;

import junit.framework.TestCase;

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
    private Set<SipxServiceBundle> m_bundleSet = new HashSet<SipxServiceBundle>();
    SipxRegistrarService m_registrarService;
    SipxMediaService m_mediaService;
    SipxService m_proxyService;
    SipxPresenceService m_presenceService;
    SipxAcdService m_acdService;

    private final static SipxService[] PROCESSES = new SipxService[] {
        new SipxRegistrarService(), new SipxMediaService()
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

        m_mediaService = new SipxMediaService();
        m_mediaService.setBeanName(SipxMediaService.BEAN_ID);
        m_mediaService.setProcessName("MediaServer");
        m_mediaService.setBundles(m_bundleSet);

        m_presenceService = new SipxPresenceService();
        m_presenceService.setBeanName(SipxPresenceService.BEAN_ID);
        m_presenceService.setProcessName("PresenceServer");
        m_presenceService.setBundles(m_bundleSet);

        m_acdService = new SipxAcdService();
        m_acdService.setBeanName(SipxAcdService.BEAN_ID);
        m_acdService.setProcessName("ACDServer");

        location.setServiceDefinitions(Arrays.asList(m_registrarService, m_proxyService, m_mediaService));
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

        m_processContextImpl = new SipxProcessContextImpl();
        m_processContextImpl.setLocationsManager(m_locationsManager);
    }

    public void testGetStatus() {
        Location location = m_locationsManager.getLocations()[0];

        Map<String, String> result = new LinkedHashMap<String, String>();
        result.put("SIPRegistrar", "Starting");
        result.put("MediaServer", "Running");
        result.put("PresenceServer", "Disabled");
        result.put("SIPXProxy", "Failed");
        result.put("ACDServer", "Undefined");

        SipxServiceManager serviceManager = getMockSipxServiceManager(false, m_registrarService, m_mediaService,
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
        m_processContextImpl.markServicesForRestart(Arrays.asList(m_registrarService, m_mediaService,
                m_presenceService));
        replay(provider, api, serviceManager);

        ServiceStatus[] resultServiceStatus = m_processContextImpl.getStatus(location, false);

        assertEquals(result.size(), resultServiceStatus.length);
        assertEquals(Starting, resultServiceStatus[0].getStatus());
        assertEquals(SipxRegistrarService.BEAN_ID, resultServiceStatus[0].getServiceBeanId());
        assertTrue(resultServiceStatus[0].isNeedsRestart());

        assertEquals(Running, resultServiceStatus[1].getStatus());
        assertEquals(SipxMediaService.BEAN_ID, resultServiceStatus[1].getServiceBeanId());
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

    public void testGetStatusForServices() {
        Location location = m_locationsManager.getLocations()[0];

        Map<String, String> result = new LinkedHashMap<String, String>();
        result.put("SIPRegistrar", "Starting");
        result.put("MediaServer", "Starting");
        result.put("PresenceServer", "Stopped");
        result.put("SIPXProxy", "Failed");
        result.put("ACDServer", "Unknown");

        SipxServiceManager serviceManager = getMockSipxServiceManager(false, m_registrarService, m_mediaService,
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
        assertEquals(SipxMediaService.BEAN_ID, resultServiceStatus[1].getServiceBeanId());
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

        SipxServiceManager serviceManager = getMockSipxServiceManager(true, m_registrarService, m_mediaService,
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
        SipxServiceManager serviceManager = getMockSipxServiceManager(true, m_registrarService, m_mediaService,
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
        m_processContextImpl.markServicesForRestart(Arrays.asList(m_proxyService, m_mediaService));
        // different location
        m_processContextImpl.restartMarkedServices(m_locationsManager.getLocations()[1]);
        // this is is only call that should result in restarting services
        m_processContextImpl.restartMarkedServices(location);

        verify(provider, api);
    }

    static String host() {
        return eq("sipx.example.org");
    }

    static boolean block() {
        return eq(true);
    }
}

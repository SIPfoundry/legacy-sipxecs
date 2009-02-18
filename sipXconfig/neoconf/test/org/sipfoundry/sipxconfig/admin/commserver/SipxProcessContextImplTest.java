/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Arrays;
import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.service.SipxAcdService;
import org.sipfoundry.sipxconfig.service.SipxMediaService;
import org.sipfoundry.sipxconfig.service.SipxPresenceService;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;

import static org.easymock.EasyMock.aryEq;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.createStrictMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status.Disabled;
import static org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status.Failed;
import static org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status.Running;
import static org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status.Starting;
import static org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status.Undefined;
import static org.sipfoundry.sipxconfig.test.TestUtil.getMockSipxServiceManager;

public class SipxProcessContextImplTest extends TestCase {
    private SipxProcessContextImpl m_processContextImpl;
    private LocationsManager m_locationsManager;

    private final static SipxService[] PROCESSES = new SipxService[] {
        new SipxRegistrarService(), new SipxMediaService()
    };

    private final static SipxService[] START_PROCESSLIST = new SipxService[] {
        new SipxPresenceService(),
    };

    @Override
    protected void setUp() throws Exception {
        m_locationsManager = EasyMock.createNiceMock(LocationsManager.class);
        Location location = new Location();
        SipxService registrarService = new SipxRegistrarService();
        registrarService.setProcessName("SIPRegistrar");
        registrarService.setBeanName(SipxRegistrarService.BEAN_ID);
        SipxService proxyService = new SipxProxyService();
        proxyService.setProcessName("SIPXProxy");
        proxyService.setBeanName(SipxProxyService.BEAN_ID);
        location.setServiceDefinitions(Arrays.asList(registrarService, proxyService));
        m_locationsManager.getLocations();
        EasyMock.expectLastCall().andReturn(new Location[] {
            location
        }).anyTimes();
        EasyMock.replay(m_locationsManager);

        m_processContextImpl = new SipxProcessContextImpl();
        m_processContextImpl.setLocationsManager(m_locationsManager);

        m_processContextImpl.setHost("localhost");
    }

    public void testGetStatus() {
        Location location = m_locationsManager.getLocations()[0];

        Map<String, String> result = new LinkedHashMap<String, String>();
        result.put("SIPRegistrar", "Starting");
        result.put("MediaServer", "Running");
        result.put("PresenceServer", "Disabled");
        result.put("SIPXProxy", "Failed");
        result.put("ACDServer", "Undefined");

        SipxRegistrarService registrar = new SipxRegistrarService();
        registrar.setBeanName(SipxRegistrarService.BEAN_ID);
        registrar.setProcessName("SIPRegistrar");
        SipxRegistrarService media = new SipxRegistrarService();
        media.setBeanName(SipxMediaService.BEAN_ID);
        media.setProcessName("MediaServer");
        SipxPresenceService presence = new SipxPresenceService();
        presence.setBeanName(SipxPresenceService.BEAN_ID);
        presence.setProcessName("PresenceServer");
        SipxProxyService proxy = new SipxProxyService();
        proxy.setBeanName(SipxProxyService.BEAN_ID);
        proxy.setProcessName("SIPXProxy");
        SipxAcdService acd = new SipxAcdService();
        acd.setBeanName(SipxAcdService.BEAN_ID);
        acd.setProcessName("ACDServer");

        SipxServiceManager serviceManager = getMockSipxServiceManager(false, registrar, media, presence, proxy, acd);

        ProcessManagerApi api = createMock(ProcessManagerApi.class);
        api.getStateAll("localhost");
        expectLastCall().andReturn(result);

        ApiProvider provider = createMock(ApiProvider.class);
        provider.getApi(location.getProcessMonitorUrl());
        expectLastCall().andReturn(api);

        m_processContextImpl.setProcessManagerApiProvider(provider);
        m_processContextImpl.setSipxServiceManager(serviceManager);
        replay(provider, api, serviceManager);

        ServiceStatus[] resultServiceStatus = m_processContextImpl.getStatus(location, false);

        assertEquals(result.size(), resultServiceStatus.length);
        assertEquals(Starting, resultServiceStatus[0].getStatus());
        assertEquals(SipxRegistrarService.BEAN_ID, resultServiceStatus[0].getServiceBeanId());
        assertEquals(Running, resultServiceStatus[1].getStatus());
        assertEquals(SipxMediaService.BEAN_ID, resultServiceStatus[1].getServiceBeanId());
        assertEquals(Disabled, resultServiceStatus[2].getStatus());
        assertEquals(SipxPresenceService.BEAN_ID, resultServiceStatus[2].getServiceBeanId());
        assertEquals(Failed, resultServiceStatus[3].getStatus());
        assertEquals(SipxProxyService.BEAN_ID, resultServiceStatus[3].getServiceBeanId());
        assertEquals(Undefined, resultServiceStatus[4].getStatus());
        assertEquals(SipxAcdService.BEAN_ID, resultServiceStatus[4].getServiceBeanId());

        verify(provider, api, serviceManager);
    }

    public void testGetStatusForServices() {
        Location location = m_locationsManager.getLocations()[0];

        Map<String, String> result = new LinkedHashMap<String, String>();
        result.put("SIPRegistrar", "Starting");
        result.put("MediaServer", "Started");
        result.put("PresenceServer", "Stopped");
        result.put("SIPXProxy", "Failed");
        result.put("ACDServer", "Unknown");

        SipxRegistrarService registrar = new SipxRegistrarService();
        registrar.setBeanName(SipxRegistrarService.BEAN_ID);
        registrar.setProcessName("SIPRegistrar");
        SipxRegistrarService media = new SipxRegistrarService();
        media.setBeanName(SipxMediaService.BEAN_ID);
        media.setProcessName("MediaServer");
        SipxPresenceService presence = new SipxPresenceService();
        presence.setBeanName(SipxPresenceService.BEAN_ID);
        presence.setProcessName("PresenceServer");
        SipxProxyService proxy = new SipxProxyService();
        proxy.setBeanName(SipxProxyService.BEAN_ID);
        proxy.setProcessName("SIPXProxy");
        SipxAcdService acd = new SipxAcdService();
        acd.setBeanName(SipxAcdService.BEAN_ID);
        acd.setProcessName("ACDServer");

        SipxServiceManager serviceManager = getMockSipxServiceManager(false, registrar, media, presence, proxy, acd);

        ProcessManagerApi api = createMock(ProcessManagerApi.class);
        api.getStateAll("localhost");
        expectLastCall().andReturn(result);

        ApiProvider provider = createMock(ApiProvider.class);
        provider.getApi(location.getProcessMonitorUrl());
        expectLastCall().andReturn(api);

        replay(provider, api, serviceManager);

        m_processContextImpl.setProcessManagerApiProvider(provider);
        m_processContextImpl.setSipxServiceManager(serviceManager);

        ServiceStatus[] resultServiceStatus = m_processContextImpl.getStatus(location, true);

        assertEquals(2, resultServiceStatus.length);
        assertEquals(SipxRegistrarService.BEAN_ID, resultServiceStatus[0].getServiceBeanId());
        assertEquals(Starting, resultServiceStatus[0].getStatus());
        assertEquals(SipxProxyService.BEAN_ID, resultServiceStatus[1].getServiceBeanId());
        assertEquals(Failed, resultServiceStatus[1].getStatus());

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

        SipxRegistrarService registrar = new SipxRegistrarService();
        registrar.setBeanName(SipxRegistrarService.BEAN_ID);
        registrar.setProcessName("SIPRegistrar");
        SipxRegistrarService media = new SipxRegistrarService();
        media.setBeanName(SipxMediaService.BEAN_ID);
        media.setProcessName("MediaServer");
        SipxPresenceService presence = new SipxPresenceService();
        presence.setBeanName(SipxPresenceService.BEAN_ID);
        presence.setProcessName("PresenceServer");
        SipxProxyService proxy = new SipxProxyService();
        proxy.setBeanName(SipxProxyService.BEAN_ID);
        proxy.setProcessName("SIPXProxy");
        SipxAcdService acd = new SipxAcdService();
        acd.setBeanName(SipxAcdService.BEAN_ID);
        acd.setProcessName("ACDServer");

        SipxServiceManager serviceManager = getMockSipxServiceManager(true, registrar, media, presence, proxy, acd);

        ProcessManagerApi api = createMock(ProcessManagerApi.class);
        api.getStateAll("localhost");
        expectLastCall().andReturn(result);

        Location location = m_locationsManager.getLocations()[0];

        ApiProvider provider = createMock(ApiProvider.class);
        provider.getApi(location.getProcessMonitorUrl());
        expectLastCall().andReturn(api).atLeastOnce();

        replay(provider, api);

        m_processContextImpl.setProcessManagerApiProvider(provider);
        m_processContextImpl.setSipxServiceManager(serviceManager);

        location.setServiceDefinitions(Arrays.asList(acd, proxy));
        LocationStatus locationStatus = m_processContextImpl.getLocationStatus(location);
        Collection<SipxService> toBeStarted = locationStatus.getToBeStarted();
        assertEquals(1, toBeStarted.size());
        assertEquals(acd, toBeStarted.iterator().next());

        Collection<SipxService> toBeStopped = locationStatus.getToBeStopped();
        assertEquals(1, toBeStopped.size());
        assertEquals(registrar, toBeStopped.iterator().next());

        verify(provider, api, serviceManager);
    }

    static <T> T[] asArray(T... items) {
        return aryEq(items);
    }

    static String host() {
        return eq("localhost");
    }

    static boolean block() {
        return eq(true);
    }
}

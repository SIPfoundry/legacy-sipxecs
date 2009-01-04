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
        registrarService.setModelId(SipxRegistrarService.BEAN_ID);
        SipxService proxyService = new SipxProxyService();
        proxyService.setProcessName("SIPXProxy");
        proxyService.setModelId(SipxProxyService.BEAN_ID);
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

        SipxServiceManager serviceManager = createMock(SipxServiceManager.class);
        serviceManager.getServiceByName("SIPRegistrar");
        SipxRegistrarService registrar = new SipxRegistrarService();
        registrar.setModelId(SipxRegistrarService.BEAN_ID);
        expectLastCall().andReturn(registrar);
        serviceManager.getServiceByName("MediaServer");
        SipxRegistrarService media = new SipxRegistrarService();
        media.setModelId(SipxMediaService.BEAN_ID);
        expectLastCall().andReturn(media);
        serviceManager.getServiceByName("PresenceServer");
        SipxPresenceService presence = new SipxPresenceService();
        presence.setBeanId(SipxPresenceService.BEAN_ID);
        expectLastCall().andReturn(presence);
        serviceManager.getServiceByName("SIPXProxy");
        SipxProxyService proxy = new SipxProxyService();
        proxy.setBeanId(SipxProxyService.BEAN_ID);
        expectLastCall().andReturn(proxy);
        serviceManager.getServiceByName("ACDServer");
        SipxAcdService acd = new SipxAcdService();
        acd.setBeanId(SipxAcdService.BEAN_ID);
        expectLastCall().andReturn(acd);

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

        SipxServiceManager serviceManager = createMock(SipxServiceManager.class);
        serviceManager.getServiceByName("SIPRegistrar");
        SipxRegistrarService registrar = new SipxRegistrarService();
        registrar.setModelId(SipxRegistrarService.BEAN_ID);
        expectLastCall().andReturn(registrar);
        serviceManager.getServiceByName("MediaServer");
        SipxRegistrarService media = new SipxRegistrarService();
        media.setModelId(SipxMediaService.BEAN_ID);
        expectLastCall().andReturn(media);
        serviceManager.getServiceByName("PresenceServer");
        SipxPresenceService presence = new SipxPresenceService();
        presence.setBeanId(SipxPresenceService.BEAN_ID);
        expectLastCall().andReturn(presence);
        serviceManager.getServiceByName("SIPXProxy");
        SipxProxyService proxy = new SipxProxyService();
        proxy.setBeanId(SipxProxyService.BEAN_ID);
        expectLastCall().andReturn(proxy);
        serviceManager.getServiceByName("ACDServer");
        SipxAcdService acd = new SipxAcdService();
        acd.setBeanId(SipxAcdService.BEAN_ID);
        expectLastCall().andReturn(acd);

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

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

import static org.easymock.EasyMock.aryEq;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.createStrictMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;

public class SipxProcessContextImplTest extends TestCase {
    private SipxProcessContextImpl m_processContextImpl;
    private LocationsManager m_locationsManager;

    private final static Process[] PROCESSES = new Process[] {
        new Process(ProcessName.REGISTRAR), new Process(ProcessName.MEDIA_SERVER)
    };

    private final static Process[] START_PROCESSLIST = new Process[] {
        new Process(ProcessName.PRESENCE_SERVER),
    };

    protected void setUp() throws Exception {
        m_locationsManager = EasyMock.createNiceMock(LocationsManager.class);
        Location location = new Location();
        location.setSipxServices(Arrays.asList(new SipxService[] {
                new SipxRegistrarService(), new SipxProxyService()
        }));
        m_locationsManager.getLocations();
        EasyMock.expectLastCall().andReturn(new Location[] {location}).anyTimes();
        EasyMock.replay(m_locationsManager);
        
        m_processContextImpl = new SipxProcessContextImpl();
        m_processContextImpl.setLocationsManager(m_locationsManager);
        m_processContextImpl.setProcessModel(new SimpleSipxProcessModel());
        m_processContextImpl.setHost("localhost");
    }

    public void testGetStatus() {
        Location location = m_locationsManager.getLocations()[0];

        Map<String, String> result = new HashMap<String, String>();
        result.put(ProcessName.REGISTRAR.getName(), "Starting");
        result.put(ProcessName.MEDIA_SERVER.getName(), "Running");
        result.put(ProcessName.PRESENCE_SERVER.getName(), "Disabled");
        result.put(ProcessName.PROXY.getName(), "Failed");
        result.put(ProcessName.ACD_SERVER.getName(), "Undefined");

        ProcessManagerApi api = createMock(ProcessManagerApi.class);
        api.getStateAll("localhost");
        expectLastCall().andReturn(result);

        ApiProvider provider = createMock(ApiProvider.class);
        provider.getApi(location.getProcessMonitorUrl());
        expectLastCall().andReturn(api);

        m_processContextImpl.setProcessManagerApiProvider(provider);
        replay(provider, api);

        ServiceStatus[] resultServiceStatus = m_processContextImpl.getStatus(location, false);

        assertEquals(result.size(), resultServiceStatus.length);
        for (ServiceStatus serviceStatus : resultServiceStatus) {
            String expected = result.get(serviceStatus.getServiceName());
            assertEquals(expected, serviceStatus.getStatus().toString());
        }
        verify(provider, api);
    }

    public void testGetStatusForServices() {
        Location location = m_locationsManager.getLocations()[0];

        Map<String, String> result = new HashMap<String, String>();
        result.put(ProcessName.REGISTRAR.getName(), "Starting");
        result.put(ProcessName.MEDIA_SERVER.getName(), "Started");
        result.put(ProcessName.PRESENCE_SERVER.getName(), "Stopped");
        result.put(ProcessName.PROXY.getName(), "Failed");
        result.put(ProcessName.ACD_SERVER.getName(), "Unknown");

        ProcessManagerApi api = createMock(ProcessManagerApi.class);
        api.getStateAll("localhost");
        expectLastCall().andReturn(result);

        ApiProvider provider = createMock(ApiProvider.class);
        provider.getApi(location.getProcessMonitorUrl());
        expectLastCall().andReturn(api);

        m_processContextImpl.setProcessManagerApiProvider(provider);
        replay(provider, api);

        ServiceStatus[] resultServiceStatus = m_processContextImpl.getStatus(location, true);

        Map<String, String> expectedServices= new HashMap();
        expectedServices.put(ProcessName.REGISTRAR.getName(), "Starting");
        expectedServices.put(ProcessName.PROXY.getName(), "Failed");
        
        assertEquals(expectedServices.keySet().size(), resultServiceStatus.length);
        for (ServiceStatus serviceStatus : resultServiceStatus) {
            if (! expectedServices.containsKey(serviceStatus.getServiceName())) {
                fail("Expected service not in result list");
            }
            String expectedStatus = expectedServices.get(serviceStatus.getServiceName());
            assertEquals(expectedStatus, serviceStatus.getStatus().toString());
        }
        
        verify(provider, api);
    }
    
    public void testManageServicesSingleLocation() {
        Location location = m_locationsManager.getLocations()[0];

        ProcessManagerApi api = createStrictMock(ProcessManagerApi.class);
        api.stop(host(), asArray(PROCESSES[0].getName(), PROCESSES[1].getName()), block());
        expectLastCall().andReturn(null);
        api.start(host(), asArray(START_PROCESSLIST[0].getName()), block());
        expectLastCall().andReturn(null);
        api.restart(host(), asArray(PROCESSES[0].getName(), PROCESSES[1].getName()), block());
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
        api.stop(host(), asArray(PROCESSES[0].getName(), PROCESSES[1].getName()), block());
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

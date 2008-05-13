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

import java.io.InputStream;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;

import static org.easymock.EasyMock.aryEq;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.createStrictMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

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
        m_locationsManager = new LocationsManagerImpl() {
            protected InputStream getTopologyAsStream() {
                return LocationsManagerImplTest.class.getResourceAsStream("topology.test.xml");
            }
        };

        m_processContextImpl = new SipxProcessContextImpl();
        m_processContextImpl.setLocationsManager(m_locationsManager);
        m_processContextImpl.setProcessModel(new SimpleSipxProcessModel());
        m_processContextImpl.setHost("localhost");
    }

    public void testGetStatus() {
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

        ProcessManagerApiProvider provider = createMock(ProcessManagerApiProvider.class);
        provider.getApi(location);
        expectLastCall().andReturn(api);

        m_processContextImpl.setProcessManagerApiProvider(provider);
        replay(provider, api);

        ServiceStatus[] resultServiceStatus = m_processContextImpl.getStatus(location);

        assertEquals(result.size(), resultServiceStatus.length);
        for (ServiceStatus serviceStatus : resultServiceStatus) {
            String expected = result.get(serviceStatus.getServiceName());
            assertEquals(expected, serviceStatus.getStatus().getName());
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

        ProcessManagerApiProvider provider = createMock(ProcessManagerApiProvider.class);
        provider.getApi(location);
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

        ProcessManagerApiProvider provider = createMock(ProcessManagerApiProvider.class);
        for (Location location : m_locationsManager.getLocations()) {
            provider.getApi(location);
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

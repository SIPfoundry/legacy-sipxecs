/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;

import java.util.HashMap;
import java.util.Map;

import junit.framework.JUnit4TestAdapter;

import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;

public class BridgeSbcStatisticsTest {
    private BridgeSbc m_sbc;
    private BridgeSbcStatistics m_sbcStats;

    public static junit.framework.Test suite() {
        return new JUnit4TestAdapter(BridgeSbcStatisticsTest.class);
    }

    @Before
    public void setUp() throws Exception {
        ModelFilesContext modelFilesContext = TestHelper.getModelFilesContext();
        Location location = new Location();
        location.setUniqueId();
        location.setAddress("98.65.1.5");
        location.setFqdn("sipx.example.org");

        m_sbc = new BridgeSbc() {
            @Override
            public boolean isBridgeSbcRunning() {
                return true;
            }
        };
        m_sbc.setLocation(location);
        m_sbc.setModelFilesContext(modelFilesContext);
        m_sbc.setSettingValue("bridge-configuration/xml-rpc-port", "8888");

        Integer callCountResult = new Integer(10);

        Map<String, String> registrationMap = new HashMap<String, String>();
        registrationMap.put("47.123.2.34", "INIT");
        registrationMap.put("47.123.2.35", "INIT");
        registrationMap.put("47.123.2.36", "INIT");

        final BridgeSbcXmlRpcApi bridgeSbcApiProvider = createMock(BridgeSbcXmlRpcApi.class);
        bridgeSbcApiProvider.getCallCount();
        expectLastCall().andReturn(callCountResult);
        bridgeSbcApiProvider.getRegistrationStatus();
        expectLastCall().andReturn(registrationMap);
        replay(bridgeSbcApiProvider);

        ApiProvider<BridgeSbcXmlRpcApi> provider = new ApiProvider<BridgeSbcXmlRpcApi>() {
            public BridgeSbcXmlRpcApi getApi(String serviceUrl) {
                return bridgeSbcApiProvider;
            }
        };

        m_sbcStats = new BridgeSbcStatistics();
        m_sbcStats.setBridgeSbcApiProvider(provider);
    }

    @Test
    public void testGetCallCount() throws Exception {
        int callCount = m_sbcStats.getCallCount(m_sbc);
        assertEquals(10, callCount);
    }

    @Test
    public void testGetRegistrationRecords() throws Exception {
        BridgeSbcRegistrationRecord[] bridgeSbcRegistrationRecords = m_sbcStats.
            getRegistrationRecords(m_sbc);
        assertEquals(3, bridgeSbcRegistrationRecords.length);
        assertEquals("47.123.2.36", bridgeSbcRegistrationRecords[0].getRegisteredAddress());
        assertEquals("47.123.2.34", bridgeSbcRegistrationRecords[1].getRegisteredAddress());
        assertEquals("47.123.2.35", bridgeSbcRegistrationRecords[2].getRegisteredAddress());
        assertEquals("INIT", bridgeSbcRegistrationRecords[0].getRegistrationStatus());
        assertEquals("INIT", bridgeSbcRegistrationRecords[0].getRegistrationStatus());
        assertEquals("INIT", bridgeSbcRegistrationRecords[0].getRegistrationStatus());
    }
}

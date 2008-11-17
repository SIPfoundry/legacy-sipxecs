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

import java.io.InputStream;
import java.util.Arrays;

import junit.framework.JUnit4TestAdapter;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.SipTrunk;
import org.sipfoundry.sipxconfig.gateway.Gateway.AddressTransport;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

public class BridgeSbcTest {
    private BridgeSbc m_sbc;
    private MemoryProfileLocation m_location;

    public static junit.framework.Test suite() {
        return new JUnit4TestAdapter(BridgeSbcTest.class);
    }

    @Before
    public void setUp() throws Exception {
        ModelFilesContext modelFilesContext = TestHelper.getModelFilesContext();
        DeviceDefaults deviceDefaults = PhoneTestDriver.getDeviceDefaults();

        m_sbc = new BridgeSbc();
        m_location = TestHelper.setVelocityProfileGenerator(m_sbc);

        m_sbc.setDefaults(deviceDefaults);
        m_sbc.setModelFilesContext(modelFilesContext);

        m_sbc.setSerialNumber("001122334455");

        m_sbc.setAddress("192.168.5.240");
        m_sbc.setPort(5090);
        m_sbc.setSettingValue("bridge-configuration/global-address", "98.65.1.5");
        m_sbc.setSettingValue("bridge-configuration/global-port", "5060");
        m_sbc.setSettingValue("bridge-configuration/external-address", "10.1.1.5");
        m_sbc.setSettingValue("bridge-configuration/external-port", "5080");
        m_sbc.setSettingValue("bridge-configuration/log-level", "INFO");
        m_sbc.setSettingValue("bridge-configuration/route-inbound-calls-to-extension", "operator");
        m_sbc.setSettingValue("bridge-configuration/log-directory", "/var/log/sipxpbx/");

        SipTrunk sipTrunk = new SipTrunk();
        sipTrunk.setDefaults(deviceDefaults);
        sipTrunk.setModelFilesContext(modelFilesContext);
        sipTrunk.setSbcDevice(m_sbc);
        sipTrunk.setAddress("itsp.example.com");
        sipTrunk.setAddressPort(5061);
        sipTrunk.setAddressTransport(AddressTransport.UDP);
        sipTrunk.setSettingValue("itsp-account/user-name", "juser");
        sipTrunk.setSettingValue("itsp-account/password", "1234");

        GatewayContext gatewayContext = createMock(GatewayContext.class);
        gatewayContext.getGatewayByType(SipTrunk.class);
        expectLastCall().andReturn(Arrays.asList(sipTrunk));
        replay(gatewayContext);

        m_sbc.setGatewayContext(gatewayContext);
    }

    @Test
    public void testGenerateConfig() throws Exception {
        Profile[] profileTypes = m_sbc.getProfileTypes();
        assertEquals(1, profileTypes.length);

        Profile profile = profileTypes[0];
        profile.generate(m_sbc, m_location);

        String actual = m_location.toString();
        InputStream expectedConfig = getClass().getResourceAsStream("sipxbridge.xml");
        assertNotNull(expectedConfig);
        String expected = IOUtils.toString(expectedConfig);
        assertEquals(expected, actual);
    }
}

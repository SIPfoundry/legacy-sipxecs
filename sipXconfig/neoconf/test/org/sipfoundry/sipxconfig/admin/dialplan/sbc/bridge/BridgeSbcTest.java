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
    private SipTrunk m_sipTrunk;
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

        m_sipTrunk = new SipTrunk();
        m_sipTrunk.setDefaults(deviceDefaults);
        m_sipTrunk.setModelFilesContext(modelFilesContext);
        m_sipTrunk.setSbcDevice(m_sbc);
        m_sipTrunk.setAddress("itsp.example.com");
        m_sipTrunk.setAddressPort(5061);
        m_sipTrunk.setAddressTransport(AddressTransport.UDP);
        m_sipTrunk.setSettingValue("itsp-account/user-name", "juser");
        m_sipTrunk.setSettingValue("itsp-account/password", "1234");
        m_sipTrunk.setSettingValue("itsp-account/itsp-registrar-address", "10.1.1.1");
        m_sipTrunk.setSettingValue("itsp-account/itsp-registrar-listening-port", "5071");

        GatewayContext gatewayContext = createMock(GatewayContext.class);
        gatewayContext.getGatewayByType(SipTrunk.class);
        expectLastCall().andReturn(Arrays.asList(m_sipTrunk));
        replay(gatewayContext);

        m_sbc.setGatewayContext(gatewayContext);
    }

    @Test
    public void testGenerateConfig() throws Exception {
        // Use default asserted identity
        m_sipTrunk.setSettingValue("itsp-account/default-asserted-identity", "true");
        m_sipTrunk.setSettingValue("itsp-account/is-user-phone", "true");

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

    @Test
    public void testOverrideDefaultAssertedIdentity() throws Exception {
        // Override the default asserted identity
        m_sipTrunk.setSettingValue("itsp-account/default-asserted-identity", "false");
        m_sipTrunk.setSettingValue("itsp-account/asserted-identity", "juser@itsp.example.com");
        m_sipTrunk.setSettingValue("itsp-account/is-user-phone", "false");

        Profile[] profileTypes = m_sbc.getProfileTypes();
        assertEquals(1, profileTypes.length);

        Profile profile = profileTypes[0];
        profile.generate(m_sbc, m_location);

        String actual = m_location.toString();
        InputStream expectedConfig = getClass().getResourceAsStream("sipxbridge.test.xml");
        assertNotNull(expectedConfig);
        String expected = IOUtils.toString(expectedConfig);
        assertEquals(expected, actual);
    }
}

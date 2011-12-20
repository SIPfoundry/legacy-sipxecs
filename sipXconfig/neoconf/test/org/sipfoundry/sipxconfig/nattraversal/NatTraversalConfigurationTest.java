/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.nattraversal;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.sbc.SbcRoutes;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class NatTraversalConfigurationTest {
    private SbcRoutes m_routes;
    private NatConfiguration m_config;
    private NatSettings m_settings;
    private Location m_location;

    @Before
    public void setUp() throws Exception {
        m_routes = new SbcRoutes();
        m_routes.setDomains(Arrays.asList("*.example.org"));
        m_routes.setSubnets(Arrays.asList("10.0.0.0/22", "172.16.0.0/12", "192.168.0.0/16"));

        m_location = TestHelper.createDefaultLocation();
        m_location.setStartRtpPort(30000);
        m_location.setStopRtpPort(31000);

        m_config = new NatConfiguration();
        m_settings = new NatSettings();
        m_settings.setModelFilesContext(TestHelper.getModelFilesContext());
        m_config.setVelocityEngine(TestHelper.getVelocityEngine());        
    }

    @Test
    public void testGenerateNoBehindNat() throws Exception {
        m_settings.setSettingTypedValue("nat/behind-nat", false);        
        m_location.setUseStun(true);
        assertEquals(expected("nattraversalrules.test.no-behind-nat.xml"), actual());
    }

    @Test
    public void testGenerateNoPublicAddress() throws Exception {
        m_settings.setSettingTypedValue("nat/behind-nat", true);
        m_location.setUseStun(true);
        m_location.setPublicAddress(null);
        assertEquals(expected("nattraversalrules.test.no-public-address.xml"), actual());
    }

    @Test
    public void testGenerateNoStunServer() throws Exception {
        m_settings.setSettingTypedValue("nat/behind-nat", true);
        m_location.setUseStun(false);
        m_location.setPublicAddress("1.2.3.4");
        m_location.setStunAddress(null);
        assertEquals(expected("nattraversalrules.test.no-stun.xml"), actual());
    }
    
    @Test
    public void testGenerateNoRejectStrayPackets() throws Exception {
        m_settings.setSettingTypedValue("nat/behind-nat", true);
        m_location.setUseStun(false);
        m_location.setPublicAddress("1.2.3.4");
        m_location.setStunAddress(null);
        m_settings.setSettingValue("nat/reject-stray-packets", "false");
        assertEquals(expected("nattraversalrules.test.no-reject-stray-packets.xml"), actual());
    }

    String expected(String name) throws IOException {
        return IOUtils.toString(getClass().getResourceAsStream(name));
    }
    
    String actual() throws IOException {
        StringWriter actual = new StringWriter();
        m_config.write(actual, m_settings, m_location, m_routes, 5060, 5061);
        return actual.toString();        
    }
}

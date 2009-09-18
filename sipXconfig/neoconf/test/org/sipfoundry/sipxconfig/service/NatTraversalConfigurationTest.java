/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.List;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcRoutes;
import org.sipfoundry.sipxconfig.nattraversal.NatLocation;
import org.sipfoundry.sipxconfig.test.TestUtil;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

public class NatTraversalConfigurationTest extends TestCase {
    private SbcManager m_sbcManager;
    private NatLocation m_natLocation;
    private LocationsManager m_locationsManager;
    private SipxServiceManager m_sipxServiceManager;
    private SipxRelayService m_sipxRelayService;

    @Override
    protected void setUp() throws Exception {
        SbcRoutes routes = configureSbcRoutes(Arrays.asList("*.example.org"), Arrays.asList("10.0.0.0/22",
                "172.16.0.0/12", "192.168.0.0/16"));

        m_sbcManager = createNiceMock(SbcManager.class);
        m_sbcManager.getRoutes();
        expectLastCall().andReturn(routes);

        m_locationsManager = createMock(LocationsManager.class);
        m_locationsManager.getPrimaryLocation();
        Location location = TestUtil.createDefaultLocation();
        expectLastCall().andReturn(location).anyTimes();

        replay(m_sbcManager, m_locationsManager);

        m_natLocation = new NatLocation();
        m_natLocation.setStartRtpPort(30000);
        m_natLocation.setStopRtpPort(31000);

        location.setNat(m_natLocation);

        SipxProxyService sipxProxyService = new SipxProxyService();
        sipxProxyService.setBeanName(SipxProxyService.BEAN_ID);
        sipxProxyService.setModelDir("sipxproxy");
        sipxProxyService.setModelName("sipxproxy.xml");
        sipxProxyService.setModelFilesContext(TestHelper.getModelFilesContext());
        sipxProxyService.setSipPort("5060");

        m_sipxRelayService = new SipxRelayService();
        m_sipxRelayService.setBeanName(SipxRelayService.BEAN_ID);
        m_sipxRelayService.setModelFilesContext(TestHelper.getModelFilesContext());
        m_sipxRelayService.setModelDir("nattraversal");
        m_sipxRelayService.setModelName("nattraversal.xml");
        m_sipxRelayService.setLogDir("/usr/local/sipx/var/log/sipxpbx");
        m_sipxRelayService.setSettingValue("relay-config/SIP_RELAY_LOG_LEVEL", "NOTICE");
        m_sipxRelayService.setSettingTypedValue("nat/enabled", true);
        m_sipxRelayService.setSettingValue("nat/media-relay-temperament", "Aggressive");

        m_sipxServiceManager = TestUtil.getMockSipxServiceManager(true, sipxProxyService, m_sipxRelayService);
    }

    public void testGenerateNoBehindNat() throws Exception {
        m_sipxRelayService.setSettingTypedValue("nat/behind-nat", false);
        m_natLocation.setUseStun(true);

        NatTraversalConfiguration rules = generate();
        assertCorrectFileGeneration(rules, "nattraversalrules.test.no-behind-nat.xml");
    }

    public void testGenerateNoPublicAddress() throws Exception {
        m_sipxRelayService.setSettingTypedValue("nat/behind-nat", true);
        m_natLocation.setUseStun(true);
        m_natLocation.setPublicAddress(null);

        NatTraversalConfiguration rules = generate();
        assertCorrectFileGeneration(rules, "nattraversalrules.test.no-public-address.xml");
    }

    public void testGenerateNoStunServer() throws Exception {
        m_sipxRelayService.setSettingValue("nat/behind-nat", "true");
        m_natLocation.setUseStun(false);
        m_natLocation.setPublicAddress("1.2.3.4");
        m_natLocation.setStunAddress(null);

        NatTraversalConfiguration rules = generate();
        assertCorrectFileGeneration(rules, "nattraversalrules.test.no-stun.xml");
    }

    public void testGenerateNoRejectStrayPackets() throws Exception {
        m_sipxRelayService.setSettingValue("nat/behind-nat", "true");
        m_natLocation.setUseStun(false);
        m_natLocation.setPublicAddress("1.2.3.4");
        m_natLocation.setStunAddress(null);
        m_sipxRelayService.setSettingValue("nat/reject-stray-packets", "false");

        NatTraversalConfiguration rules = generate();
        assertCorrectFileGeneration(rules, "nattraversalrules.test.no-reject-stray-packets.xml");
    }

    private NatTraversalConfiguration generate() {
        NatTraversalConfiguration rules = new NatTraversalConfiguration();
        rules.setVelocityEngine(TestHelper.getVelocityEngine());
        rules.setTemplate("nattraversal/nattraversalrules.vm");
        rules.setSbcManager(m_sbcManager);
        rules.setSipxServiceManager(m_sipxServiceManager);
        return rules;
    }

    private void assertCorrectFileGeneration(NatTraversalConfiguration natTraveralRules, String expectedFileName)
            throws Exception {
        natTraveralRules.setVelocityEngine(TestHelper.getVelocityEngine());

        StringWriter actualConfigWriter = new StringWriter();
        natTraveralRules.write(actualConfigWriter, m_locationsManager.getPrimaryLocation());

        InputStream resourceAsStream = natTraveralRules.getClass().getResourceAsStream(expectedFileName);
        assertNotNull(resourceAsStream);

        Reader referenceConfigReader = new InputStreamReader(resourceAsStream);
        String referenceConfig = IOUtils.toString(referenceConfigReader);

        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
        String actualConfig = IOUtils.toString(actualConfigReader);

        assertEquals(referenceConfig, actualConfig);
    }

    private static SbcRoutes configureSbcRoutes(List<String> domains, List<String> subnets) {
        SbcRoutes routes = new SbcRoutes();
        routes.setDomains(domains);
        routes.setSubnets(subnets);

        return routes;
    }
}

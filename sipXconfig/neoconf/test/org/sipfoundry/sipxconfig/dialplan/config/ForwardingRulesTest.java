/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan.config;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.SipTrunk;
import org.sipfoundry.sipxconfig.mwi.Mwi;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.registrar.Registrar;
import org.sipfoundry.sipxconfig.sbc.AuxSbc;
import org.sipfoundry.sipxconfig.sbc.DefaultSbc;
import org.sipfoundry.sipxconfig.sbc.Sbc;
import org.sipfoundry.sipxconfig.sbc.SbcDescriptor;
import org.sipfoundry.sipxconfig.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.sbc.SbcManager;
import org.sipfoundry.sipxconfig.sbc.SbcRoutes;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.sipfoundry.sipxconfig.test.XmlUnitHelper;

public class ForwardingRulesTest extends XMLTestCase {
    private Location m_statusLocation;
    private SbcDeviceManager m_sbcDeviceManager;
    private AddressManager m_addressManager;
    private Location m_location;

    @Override
    protected void setUp() throws Exception {
        TestHelper.initDefaultDomain();
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);

        DomainManager domainManager = TestHelper.getMockDomainManager();
        replay(domainManager);
        
        m_location = TestHelper.createDefaultLocation();

        m_addressManager = createMock(AddressManager.class);
        m_addressManager.getSingleAddress(ProxyManager.TCP_ADDRESS, m_location);
        expectLastCall().andReturn(new Address(ProxyManager.TCP_ADDRESS, "proxy.example.org", 9901)).once();
        m_addressManager.getSingleAddress(Mwi.SIP_TCP, m_location);
        expectLastCall().andReturn(new Address(Mwi.SIP_TCP, "mwi.example.org", 9902)).once();
        m_addressManager.getSingleAddress(Registrar.EVENT_ADDRESS, m_location);
        expectLastCall().andReturn(new Address(Registrar.EVENT_ADDRESS, "regevent.example.org", 9903)).once();
        m_addressManager.getSingleAddress(Registrar.TCP_ADDRESS, m_location);        
        expectLastCall().andReturn(new Address(Registrar.TCP_ADDRESS, "reg.example.org", 9904)).once();
        replay(m_addressManager);

        List<Location> locations = new ArrayList<Location>();
        m_statusLocation = new Location();
        m_statusLocation.setAddress("192.168.1.5");
        locations.add(m_statusLocation);
        m_sbcDeviceManager = createMock(SbcDeviceManager.class);

        GatewayContext gatewayContext = createMock(GatewayContext.class);
        gatewayContext.getGatewayByType(SipTrunk.class);
        expectLastCall().andReturn(null);
        replay(gatewayContext);

        m_sbcDeviceManager.getBridgeSbc(m_statusLocation);
        expectLastCall().andReturn(null).anyTimes();

        m_sbcDeviceManager.getSbcDevices();
        expectLastCall().andReturn(Collections.emptyList());
        replay(m_sbcDeviceManager);
    }

    public void testGenerate() throws Exception {

        IDialingRule rule = createNiceMock(IDialingRule.class);
        rule.getHostPatterns();
        expectLastCall().andReturn(new String[] {
            "gander"
        });

        Sbc sbc = configureSbc(new DefaultSbc(), configureSbcDevice("10.1.2.3"), Arrays.asList("*.example.org",
                "*.example.net"), Arrays.asList("10.1.2.3/16"));
        sbc.setAddress("10.1.2.3");
        SbcManager sbcManager = createNiceMock(SbcManager.class);
        sbcManager.loadDefaultSbc();
        expectLastCall().andReturn(sbc);

        replay(rule, sbcManager);

        ForwardingRules rules = generate(rule, sbcManager);
        rules.setLocation(m_location);
        rules.setAddressManager(m_addressManager);
        String actual = toString(rules);
        InputStream referenceXmlStream = ForwardingRulesTest.class.getResourceAsStream("forwardingrules.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(actual));
        assertXpathEvaluatesTo("gander", "/routes/route/routeFrom[10]", actual);

        verify(rule, sbcManager);
    }
    
    String toString(ForwardingRules rules) throws IOException {
        rules.setLocation(TestHelper.createDefaultLocation());
        StringWriter w = new StringWriter();
        rules.write(w);
        return w.toString();        
    }

    public void testGenerateWithEmptyDomainsAndIntranets() throws Exception {

        IDialingRule rule = createNiceMock(IDialingRule.class);
        rule.getHostPatterns();
        expectLastCall().andReturn(new String[] {
            "gander"
        });

        Sbc sbc = configureSbc(new DefaultSbc(), configureSbcDevice("10.1.2.3"), new ArrayList<String>(),
                new ArrayList<String>());
        sbc.setAddress("10.1.2.3");
        SbcManager sbcManager = createNiceMock(SbcManager.class);
        sbcManager.loadDefaultSbc();
        expectLastCall().andReturn(sbc);

        replay(rule, sbcManager);

        ForwardingRules rules = generate(rule, sbcManager);
        rules.setAddressManager(m_addressManager);
        String generatedXml = toString(rules);
        InputStream referenceXmlStream = ForwardingRulesTest.class
                .getResourceAsStream("forwardingrules-no-local-ip.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));

        assertXpathEvaluatesTo("gander", "/routes/route/routeFrom[10]", generatedXml);

        verify(rule, sbcManager);
    }

    // TODO : Fix then when adding DNS server back in
    public void DISABLED_testGenerateMultipleRegistrars() throws Exception {

        IDialingRule rule = createNiceMock(IDialingRule.class);
        rule.getHostPatterns();
        expectLastCall().andReturn(new String[] {
            "gander"
        });

        m_statusLocation = new Location();
        m_statusLocation.setAddress("192.168.1.5");

        Location location2 = new Location();

        Sbc sbc = configureSbc(new DefaultSbc(), configureSbcDevice("10.1.2.3"), new ArrayList<String>(),
                new ArrayList<String>());
        sbc.setAddress("10.1.2.3");
        SbcManager sbcManager = createNiceMock(SbcManager.class);
        sbcManager.loadDefaultSbc();
        expectLastCall().andReturn(sbc);

      replay(rule, sbcManager);

        ForwardingRules rules = generate(rule, sbcManager);
        rules.setAddressManager(m_addressManager);
        String generatedXml = toString(rules);
        InputStream referenceXmlStream = ForwardingRulesTest.class
                .getResourceAsStream("forwardingrules-ha.test.xml");

        // assertEquals(IOUtils.toString(referenceXmlStream), generatedXml);
        assertEquals(generatedXml, IOUtils.toString(referenceXmlStream));

        assertXpathEvaluatesTo("gander", "/routes/route/routeFrom[10]", generatedXml);

        verify(rule, sbcManager);
    }

    public void testGenerateAuxSbcs() throws Exception {

        IDialingRule rule = createNiceMock(IDialingRule.class);
        rule.getHostPatterns();
        expectLastCall().andReturn(new String[] {
            "gander"
        });

        Sbc sbc = configureSbc(new DefaultSbc(), configureSbcDevice("10.1.2.3", 5070), Arrays.asList("*.example.org",
                "*.example.net"), Arrays.asList("10.1.2.3/16"));
        Sbc aux1 = configureSbc(new AuxSbc(), configureSbcDevice("10.1.2.4"), Arrays.asList("*.sipfoundry.org",
                "*.sipfoundry.net"), new ArrayList<String>());
        Sbc aux2 = configureSbc(new AuxSbc(), configureSbcDevice("sbc.example.org"), Arrays.asList("*.xxx",
                "*.example.tm"), Arrays.asList("10.4.4.1/24"));
        sbc.setAddress("10.1.2.3");
        sbc.setPort(5070);
        aux1.setAddress("10.1.2.4");
        aux2.setAddress("sbc.example.org");
        SbcManager sbcManager = createNiceMock(SbcManager.class);
        sbcManager.loadDefaultSbc();
        expectLastCall().andReturn(sbc);
        sbcManager.loadAuxSbcs();
        expectLastCall().andReturn(Arrays.asList(aux1, aux2));

        replay(rule, sbcManager);

        ForwardingRules rules = generate(rule, sbcManager);
        rules.setAddressManager(m_addressManager);
        String generatedXml = toString(rules);
        InputStream referenceXmlStream = ForwardingRulesTest.class
                .getResourceAsStream("forwardingrules-aux.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
        verify(rule, sbcManager);
    }

    public void testGenerateAuxSbcsDisabled() throws Exception {

        IDialingRule rule = createNiceMock(IDialingRule.class);
        rule.getHostPatterns();
        expectLastCall().andReturn(new String[] {
            "gander"
        });

        Sbc sbc = configureSbc(new DefaultSbc(), configureSbcDevice("10.1.2.3"), Arrays.asList("*.example.org",
                "*.example.net"), Arrays.asList("10.1.2.3/16"));
        sbc.setAddress("10.1.2.3");
        Sbc aux1 = configureSbc(new AuxSbc(), configureSbcDevice("10.1.2.4"), Arrays.asList("*.sipfoundry.org",
                "*.sipfoundry.net"), new ArrayList<String>());
        aux1.setAddress("10.1.2.4");
        aux1.setEnabled(false);
        Sbc aux2 = configureSbc(new AuxSbc(), configureSbcDevice("sbc.example.org"), Arrays.asList("*.xxx",
                "*.example.tm"), Arrays.asList("10.4.4.1/24"));
        aux2.setEnabled(false);
        aux2.setAddress("sbc.example.org");

        SbcManager sbcManager = createNiceMock(SbcManager.class);
        sbcManager.loadDefaultSbc();
        expectLastCall().andReturn(sbc);
        sbcManager.loadAuxSbcs();
        expectLastCall().andReturn(Arrays.asList(aux1, aux2));

        replay(rule, sbcManager);

        ForwardingRules rules = generate(rule, sbcManager);
        rules.setAddressManager(m_addressManager);
        String generatedXml = toString(rules);
        InputStream referenceXmlStream = ForwardingRulesTest.class.getResourceAsStream("forwardingrules.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
        verify(rule, sbcManager);
    }

    public void testGenerateWithItspNameCallback() throws Exception {
        IDialingRule rule = createNiceMock(IDialingRule.class);
        rule.getHostPatterns();
        expectLastCall().andReturn(new String[] {
            "gander"
        });

        ModelFilesContext modelFilesContext = TestHelper.getModelFilesContext();
        DeviceDefaults deviceDefaults = PhoneTestDriver.getDeviceDefaults();

        SbcDescriptor sbcDescriptor = new SbcDescriptor();
        sbcDescriptor.setInternalSbc(true);

        BridgeSbc bridgeSbc = new BridgeSbc();
        bridgeSbc.setModel(sbcDescriptor);
        bridgeSbc.setDefaults(deviceDefaults);
        bridgeSbc.setAddress("192.168.5.240");
        bridgeSbc.setPort(5090);

        SipTrunk sipTrunk = new SipTrunk();
        sipTrunk.setDefaults(deviceDefaults);
        sipTrunk.setModelFilesContext(modelFilesContext);
        sipTrunk.setSbcDevice(bridgeSbc);
        sipTrunk.setAddress("itsp.example.com");
        sipTrunk.setSettingValue("itsp-account/itsp-proxy-domain", "default.itsp.proxy.domain");

        GatewayContext gatewayContext = createMock(GatewayContext.class);
        gatewayContext.getGatewayByType(SipTrunk.class);
        expectLastCall().andReturn(Arrays.asList(sipTrunk));
        replay(gatewayContext);
        bridgeSbc.setGatewayContext(gatewayContext);

        m_sbcDeviceManager = createNiceMock(SbcDeviceManager.class);
        m_sbcDeviceManager.getSbcDevices();
        expectLastCall().andReturn(Collections.singletonList(bridgeSbc));
        replay(m_sbcDeviceManager);

        Sbc sbc = configureSbc(new DefaultSbc(), configureSbcDevice("10.1.2.3"), Arrays.asList("*.example.org",
                "*.example.net"), Arrays.asList("10.1.2.3/16"));
        sbc.setAddress("10.1.2.3");

        SbcManager sbcManager = createNiceMock(SbcManager.class);
        sbcManager.loadDefaultSbc();
        expectLastCall().andReturn(sbc);
        sbcManager.loadAuxSbcs();
        expectLastCall().andReturn(null);

        replay(rule, sbcManager);

        ForwardingRules rules = generate(rule, sbcManager);
        rules.setAddressManager(m_addressManager);
        String generatedXml = toString(rules);
        InputStream referenceXmlStream = ForwardingRulesTest.class
                .getResourceAsStream("forwardingrules-itsp-callback-test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
    }

    private ForwardingRules generate(IDialingRule rule, SbcManager sbcManager) {
        ForwardingRules rules = new ForwardingRules();
        rules.setVelocityEngine(TestHelper.getVelocityEngine());
        rules.setSbcManager(sbcManager);
        rules.setSbcDeviceManager(m_sbcDeviceManager);
        rules.setDomainName("example.org");
        rules.begin();
        rules.generate(rule);
        rules.end();
        return rules;
    }

    private Sbc configureSbc(Sbc sbc, SbcDevice sbcDevice, List<String> domains, List<String> subnets) {
        SbcRoutes routes = new SbcRoutes();
        routes.setDomains(domains);
        routes.setSubnets(subnets);

        sbc.setRoutes(routes);
        sbc.setSbcDevice(sbcDevice);
        sbc.setEnabled(true);
        return sbc;
    }

    private SbcDevice configureSbcDevice(String address, int port) {
        SbcDevice sbcDevice = new SbcDevice();
        sbcDevice.setAddress(address);
        sbcDevice.setPort(port);
        return sbcDevice;
    }

    private SbcDevice configureSbcDevice(String address) {
        return configureSbcDevice(address, 0);
    }
}

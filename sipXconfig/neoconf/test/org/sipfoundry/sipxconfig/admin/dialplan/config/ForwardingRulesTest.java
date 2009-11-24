/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.service.SipxService;

import org.apache.commons.io.IOUtils;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.AuxSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.DefaultSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.Sbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDescriptor;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcRoutes;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.SipTrunk;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.service.SipxBridgeService;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.SipxStatusService;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestUtil;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class ForwardingRulesTest extends XMLTestCase {
    private SipxServiceManager m_sipxServiceManager;
    private LocationsManager m_locationsManager;
    private Location m_statusLocation;
    private SipxBridgeService m_bridgeService;
    private SipxService m_statusService;
    private SipxRegistrarService m_registrarService;
    private SbcDeviceManager m_sbcDeviceManager;

    @Override
    protected void setUp() throws Exception {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);

        DomainManager domainManager = TestUtil.getMockDomainManager();
        replay(domainManager);

        m_bridgeService = new SipxBridgeService();
        m_statusService = new SipxStatusService();
        m_registrarService = new SipxRegistrarService();
        m_statusLocation = new Location();
        m_statusLocation.setAddress("192.168.1.5");
        List<Location> locations = new ArrayList<Location>();
        locations.add(m_statusLocation);
        m_locationsManager = createNiceMock(LocationsManager.class);
        m_locationsManager.getLocationsForService(m_statusService);
        expectLastCall().andReturn(locations).anyTimes();
        m_locationsManager.getLocationsForService(m_registrarService);
        expectLastCall().andReturn(locations).anyTimes();
        m_locationsManager.getPrimaryLocation();
        expectLastCall().andReturn(m_statusLocation).anyTimes();
        m_locationsManager.getLocationsForService(m_bridgeService);
        expectLastCall().andReturn(locations).anyTimes();
        replay(m_locationsManager);

        SipxProxyService proxyService = new SipxProxyService();
        proxyService.setBeanName(SipxProxyService.BEAN_ID);
        proxyService.setModelDir("sipxproxy");
        proxyService.setModelName("sipxproxy.xml");
        proxyService.setModelFilesContext(TestHelper.getModelFilesContext());
        proxyService.setSipPort("9901");
        Setting proxySettings = proxyService.getSettings();
        proxySettings.getSetting("proxy-configuration/SIP_PORT").setValue("9901");
        proxyService.setDomainManager(domainManager);
        m_statusLocation.addService(proxyService);

        m_statusService.setBeanName(SipxStatusService.BEAN_ID);
        m_statusService.setModelFilesContext(TestHelper.getModelFilesContext());
        m_statusService.setSettings(TestHelper.loadSettings("sipxstatus/sipxstatus.xml"));
        m_statusService.setLocationsManager(m_locationsManager);
        Setting statusConfigSettings = m_statusService.getSettings().getSetting("status-config");
        statusConfigSettings.getSetting("SIP_STATUS_SIP_PORT").setValue("9905");

        m_registrarService.setBeanName(SipxRegistrarService.BEAN_ID);
        m_registrarService.setRegistrarEventSipPort("9906");
        m_registrarService.setSipPort("9907");
        m_statusLocation.addService(m_registrarService);

        m_bridgeService.setBeanName(SipxBridgeService.BEAN_ID);
        m_bridgeService.setSipPort("9908");
        m_statusLocation.addService(m_bridgeService);
        m_bridgeService.setLocationsManager(m_locationsManager);

        m_sipxServiceManager = TestUtil.getMockSipxServiceManager(false, proxyService, m_registrarService,
                m_statusService, m_bridgeService);
        m_sipxServiceManager.isServiceInstalled(m_statusLocation.getId(), SipxRegistrarService.BEAN_ID);
        expectLastCall().andReturn(true).anyTimes();
        replay(m_sipxServiceManager);

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

        SbcManager sbcManager = createNiceMock(SbcManager.class);
        sbcManager.loadDefaultSbc();
        expectLastCall().andReturn(sbc);

        replay(rule, sbcManager);

        ForwardingRules rules = generate(rule, sbcManager);

        String generatedXml = getGeneratedXmlFileAsString(rules);

        InputStream referenceXmlStream = ForwardingRulesTest.class.getResourceAsStream("forwardingrules.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));

        assertXpathEvaluatesTo("gander", "/routes/route/routeFrom[10]", generatedXml);

        verify(rule, sbcManager);
    }

    public void testGenerateWithEmptyDomainsAndIntranets() throws Exception {

        IDialingRule rule = createNiceMock(IDialingRule.class);
        rule.getHostPatterns();
        expectLastCall().andReturn(new String[] {
            "gander"
        });

        Sbc sbc = configureSbc(new DefaultSbc(), configureSbcDevice("10.1.2.3"), new ArrayList<String>(),
                new ArrayList<String>());

        SbcManager sbcManager = createNiceMock(SbcManager.class);
        sbcManager.loadDefaultSbc();
        expectLastCall().andReturn(sbc);

        replay(rule, sbcManager);

        ForwardingRules rules = generate(rule, sbcManager);

        String generatedXml = getGeneratedXmlFileAsString(rules);

        InputStream referenceXmlStream = ForwardingRulesTest.class
                .getResourceAsStream("forwardingrules-no-local-ip.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));

        assertXpathEvaluatesTo("gander", "/routes/route/routeFrom[10]", generatedXml);

        verify(rule, sbcManager);
    }

    public void testGenerateMultipleRegistrars() throws Exception {

        IDialingRule rule = createNiceMock(IDialingRule.class);
        rule.getHostPatterns();
        expectLastCall().andReturn(new String[] {
            "gander"
        });

        m_statusLocation = new Location();
        m_statusLocation.setAddress("192.168.1.5");

        Location location2 = new Location();

        m_locationsManager = createMock(LocationsManager.class);
        m_locationsManager.getLocationsForService(m_statusService);
        expectLastCall().andReturn(Arrays.asList(m_statusLocation)).anyTimes();
        m_locationsManager.getLocationsForService(m_registrarService);
        expectLastCall().andReturn(Arrays.asList(m_statusLocation, location2)).anyTimes();
        m_locationsManager.getLocationsForService(m_bridgeService);
        expectLastCall().andReturn(Arrays.asList(m_statusLocation)).anyTimes();
        m_locationsManager.getPrimaryLocation();
        expectLastCall().andReturn(m_statusLocation).anyTimes();

        Sbc sbc = configureSbc(new DefaultSbc(), configureSbcDevice("10.1.2.3"), new ArrayList<String>(),
                new ArrayList<String>());

        SbcManager sbcManager = createNiceMock(SbcManager.class);
        sbcManager.loadDefaultSbc();
        expectLastCall().andReturn(sbc);

        replay(rule, sbcManager, m_locationsManager);

        ForwardingRules rules = generate(rule, sbcManager);

        String generatedXml = getGeneratedXmlFileAsString(rules);

        InputStream referenceXmlStream = ForwardingRulesTest.class
                .getResourceAsStream("forwardingrules-ha.test.xml");

        // assertEquals(IOUtils.toString(referenceXmlStream), generatedXml);
        assertEquals(generatedXml, IOUtils.toString(referenceXmlStream));

        assertXpathEvaluatesTo("gander", "/routes/route/routeFrom[10]", generatedXml);

        verify(rule, sbcManager, m_locationsManager);
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

        SbcManager sbcManager = createNiceMock(SbcManager.class);
        sbcManager.loadDefaultSbc();
        expectLastCall().andReturn(sbc);
        sbcManager.loadAuxSbcs();
        expectLastCall().andReturn(Arrays.asList(aux1, aux2));

        replay(rule, sbcManager);

        ForwardingRules rules = generate(rule, sbcManager);

        String generatedXml = getGeneratedXmlFileAsString(rules);

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
        Sbc aux1 = configureSbc(new AuxSbc(), configureSbcDevice("10.1.2.4"), Arrays.asList("*.sipfoundry.org",
                "*.sipfoundry.net"), new ArrayList<String>());
        aux1.setEnabled(false);
        Sbc aux2 = configureSbc(new AuxSbc(), configureSbcDevice("sbc.example.org"), Arrays.asList("*.xxx",
                "*.example.tm"), Arrays.asList("10.4.4.1/24"));
        aux2.setEnabled(false);

        SbcManager sbcManager = createNiceMock(SbcManager.class);
        sbcManager.loadDefaultSbc();
        expectLastCall().andReturn(sbc);
        sbcManager.loadAuxSbcs();
        expectLastCall().andReturn(Arrays.asList(aux1, aux2));

        replay(rule, sbcManager);

        ForwardingRules rules = generate(rule, sbcManager);

        String generatedXml = getGeneratedXmlFileAsString(rules);

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
        sipTrunk.setSettingValue("itsp-account/itsp-proxy-domain", "itsp.example.com");

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

        SbcManager sbcManager = createNiceMock(SbcManager.class);
        sbcManager.loadDefaultSbc();
        expectLastCall().andReturn(sbc);
        sbcManager.loadAuxSbcs();
        expectLastCall().andReturn(null);

        replay(rule, sbcManager);

        ForwardingRules rules = generate(rule, sbcManager);

        String generatedXml = getGeneratedXmlFileAsString(rules);

        InputStream referenceXmlStream = ForwardingRulesTest.class
                .getResourceAsStream("forwardingrules-itsp-callback-test.xml");

        assertXMLEqual(new StringReader(generatedXml), new InputStreamReader(referenceXmlStream));
    }

    private ForwardingRules generate(IDialingRule rule, SbcManager sbcManager) {
        ForwardingRules rules = new ForwardingRules();
        rules.setTemplate("commserver/forwardingrules.vm");
        rules.setSipxServiceManager(m_sipxServiceManager);
        rules.setLocationsManager(m_locationsManager);
        rules.setVelocityEngine(TestHelper.getVelocityEngine());
        rules.setSbcManager(sbcManager);
        rules.setSbcDeviceManager(m_sbcDeviceManager);
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

    private String getGeneratedXmlFileAsString(ConfigurationFile xmlFile) throws Exception {
        StringWriter actualXmlFileWriter = new StringWriter();
        xmlFile.write(actualXmlFileWriter, TestUtil.createDefaultLocation());

        Reader actualXmlFileReader = new StringReader(actualXmlFileWriter.toString());
        String actualXmlFile = IOUtils.toString(actualXmlFileReader);

        return actualXmlFile;
    }
}

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

import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import javax.imageio.spi.RegisterableService;

import org.apache.commons.io.IOUtils;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.AuxSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.DefaultSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.Sbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcRoutes;
import org.sipfoundry.sipxconfig.admin.localization.Localization;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.domain.DomainManagerImpl;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxServiceConfiguration;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.SipxServiceTestBase;
import org.sipfoundry.sipxconfig.service.SipxStatusService;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class ForwardingRulesTest extends XMLTestCase {

    private SipxServiceManager m_sipxServiceManager;

    protected void setUp() throws Exception {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);

        DomainManager domainManager = TestUtil.getMockDomainManager();

        SipxProxyService proxyService = new SipxProxyService();
        proxyService.setSipPort("9901");
        proxyService.setDomainManager(domainManager);

        SipxStatusService statusService = new SipxStatusService();
        statusService.setModelFilesContext(TestHelper.getModelFilesContext());
        statusService.setSettings(TestHelper.loadSettings("sipxstatus/sipxstatus.xml"));
        Setting statusConfigSettings = statusService.getSettings().getSetting("status-config");
        statusConfigSettings.getSetting("SIP_STATUS_SIP_PORT").setValue("9905");

        SipxRegistrarService registrarService = new SipxRegistrarService();
        registrarService.setRegistrarEventSipPort("9906");
        registrarService.setSipPort("9907");

        m_sipxServiceManager = TestUtil.getMockSipxServiceManager(proxyService, registrarService, statusService);
        EasyMock.replay(domainManager, m_sipxServiceManager);
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

        InputStream referenceXmlStream = ForwardingRulesTest.class
                .getResourceAsStream("forwardingrules.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));

        assertXpathEvaluatesTo("gander", "/routes/route/routeFrom[5]", generatedXml);

        verify(rule, sbcManager);
    }
    
    public void testGenerateWithEmptyDomainsAndIntranets() throws Exception {

        IDialingRule rule = createNiceMock(IDialingRule.class);
        rule.getHostPatterns();
        expectLastCall().andReturn(new String[] {
            "gander"
        });

        Sbc sbc = configureSbc(new DefaultSbc(), configureSbcDevice("10.1.2.3"),
        		new ArrayList<String>(), new ArrayList<String>());

        SbcManager sbcManager = createNiceMock(SbcManager.class);
        sbcManager.loadDefaultSbc();
        expectLastCall().andReturn(sbc);

        replay(rule, sbcManager);

        ForwardingRules rules = generate(rule, sbcManager);

        String generatedXml = getGeneratedXmlFileAsString(rules);
        
        InputStream referenceXmlStream = ForwardingRulesTest.class
                .getResourceAsStream("forwardingrules-no-local-ip.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));

        assertXpathEvaluatesTo("gander", "/routes/route/routeFrom[5]", generatedXml);

        verify(rule, sbcManager);
    }

    public void testGenerateAuxSbcs() throws Exception {

        IDialingRule rule = createNiceMock(IDialingRule.class);
        rule.getHostPatterns();
        expectLastCall().andReturn(new String[] {
            "gander"
        });

        Sbc sbc = configureSbc(new DefaultSbc(), configureSbcDevice("10.1.2.3"), Arrays.asList("*.example.org",
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

        InputStream referenceXmlStream = ForwardingRulesTest.class
                .getResourceAsStream("forwardingrules.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
        verify(rule, sbcManager);
    }

    private ForwardingRules generate(IDialingRule rule, SbcManager sbcManager) {
        ForwardingRules rules = new ForwardingRules();
        rules.setSipxServiceManager(m_sipxServiceManager);
        rules.setVelocityEngine(TestHelper.getVelocityEngine());
        rules.setSbcManager(sbcManager);
        rules.begin();
        rules.generate(rule);
        rules.end();
        return rules;
    }

    private Sbc configureSbc(Sbc sbc, SbcDevice sbcDevice, List<String> domains,
            List<String> subnets) {
        SbcRoutes routes = new SbcRoutes();
        routes.setDomains(domains);
        routes.setSubnets(subnets);

        sbc.setRoutes(routes);
        sbc.setSbcDevice(sbcDevice);
        sbc.setEnabled(true);
        return sbc;
    }

    private SbcDevice configureSbcDevice(String address) {
        SbcDevice sbcDevice = new SbcDevice();
        sbcDevice.setAddress(address);
        return sbcDevice;
    }

    private void assertCorrectFileGeneration(XmlFile xmlFile,
            String expectedFileName) throws Exception {

        InputStream resourceAsStream = xmlFile.getClass().getResourceAsStream(expectedFileName);
        assertNotNull(resourceAsStream);

        Reader referenceXmlFileReader = new InputStreamReader(resourceAsStream);
        String referenceXmlFile = IOUtils.toString(referenceXmlFileReader);

        String actualXmlFile = getGeneratedXmlFileAsString(xmlFile);

        assertEquals(referenceXmlFile, actualXmlFile);
    }

    private String getGeneratedXmlFileAsString(XmlFile xmlFile) throws Exception {
        StringWriter actualXmlFileWriter = new StringWriter();
        xmlFile.write(actualXmlFileWriter, createDefaultLocation());

        Reader actualXmlFileReader = new StringReader(actualXmlFileWriter.toString());
        String actualXmlFile = IOUtils.toString(actualXmlFileReader);

        return actualXmlFile;
    }

    private Location createDefaultLocation() {
        Location location = new Location();
        location.setName("localLocation");
        location.setFqdn("sipx.example.org");
        location.setAddress("192.168.1.1");
        return location;
    }
}

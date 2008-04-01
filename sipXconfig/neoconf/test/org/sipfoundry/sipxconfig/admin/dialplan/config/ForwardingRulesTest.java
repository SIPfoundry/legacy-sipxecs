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
import java.io.StringReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.AuxSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.DefaultSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.Sbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcRoutes;

public class ForwardingRulesTest extends XMLTestCase {
    protected void setUp() throws Exception {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
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

        Document document = rules.getDocument();
        String generatedXml = XmlUnitHelper.asString(document);

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

        Document document = rules.getDocument();
        String generatedXml = XmlUnitHelper.asString(document);
        
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

        Document document = rules.getDocument();
        String generatedXml = XmlUnitHelper.asString(document);

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

        Document document = rules.getDocument();
        String generatedXml = XmlUnitHelper.asString(document);

        InputStream referenceXmlStream = ForwardingRulesTest.class
                .getResourceAsStream("forwardingrules.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
        verify(rule, sbcManager);
    }

    private static ForwardingRules generate(IDialingRule rule, SbcManager sbcManager) {
        ForwardingRules rules = new ForwardingRules();
        rules.setVelocityEngine(TestHelper.getVelocityEngine());
        rules.setSbcManager(sbcManager);
        rules.begin();
        rules.generate(rule);
        rules.end();
        return rules;
    }

    private static Sbc configureSbc(Sbc sbc, SbcDevice sbcDevice, List<String> domains,
            List<String> subnets) {
        SbcRoutes routes = new SbcRoutes();
        routes.setDomains(domains);
        routes.setSubnets(subnets);

        sbc.setRoutes(routes);
        sbc.setSbcDevice(sbcDevice);
        sbc.setEnabled(true);
        return sbc;
    }

    private static SbcDevice configureSbcDevice(String address) {
        SbcDevice sbcDevice = new SbcDevice();
        sbcDevice.setAddress(address);
        return sbcDevice;
    }
}

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
import java.util.Arrays;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.DefaultSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.Sbc;
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

        SbcRoutes routes = new SbcRoutes();
        routes.setDomains(Arrays.asList("*.example.org", "*.example.net"));
        routes.setSubnets(Arrays.asList("10.1.2.3/16"));

        Sbc sbc = new DefaultSbc();
        sbc.setRoutes(routes);
        sbc.setAddress("10.1.2.3");
        sbc.setEnabled(true);

        SbcManager sbcManager = createNiceMock(SbcManager.class);
        sbcManager.loadDefaultSbc();
        expectLastCall().andReturn(sbc);

        replay(rule, sbcManager);

        ForwardingRules rules = new ForwardingRules();
        rules.setVelocityEngine(TestHelper.getVelocityEngine());
        rules.setSbcManager(sbcManager);
        rules.begin();
        rules.generate(rule);
        rules.end();

        Document document = rules.getDocument();
        String generatedXml = XmlUnitHelper.asString(document);

        InputStream referenceXmlStream = ForwardingRulesTest.class
                .getResourceAsStream("forwardingrules.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));

        assertXpathEvaluatesTo("gander", "/routes/route/routeFrom[5]", generatedXml);

        verify(rule, sbcManager);
    }
}

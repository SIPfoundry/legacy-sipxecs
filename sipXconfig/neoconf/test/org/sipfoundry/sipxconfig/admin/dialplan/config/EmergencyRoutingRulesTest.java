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
import java.io.StringReader;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyRouting;
import org.sipfoundry.sipxconfig.admin.dialplan.RoutingException;
import org.sipfoundry.sipxconfig.gateway.Gateway;

/**
 * EmergencyRoutingRulesTest
 */
public class EmergencyRoutingRulesTest extends XMLTestCase {
    private EmergencyRouting m_routing;

    protected void setUp() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);

        Gateway g1 = new Gateway();
        g1.setAddress("47.1.2.22");
        g1.setUniqueId();
        RoutingException e1 = new RoutingException("823124234, 723124234, 422344234", "9911", g1);
        e1.setUniqueId();

        Gateway g2 = new Gateway();
        g2.setAddress("10.1.2.1");
        g2.setUniqueId();
        RoutingException e2 = new RoutingException("623124234, 723125344, 8354234, 422344234",
                "911", g2);
        e2.setUniqueId();

        Gateway g3 = new Gateway();
        g3.setAddress("xxx.yyy.com");
        g3.setUniqueId();
        RoutingException e3 = new RoutingException("523124234", "922", g3);
        e3.setUniqueId();

        m_routing = new EmergencyRouting();
        m_routing.addException(e1);
        m_routing.addException(e2);
        m_routing.addException(e3);

        Gateway g4 = new Gateway();
        g4.setUniqueId();
        g4.setAddress("10.2.3.4");
        m_routing.setDefaultGateway(g4);
        m_routing.setExternalNumber("919");
    }

    public void testGenerate() throws Exception {
        EmergencyRoutingRules rules = new EmergencyRoutingRules();
        rules.generate(m_routing, "domain.com");
        String generatedXml = rules.getFileContent();
        InputStream referenceXml = EmergencyRoutingRulesTest.class
                .getResourceAsStream("e911rules.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(generatedXml));
    }
}

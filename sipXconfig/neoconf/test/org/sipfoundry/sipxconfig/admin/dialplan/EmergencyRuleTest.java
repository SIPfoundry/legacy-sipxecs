/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.UrlTransform;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.permission.Permission;

/**
 * EmergencyRuleTest
 */
public class EmergencyRuleTest extends TestCase {
    private EmergencyRule m_rule;

    protected void setUp() throws Exception {
        m_rule = new EmergencyRule();
        m_rule.setEmergencyNumber("911");
        m_rule.setOptionalPrefix("9");

        Gateway g1 = new Gateway();
        g1.setAddress("sosgateway1.com");
        g1.setAddressPort(4000);
        Gateway g2 = new Gateway();
        g2.setAddress("sosgateway2.com");
        g2.setAddressPort(0);
        g2.setPrefix("4321");
        m_rule.setGateways(Arrays.asList(new Gateway[] {
            g1, g2
        }));
    }

    public void testGetPatterns() {
        String[] patterns = m_rule.getPatterns();
        assertEquals(3, patterns.length);
        assertEquals("sos", patterns[0]);
        assertEquals("911", patterns[1]);
        assertEquals("9911", patterns[2]);
    }

    public void testGetTransforms() {
        Transform[] transforms = m_rule.getTransforms();
        assertEquals(2, transforms.length);
        UrlTransform emergencyTransform = (UrlTransform) transforms[0];
        assertEquals("<sip:911@sosgateway1.com:4000>;q=0.933", emergencyTransform.getUrl());
        emergencyTransform = (UrlTransform) transforms[1];
        assertEquals("<sip:4321911@sosgateway2.com>;q=0.867", emergencyTransform.getUrl());
    }

    public void testCallerSensitiveForwarding() {
        m_rule.setUseMediaServer(true);
        Transform[] transforms = m_rule.getTransforms();
        assertEquals(1, transforms.length);
        UrlTransform emergencyTransform = (UrlTransform) transforms[0];
        assertEquals(
                "<sip:{digits}@{mediaserver};voicexml={voicemail}%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Dsos>",
                emergencyTransform.getUrl());
    }

    public void testPermissions() {
        List<Permission> permissions = m_rule.getPermissions();
        assertEquals(0, permissions.size());
    }

    public void testAppendToGenerationRulesEnabled() {
        final String testDescription = "emergency rule test description";
        m_rule.setEnabled(true);
        m_rule.setDescription(testDescription);
        List<Gateway> gateways = m_rule.getGateways();
        assertEquals(2, gateways.size());
        ArrayList<DialingRule> rules = new ArrayList<DialingRule>();
        m_rule.appendToGenerationRules(rules);
        assertEquals(1, rules.size());
        DialingRule rule = rules.get(0);
        assertEquals(2, rule.getGateways().size());
        assertEquals(testDescription, rule.getDescription());

        // if we are using media server return empty gateways list
        m_rule.setUseMediaServer(true);
        gateways = m_rule.getGateways();
        assertEquals(2, gateways.size());
        rules = new ArrayList<DialingRule>();
        m_rule.appendToGenerationRules(rules);
        assertEquals(1, rules.size());
        rule = rules.get(0);
        assertTrue(rule.getGateways().isEmpty());
        assertEquals(testDescription, rule.getDescription());
    }

    public void testAppendToGenerationRulesDisabled() {
        m_rule.setEnabled(false);
        m_rule.setUseMediaServer(true);
        ArrayList<DialingRule> rules = new ArrayList<DialingRule>();
        m_rule.appendToGenerationRules(rules);
        assertTrue(rules.isEmpty());
    }
}

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
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.gateway.Gateway;

public class EmergencyRoutingTest extends TestCase {
    private static final String DEFAULT_EXTENSION = "333";
    private final static String TEST_DATA[][] = {
        {
            "911", "default.abc.com"
        }, {
            "920", "g1.abc.com"
        }, {
            "911", "g2.abc.com"
        }
    };
    private Gateway m_defaultGateway;

    protected void setUp() throws Exception {
        m_defaultGateway = new Gateway();
        m_defaultGateway.setAddress("aaa.bbb.com");
    }

    public void testAsDialingRulesListEmpty() {
        EmergencyRouting routing = new EmergencyRouting();
        List rules = routing.asDialingRulesList();
        assertTrue(rules.isEmpty());
    }

    public void testAsDialingRulesListNoExceptions() {
        EmergencyRouting routing = new EmergencyRouting();
        routing.setDefaultGateway(m_defaultGateway);
        routing.setExternalNumber(DEFAULT_EXTENSION);
        List rules = routing.asDialingRulesList();
        assertEquals(1, rules.size());

        IDialingRule rule = (IDialingRule) rules.get(0);
        List gateways = rule.getGateways();
        assertEquals(1, gateways.size());
        assertEquals(m_defaultGateway.getAddress(), ((Gateway) gateways.get(0)).getAddress());

        assertTrue(rule.getPermissionNames().isEmpty());

        assertEquals(1, rule.getPatterns().length);

        assertEquals(DEFAULT_EXTENSION, rule.getPatterns()[0]);
    }

    public void testAsDialingRulesListExceptionsWithoutGateways() {
        EmergencyRouting routing = new EmergencyRouting();
        routing.setDefaultGateway(m_defaultGateway);
        routing.setExternalNumber(DEFAULT_EXTENSION);
        for (int i = 0; i < TEST_DATA.length; i++) {
            RoutingException re = new RoutingException("abc, ddd", TEST_DATA[i][0], null);
            routing.addException(re);
        }        
        List rules = routing.asDialingRulesList();
        assertEquals(1, rules.size());

        IDialingRule rule = (IDialingRule) rules.get(0);
        List gateways = rule.getGateways();
        assertEquals(1, gateways.size());
        assertEquals(m_defaultGateway.getAddress(), ((Gateway) gateways.get(0)).getAddress());

        assertTrue(rule.getPermissionNames().isEmpty());

        assertEquals(1, rule.getPatterns().length);

        assertEquals(DEFAULT_EXTENSION, rule.getPatterns()[0]);
    }
    
    public void testAsDialingRulesListFull() {
        EmergencyRouting routing = new EmergencyRouting();
        routing.setDefaultGateway(m_defaultGateway);
        routing.setExternalNumber(DEFAULT_EXTENSION);
        for (int i = 0; i < TEST_DATA.length; i++) {
            Gateway gateway = new Gateway();
            gateway.setAddress(TEST_DATA[i][1]);

            RoutingException re = new RoutingException("abc, ddd", TEST_DATA[i][0], gateway);
            routing.addException(re);
        }

        List rules = routing.asDialingRulesList();
        assertEquals(TEST_DATA.length + 1, rules.size());

        for (int i = 1; i < TEST_DATA.length; i++) {
            IDialingRule rule = (IDialingRule) rules.get(i + 1);
            List gateways = rule.getGateways();
            assertEquals(1, gateways.size());
            Gateway g = (Gateway) gateways.get(0);

            assertEquals(TEST_DATA[i][1], g.getAddress());

            assertTrue(rule.getPermissionNames().isEmpty());

            assertEquals(1, rule.getPatterns().length);

            assertEquals(TEST_DATA[i][0], rule.getPatterns()[0]);
        }
    }

    public void testRemoveGateways() {
        final int len = 5;
        Gateway[] gs = new Gateway[len];
        for (int i = 0; i < len; i++) {
            gs[i] = new Gateway();
            gs[i].setUniqueId();
        }

        RoutingException e3 = new RoutingException();
        e3.setGateway(gs[3]);

        RoutingException e4 = new RoutingException();
        e4.setGateway(gs[4]);

        EmergencyRouting routing = new EmergencyRouting();
        routing.setDefaultGateway(gs[0]);
        routing.addException(e3);
        routing.addException(e4);

        routing.removeGateways(Collections.EMPTY_LIST);
        assertSame(gs[0], routing.getDefaultGateway());
        assertSame(gs[3], e3.getGateway());
        assertSame(gs[4], e4.getGateway());

        Collection gatewaysToRemove = new ArrayList();
        gatewaysToRemove.add(gs[2].getId());
        gatewaysToRemove.add(gs[3].getId());
        routing.removeGateways(gatewaysToRemove);

        assertSame(gs[0], routing.getDefaultGateway());
        assertNull(e3.getGateway());
        assertSame(gs[4], e4.getGateway());

        gatewaysToRemove.add(gs[0].getId());
        routing.removeGateways(gatewaysToRemove);

        assertNull(routing.getDefaultGateway());
        assertNull(e3.getGateway());
        assertSame(gs[4], e4.getGateway());
    }
}

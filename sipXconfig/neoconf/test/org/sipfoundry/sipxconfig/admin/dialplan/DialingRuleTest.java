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
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.gateway.Gateway;

/**
 * DialingRuleTest
 */
public class DialingRuleTest extends TestCase {
    public void testDetach() throws Exception {
        CustomDialingRule orgRule = new CustomDialingRule();
        orgRule.setName("name");
        orgRule.setDescription("description");
        DialingRule detachedRule = (DialingRule) orgRule.duplicate();
        assertNotSame(orgRule, detachedRule);
        assertEquals(orgRule.getDescription(), detachedRule.getDescription());
        assertEquals(orgRule.getName(), detachedRule.getName());
    }

    public void testUpdate() {
        DialingRule orgRule = new CustomDialingRule();
        orgRule.setName("name");
        orgRule.setDescription("description");

        DialingRule newRule = new CustomDialingRule();
        Integer id = newRule.getId();
        // //assertFalse(id.equals(orgRule.getId()));
        newRule.update(orgRule);
        assertEquals(id, newRule.getId());
        assertEquals("name", newRule.getName());
        assertEquals("description", newRule.getDescription());
    }

    public void testDuplicate() {
        DialingRule orgRule = new CustomDialingRule();
        orgRule.setName("name");
        orgRule.setDescription("description");

        Set rules = new HashSet();
        rules.add(orgRule.setUniqueId());
        rules.add(orgRule.duplicate().setUniqueId());
        rules.add(orgRule.duplicate().setUniqueId());
        assertEquals(3, rules.size());
        for (Iterator i = rules.iterator(); i.hasNext();) {
            DialingRule rule = (DialingRule) i.next();
            assertEquals("name", rule.getName());
            assertEquals("description", rule.getDescription());
        }
    }

    public void testGetAvailableGateways() {
        List allGateways = new ArrayList();
        DialingRule rule1 = new CustomDialingRule();
        DialingRule rule2 = new CustomDialingRule();
        Collection availableGateways = rule1.getAvailableGateways(allGateways);
        assertEquals(0, availableGateways.size());
        availableGateways = rule2.getAvailableGateways(allGateways);
        assertEquals(0, availableGateways.size());

        Gateway g1 = new Gateway();
        g1.setUniqueId();
        Gateway g2 = new Gateway();
        g2.setUniqueId();
        Gateway g3 = new Gateway();
        g3.setUniqueId();
        allGateways.add(g1);
        allGateways.add(g2);
        allGateways.add(g3);

        rule1.addGateway(g2);
        rule2.addGateway(g1);
        rule2.addGateway(g3);

        availableGateways = rule1.getAvailableGateways(allGateways);
        assertEquals(2, availableGateways.size());
        assertTrue(availableGateways.contains(g1));
        assertFalse(availableGateways.contains(g2));
        assertTrue(availableGateways.contains(g3));

        availableGateways = rule2.getAvailableGateways(allGateways);
        assertEquals(1, availableGateways.size());
        assertFalse(availableGateways.contains(g1));
        assertTrue(availableGateways.contains(g2));
        assertFalse(availableGateways.contains(g3));
    }

    public void testAddGateway() {
        Gateway g = new Gateway();
        g.setUniqueId();

        DialingRule rule = new CustomDialingRule();
        assertTrue(rule.addGateway(g));
        List gateways = rule.getGateways();
        assertEquals(1, gateways.size());
        assertEquals(g, gateways.get(0));

        // add the same gateway again
        assertFalse(rule.addGateway(g));
        gateways = rule.getGateways();
        assertEquals(1, gateways.size());
        assertEquals(g, gateways.get(0));
    }
}

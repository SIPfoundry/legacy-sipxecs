/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;

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

    public void testPreserveGatewayPosition() {
        Gateway g1 = new Gateway();
        g1.setUniqueId();
        g1.setName("first");

        Gateway g2 = new Gateway();
        g2.setUniqueId();
        g2.setName("second");

        DialingRule rule = new CustomDialingRule();
        assertTrue(rule.addGateway(g1));
        assertTrue(rule.addGateway(g2));
        List gateways = rule.getGateways();
        assertEquals(2, gateways.size());
        assertEquals(g1, gateways.get(0));
        assertEquals(g2, gateways.get(1));

        // add the same gateway again
        rule.addGateway(g1);
        gateways = rule.getGateways();
        assertEquals(2, gateways.size());
        assertEquals(g1, gateways.get(0));
        assertEquals(g2, gateways.get(1));
    }

    public void testRemoveGateways() {
        Gateway firstGateway = new Gateway();
        firstGateway.setUniqueId();
        Gateway secondGateway = new Gateway();
        secondGateway.setUniqueId();

        DialingRule rule = new InternationalRule();
        rule.addGateway(firstGateway);
        rule.addGateway(secondGateway);
        rule.setEnabled(true);

        rule.removeGateways(Arrays.asList(firstGateway.getId()));
        assertTrue(rule.isEnabled());
        assertEquals(1, rule.getGateways().size());

        rule.removeGateways(Arrays.asList(secondGateway.getId()));
        assertFalse(rule.isEnabled());
        assertEquals(0, rule.getGateways().size());
    }

    public void testGetEnabledGateways() {
        Gateway g1 = new Gateway();
        g1.setUniqueId();
        Gateway g2 = new Gateway();
        g2.setUniqueId();
        Gateway g3 = new Gateway();
        g3.setUniqueId();

        DialingRule rule = new InternationalRule();
        assertTrue(rule.getEnabledGateways().isEmpty());

        rule.setGateways(Arrays.asList(g1, g2, g3));
        List<Gateway> eg = rule.getEnabledGateways();
        assertEquals(3, eg.size());
        assertTrue(eg.contains(g1));
        assertTrue(eg.contains(g2));
        assertTrue(eg.contains(g3));

        g2.setEnabled(false);
        eg = rule.getEnabledGateways();
        assertEquals(2, eg.size());
        assertTrue(eg.contains(g1));
        assertFalse(eg.contains(g2));
        assertTrue(eg.contains(g3));
    }

    public void testGetSiteTransforms() {
        Branch montrealSite = new Branch();
        montrealSite.setName("Montreal");

        Branch lisbonSite = new Branch();
        lisbonSite.setName("Lisbon");

        Gateway montreal = new Gateway();
        montreal.setUniqueId();
        montreal.setAddress("montreal.example.org");
        montreal.setBranch(montrealSite);
        montreal.setShared(false);

        Gateway montreal2 = new Gateway();
        montreal2.setUniqueId();
        montreal2.setAddress("montreal2.example.org");
        montreal2.setBranch(montrealSite);
        montreal2.setShared(false);

        Gateway lisbon = new Gateway();
        lisbon.setUniqueId();
        lisbon.setAddress("lisbon.example.org");
        lisbon.setBranch(lisbonSite);

        Gateway shared = new Gateway();
        shared.setUniqueId();
        shared.setAddress("example.org");
        shared.setShared(false);

        CustomDialingRule rule = new CustomDialingRule();
        rule.addGateway(shared);
        rule.addGateway(montreal);
        rule.addGateway(lisbon);
        rule.addGateway(montreal2);
        rule.setCallPattern(new CallPattern("444", CallDigits.NO_DIGITS));
        rule.setDialPatterns(Arrays.asList(new DialPattern("x", DialPattern.VARIABLE_DIGITS)));

        Map<String, List<Transform>> siteTransforms = rule.getSiteTransforms();

        assertEquals(3, siteTransforms.size());
        assertEquals(4, siteTransforms.get("Montreal").size());
        // lisbon, shared
        assertEquals(2, siteTransforms.get("Lisbon").size());
        // shared, lisbon
        assertEquals(2, siteTransforms.get("").size());
    }

    public void testGetCallType() {
        DummyDialingRule rule = new DummyDialingRule();

        assertNull(rule.getCallTag());

        rule.setPermissionNames("LongDistanceDialing");
        assertSame(CallTag.LD, rule.getCallTag());

        rule.setType(DialingRuleType.ATTENDANT);
        assertSame(CallTag.AA, rule.getCallTag());
    }

    private static final class DummyDialingRule extends DialingRule {
        private DialingRuleType m_type;
        private List<String> m_permissionNames = Collections.emptyList();

        public DummyDialingRule() {
            PermissionManagerImpl impl = new PermissionManagerImpl();
            impl.setModelFilesContext(TestHelper.getModelFilesContext());
            setPermissionManager(impl);
        }

        public void setType(DialingRuleType type) {
            m_type = type;
        }

        @Override
        public List<String> getPermissionNames() {
            return m_permissionNames;
        }

        public void setPermissionNames(String... permissionNames) {
            m_permissionNames = Arrays.asList(permissionNames);
        }

        @Override
        public DialingRuleType getType() {
            return m_type;
        }

        @Override
        public String[] getPatterns() {
            return null;
        }

        @Override
        public Transform[] getTransforms() {
            return null;
        }

        @Override
        public boolean isGatewayAware() {
            return false;
        }

        @Override
        public boolean isInternal() {
            return false;
        }
    }
}

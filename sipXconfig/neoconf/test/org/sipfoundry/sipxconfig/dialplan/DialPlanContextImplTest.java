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

import java.util.Arrays;
import java.util.Collections;

import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.springframework.orm.hibernate3.HibernateTemplate;

import junit.framework.TestCase;

import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.verify;

/**
 * DialPlanContextImplTest
 */
public class DialPlanContextImplTest extends TestCase {

    public void testMoveRules() throws Exception {
        DialPlan plan = createMock(DialPlan.class);
        plan.moveRules(Collections.singletonList(new Integer(5)), 3);
        replay(plan);

        MockDialPlanContextImpl manager = new MockDialPlanContextImpl(plan);
        manager.moveRules(Collections.singletonList(new Integer(5)), 3);
        verify(plan);
    }

    public void testLikelyVoiceMail() {
        DialPlan plan = new DialPlan();
        DialPlanContextImpl manager = new MockDialPlanContextImpl(plan);
        assertEquals("101", manager.getVoiceMail());

        DialingRule[] rules = new DialingRule[] {
            new CustomDialingRule()
        };
        plan.setRules(Arrays.asList(rules));
        assertEquals("101", manager.getVoiceMail());

        InternalRule irule = new InternalRule();
        irule.setVoiceMail("2000");
        rules = new DialingRule[] {
            new CustomDialingRule(), irule
        };
        plan.setRules(Arrays.asList(rules));
        assertEquals("2000", manager.getVoiceMail());
    }

    public void testNoLikelyEmergencyInfo() {
        DialPlan plan = new DialPlan();
        DialPlanContextImpl manager = new MockDialPlanContextImpl(plan);

        // no rules
        assertNull(manager.getLikelyEmergencyInfo());

        // no gateway
        EmergencyRule emergency = new EmergencyRule();
        emergency.setEnabled(true);
        DialingRule[] rules = new DialingRule[] {
            emergency
        };
        emergency.setEmergencyNumber("sos");
        plan.setRules(Arrays.asList(rules));
        assertNull(manager.getLikelyEmergencyInfo());

        // disabled rule
        emergency.setEnabled(false);
        assertNull(manager.getLikelyEmergencyInfo());

        // gateway has routing (e.g. SBC)
        emergency.setEnabled(true);
        Gateway gatewayWithSbc = new Gateway() {
            @Override
            public String getRoute() {
                return "sbc.example.org";
            }
        };
        rules[0].setGateways(Collections.singletonList(gatewayWithSbc));
        assertNull(manager.getLikelyEmergencyInfo());
    }

    public void testLikelyEmergencyInfo() {
        DialPlan plan = new DialPlan();
        DialPlanContextImpl manager = new MockDialPlanContextImpl(plan);

        EmergencyRule emergency = new EmergencyRule();
        emergency.setEnabled(true);
        DialingRule[] rules = new DialingRule[] {
            emergency
        };
        emergency.setEmergencyNumber("sos");
        plan.setRules(Arrays.asList(rules));
        Gateway gateway = new Gateway();
        gateway.setAddress("pstn.example.org");
        emergency.setGateways(Collections.singletonList(gateway));

        EmergencyInfo info = manager.getLikelyEmergencyInfo();
        assertEquals("pstn.example.org", info.getAddress());
        assertEquals("sos", info.getNumber());
        assertNull(info.getPort());

        gateway.setAddressPort(9050);
        assertEquals((Integer) 9050, manager.getLikelyEmergencyInfo().getPort());
    }

    static class MockDialPlanContextImpl extends DialPlanContextImpl {
        private final DialPlan m_plan;

        MockDialPlanContextImpl(DialPlan plan) {
            m_plan = plan;
            setHibernateTemplate(createMock(HibernateTemplate.class));
        }

        @Override
        DialPlan getDialPlan() {
            return m_plan;
        }

        @Override
        public DialPlanActivationManager getDialPlanActivationManager() {
            DialPlanActivationManager dpam = createNiceMock(DialPlanActivationManager.class);
            return dpam;
        }
    }
}

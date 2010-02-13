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
import java.util.Calendar;
import java.util.List;
import java.util.TimeZone;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.ScheduledDay;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.WorkingHours;
import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.admin.forwarding.GeneralSchedule;
import org.sipfoundry.sipxconfig.admin.forwarding.Schedule;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.SipTrunk;
import org.sipfoundry.sipxconfig.permission.Permission;

/**
 * EmergencyRuleTest
 */
public class EmergencyRuleTest extends TestCase {
    private static final String VALIDTIME_PARAMS = "sipx-ValidTime=\"";
    private EmergencyRule m_rule;
    private Schedule m_schedule;

    @Override
    protected void setUp() throws Exception {
        m_schedule = new GeneralSchedule();
        m_schedule.setName("Custom schedule");
        WorkingHours[] hours = new WorkingHours[1];
        WorkingTime wt = new WorkingTime();
        hours[0] = new WorkingHours();
        Calendar cal = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        cal.set(2006, Calendar.DECEMBER, 31, 10, 12);
        hours[0].setStart(cal.getTime());
        cal.set(2006, Calendar.DECEMBER, 31, 11, 88);
        hours[0].setStop(cal.getTime());
        hours[0].setEnabled(true);
        hours[0].setDay(ScheduledDay.WEDNESDAY);
        wt.setWorkingHours(hours);
        wt.setEnabled(true);
        m_schedule.setWorkingTime(wt);

        m_rule = new EmergencyRule();
        m_rule.setEmergencyNumber("911");
        m_rule.setOptionalPrefix("9");

        Gateway g1 = new Gateway();
        g1.setAddress("sosgateway1.com");
        g1.setAddressPort(4000);
        Gateway g2 = new Gateway();
        g2.setAddress("sosgateway2.com");
        g2.setAddressPort(0);
        g2.setAddressTransport(Gateway.AddressTransport.TCP);
        g2.setPrefix("4321");
        Gateway g3 = new SipTrunk();
        g3.setAddress("sosgateway3.com");
        g3.setAddressPort(5555);
        SbcDevice sbcDevice = new SbcDevice();
        sbcDevice.setAddress("bridge.example.org");
        g3.setSbcDevice(sbcDevice);
        m_rule.setGateways(Arrays.asList(g1, g2, g3));
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
        assertEquals(3, transforms.length);
        FullTransform emergencyTransform = (FullTransform) transforms[0];
        assertEquals("911", emergencyTransform.getUser());
        assertEquals("sosgateway1.com:4000", emergencyTransform.getHost());
        assertNull(emergencyTransform.getUrlParams());
        assertEquals("q=0.95", emergencyTransform.getFieldParams()[0]);

        emergencyTransform = (FullTransform) transforms[1];
        assertEquals("4321911", emergencyTransform.getUser());
        assertEquals("sosgateway2.com", emergencyTransform.getHost());
        assertEquals("transport=tcp", emergencyTransform.getUrlParams()[0]);
        assertEquals("q=0.9", emergencyTransform.getFieldParams()[0]);

        emergencyTransform = (FullTransform) transforms[2];
        assertEquals("sosgateway3.com:5555", emergencyTransform.getHost());
        assertNull(emergencyTransform.getUrlParams());
        assertEquals("q=0.85", emergencyTransform.getFieldParams()[0]);
        assertEquals("route=bridge.example.org", emergencyTransform.getHeaderParams()[0]);
    }

    public void testGetTransformsWithSchedule() {
        m_rule.setSchedule(m_schedule);
        Transform[] transforms = m_rule.getTransforms();
        assertEquals(3, transforms.length);
        FullTransform emergencyTransform = (FullTransform) transforms[0];
        assertEquals("911", emergencyTransform.getUser());
        assertEquals("sosgateway1.com:4000", emergencyTransform.getHost());
        assertNull(emergencyTransform.getUrlParams());
        String[] fieldParams = emergencyTransform.getFieldParams();
        assertEquals(2, fieldParams.length);
        assertEquals("q=0.95", fieldParams[0]);
        assertTrue(fieldParams[1].startsWith(VALIDTIME_PARAMS));

        emergencyTransform = (FullTransform) transforms[1];
        assertEquals("4321911", emergencyTransform.getUser());
        assertEquals("sosgateway2.com", emergencyTransform.getHost());
        fieldParams = emergencyTransform.getFieldParams();
        assertEquals(2, fieldParams.length);
        assertEquals("q=0.9", fieldParams[0]);
        assertEquals("transport=tcp", emergencyTransform.getUrlParams()[0]);
        assertTrue(fieldParams[1].startsWith(VALIDTIME_PARAMS));

        emergencyTransform = (FullTransform) transforms[2];
        assertEquals("sosgateway3.com:5555", emergencyTransform.getHost());
        assertEquals("route=bridge.example.org", emergencyTransform.getHeaderParams()[0]);
        assertNull(emergencyTransform.getUrlParams());
        fieldParams = emergencyTransform.getFieldParams();
        assertEquals(2, fieldParams.length);
        assertEquals("q=0.85", fieldParams[0]);
        assertTrue(fieldParams[1].startsWith(VALIDTIME_PARAMS));
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
        assertEquals(3, gateways.size());
        ArrayList<DialingRule> rules = new ArrayList<DialingRule>();
        m_rule.appendToGenerationRules(rules);
        assertEquals(1, rules.size());
        DialingRule rule = rules.get(0);
        assertEquals(3, rule.getGateways().size());
        assertEquals(testDescription, rule.getDescription());
    }

    public void testAppendToGenerationRulesDisabled() {
        m_rule.setEnabled(false);
        ArrayList<DialingRule> rules = new ArrayList<DialingRule>();
        m_rule.appendToGenerationRules(rules);
        assertTrue(rules.isEmpty());
    }
}

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
import java.util.Calendar;
import java.util.Collections;
import java.util.List;
import java.util.TimeZone;

import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;

import org.sipfoundry.sipxconfig.TestHelper;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.admin.ScheduledDay;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.WorkingHours;
import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.forwarding.GeneralSchedule;
import org.sipfoundry.sipxconfig.admin.forwarding.Schedule;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.permission.PermissionName;

/**
 * LongDistanceRuleTest
 */
public class LongDistanceRuleTest extends TestCase {
    private static final String VALIDTIME_PARAMS = "sipx-ValidTime=\"";
    private LongDistanceRule m_rule;
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

        m_rule = new LongDistanceRule();
        m_rule.setEnabled(true);
        m_rule.setLongDistancePrefix("1");
        m_rule.setLongDistancePrefixOptional(false);
        m_rule.setPstnPrefix("9");
        m_rule.setPstnPrefixOptional(true);
        m_rule.setExternalLen(7);

        Gateway g = new Gateway();
        g.setAddress("longdistance.gateway.com");
        m_rule.setGateways(Collections.singletonList(g));
    }

    private DialingRule getGenerationRule(DialingRule rule) {
        List<DialingRule> list = new ArrayList<DialingRule>();
        rule.appendToGenerationRules(list);
        return list.get(0);
    }

    public void testGetPatterns() {
        DialingRule rule = getGenerationRule(m_rule);
        String[] patterns = rule.getPatterns();
        assertEquals(2, patterns.length);
        assertEquals("91xxxxxxx", patterns[0]);
        assertEquals("1xxxxxxx", patterns[1]);
    }

    public void testGetPatternsPstnPrefixRequired() {
        m_rule.setPstnPrefixOptional(false);
        m_rule.setLongDistancePrefixOptional(true);
        DialingRule rule = getGenerationRule(m_rule);
        String[] patterns = rule.getPatterns();
        assertEquals(2, patterns.length);
        assertEquals("91xxxxxxx", patterns[0]);
        assertEquals("9xxxxxxx", patterns[1]);
    }

    public void testGetTransformedPatterns() {
        Gateway gateway = new Gateway();
        DialingRule rule = getGenerationRule(m_rule);
        String[] patterns = rule.getTransformedPatterns(gateway);
        assertEquals(1, patterns.length);
        assertEquals("1xxxxxxx", patterns[0]);
        gateway.setPrefix("555");
        patterns = rule.getTransformedPatterns(gateway);
        assertEquals(1, patterns.length);
        assertEquals("5551xxxxxxx", patterns[0]);
    }

    public void testGetTransformedPatternsNoPermission() {
        Gateway gateway = new Gateway();
        m_rule.setPstnPrefixOptional(false);
        m_rule.setPstnPrefix(null);
        m_rule.setLongDistancePrefix("011");
        m_rule.setLongDistancePrefixOptional(false);
        m_rule.setExternalLen(-1);
        m_rule.setPermission(null);
        DialingRule rule = getGenerationRule(m_rule);
        String[] patterns = rule.getTransformedPatterns(gateway);
        assertEquals(1, patterns.length);
        assertEquals("011.", patterns[0]);
    }

    public void testGetTransforms() {
        DialingRule rule = getGenerationRule(m_rule);
        Transform[] transforms = rule.getTransforms();
        assertEquals(1, transforms.length);
        FullTransform transform = (FullTransform) transforms[0];
        assertEquals("1{vdigits}", transform.getUser());
        assertEquals("longdistance.gateway.com", transform.getHost());
        EmergencyRuleTest.assertUrlParams(transform.getUrlParams());
    }

    public void testGetTransformsWithSchedule() {
        m_rule.setSchedule(m_schedule);
        DialingRule rule = getGenerationRule(m_rule);
        Transform[] transforms = rule.getTransforms();
        assertEquals(1, transforms.length);
        FullTransform transform = (FullTransform) transforms[0];
        assertEquals("1{vdigits}", transform.getUser());
        assertEquals("longdistance.gateway.com", transform.getHost());
        String[] fieldParams = transform.getFieldParams();
        assertEquals(2, fieldParams.length);
        assertTrue(fieldParams[0].startsWith("q="));
        assertTrue(fieldParams[1].startsWith(VALIDTIME_PARAMS));
    }

    public void testGetPermissionNames() {
        DialingRule rule = getGenerationRule(m_rule);
        List<String> permissions = rule.getPermissionNames();
        assertEquals(1, permissions.size());
        assertEquals(PermissionName.LONG_DISTANCE_DIALING.getName(), permissions.get(0));
    }

    public void testNoPermissionRequired() {
        m_rule.setPermissionName(PermissionName.INTERNATIONAL_DIALING.getName());
        DialingRule rule = getGenerationRule(m_rule);
        assertEquals(1, rule.getPermissionNames().size());
        m_rule.setPermission(null);
        rule = getGenerationRule(m_rule);
        assertEquals(0, rule.getPermissionNames().size());
    }

    public void testCalculateDialPatterns() {
        m_rule.setLongDistancePrefixOptional(true);
        List<DialPattern> list = m_rule.calculateDialPatterns("305");
        assertEquals(4, list.size());
        assertEquals("91305xxxx", list.get(0).calculatePattern());
        assertEquals("1305xxxx", list.get(1).calculatePattern());
        assertEquals("9305xxxx", list.get(2).calculatePattern());
        assertEquals("305xxxx", list.get(3).calculatePattern());
    }

    public void testCalculateDialPatternsLongAreaCode() {
        m_rule.setLongDistancePrefixOptional(true);
        m_rule.setExternalLen(4);
        List<DialPattern> list = m_rule.calculateDialPatterns("305305305");
        assertEquals(4, list.size());
        assertEquals("913053", list.get(0).calculatePattern());
        assertEquals("13053", list.get(1).calculatePattern());
        assertEquals("93053", list.get(2).calculatePattern());
        assertEquals("3053", list.get(3).calculatePattern());
    }

    public void testCalculateDialPatternsEmptyPrefixes() {
        m_rule.setLongDistancePrefixOptional(true);
        m_rule.setPstnPrefix(null);
        m_rule.setLongDistancePrefix(null);
        List<DialPattern> list = m_rule.calculateDialPatterns("305");
        assertEquals(1, list.size());
        assertEquals("305xxxx", list.get(0).calculatePattern());
    }

    public void testCalculateDialPatternsPstnPrefixRequired() {
        m_rule.setLongDistancePrefixOptional(true);
        m_rule.setPstnPrefixOptional(false);
        List<DialPattern> list = m_rule.calculateDialPatterns("305");
        assertEquals(2, list.size());
        assertEquals("91305xxxx", list.get(0).calculatePattern());
        assertEquals("9305xxxx", list.get(1).calculatePattern());
        m_rule.setPstnPrefix(null);
        list = m_rule.calculateDialPatterns("305");
        assertEquals(2, list.size());
        assertEquals("1305xxxx", list.get(0).calculatePattern());
        assertEquals("305xxxx", list.get(1).calculatePattern());
    }

    public void testCalculateDialPatternsLongDistancePrefixRequired() {
        m_rule.setLongDistancePrefixOptional(false);
        List<DialPattern> list = m_rule.calculateDialPatterns("305");
        assertEquals(2, list.size());
        assertEquals("91305xxxx", list.get(0).calculatePattern());
        assertEquals("1305xxxx", list.get(1).calculatePattern());
        m_rule.setLongDistancePrefix(null);
        list = m_rule.calculateDialPatterns("305");
        assertEquals(2, list.size());
        assertEquals("9305xxxx", list.get(0).calculatePattern());
        assertEquals("305xxxx", list.get(1).calculatePattern());
    }

    public void testCalculateEmptyPstnPrefix() {
        m_rule.setLongDistancePrefixOptional(false);
        m_rule.setLongDistancePrefix("011");
        m_rule.setPstnPrefixOptional(false);
        m_rule.setPstnPrefix(null);
        m_rule.setExternalLen(-1);
        assertEquals("011{vdigits}", m_rule.calculateCallPattern("").calculatePattern());
        List<DialPattern> list = m_rule.calculateDialPatterns("");
        assertEquals(1, list.size());
        assertEquals("011.", list.get(0).calculatePattern());
    }

    public void testCalculateEmptyLongDistancePrefix() {
        m_rule.setLongDistancePrefixOptional(false);
        m_rule.setLongDistancePrefix(null);
        m_rule.setPstnPrefixOptional(false);
        m_rule.setPstnPrefix("1234");
        m_rule.setExternalLen(-1);
        assertEquals("{vdigits}", m_rule.calculateCallPattern("").calculatePattern());
        List<DialPattern> list = m_rule.calculateDialPatterns("");
        assertEquals(1, list.size());
        assertEquals("1234.", list.get(0).calculatePattern());
    }

    public void testAnyNumberOfDigits() {
        m_rule.setExternalLen(-1);
        List<DialPattern> list = m_rule.calculateDialPatterns("305");
        assertEquals(2, list.size());
        assertEquals("91305.", list.get(0).calculatePattern());
        assertEquals("1305.", list.get(1).calculatePattern());
    }

    public void testCalculateCallPattern() {
        CallPattern callPattern = m_rule.calculateCallPattern("503");
        assertEquals("1503{vdigits}", callPattern.calculatePattern());
    }

    public void testAreaCodes() {
        m_rule.setLongDistancePrefixOptional(true);
        m_rule.setAreaCodes("  305 411,222");
        m_rule.setDescription("testDescription");
        List<DialingRule> list = new ArrayList<DialingRule>();
        m_rule.appendToGenerationRules(list);
        assertEquals(3, list.size());
        for (DialingRule r : list) {
            assertEquals("testDescription", r.getDescription());
            String[] patterns = r.getPatterns();
            assertEquals(4, patterns.length);
            assertTrue(patterns[0].endsWith("xxxx"));
            assertTrue(patterns[1].endsWith("xxxx"));
            assertTrue(patterns[2].endsWith("xxxx"));
            assertTrue(patterns[3].endsWith("xxxx"));
            List<String> permissions = r.getPermissionNames();
            assertEquals(1, permissions.size());
            assertEquals(PermissionName.LONG_DISTANCE_DIALING.getName(), permissions.get(0));
            Transform[] transforms = r.getTransforms();
            assertEquals(1, transforms.length);
            FullTransform transform = (FullTransform) transforms[0];
            assertTrue(transform.getUser().endsWith("{vdigits}"));
            assertEquals("longdistance.gateway.com", transform.getHost());
            EmergencyRuleTest.assertUrlParams(transform.getUrlParams());
        }
    }

    public void testTollFreeDialing() {
        m_rule.setPermissionName(PermissionName.TOLL_FREE_DIALING.getName());
        DialingRule generationRule = getGenerationRule(m_rule);
        List<String> permissions = generationRule.getPermissionNames();
        assertEquals(PermissionName.TOLL_FREE_DIALING.getName(), permissions.get(0));
    }

    public void testAppendToGenerationRules() {
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        m_rule.setPermissionManager(pm);
        m_rule.setPermissionName(PermissionName.LOCAL_DIALING.getName());

        ArrayList<DialingRule> rules = new ArrayList<DialingRule>();
        m_rule.appendToGenerationRules(rules);
        assertEquals(1, rules.size());
        DialingRule rule = rules.get(0);
        assertEquals(PermissionName.LOCAL_DIALING.getName(), rule.getPermissions().get(0).getName());
    }
}

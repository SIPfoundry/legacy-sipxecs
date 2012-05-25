/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan;


import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.TimeZone;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Transformer;
import org.apache.commons.lang.ArrayUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.ScheduledDay;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.dialplan.attendant.Holiday;
import org.sipfoundry.sipxconfig.dialplan.attendant.ScheduledAttendant;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime.WorkingHours;
import org.sipfoundry.sipxconfig.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.forwarding.GeneralSchedule;
import org.sipfoundry.sipxconfig.forwarding.Schedule;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

/**
 * DialPlanContextImplTest
 */
public class DialPlanContextTestIntegration extends IntegrationTestCase {
    private static final int DEFAULT_DIAL_PLAN_SIZE = 8;

    private DialPlanContext m_dialPlanContext;
    private GatewayContext m_gatewayContext;
    private ForwardingContext m_forwardingContext;
    private AutoAttendantManager m_autoAttendantManager;
    private DialPlanSetup m_dialPlanSetup;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testAddDeleteRule() {
        m_dialPlanSetup.setupDefaultRegion();
        // TODO - replace with IDialingRule mocks

        DialingRule r1 = new CustomDialingRule();
        r1.setName("a1");

        Schedule schedule = new GeneralSchedule();
        schedule.setName("R1 Schedule");
        WorkingHours[] hours = new WorkingHours[1];
        WorkingTime wt = new WorkingTime();
        hours[0] = new WorkingHours();
        Calendar cal = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        cal.set(2006, Calendar.DECEMBER, 31, 10, 00);
        hours[0].setStart(cal.getTime());
        cal.set(2006, Calendar.DECEMBER, 31, 11, 00);
        hours[0].setStop(cal.getTime());
        hours[0].setEnabled(true);
        hours[0].setDay(ScheduledDay.WEDNESDAY);
        wt.setWorkingHours(hours);
        wt.setEnabled(true);
        schedule.setWorkingTime(wt);
        m_forwardingContext.saveSchedule(schedule);
        r1.setSchedule(schedule);

        CustomDialingRule r2 = new CustomDialingRule();
        r2.setName("a2");
        r2.setPermissionNames(Collections.singletonList(PermissionName.VOICEMAIL.getName()));

        m_dialPlanContext.storeRule(r1);
        assertEquals(1 + DEFAULT_DIAL_PLAN_SIZE, m_dialPlanContext.getRules().size());
        assertEquals("R1 Schedule", ((CustomDialingRule) m_dialPlanContext.load(DialingRule.class, r1.getId()))
                .getSchedule().getName());
        m_dialPlanContext.storeRule(r2);
        assertEquals(2 + DEFAULT_DIAL_PLAN_SIZE, m_dialPlanContext.getRules().size());

        CustomDialingRule r = (CustomDialingRule) m_dialPlanContext.load(DialingRule.class, r2.getId());
        assertEquals(r2.getPermissionNames().get(0), r.getPermissionNames().get(0));

        Integer id1 = r1.getId();

        m_dialPlanContext.deleteRules(Collections.singletonList(id1));
        Collection rules = m_dialPlanContext.getRules();
        assertTrue(rules.contains(r2));
        assertFalse(rules.contains(r1));
        assertEquals(1 + DEFAULT_DIAL_PLAN_SIZE, rules.size());
    }

    public void testAddRuleDuplicateName() {
        m_dialPlanSetup.setupDefaultRegion();

        DialingRule r1 = new CustomDialingRule();
        r1.setName("a1");
        DialingRule r2 = new CustomDialingRule();
        r2.setName("a1");

        m_dialPlanContext.storeRule(r1);

        try {
            m_dialPlanContext.storeRule(r2);
            fail("Exception expected");
        } catch (UserException e) {
            assertEquals(e.getLocalizedMessage(),"&error.nameInUse.long");
        }
    }

    public void testDefaultRuleTypes() throws Exception {
        m_dialPlanSetup.setupDefaultRegion();

        assertEquals(8, countRowsInTable("dialing_rule"));
        // FIXME: test agains the real data - need to remove ids...
        // IncludeTableFilter filter = new IncludeTableFilter();
        // filter.includeTable("*dialing_rule");
        //IDataSet set = new FilteredDataSet(filter, TestHelper.getConnection().createDataSet());
        // ITable table = set.getTable("dialing_rule");
        // IDataSet reference = new FilteredDataSet(filter, TestHelper
        // .loadDataSet("dialplan/defaultFlexibleDialPlan.xml"));
        // Assertion.assertEquals(set, reference);

        assertEquals(1, countRowsInTable("attendant_dialing_rule"));
        Map<String, Object> actual = db().queryForMap("select * from attendant_dialing_rule");
        assertEquals("operator 0", actual.get("attendant_aliases"));
    }

    public void testDuplicateRules() throws Exception {
        m_dialPlanSetup.setupDefaultRegion();
        DialingRule r1 = new CustomDialingRule();
        r1.setName("a1");
        m_dialPlanContext.storeRule(r1);
        assertFalse(r1.isNew());
        m_dialPlanContext.duplicateRules(Collections.singletonList(r1.getId()));
        commit();
        assertEquals(2 + DEFAULT_DIAL_PLAN_SIZE, m_dialPlanContext.getRules().size());
        assertEquals(2 + DEFAULT_DIAL_PLAN_SIZE, countRowsInTable("dialing_rule"));
    }

    public void testDuplicateDefaultRules() throws Exception {
        m_dialPlanSetup.setupDefaultRegion();
        List rules = m_dialPlanContext.getRules();
        Transformer bean2id = new BeanWithId.BeanToId();
        Collection ruleIds = CollectionUtils.collect(rules, bean2id);
        m_dialPlanContext.duplicateRules(ruleIds);
        commit();
        assertEquals(ruleIds.size() * 2, m_dialPlanContext.getRules().size());
        assertEquals(ruleIds.size() * 2, countRowsInTable("dialing_rule"));
    }

    public void testIsAliasInUse() throws Exception {
        TestHelper.cleanInsert("dialplan/seedDialPlanWithAttendant.xml");
        // voicemail extension
        assertTrue(m_dialPlanContext.isAliasInUse("100"));
        assertTrue(m_dialPlanContext.isAliasInUse("123456789"));
        // a random extension that should not be in use
        assertFalse(m_dialPlanContext.isAliasInUse("200"));
    }

    public void testIsAliasInUseAttendant() throws Exception {
        TestHelper.cleanInsertFlat("dialplan/attendant_rule.db.xml");
        assertTrue(m_dialPlanContext.isAliasInUse("333")); // auto attendant extension
        assertTrue(m_dialPlanContext.isAliasInUse("123456789")); // auto attendant extension
        assertTrue(m_dialPlanContext.isAliasInUse("operator")); // auto attendant alias
        assertTrue(m_dialPlanContext.isAliasInUse("0")); // auto attendant alias
    }

    public void testGetBeanIdsOfObjectsWithAlias() throws Exception {
        TestHelper.cleanInsert("dialplan/seedDialPlanWithAttendant.xml");
        // voicemail extension and did
        assertTrue(m_dialPlanContext.getBeanIdsOfObjectsWithAlias("100").size() == 1);
        assertTrue(m_dialPlanContext.getBeanIdsOfObjectsWithAlias("123456789").size() == 1);
        // a random extension that should not be in use
        assertTrue(m_dialPlanContext.getBeanIdsOfObjectsWithAlias("200").size() == 0);
    }

    public void testGetBeanIdsOfObjectsWithAliasAttendant() throws Exception {
        TestHelper.cleanInsertFlat("dialplan/attendant_rule.db.xml");
        // auto attendant extension and did
        assertTrue(m_dialPlanContext.getBeanIdsOfObjectsWithAlias("333").size() == 1);
        assertTrue(m_dialPlanContext.getBeanIdsOfObjectsWithAlias("123456789").size() == 1);
        // auto attendant alias
        assertTrue(m_dialPlanContext.getBeanIdsOfObjectsWithAlias("operator").size() == 1);

        assertTrue(m_dialPlanContext.getBeanIdsOfObjectsWithAlias("0").size() == 1);
    }

    public void testLoadAttendantRule() throws Exception {
        TestHelper.cleanInsertFlat("dialplan/attendant_rule.db.xml");

        DialingRule rule = m_dialPlanContext.getRule(new Integer(2002));
        assertTrue(rule instanceof AttendantRule);
        AttendantRule ar = (AttendantRule) rule;
        assertTrue(ar.getAfterHoursAttendant().isEnabled());
        assertFalse(ar.getWorkingTimeAttendant().isEnabled());
        assertTrue(ar.getHolidayAttendant().isEnabled());

        assertEquals(2, ar.getHolidayAttendant().getDates().size());

        // This test relies on assumption java and postgres timezones match, which is normally an ok 
        // assumption unless some other unit test in the suite calls TimeZone.setTimeZone...which they do.
        //
        // We could fix those tests to restore tz, but another test could be written someday 
        // that unknowingly does the same thing.
        // 
        //assertEquals("19:25", ar.getWorkingTimeAttendant().getWorkingHours()[4].getStopTime());
    }

    public void testStoreAttendantRule() throws Exception {
        TestHelper.cleanInsert("dialplan/seedDialPlanWithAttendant.xml");
        AutoAttendant autoAttendant = m_autoAttendantManager.getAutoAttendant(new Integer(2000));

        m_dialPlanSetup.setupDefaultRegion();

        ScheduledAttendant sa = new ScheduledAttendant();
        sa.setAttendant(autoAttendant);

        DateFormat format = new SimpleDateFormat("MM/dd/yyy");

        Holiday holiday = new Holiday();
        holiday.setAttendant(autoAttendant);
        holiday.addDay(format.parse("01/01/2005"));
        holiday.addDay(format.parse("06/06/2005"));
        holiday.addDay(format.parse("12/24/2005"));

        WorkingTime wt = new WorkingTime();
        wt.setAttendant(autoAttendant);
        WorkingHours[] workingHours = wt.getWorkingHours();
        Date stop = workingHours[4].getStop();
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(stop);
        calendar.add(Calendar.HOUR_OF_DAY, -1);
        workingHours[4].setStop(calendar.getTime());

        AttendantRule rule = new AttendantRule();
        rule.setName("myattendantschedule");
        rule.setAfterHoursAttendant(sa);
        rule.setHolidayAttendant(holiday);
        rule.setWorkingTimeAttendant(wt);

        m_dialPlanContext.storeRule(rule);
        commit();

        // HACK: multiple by 2 since there is already an auto attendant in default plan
        assertEquals(1 * 2, countRowsInTable("attendant_dialing_rule"));
        assertEquals(7 * 2, countRowsInTable("attendant_working_hours"));
        assertEquals(3, countRowsInTable("holiday_dates"));
    }

    public void testSaveExtensionThatIsDuplicateAlias() throws Exception {
        TestHelper.cleanInsertFlat("dialplan/attendant_rule.db.xml");
        AttendantRule ar = new AttendantRule();
        ar.setName("autodafe");
        ar.setExtension("0");
        try {
            m_dialPlanContext.storeRule(ar);
            fail();
        } catch (UserException e) {
            // this is expected
            assertEquals(e.getMessage(),"&error.extensionInUse");
        }
    }

    public void testSaveAliasThatIsDuplicateAlias() throws Exception {
        TestHelper.cleanInsertFlat("dialplan/attendant_rule.db.xml");
        AttendantRule ar = new AttendantRule();
        ar.setName("autodafe");
        ar.setExtension("222");
        ar.setAttendantAliases("operator");
        try {
            m_dialPlanContext.storeRule(ar);
            fail();
        } catch (UserException e) {
            // this is expected
            assertEquals(e.getMessage(),"&error.aliasCollisionException");
        }
    }

    /**
     * Tests the getRulesForGateway() method to ensure it only returns rules that are being used
     * with the specified gateway.
     */
    public void getRulesForGateway() throws Exception {
        TestHelper.cleanInsertFlat("dialplan/dialPlanGatewayAssociations.db.xml");
        Gateway gateway = m_gatewayContext.getGateway(1001);

        // Get our three rules we're testing with
        DialingRule rule0 = m_dialPlanContext.getRule(0);
        DialingRule rule1 = m_dialPlanContext.getRule(1);
        DialingRule rule2 = m_dialPlanContext.getRule(2);

        // The result of getRulesForGateway() should contain only rules 0 and 1,
        // since they contain the gateway.
        List<DialingRule> rulesForGateway = m_dialPlanContext.getRulesForGateway(gateway.getId());
        assertEquals(2, rulesForGateway.size());
        assertTrue(rulesForGateway.contains(rule0));
        assertTrue(rulesForGateway.contains(rule1));
        assertFalse(rulesForGateway.contains(rule2));
    }

    /**
     * Tests the getAvailableRules() method to ensure it only returns rules that are NOT being
     * used with the specified gateway.
     */
    public void testGetAvailableRules() throws Exception {
        TestHelper.cleanInsertFlat("dialplan/dialPlanGatewayAssociations.db.xml");
        Gateway gateway = m_gatewayContext.getGateway(1001);

        DialingRule rule0 = m_dialPlanContext.getRule(0);
        DialingRule rule1 = m_dialPlanContext.getRule(1);
        DialingRule rule2 = m_dialPlanContext.getRule(2);

        DialingRule attendantRule = m_dialPlanContext.getRule(3);
        DialingRule emergencyRule = m_dialPlanContext.getRule(4);
        DialingRule internalRule = m_dialPlanContext.getRule(5);
        DialingRule internationalRule = m_dialPlanContext.getRule(6);
        DialingRule localRule = m_dialPlanContext.getRule(7);
        DialingRule longDistanceRule = m_dialPlanContext.getRule(8);
        DialingRule siteToSiteRule = m_dialPlanContext.getRule(9);

        DialingRule[] nonGatewayAwareRules = {
            attendantRule, internalRule
        };
        DialingRule[] gatewayAwareRules = {
            rule2, emergencyRule, internationalRule, localRule, longDistanceRule, siteToSiteRule
        };

        List<DialingRule> availableRules = m_dialPlanContext.getAvailableRules(gateway.getId());
        assertEquals(gatewayAwareRules.length, availableRules.size());
        assertFalse(availableRules.contains(rule0));
        assertFalse(availableRules.contains(rule1));

        for (DialingRule rule : gatewayAwareRules) {
            assertTrue(availableRules.contains(rule));
        }

        for (DialingRule rule : nonGatewayAwareRules) {
            assertFalse(availableRules.contains(rule));
        }
    }

    public void testGet() {
        String[] dialPlanBeans = m_dialPlanContext.getDialPlanBeans();
        assertTrue(dialPlanBeans.length > 0);
        assertTrue(ArrayUtils.contains(dialPlanBeans, "na.dialPlan"));
    }

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public void setGatewayContext(GatewayContext gatewayContext) {
        m_gatewayContext = gatewayContext;
    }

    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }

    public void setAutoAttendantManager(AutoAttendantManager autoAttendantManager) {
        m_autoAttendantManager = autoAttendantManager;
    }

    public void setDialPlanSetup(DialPlanSetup dialPlanSetup) {
        m_dialPlanSetup = dialPlanSetup;
    }
}

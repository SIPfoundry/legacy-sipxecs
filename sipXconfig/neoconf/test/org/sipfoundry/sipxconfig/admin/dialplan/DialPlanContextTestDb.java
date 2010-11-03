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

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Transformer;
import org.apache.commons.lang.ArrayUtils;
import org.dbunit.dataset.FilteredDataSet;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.filter.IncludeTableFilter;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.ScheduledDay;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.Holiday;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.ScheduledAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.WorkingHours;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.forwarding.GeneralSchedule;
import org.sipfoundry.sipxconfig.admin.forwarding.Schedule;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.springframework.context.ApplicationContext;

/**
 * DialPlanContextImplTest
 */
public class DialPlanContextTestDb extends SipxDatabaseTestCase {
    private static final int DEFAULT_DIAL_PLAN_SIZE = 8;

    private DialPlanContext m_context;
    private GatewayContext m_gatewayContext;
    private ForwardingContext m_fwdContext;
    private AutoAttendantManager m_autoAttendantManager;
    private ResetDialPlanTask m_resetDialPlanTask;

    @Override
    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_gatewayContext = (GatewayContext) appContext.getBean(GatewayContext.CONTEXT_BEAN_NAME);
        m_context = (DialPlanContext) appContext.getBean(DialPlanContext.CONTEXT_BEAN_NAME);
        m_fwdContext = (ForwardingContext) appContext.getBean(ForwardingContext.CONTEXT_BEAN_NAME);
        m_autoAttendantManager = (AutoAttendantManager) appContext.getBean("autoAttendantManager");
        m_resetDialPlanTask = (ResetDialPlanTask) appContext.getBean("resetDialPlanTask");
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testAddDeleteRule() {
        m_resetDialPlanTask.reset(true);
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
        m_fwdContext.saveSchedule(schedule);
        m_fwdContext.flush();
        r1.setSchedule(schedule);

        CustomDialingRule r2 = new CustomDialingRule();
        r2.setName("a2");
        r2.setPermissionNames(Collections.singletonList(PermissionName.VOICEMAIL.getName()));

        m_context.storeRule(r1);
        assertEquals(1 + DEFAULT_DIAL_PLAN_SIZE, m_context.getRules().size());
        assertEquals("R1 Schedule", ((CustomDialingRule) m_context.load(DialingRule.class, r1.getId()))
                .getSchedule().getName());
        m_context.storeRule(r2);
        assertEquals(2 + DEFAULT_DIAL_PLAN_SIZE, m_context.getRules().size());

        CustomDialingRule r = (CustomDialingRule) m_context.load(DialingRule.class, r2.getId());
        assertEquals(r2.getPermissionNames().get(0), r.getPermissionNames().get(0));

        Integer id1 = r1.getId();

        m_context.deleteRules(Collections.singletonList(id1));
        Collection rules = m_context.getRules();
        assertTrue(rules.contains(r2));
        assertFalse(rules.contains(r1));
        assertEquals(1 + DEFAULT_DIAL_PLAN_SIZE, rules.size());
    }

    public void testAddRuleDuplicateName() {
        m_resetDialPlanTask.reset(true);

        DialingRule r1 = new CustomDialingRule();
        r1.setName("a1");
        DialingRule r2 = new CustomDialingRule();
        r2.setName("a1");

        m_context.storeRule(r1);

        try {
            m_context.storeRule(r2);
            fail("Exception expected");
        } catch (UserException e) {
            assertTrue(e.getLocalizedMessage().indexOf("a1") > 0);
        }
    }

    public void testMultipleReset() throws Exception {
        // strange errors if resetting more than once
        m_resetDialPlanTask.reset(true);
        m_resetDialPlanTask.reset(false);
        m_resetDialPlanTask.reset(true);
    }

    public void testDefaultRuleTypes() throws Exception {
        m_resetDialPlanTask.reset(true);

        IncludeTableFilter filter = new IncludeTableFilter();
        filter.includeTable("*dialing_rule");

        IDataSet set = new FilteredDataSet(filter, TestHelper.getConnection().createDataSet());
        ITable table = set.getTable("dialing_rule");
        assertEquals(8, table.getRowCount());
        // FIXME: test agains the real data - need to remove ids...
        // IDataSet reference = new FilteredDataSet(filter, TestHelper
        // .loadDataSet("admin/dialplan/defaultFlexibleDialPlan.xml"));
        // Assertion.assertEquals(set, reference);

        ITable internal = set.getTable("attendant_dialing_rule");
        assertEquals(1, internal.getRowCount());
        assertEquals("operator 0", internal.getValue(0, "attendant_aliases"));
    }

    public void testDuplicateRules() throws Exception {
        m_resetDialPlanTask.reset(true);

        DialingRule r1 = new CustomDialingRule();
        r1.setName("a1");
        m_context.storeRule(r1);
        assertFalse(r1.isNew());

        m_context.duplicateRules(Collections.singletonList(r1.getId()));

        assertEquals(2 + DEFAULT_DIAL_PLAN_SIZE, m_context.getRules().size());

        IDataSet set = TestHelper.getConnection().createDataSet();
        ITable table = set.getTable("dialing_rule");
        assertEquals(2 + DEFAULT_DIAL_PLAN_SIZE, table.getRowCount());
    }

    public void testDuplicateDefaultRules() throws Exception {
        m_resetDialPlanTask.reset(true);

        List rules = m_context.getRules();

        Transformer bean2id = new BeanWithId.BeanToId();

        Collection ruleIds = CollectionUtils.collect(rules, bean2id);

        m_context.duplicateRules(ruleIds);

        assertEquals(ruleIds.size() * 2, m_context.getRules().size());

        IDataSet set = TestHelper.getConnection().createDataSet();
        ITable table = set.getTable("dialing_rule");
        assertEquals(ruleIds.size() * 2, table.getRowCount());
    }

    public void testIsAliasInUse() throws Exception {
        TestHelper.cleanInsert("admin/dialplan/seedDialPlanWithAttendant.xml");
        // voicemail extension
        assertTrue(m_context.isAliasInUse("100"));
        assertTrue(m_context.isAliasInUse("123456789"));
        // a random extension that should not be in use
        assertFalse(m_context.isAliasInUse("200"));
    }

    public void testIsAliasInUseAttendant() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/attendant_rule.db.xml");
        assertTrue(m_context.isAliasInUse("333")); // auto attendant extension
        assertTrue(m_context.isAliasInUse("123456789")); // auto attendant extension
        assertTrue(m_context.isAliasInUse("operator")); // auto attendant alias
        assertTrue(m_context.isAliasInUse("0")); // auto attendant alias
    }

    public void testGetBeanIdsOfObjectsWithAlias() throws Exception {
        TestHelper.cleanInsert("admin/dialplan/seedDialPlanWithAttendant.xml");
        // voicemail extension and did
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("100").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("123456789").size() == 1);
        // a random extension that should not be in use
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("200").size() == 0);
    }

    public void testGetBeanIdsOfObjectsWithAliasAttendant() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/attendant_rule.db.xml");
        // auto attendant extension and did
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("333").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("123456789").size() == 1);
        // auto attendant alias
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("operator").size() == 1);

        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("0").size() == 1);
    }

    public void testLoadAttendantRule() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/attendant_rule.db.xml");

        DialingRule rule = m_context.getRule(new Integer(2002));
        assertTrue(rule instanceof AttendantRule);
        AttendantRule ar = (AttendantRule) rule;
        assertTrue(ar.getAfterHoursAttendant().isEnabled());
        assertFalse(ar.getWorkingTimeAttendant().isEnabled());
        assertTrue(ar.getHolidayAttendant().isEnabled());

        assertEquals(2, ar.getHolidayAttendant().getDates().size());

        assertEquals("19:25", ar.getWorkingTimeAttendant().getWorkingHours()[4].getStopTime());
    }

    public void testStoreAttendantRule() throws Exception {
        TestHelper.cleanInsert("admin/dialplan/seedDialPlanWithAttendant.xml");
        AutoAttendant autoAttendant = m_autoAttendantManager.getAutoAttendant(new Integer(2000));

        m_resetDialPlanTask.reset(true);

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

        m_context.storeRule(rule);

        // HACK: multiple by 2 since there is already an auto attendant in default plan
        assertEquals(1 * 2, getConnection().getRowCount("attendant_dialing_rule"));
        assertEquals(7 * 2, getConnection().getRowCount("attendant_working_hours"));
        assertEquals(3, getConnection().getRowCount("holiday_dates"));
    }

    public void testSaveExtensionThatIsDuplicateAlias() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/attendant_rule.db.xml");
        AttendantRule ar = new AttendantRule();
        ar.setName("autodafe");
        ar.setExtension("0");
        try {
            m_context.storeRule(ar);
            fail();
        } catch (UserException e) {
            // this is expected
            assertTrue(e.getMessage().indexOf("0") > 0);
        }
    }

    public void testSaveAliasThatIsDuplicateAlias() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/attendant_rule.db.xml");
        AttendantRule ar = new AttendantRule();
        ar.setName("autodafe");
        ar.setExtension("222");
        ar.setAttendantAliases("operator");
        try {
            m_context.storeRule(ar);
            fail();
        } catch (UserException e) {
            // this is expected
            assertTrue(e.getMessage().indexOf("operator") > 0);
        }
    }

    /**
     * Tests the getRulesForGateway() method to ensure it only returns rules that are being used
     * with the specified gateway.
     */
    public void getRulesForGateway() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/dialPlanGatewayAssociations.db.xml");
        Gateway gateway = m_gatewayContext.getGateway(1001);

        // Get our three rules we're testing with
        DialingRule rule0 = m_context.getRule(0);
        DialingRule rule1 = m_context.getRule(1);
        DialingRule rule2 = m_context.getRule(2);

        // The result of getRulesForGateway() should contain only rules 0 and 1,
        // since they contain the gateway.
        List<DialingRule> rulesForGateway = m_context.getRulesForGateway(gateway.getId());
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
        TestHelper.cleanInsertFlat("admin/dialplan/dialPlanGatewayAssociations.db.xml");
        Gateway gateway = m_gatewayContext.getGateway(1001);

        DialingRule rule0 = m_context.getRule(0);
        DialingRule rule1 = m_context.getRule(1);
        DialingRule rule2 = m_context.getRule(2);

        DialingRule attendantRule = m_context.getRule(3);
        DialingRule emergencyRule = m_context.getRule(4);
        DialingRule internalRule = m_context.getRule(5);
        DialingRule internationalRule = m_context.getRule(6);
        DialingRule localRule = m_context.getRule(7);
        DialingRule longDistanceRule = m_context.getRule(8);
        DialingRule siteToSiteRule = m_context.getRule(9);

        DialingRule[] nonGatewayAwareRules = {
            attendantRule, internalRule
        };
        DialingRule[] gatewayAwareRules = {
            rule2, emergencyRule, internationalRule, localRule, longDistanceRule, siteToSiteRule
        };

        List<DialingRule> availableRules = m_context.getAvailableRules(gateway.getId());
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
        String[] dialPlanBeans = m_context.getDialPlanBeans();
        assertTrue(dialPlanBeans.length > 1);
        assertTrue(ArrayUtils.contains(dialPlanBeans, "na.dialPlan"));
    }
}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.setting;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class SchedulesTableTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(SchedulesTableTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("resetDialPlans");
        clickLink("resetCallForwarding");
        SiteTestHelper.home(tester);
        SiteTestHelper.setScriptingEnabled(tester, true);
    }

    public void testAddDeleteForwardingRulesUserSchedule() throws Exception {
        // login as the test user and go to call forwarding page
        clickLink("loginFirstTestUser");
        clickLink("toggleNavigation");
        clickLink("menu.callForwarding");

        // go to schedules and add "userSchedule" schedule
        clickLink("link:schedules");
        clickLink("link:addSchedule");
        setTextField("item:name", "userSchedule");
        clickLink("addPeriod");
        submit("form:ok");
        SiteTestHelper.assertNoUserError(tester);

        // go to rings and add ring with "userSchedule" schedule
        clickLink("link:forwarding");
        setWorkingForm("callForwardingForm");
        clickLink("addRingLink");
        assertElementPresent("forward");
        setTextField("forward", "200");
        selectOption("schedule", "userSchedule");
        submit("form:apply");
        SiteTestHelper.assertNoUserError(tester);

        // go to schedules and delete "userSchedule" schedule
        clickLink("link:schedules");
        setWorkingForm("userSchedules");
        int rowCount = SiteTestHelper.getRowCount(tester, "schedule:list");
        assertEquals(3, rowCount);
        SiteTestHelper.selectRow(tester, 0, true);
        setExpectedJavaScriptConfirm("Are you sure you want to delete selected schedules?", true);
        submit("schedule:delete");

        // go to rings and assume ring switched to "Always" schedules
        clickLink("link:forwarding");
        assertSelectedOptionEquals("schedule", "Always");

        // cleanup (delete ring)
        clickLinkWithText("Delete");
        submit("form:apply");
    }

    public void testAddDeleteForwardingRulesGroupSchedule() throws Exception {
        SiteTestHelper.home(tester);
        SiteTestHelper.seedGroup(tester, "NewUserGroup", 1);
        SiteTestHelper.setScriptingEnabled(getTester(), true);

        clickLink("UserGroups");
        clickLinkWithExactText("seedGroup0");

        clickLink("link:schedules");
        clickLink("group:addSchedules");
        setTextField("item:name", "groupSchedule");
        clickLink("addPeriod");
        submit("form:ok");
        SiteTestHelper.assertNoUserError(tester);

        // edit test user and assing it to newly created group
        SiteTestHelper.home(tester);
        SiteTestHelper.setScriptingEnabled(getTester(), true);
        clickLink("ManageUsers");
        clickLinkWithExactText("testuser");
        setTextField("gms:groups", "seedGroup0");
        submit("form:ok");
        SiteTestHelper.assertNoUserError(tester);

        // login as the testuser, create new ring and assing it to groupSchedule
        SiteTestHelper.home(tester, false);
        SiteTestHelper.setScriptingEnabled(getTester(), true);
        clickLink("loginFirstTestUser");
        clickLink("toggleNavigation");
        clickLink("menu.callForwarding");
        setWorkingForm("callForwardingForm");
        clickLink("addRingLink");
        setTextField("forward", "200");
        selectOption("schedule", "groupSchedule");
        submit("form:apply");

        // edit test user and remove the group
        SiteTestHelper.home(tester);
        SiteTestHelper.setScriptingEnabled(getTester(), true);
        clickLink("ManageUsers");
        clickLinkWithExactText("testuser");
        setTextField("gms:groups", "");
        submit("form:ok");
        SiteTestHelper.assertNoUserError(tester);

        // login back as testuser and check that ring is switched to Always schedule
        SiteTestHelper.home(tester);
        SiteTestHelper.setScriptingEnabled(getTester(), true);
        clickLink("loginFirstTestUser");
        clickLink("toggleNavigation");
        clickLink("menu.callForwarding");
        assertSelectedOptionEquals("schedule", "Always");

        // cleanup (delete ring)
        clickLinkWithText("Delete");
        submit("form:apply");
    }

    public void testAddDeleteDialingPlansSchedule() {
        SiteTestHelper.home(tester);
        SiteTestHelper.setScriptingEnabled(getTester(), true);

        // go to dial plans page and create "dialPlanSchedule" schedule
        clickLink("FlexibleDialPlan");
        clickLink("link:schedules");
        clickLink("addSchedule");
        setTextField("item:name", "dialPlanSchedule");
        clickLink("addPeriod");
        submit("form:ok");
        SiteTestHelper.assertNoUserError(tester);

        // go to dialing rules, create new "customRule" custom rule and associate it with
        // "dialPlanSchedule" schedule
        clickLink("link:dialingRules");
        selectOption("rule:type", "Custom");
        checkCheckbox("item:enabled");
        setTextField("item:name", "customRule");
        selectOption("schedule", "dialPlanSchedule");
        submit("form:ok");

        // go to schedules and delete "dialPlanSchedule" schedule
        clickLink("link:schedules");
        int rowCount = SiteTestHelper.getRowCount(tester, "schedule:list");
        assertEquals(3, rowCount);
        SiteTestHelper.selectRow(tester, 0, true);
        setExpectedJavaScriptConfirm("Are you sure you want to delete selected schedules?", true);
        submit("schedule:delete");

        SiteTestHelper.home(tester);
        clickLink("FlexibleDialPlan");
        // go back to dialing plans and check that "customRule" rule switched to "Always" schedule
        clickLinkWithText("customRule");
        tester.assertSelectedOptionEquals("schedule", "Always");
    }
}

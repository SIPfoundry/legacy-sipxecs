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
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class SchedulesTableTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditScheduleTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(true);
    }

    public void testAddDeleteForwardingRulesUserSchedule() throws Exception {
        //login as the test user and go to call forwarding page
        clickLink("loginFirstTestUser");
        clickLink("toggleNavigation");
        clickLink("menu.callForwarding");

        //go to schedules and add "userSchedule" schedule
        clickLink("link:schedules");
        clickLink("link:addSchedule");
        setFormElement("item:name", "userSchedule");
        clickLink("addPeriod");
        submit("form:ok");

        //go to rings and add ring with "userSchedule" schedule
        clickLink("link:forwarding");
        clickLink("addRingLink");
        setFormElement("forward", "200");
        selectOption("schedule", "userSchedule");
        submit("form:apply");

        //go to schedules and delete "userSchedule" schedule
        clickLink("link:schedules");
        int rowCount = SiteTestHelper.getRowCount(getTester(), "schedule:list");
        assertEquals(2, rowCount);
        SiteTestHelper.selectRow(getTester(), 1, true);
        submit("schedule:delete");

        //go to rings and assume ring switched to "Always" schedules
        clickLink("link:forwarding");
        SiteTestHelper.assertOptionSelected(getTester(), "schedule", "Always");

        //cleanup (delete ring)
        clickLinkWithText("Delete");
        submit("form:apply");
    }

    public void testAddDeleteForwardingRulesGroupSchedule() throws Exception {
        //login as superadmin
        clickLink("login");
        clickLink("toggleNavigation");

        //go to groups and create "newGroup" group
        clickLink("menu.userGroups");
        clickLink("AddGroup");
        setFormElement("item:name", "newGroup");
        submit("form:ok");

        //create "groupSchedule" schedule for "newGroup" group
        clickLinkWithText("newGroup");
        clickLink("group:addSchedules");
        setFormElement("item:name", "groupSchedule");
        clickLink("addPeriod");
        submit("form:ok");

        //edit test user and assing it to newly created group
        clickLink("menu.users");
        clickLinkWithText("testuser");
        setFormElement("groups", "newGroup");
        submit("form:ok");

        //login as the testuser, create new ring and assing it to groupSchedule
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("loginFirstTestUser");
        clickLink("toggleNavigation");
        clickLink("menu.callForwarding");
        clickLink("addRingLink");
        setFormElement("forward", "200");
        selectOption("schedule", "groupSchedule");
        submit("form:apply");

        //login as the superadmin and delete groupSchedule
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("login");
        clickLink("toggleNavigation");
        clickLink("menu.userGroups");
        int rowCount = SiteTestHelper.getRowCount(getTester(), "group:list");
        assertEquals(2, rowCount);
        SiteTestHelper.selectRow(getTester(), 1, true);
        submit("group:delete");

        //login back as testuser and check that ring is switched to Always schedule
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("loginFirstTestUser");
        clickLink("toggleNavigation");
        clickLink("menu.callForwarding");
        SiteTestHelper.assertOptionSelected(getTester(), "schedule", "Always");

        //cleanup (delete ring)
        clickLinkWithText("Delete");
        submit("form:apply");
    }

    public void testAddDeleteDialingPlansSchedule() {
        // login as superadmin
        clickLink("login");
        clickLink("toggleNavigation");

        //go to dial plans page and create "dialPlanSchedule" schedule
        clickLink("menu.dialPlans");
        clickLink("link:schedules");
        clickLink("addSchedule");
        setFormElement("item:name", "dialPlanSchedule");
        clickLink("addPeriod");
        submit("form:ok");

        //go to dialing rules, create new "customRule" custom rule and associate it with "dialPlanSchedule" schedule
        clickLink("link:dialingRules");
        selectOption("rule:type", "Custom");
        checkCheckbox("item:enabled");
        setFormElement("item:name", "customRule");
        selectOption("schedule", "dialPlanSchedule");
        submit("form:ok");

        //go to schedules and delete "dialPlanSchedule" schedule
        clickLink("link:schedules");
        int rowCount = SiteTestHelper.getRowCount(getTester(), "schedule:list");
        assertEquals(2, rowCount);
        SiteTestHelper.selectRow(getTester(), 1, true);
        submit("schedule:delete");

        //go back to dialing plans and check that "customRule" rule switched to "Always" schedule
        clickLinkWithText("customRule");
        SiteTestHelper.assertOptionSelected(getTester(), "schedule", "Always");

        //cleanup (delete custom rule)
        submit("form:cancel");
        rowCount = SiteTestHelper.getRowCount(getTester(), "dialplan:list");
        assertEquals(2, rowCount);
        SiteTestHelper.selectRow(getTester(), 1, true);
        submit("dialplan:delete");
    }
}

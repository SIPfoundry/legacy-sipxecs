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

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.junit.WebTestCase;

public class EditScheduleTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditScheduleTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("loginFirstTestUser");
        clickLink("toggleNavigation");
        clickLink("menu.callForwarding");
        clickLink("link:schedules");
        setWorkingForm("userSchedules");
    }

    /*
     * Disabling unit test
     * For more info, see: http://track.sipfoundry.org/browse/XX-7630
     */
    public void test_DISABLED() {
        return;
    }

    public void DISABLED_testAddDeleteSchedule() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLink("link:addSchedule");
        setTextField("item:name", "schedule");
        clickLink("addPeriod");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        setWorkingForm("userSchedules");
        checkCheckbox("checkbox");
        setExpectedJavaScriptConfirm("Are you sure you want to delete selected schedules?", true);
        clickButton("schedule:delete");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void DISABLED_testAddSchedulesWithPeriodsThatOverlap() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLink("link:addSchedule");
        setTextField("item:name", "schedule_overlap");
        clickLink("addPeriod");
        selectOption("day", "Monday");
        setTextField("from", "9:00 AM");
        setTextField("to", "11:00 AM");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        setWorkingForm("editScheduleForm");
        clickLink("addPeriod");
        selectOption("day_0", "Monday");
        setTextField("from_0", "7:00 AM");
        setTextField("to_0", "10:00 AM");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        clickButton("form:cancel");
        setWorkingForm("userSchedules");
        checkCheckbox("checkbox");
        setExpectedJavaScriptConfirm("Are you sure you want to delete selected schedules?", true);
        clickButton("schedule:delete");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void DISABLED_testAddSchedulesWithStartHourGreaterThanStopHour() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLink("link:addSchedule");
        setTextField("item:name", "schedule_startstop");
        clickLink("addPeriod");
        selectOption("day", "Monday");
        setTextField("from", "11:00 AM");
        setTextField("to", "9:00 AM");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        clickButton("form:cancel");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void DISABLED_testAddSchedulesWithNoPeriodDefined() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLink("link:addSchedule");
        setTextField("item:name", "schedule_noperiod");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        clickButton("form:cancel");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void DISABLED_testAddSchedulesWithSameName() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLink("link:addSchedule");
        setTextField("item:name", "schedule");
        clickLink("addPeriod");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        clickLink("link:addSchedule");
        setTextField("item:name", "schedule");
        clickLink("addPeriod");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);
    }

    public void DISABLED_testAddDeleteScheduleWithSpacesInName() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLink("link:addSchedule");
        setTextField("item:name", "schedule name with spaces");
        clickLink("addPeriod");
        clickButton("form:ok");
        setWorkingForm("userSchedules");
        SiteTestHelper.assertNoUserError(tester);
        checkCheckbox("checkbox");
        setExpectedJavaScriptConfirm("Are you sure you want to delete selected schedules?", true);
        clickButton("schedule:delete");
        SiteTestHelper.assertNoUserError(tester);
    }
}

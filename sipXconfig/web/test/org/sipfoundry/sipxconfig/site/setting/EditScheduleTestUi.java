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

    public void testAddDeleteSchedule() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLink("link:addSchedule");
        setTextField("item:name", "schedule");
        SiteTestHelper.clickSubmitLink(tester, "addPeriod");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        setWorkingForm("userSchedules");
        checkCheckbox("checkbox");
        clickButton("schedule:delete");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddSchedulesWithPeriodsThatOverlap() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLink("link:addSchedule");
        setTextField("item:name", "schedule_overlap");
        SiteTestHelper.clickSubmitLink(tester, "addPeriod");
        selectOption("day", "Monday");
        setTextField("from", "9:00 AM");
        setTextField("to", "11:00 AM");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        setWorkingForm("editScheduleForm");
        SiteTestHelper.clickSubmitLink(tester, "addPeriod");
        selectOption("day_0", "Monday");
        setTextField("from_0", "7:00 AM");
        setTextField("to_0", "10:00 AM");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        clickButton("form:cancel");
        setWorkingForm("userSchedules");
        checkCheckbox("checkbox");
        clickButton("schedule:delete");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddSchedulesWithStartHourGreaterThanStopHour() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLink("link:addSchedule");
        setTextField("item:name", "schedule_startstop");
        SiteTestHelper.clickSubmitLink(tester, "addPeriod");
        selectOption("day", "Monday");
        setTextField("from", "11:00 AM");
        setTextField("to", "9:00 AM");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        clickButton("form:cancel");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddSchedulesWithNoPeriodDefined() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLink("link:addSchedule");
        setTextField("item:name", "schedule_noperiod");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        clickButton("form:cancel");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddSchedulesWithSameName() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLink("link:addSchedule");
        setTextField("item:name", "schedule");
        SiteTestHelper.clickSubmitLink(tester, "addPeriod");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        clickLink("link:addSchedule");
        setTextField("item:name", "schedule");
        SiteTestHelper.clickSubmitLink(tester, "addPeriod");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);
    }

    public void testAddDeleteScheduleWithSpacesInName() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLink("link:addSchedule");
        setTextField("item:name", "schedule name with spaces");
        SiteTestHelper.clickSubmitLink(tester, "addPeriod");
        clickButton("form:ok");
        setWorkingForm("userSchedules");
        SiteTestHelper.assertNoUserError(tester);
        checkCheckbox("checkbox");
        clickButton("schedule:delete");
        SiteTestHelper.assertNoUserError(tester);
    }
}

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

import net.sourceforge.jwebunit.WebTestCase;

public class EditScheduleTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditScheduleTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("loginFirstTestUser");
        clickLink("toggleNavigation");
        clickLink("menu.callForwarding");
        clickLink("link:schedules");
    }

    public void testAddDeleteSchedule() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLinkWithText("Add Schedule");
        setFormElement("name", "schedule");
        clickLink("addPeriod");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        checkCheckbox("checkbox");
        clickButton("schedule:delete");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddSchedulesWithPeriodsThatOverlap() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLinkWithText("Add Schedule");
        setFormElement("name", "schedule_overlap");
        clickLink("addPeriod");
        selectOption("day", "Monday");
        setFormElement("from", "9:00 AM");
        setFormElement("to", "11:00 AM");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);

        clickLink("addPeriod");
        selectOption("day_0", "Monday");
        setFormElement("from_0", "7:00 AM");
        setFormElement("to_0", "10:00 AM");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        clickButton("form:cancel");
        checkCheckbox("checkbox");
        clickButton("schedule:delete");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddSchedulesWithStartHourGreaterThanStopHour() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLinkWithText("Add Schedule");
        setFormElement("name", "schedule_startstop");
        clickLink("addPeriod");
        selectOption("day", "Monday");
        setFormElement("from", "11:00 AM");
        setFormElement("to", "9:00 AM");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        clickButton("form:cancel");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddSchedulesWithNoPeriodDefined() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLinkWithText("Add Schedule");
        setFormElement("name", "schedule_noperiod");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        clickButton("form:cancel");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddSchedulesWithSameName() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLinkWithText("Add Schedule");
        setFormElement("name", "schedule");
        clickLink("addPeriod");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        clickLinkWithText("Add Schedule");
        setFormElement("name", "schedule");
        clickLink("addPeriod");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);
    }
}

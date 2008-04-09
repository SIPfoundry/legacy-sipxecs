/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class BackupPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(BackupPageTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("BackupPage");
    }

    /**
     * Does not check if backup was successful - just checks if no Tapestry exceptions show up
     * would have to send mock backup shell script to artificial root.
     */
    public void testBackupNow() {
        SiteTestHelper.assertNoException(tester);
        clickButton("backup:now");
        SiteTestHelper.assertNoException(tester);
    }

    /**
     * Does not check if backup was successful - just checks if no Tapestry exceptions show up
     */
    public void testOk() {
        SiteTestHelper.assertNoException(tester);
        checkCheckbox("backup:check:voicemail");
        checkCheckbox("backup:check:configs");
        selectOption("backup:limit", "10");
        checkCheckbox("dailyScheduleEnabled");
        selectOption("dailyScheduledDay", "Wednesday");
        setTextField("dailyScheduledTime", "3:24 AM");
        clickButton("backup:ok");
        SiteTestHelper.assertNoUserError(tester);
        assertCheckboxSelected("backup:check:voicemail");
        assertCheckboxSelected("backup:check:configs");
        assertSelectedOptionEquals("backup:limit", "10");
        assertCheckboxSelected("dailyScheduleEnabled");
        assertSelectedOptionEquals("dailyScheduledDay", "Wednesday");
        assertTextFieldEquals("dailyScheduledTime", "3:24 AM");
    }

    public void testEmptyTime() {
        SiteTestHelper.assertNoException(tester);
        checkCheckbox("backup:check:voicemail");
        checkCheckbox("backup:check:configs");
        selectOption("backup:limit", "10");
        checkCheckbox("dailyScheduleEnabled");
        selectOption("dailyScheduledDay", "Wednesday");
        setTextField("dailyScheduledTime", "");
        clickButton("backup:ok");
        SiteTestHelper.assertUserError(tester);
    }

    /**
     * Tests if the email address text field is present
     */
    public void testEmailAddress() {
        SiteTestHelper.assertNoException(getTester());
        assertElementPresent("backup:emailAddress");
        SiteTestHelper.assertNoException(getTester());
    }

    /**
     * Tests that the backup time is rendered in the correct format for a browser-specified
     * locale.
     */
    public void testLocaleTimeFormat() {
        SiteTestHelper.assertNoException(tester);
        setTextField("dailyScheduledTime", "3:24 AM");

        getTestContext().addRequestHeader("Accept-Language", "de");
        clickButton("backup:ok");
        SiteTestHelper.assertNoException(getTester());
        tester.assertTextFieldEquals("dailyScheduledTime", "03:24");

        getTestContext().addRequestHeader("Accept-Language", "en");
        clickButton("backup:ok");
        SiteTestHelper.assertNoException(getTester());
        tester.assertTextFieldEquals("dailyScheduledTime", "3:24 AM");
    }
}

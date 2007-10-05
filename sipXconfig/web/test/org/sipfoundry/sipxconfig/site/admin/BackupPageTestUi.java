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
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class BackupPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(BackupPageTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("BackupPage");
    }

    /**
     * Does not check if backup was successful - just checks if no Tapestry exceptions show up
     * would have to send mock backup shell script to artificial root.
     */
    public void testBackupNow() {
        SiteTestHelper.assertNoException(getTester());
        clickButton("backup:now");
        SiteTestHelper.assertNoException(getTester());
    }

    /**
     * Does not check if backup was successful - just checks if no Tapestry exceptions show up
     */
    public void testOk() {
        SiteTestHelper.assertNoException(getTester());
        checkCheckbox("checkVoicemail");
        checkCheckbox("checkConfigs");
        selectOption("limitCount", "10");
        checkCheckbox("dailyScheduleEnabled");
        selectOption("dailyScheduledDay", "Wednesday");
        setFormElement("dailyScheduledTime", "3:24 AM");
        clickButton("backup:ok");
        SiteTestHelper.assertNoException(getTester());
        assertCheckboxSelected("checkVoicemail");
        assertCheckboxSelected("checkConfigs");
        assertOptionEquals("limitCount", "10");
        assertCheckboxSelected("dailyScheduleEnabled");
        assertOptionEquals("dailyScheduledDay", "Wednesday");
        assertFormElementEquals("dailyScheduledTime", "3:24 AM");
    }

    public void testEmptyTime() {
        SiteTestHelper.assertNoException(getTester());
        checkCheckbox("checkVoicemail");
        checkCheckbox("checkConfigs");
        selectOption("limitCount", "10");
        checkCheckbox("dailyScheduleEnabled");
        selectOption("dailyScheduledDay", "Wednesday");
        setFormElement("dailyScheduledTime", "");
        clickButton("backup:ok");
        SiteTestHelper.assertNoException(getTester());
        SiteTestHelper.assertUserError(getTester());
    }

    /**
     * Tests if the email address text field is present
     */
    public void testEmailAddress() {
        SiteTestHelper.assertNoException(getTester());
        assertElementPresent("backup:emailAddress");
        SiteTestHelper.assertNoException(getTester());
    }
}

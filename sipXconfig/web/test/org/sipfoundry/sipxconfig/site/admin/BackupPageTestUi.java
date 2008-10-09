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

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.setScriptingEnabled(tester, true);
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
     * Tests that ftp backup fails upon unsucessful connection
     */
    public void testFtpBackupNowError() {
        SiteTestHelper.assertNoException(tester);
        selectOption("backupPlan:type","FTP");
        clickButton("backup:now");
        SiteTestHelper.assertUserError(tester);
        selectOption("backupPlan:type","Local");
        SiteTestHelper.assertNoException(tester);
    }

    /**
     * Does not check if backup was successful - just checks if no Tapestry exceptions show up
     */
    public void testOk() {
        SiteTestHelper.assertNoException(tester);
        setWorkingForm("form");
        checkCheckbox("backup:check:voicemail");
        checkCheckbox("backup:check:configs");
        selectOption("backup:limit", "10");
        checkCheckbox("dailyScheduleEnabled");
        selectOption("dailyScheduledDay", "Wednesday");
        setTextField("dailyScheduledTime", "3:24 AM");
        clickButton("backup:ok");
        SiteTestHelper.assertNoUserError(tester);
        setWorkingForm("form");
        assertCheckboxSelected("backup:check:voicemail");
        assertCheckboxSelected("backup:check:configs");
        assertSelectedOptionEquals("backup:limit", "10");
        assertCheckboxSelected("dailyScheduleEnabled");
        assertSelectedOptionEquals("dailyScheduledDay", "Wednesday");
        assertTextFieldEquals("dailyScheduledTime", "3:24 AM");
    }

    public void testEmptyPlan() {
        SiteTestHelper.assertNoException(tester);
        setWorkingForm("form");
        uncheckCheckbox("backup:check:voicemail");
        uncheckCheckbox("backup:check:configs");
        selectOption("backup:limit", "10");
        checkCheckbox("dailyScheduleEnabled");
        selectOption("dailyScheduledDay", "Wednesday");
        setTextField("dailyScheduledTime", "3:24 AM");
        clickButton("backup:ok");
        SiteTestHelper.assertUserError(tester);
    }

    public void testEmptyTime() {
        SiteTestHelper.assertNoException(tester);
        setWorkingForm("form");
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
    * Tests if the backup plan combo-box is present
    */
    public void testBackupPlanComboBox() {
        SiteTestHelper.assertNoException(getTester());
        assertElementPresent("backupPlan:type");
        SiteTestHelper.assertNoException(getTester());
    }

    //FIXME: commented because this test needs Ajax capabilities (DOJO based) not supported by the current
    //version of httpunit
    public void _testToggleFtpPanel() {
        SiteTestHelper.assertNoException(getTester());
        setWorkingForm("backupPlan");
        assertElementPresent("backupPlan:type");
        selectOption("backupPlan:type","FTP");
        assertElementPresent("link");
        assertElementNotPresent("ftp:address");
        assertElementNotPresent("ftp:user");
        assertElementNotPresent("ftp:password");
        clickButton("link");
        setWorkingForm("configurationForm");
        assertElementPresent("ftp:address");
        assertElementPresent("ftp:user");
        assertElementPresent("ftp:password");
        SiteTestHelper.assertNoException(getTester());
        setWorkingForm("backupPlan");
        selectOption("backupPlan:type","Local");
        assertElementNotPresent("link");
        setWorkingForm("configurationForm");
        assertElementNotPresent("ftp:address");
        assertElementNotPresent("ftp:user");
        assertElementNotPresent("ftp:password");
        SiteTestHelper.assertNoException(tester);
    }
    //FIXME: commented because this test needs Ajax capabilities (DOJO based) not supported by the current
    //version of httpunit
    public void _testApplyFtpPanel() {
        SiteTestHelper.assertNoException(getTester());
        setWorkingForm("backupPlan");
        selectOption("backupPlan:type","FTP");
        clickButton("link");
        setWorkingForm("configurationForm");
        setTextField("ftp:address","address");
        setTextField("ftp:user", "user");
        setTextField("ftp:password", "password");
        clickButton("form:apply");
        //refresh the panel - read again the data
        //hide panel
        clickButton("link");
        //show panel
        clickButton("link");
        tester.assertTextFieldEquals("ftp:address", "address");
        tester.assertTextFieldEquals("ftp:user", "user");
        tester.assertTextFieldEquals("ftp:password", "password");
    }

    public void testTimeFormat() {
        SiteTestHelper.assertNoException(tester);
        setTextField("dailyScheduledTime", "3:24 AM");
    }

    /**
     * Tests that the backup time is rendered in the correct format for a browser-specified
     * locale.
     */
    // FIXME: commented out because setting "Accept-Language" does not seem to work?
    public void _testLocaleTimeFormat() {
        SiteTestHelper.assertNoException(tester);

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

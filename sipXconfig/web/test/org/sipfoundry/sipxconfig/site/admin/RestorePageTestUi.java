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

import java.io.File;
import java.io.IOException;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class RestorePageTestUi extends WebTestCase {
    private static final String FIRST_BACKUP = "200706101100";

    private static final String SECOND_BACKUP = "200706101101";

    private static final String THIRD_BACKUP = "200706101102";

    private static final String CONFIG_FILE = "/configuration.tar.gz";

    private static final String VOICEMAIL_FILE = "/voicemail.tar.gz";

    private static final String BACKUP_FOLDER = "/backup/";

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(RestorePageTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, false);
        clickLink("toggleNavigation");
        clickLink("menu.restore");
        SiteTestHelper.assertNoException(getTester());
        buildBackupsFolders();
    }

    private void buildBackupsFolders() {
        // first backup folder;
        buildConfigurationBackup(FIRST_BACKUP);
        buildVoicemailBackup(FIRST_BACKUP);

        // second backup folder;
        buildConfigurationBackup(SECOND_BACKUP);

        // third backup folder;
        buildVoicemailBackup(THIRD_BACKUP);
    }

    public void testRestorePage() throws Exception {
        SiteTestHelper.assertNoException(getTester());
        clickLink("link:restore");
        SiteTestHelper.assertNoException(getTester());
    }

    public void testRestoreTwoVoicemailBackups() throws Exception {
        SiteTestHelper.assertNoException(getTester());
        clickLink("link:restore");
        setWorkingForm("Form");
        checkCheckbox("defaultValue");
        checkCheckbox("defaultValue_2");
        assertButtonPresent("backups:restore");
        clickButton("backups:restore");
        SiteTestHelper.assertUserError(getTester());
    }

    public void testRestoreThreeBackups() throws Exception {
        SiteTestHelper.assertNoException(getTester());
        clickLink("link:restore");
        setWorkingForm("Form");
        SiteTestHelper.assertNoException(getTester());
        checkCheckbox("defaultValue");
        checkCheckbox("defaultValue_1");
        checkCheckbox("defaultValue_2");
        assertButtonPresent("backups:restore");
        clickButton("backups:restore");
        SiteTestHelper.assertUserError(getTester());
    }

    public void testRestoreWithNoSelections() throws Exception {
        SiteTestHelper.assertNoException(getTester());
        clickLink("link:restore");
        setWorkingForm("Form");
        SiteTestHelper.assertNoException(getTester());
        checkCheckbox("defaultValue");
        uncheckCheckbox("defaultValue");
        assertButtonPresent("backups:restore");
        clickButton("backups:restore");
        SiteTestHelper.assertUserError(getTester());
    }

    private void buildConfigurationBackup(String folder) {
        String testFolder = TestUtil.getTestOutputDirectory("web");
        String configBackup = testFolder + BACKUP_FOLDER + folder;
        try {
            File file = new File(configBackup);
            file.mkdirs();
            configBackup += CONFIG_FILE;
            File backup = new File(configBackup);
            backup.createNewFile();
        } catch (IOException ex) {
            fail("Could not create the config backup");
        }

    }

    private void buildVoicemailBackup(String folder) {
        String testFolder = TestUtil.getTestOutputDirectory("web");
        String voicemailBackup = testFolder + BACKUP_FOLDER + folder;
        try {
            File file = new File(voicemailBackup);
            file.mkdirs();
            voicemailBackup += VOICEMAIL_FILE;
            File backup = new File(voicemailBackup);
            backup.createNewFile();
        } catch (IOException ex) {
            fail("Could not create the mailstore backup");
        }
    }

    public void testUploadPage() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLink("link:upload");
        setWorkingForm("upload");
        SiteTestHelper.assertNoException(tester);
        assertElementPresent("configuration");
        assertElementPresent("voicemail");
        assertButtonPresent("backups:uploadbutton");
    }

    public void testShowLogPage() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLink("link:log");
        SiteTestHelper.assertNoException(tester);
        assertElementPresent("TextArea");

    }

    public void testBackupPlanComboBox() throws Exception {
        SiteTestHelper.assertNoException(getTester());
        clickLink("link:restore");
        setWorkingForm("renderForm");
        assertElementPresent("backupPlan:type");
        SiteTestHelper.assertNoException(getTester());
    }

    //FIXME: commented because this test needs Ajax capabilities (DOJO based) not supported by the current
    //version of httpunit
    public void _testToggleFtpPanel() {
        SiteTestHelper.setScriptingEnabled(getTester(), true);
        SiteTestHelper.assertNoException(getTester());
        clickLink("link:restore");
        setWorkingForm("renderForm");
        assertElementPresent("backupPlan:type");
        selectOption("backupPlan:type","FTP");
        assertElementPresent("link");
        assertElementNotPresent("ftp:address");
        assertElementNotPresent("ftp:user");
        assertElementNotPresent("ftp:password");
        clickButton("link");
        assertElementPresent("ftp:address");
        assertElementPresent("ftp:user");
        assertElementPresent("ftp:password");
        SiteTestHelper.assertNoException(getTester());
        setWorkingForm("renderForm");
        selectOption("backupPlan:type","Local");
        assertElementNotPresent("link");
        assertElementNotPresent("ftp:address");
        assertElementNotPresent("ftp:user");
        assertElementNotPresent("ftp:password");
        SiteTestHelper.assertNoException(tester);
    }
    //FIXME: commented because this test needs Ajax capabilities (DOJO based) not supported by the current
    //version of httpunit
    public void _testApplyFtpPanel() {
        SiteTestHelper.setScriptingEnabled(getTester(), true);
        SiteTestHelper.assertNoException(getTester());
        setWorkingForm("renderForm");
        selectOption("backupPlan:type","FTP");
        clickButton("link");
        getElementByXPath("//input[@id='ftp:address']").setTextContent("address");
        getElementByXPath("//input[@id='ftp:user']").setTextContent("user");
        getElementByXPath("//input[@id='ftp:password']").setTextContent("password");
        //setTextField("ftp:address","address");
        //setTextField("ftp:user", "user");
        //setTextField("ftp:password", "password");
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

    public void testErrorConnectFtpServer() throws Exception {
        SiteTestHelper.setScriptingEnabled(tester, true);
        SiteTestHelper.assertNoException(getTester());
        clickLink("link:restore");
        setWorkingForm("renderForm");
        selectOption("backupPlan:type","FTP");
        SiteTestHelper.assertUserError(getTester());
        selectOption("backupPlan:type","Local");
        SiteTestHelper.assertNoException(getTester());
    }

}

package org.sipfoundry.sipxconfig.site.admin;

import junit.framework.Test;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.junit.WebTestCase;

public class BackupConfigurationPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(BackupPageTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("BackupPage");
        clickLink("ftpConfiguration");
    }

    /**
     * Tests if the ftp address field exists
     */
    public void testFtpAddress() {
        SiteTestHelper.assertNoException(getTester());
        assertElementPresent("ftp:address");
        SiteTestHelper.assertNoException(getTester());
    }

    /**
     * Tests if the ftp user field exists
     */
    public void testFtpUser() {
        SiteTestHelper.assertNoException(getTester());
        assertElementPresent("ftp:user");
        SiteTestHelper.assertNoException(getTester());
    }

    /**
     * Tests if the ftp password field exists
     */
    public void testFtpPassword() {
        SiteTestHelper.assertNoException(getTester());
        assertElementPresent("ftp:password");
        SiteTestHelper.assertNoException(getTester());
    }

    /**
     * Tests press OK/Cancel button
     */
    public void testOkCancel() {
        setTextField("ftp:address","address");
        setTextField("ftp:user", "user");
        setTextField("ftp:password", "password");
        clickButton("form:ok");
        clickLink("ftpConfiguration");
        tester.assertTextFieldEquals("ftp:address", "address");
        tester.assertTextFieldEquals("ftp:user", "user");
        tester.assertTextFieldEquals("ftp:password", "password");
        setTextField("ftp:address","address2");
        setTextField("ftp:user", "user2");
        setTextField("ftp:password", "password2");
        clickButton("form:cancel");
        clickLink("ftpConfiguration");
        tester.assertTextFieldEquals("ftp:address", "address");
        tester.assertTextFieldEquals("ftp:user", "user");
        tester.assertTextFieldEquals("ftp:password", "password");

    }

}

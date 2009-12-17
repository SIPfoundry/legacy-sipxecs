package org.sipfoundry.sipxconfig.site.user_portal;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ExternalUserImAccountTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ExternalUserImAccountTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.seedUser(getTester());
        tester.clickLink("loginFirstTestUser");
        SiteTestHelper.home(getTester());
        tester.clickLink("EditMyInformation");
        tester.clickLink("link:openfire");
        tester.clickLink("link:addExternalAccount");
    }

    public void testInsertImAccountMissingFields() throws Exception {
        SiteTestHelper.assertNoUserError(tester);
        assertElementPresent("protocol");
        selectOption("protocol", "ICQ");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        setTextField("username", "username");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        setTextField("cp:password", "password");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        setTextField("cp:confirmPassword", "password1");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        setTextField("cp:confirmPassword", "password");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        assertElementPresentByXPath("//a[@id='editRowLink']");
        assertElementNotPresentByXPath("//a[@id='editRowLink_0']");
    }

    public void testEditExternalAccount() throws Exception {
        insertImAccount();
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        assertLinkPresentWithText("username");
        clickLink("editRowLink");
        setWorkingForm("Form");
        assertCheckboxSelected("enableExternalImAccount");
        assertSelectOptionPresent("protocol", "ICQ");
        assertTextFieldEquals("username", "username");
        assertTextFieldEquals("displayName", "displayName");

        selectOption("protocol", "AIM");
        setTextField("displayName", "anotherDisplayName");
        assertButtonPresent("form:ok");
        clickButton("form:ok");
        assertLinkPresentWithText("username");
        clickLink("editRowLink");
        assertSelectOptionPresent("protocol", "AIM");
        assertTextFieldEquals("displayName", "anotherDisplayName");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
    }

    private void insertImAccount() throws Exception {
        SiteTestHelper.assertNoUserError(tester);
        assertElementPresent("protocol");
        selectOption("protocol", "ICQ");
        setTextField("username", "username");
        setTextField("cp:password", "password");
        setTextField("cp:confirmPassword", "password");
        setTextField("displayName", "displayName");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        assertElementPresentByXPath("//a[@id='editRowLink']");
        assertElementNotPresentByXPath("//a[@id='editRowLink_0']");
    }
}

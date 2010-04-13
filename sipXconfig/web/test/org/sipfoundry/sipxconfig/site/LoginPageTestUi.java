/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

public class LoginPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(LoginPageTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester, false);
        clickLink("seedTestUser");
        clickLink("ManageUsers");   // will redirect to login page
    }

    public void testLoginWithUserName() throws Exception {
        checkLogin(TestPage.TEST_USER_USERNAME);
    }

    public void testLoginWithFirstAlias() throws Exception {
        checkLogin(TestPage.TEST_USER_ALIAS1);
    }

    public void testLoginWithSecondAlias() throws Exception {
        checkLogin(TestPage.TEST_USER_ALIAS2);
    }

    private void checkLogin(String userId) {
        SiteTestHelper.assertNoUserError(getTester());

        setTextField("j_username", userId);
        setTextField("j_password", TestPage.TEST_USER_PIN);
        clickButton("login:submit");

        // we are on the home page now - no errors no login form
        SiteTestHelper.assertNoUserError(getTester());
        assertFormNotPresent("loginForm");
    }

    // successful login is tested by "home" function
    public void testLoginFailed() throws Exception {
        SiteTestHelper.assertNoUserError(getTester());

        setTextField("j_username", "xyz");
        setTextField("j_password", "abc");
        clickButton("login:submit");

        // still on the same page
        assertElementPresent("loginForm");
        SiteTestHelper.assertUserError(getTester());
    }

    public void testLoginBlankPassword() throws Exception {
        setTextField("j_username", TestPage.TEST_USER_USERNAME);
        clickButton("login:submit");
        SiteTestHelper.assertUserError(getTester());
    }
}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class FirstUserTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(FirstUserTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        tester.clickLink("resetCoreContext");
        SiteTestHelper.home(getTester());
        tester.clickLink("deleteAllUsers");
        // Go home. *Don't* log in because that requires creating a user, but we want no users.
        SiteTestHelper.home(getTester(), false);
    }

    /**
     * Test that clicking on ManageUsers (or any link that requires login) redirects to FirstUser
     * when there are no users. The Border component takes care of this.
     */
    public void testBorderRedirect() throws Exception {
        clickLink("ManageUsers");
        assertLandedOnFirstUserPage();
    }

    /**
     * Test that the LoginPage redirects to FirstUser page when there are no users. LoginPage has
     * no Border, so it is a special case.
     */
    public void testLoginPageRedirect() throws Exception {
        clickLink("toggleNavigation");
        clickLink("LoginPage");
        assertLandedOnFirstUserPage();
    }

    public void testCreateSuperadmin() throws Exception {
        clickLink("ManageUsers");
        assertLandedOnFirstUserPage();

        // Fill in the PIN and create the superadmin user
        final String PIN = "ch33zeW1z";
        setTextField("cp:password", PIN);
        setTextField("cp:confirmPassword", PIN);
        clickButton("form:apply");
        assertCopacetic();

        // We should land on the login page
        assertElementPresent("loginForm");
    }

    private void assertLandedOnFirstUserPage() {
        assertCopacetic();
        assertElementPresent("firstuser:form");
    }

    /** Assert that we landed on a page with no complaints */
    private void assertCopacetic() {
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.assertNoException(tester);
    }
}

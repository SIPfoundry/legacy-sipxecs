/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.user;

import static org.sipfoundry.sipxconfig.site.SiteTestHelper.assertNoException;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.assertNoUserError;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.assertTextFieldEmpty;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.assertUserError;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.getBaseUrl;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.home;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.webTestSuite;
import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.TestPage;

public class NewUserTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return webTestSuite(NewUserTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(getBaseUrl());
        home(getTester());
        tester.clickLink("resetCoreContext");
        home(getTester());
        clickLink("seedTestUser");
    }

    public void testListUsers() throws Exception {
        final String NEW_USER_USERNAME = "NewUserUsername";
        final String NEW_USER_FNAME = "NewUserFname";
        final String NEW_USER_LNAME = "NewUserLname";
        final String NEW_USER_PWORD = "1234";
        final String NEW_USER_ALIASES = "lazyboy, 993";

        home(tester);
        clickLink("NewUser");
        setTextField("user:userId", NEW_USER_USERNAME);
        setTextField("user:firstName", NEW_USER_FNAME);
        setTextField("user:lastName", NEW_USER_LNAME);
        setTextField("cp:password", NEW_USER_PWORD);
        setTextField("cp:confirmPassword", NEW_USER_PWORD);
        setTextField("user:aliases", NEW_USER_ALIASES);
        clickButton("form:apply");
        assertNoUserError(tester);
        assertNoException(tester);

        home(tester);
        clickLink("ManageUsers");

        // Instead of specifying exactly what the table should look like, just look
        // for text that we expect to be there. Since UI tests don't reset the DB,
        // there may be users in the table that we don't expect.
        assertTextInTable("user:list", new String[] {
            TestPage.TEST_USER_FIRSTNAME, TestPage.TEST_USER_LASTNAME, TestPage.TEST_USER_USERNAME,
            TestPage.TEST_USER_ALIASES
        });
    }

    public void testDuplicateName() {
        final String NEW_USER_USERNAME[] = {
            "auser", "buser"
        };
        final String NEW_USER_PWORD = "1234";

        for (int i = 0; i < NEW_USER_USERNAME.length; i++) {
            home(tester);
            clickLink("NewUser");
            setTextField("user:userId", NEW_USER_USERNAME[i]);
            setTextField("cp:password", NEW_USER_PWORD);
            setTextField("cp:confirmPassword", NEW_USER_PWORD);
            clickButton("form:apply");
            assertNoUserError(tester);
            assertNoException(tester);
        }

        home(tester);
        clickLink("ManageUsers");

        clickLinkWithText(NEW_USER_USERNAME[0]);
        setTextField("user:userId", NEW_USER_USERNAME[1]);
        clickButton("form:apply");

        assertUserError(tester);
        assertNoException(tester);
    }

    public void testDuplicateNameOnNew() {
        final String NEW_USER_USERNAME = "cuser";
        final String NEW_USER_PWORD = "1234";

        clickLink("ManageUsers");
        clickLink("AddUser");
        setTextField("user:userId", NEW_USER_USERNAME);
        setTextField("cp:password", NEW_USER_PWORD);
        setTextField("cp:confirmPassword", NEW_USER_PWORD);
        clickButton("form:ok");
        assertNoUserError(tester);
        assertNoException(tester);

        home(tester);
        clickLink("ManageUsers");
        clickLink("AddUser");
        setTextField("user:userId", NEW_USER_USERNAME);
        setTextField("cp:password", NEW_USER_PWORD);
        setTextField("cp:confirmPassword", NEW_USER_PWORD);
        clickButton("form:ok");
        assertUserError(tester);
        assertNoException(tester);
    }

    public void testStay() {
        clickLink("ManageUsers");
        clickLink("AddUser");
        setTextField("user:userId", "x");
        setTextField("cp:password", "1234");
        setTextField("cp:confirmPassword", "1234");
        setTextField("user:aliases", "aa bb cc");
        checkCheckbox("stay");
        clickButton("form:ok");
        assertElementPresent("user:success");

        // Make sure all the correct fields are empty
        assertTextFieldEmpty(tester, "user:lastName");
        assertTextFieldEmpty(tester, "user:firstName");
        assertTextFieldEmpty(tester, "cp:password");
        assertTextFieldEmpty(tester, "cp:confirmPassword");
        assertTextFieldEmpty(tester, "gms:groups");
        assertTextFieldEmpty(tester, "user:aliases");

        setTextField("user:userId", "y");
        setTextField("cp:password", "1234");
        setTextField("cp:confirmPassword", "1234");
        uncheckCheckbox("stay");
        clickButton("form:ok");
        assertElementNotPresent("user:success");
    }

    public void testStayFromHomePage() throws Exception {
        clickLink("toggleNavigation");
        clickLink("link.home");
        clickLink("addUser");
        assertNoException(tester);
        assertNoUserError(tester);

        setTextField("user:userId", "xx");
        setTextField("cp:password", "1234");
        setTextField("cp:confirmPassword", "1234");
        checkCheckbox("stay");
        clickButton("form:ok");
        assertElementPresent("user:success");

        setTextField("user:userId", "yy");
        setTextField("cp:password", "1234");
        setTextField("cp:confirmPassword", "1234");
        uncheckCheckbox("stay");
        clickButton("form:ok");
        assertElementNotPresent("user:success");
    }

    public void testNoStayOnCancel() {
        clickLink("ManageUsers");
        clickLink("AddUser");
        checkCheckbox("stay");
        assertFormPresent("form");
        clickButton("form:cancel");
        assertElementNotPresent("stay");
    }

    public void testAddGroups() {
        clickLink("ManageUsers");
        clickLink("AddUser");
        setTextField("user:userId", "x");
        setTextField("cp:password", "1234");
        setTextField("cp:confirmPassword", "1234");
        setTextField("gms:groups", "a b c");
        clickButton("form:apply");
        assertTextFieldEquals("gms:groups", "a b c");
    }

    public void testExtensionPoolLink() {
        clickLink("ManageUsers");
        clickLink("AddUser");
        clickLink("link:extensionPool");
        assertNoUserError(tester);
        clickButton("form:ok");
        assertNoUserError(tester);
    }

    public void testSipPassword() {
        clickLink("ManageUsers");
        clickLink("AddUser");
        setTextField("user:userId", "x");
        setTextField("cp:password", "1234");
        setTextField("cp:confirmPassword", "1234");
        setTextField("user:sipPassword", "");
        setTextField("user:aliases", "aa bb cc");
        checkCheckbox("stay");
        clickButton("form:ok");
        assertUserError(tester);

        setTextField("user:sipPassword", "abcd");
        clickButton("form:ok");
        assertNoUserError(tester);
    }
}

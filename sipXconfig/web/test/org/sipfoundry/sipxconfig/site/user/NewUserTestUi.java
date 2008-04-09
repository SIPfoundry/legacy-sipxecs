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
import org.sipfoundry.sipxconfig.site.TestPage;

public class NewUserTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(NewUserTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        tester.clickLink("resetCoreContext");
        SiteTestHelper.home(getTester());
        clickLink("seedTestUser");
    }

    public void testListUsers() throws Exception {
        final String NEW_USER_USERNAME = "NewUserUsername";
        final String NEW_USER_FNAME = "NewUserFname";
        final String NEW_USER_LNAME = "NewUserLname";
        final String NEW_USER_PWORD = "1234";
        final String NEW_USER_ALIASES = "lazyboy, 993";

        SiteTestHelper.home(tester);
        clickLink("NewUser");
        setTextField("userId", NEW_USER_USERNAME);
        setTextField("firstName", NEW_USER_FNAME);
        setTextField("lastName", NEW_USER_LNAME);
        setTextField("password", NEW_USER_PWORD);
        setTextField("confirmPassword", NEW_USER_PWORD);
        setTextField("aliases", NEW_USER_ALIASES);
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.assertNoException(tester);

        SiteTestHelper.home(tester);
        clickLink("ManageUsers");

        // Instead of specifying exactly what the table should look like, just look
        // for text that we expect to be there. Since UI tests don't reset the DB,
        // there may be users in the table that we don't expect.
        assertTextInTable("user:list", new String[] {
            TestPage.TEST_USER_FIRSTNAME, TestPage.TEST_USER_LASTNAME,
            TestPage.TEST_USER_USERNAME, TestPage.TEST_USER_ALIASES
        });
    }

    public void testDuplicateName() {
        final String NEW_USER_USERNAME[] = {
            "auser", "buser"
        };
        final String NEW_USER_PWORD = "1234";

        for (int i = 0; i < NEW_USER_USERNAME.length; i++) {
            SiteTestHelper.home(tester);
            clickLink("NewUser");
            setTextField("userId", NEW_USER_USERNAME[i]);
            setTextField("password", NEW_USER_PWORD);
            setTextField("confirmPassword", NEW_USER_PWORD);
            clickButton("form:apply");
            SiteTestHelper.assertNoUserError(tester);
            SiteTestHelper.assertNoException(tester);
        }

        SiteTestHelper.home(tester);
        clickLink("ManageUsers");

        clickLinkWithText(NEW_USER_USERNAME[0]);
        setTextField("userId", NEW_USER_USERNAME[1]);
        clickButton("form:apply");

        SiteTestHelper.assertUserError(tester);
        SiteTestHelper.assertNoException(tester);
    }

    public void testDuplicateNameOnNew() {
        final String NEW_USER_USERNAME = "cuser";
        final String NEW_USER_PWORD = "1234";

        clickLink("ManageUsers");
        clickLink("AddUser");
        setTextField("userId", NEW_USER_USERNAME);
        setTextField("password", NEW_USER_PWORD);
        setTextField("confirmPassword", NEW_USER_PWORD);
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.assertNoException(tester);

        SiteTestHelper.home(tester);
        clickLink("ManageUsers");
        clickLink("AddUser");
        setTextField("userId", NEW_USER_USERNAME);
        setTextField("password", NEW_USER_PWORD);
        setTextField("confirmPassword", NEW_USER_PWORD);
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);
        SiteTestHelper.assertNoException(tester);
    }
    
    public void testStay() {
        clickLink("ManageUsers");
        clickLink("AddUser");
        setTextField("userId", "x");
        setTextField("password", "1234");
        setTextField("confirmPassword", "1234");
        setTextField("aliases", "aa bb cc");
        checkCheckbox("stay");        
        clickButton("form:ok");
        assertElementPresent("user:success");  
        
        // Make sure all the correct fields are empty
        SiteTestHelper.assertTextFieldEmpty(tester, "lastName");
        SiteTestHelper.assertTextFieldEmpty(tester, "firstName");        
        SiteTestHelper.assertTextFieldEmpty(tester, "emailAddress");
        assertCheckboxNotSelected("attachVoicemailToEmail");
        SiteTestHelper.assertTextFieldEmpty(tester, "alternateEmailAddress");
        assertCheckboxNotSelected("attachVoicemailToAlternateEmail");
        SiteTestHelper.assertTextFieldEmpty(tester, "password");
        SiteTestHelper.assertTextFieldEmpty(tester, "confirmPassword");
        SiteTestHelper.assertTextFieldEmpty(tester, "groups");
        SiteTestHelper.assertTextFieldEmpty(tester, "aliases");
        
        setTextField("userId", "y");
        setTextField("password", "1234");
        setTextField("confirmPassword", "1234");
        uncheckCheckbox("stay");        
        clickButton("form:ok");
        assertElementNotPresent("user:success");        
    }
    
    public void testNoStayOnCancel() {
        clickLink("ManageUsers");
        clickLink("AddUser");
        checkCheckbox("stay");        
        assertElementPresent("page:newuser");
        clickButton("form:cancel");
        assertElementNotPresent("page:newuser");
    }
    
    public void testAddGroups() {
        clickLink("ManageUsers");
        clickLink("AddUser");
        setTextField("userId", "x");
        setTextField("password", "1234");
        setTextField("confirmPassword", "1234");
        setTextField("groups", "a b c");
        clickButton("form:apply");
        assertTextFieldEquals("groups", "a b c");
    }
    
    public void testExtensionPoolLink() {
        clickLink("ManageUsers");
        clickLink("AddUser");
        clickLink("link:extensionPool");
        SiteTestHelper.assertNoUserError(tester);
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
    }
}

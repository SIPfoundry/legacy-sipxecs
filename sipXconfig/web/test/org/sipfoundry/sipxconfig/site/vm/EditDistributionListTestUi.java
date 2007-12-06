/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.vm;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.WebTestCase;

public class EditDistributionListTestUi extends WebTestCase {
    final String NEW_USER_USERNAME[] = {
        "1_NewUserUsername", "2_NewUserUsername"
    };
    final String NEW_USER_PWORD = "1234";
    final String NEW_USER_ALIASES[] = {
        "user1, 1111", "user2, 2222"
    };

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditDistributionListTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        goToDistributionListsPage();
    }

    private void goToDistributionListsPage() {
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("resetVoicemail");
        clickLink("loginFirstTestUser");
        clickLink("toggleNavigation");
        clickLink("menu.myInformation");
        clickLink("link:distributionLists");
    }

    public void testSetListWithValidUser() throws Exception {
        SiteTestHelper.assertNoException(getTester());
        setFormElement("subject", "200");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        assertFormElementEquals("subject", "200");
    }

    public void testInexistentUser() throws Exception {
        SiteTestHelper.assertNoException(tester);
        setFormElement("subject", "2000000000");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        assertFormElementEquals("subject", "2000000000");
    }

    public void testSetListWithValidUserAndNoVoicemailPermission() throws Exception {

        addNewUser(0);
        setNoVoicemailPermission(0);

        goToDistributionListsPage();

        SiteTestHelper.assertNoException(tester);
        setFormElement("subject", "1111");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        assertFormElementEquals("subject", "1111");
    }

    public void testSetListWithTwoValidUsers() throws Exception {
        addNewUser(1);

        goToDistributionListsPage();

        SiteTestHelper.assertNoException(tester);
        setFormElement("subject", "200");
        setFormElement("subject_0", "2222");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        assertFormElementEquals("subject", "200");
        assertFormElementEquals("subject_0", "2222");
    }

    public void testSetOneListWithTwoValidUsers() throws Exception {

        goToDistributionListsPage();

        SiteTestHelper.assertNoException(tester);
        setFormElement("subject", "200 2222");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        assertFormElementEquals("subject", "200 2222");
    }

    public void testSetTwoListsWithTwoValidUsers() throws Exception {

        goToDistributionListsPage();

        SiteTestHelper.assertNoException(tester);
        setFormElement("subject", "200");
        setFormElement("subject_0", "2222");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        assertFormElementEquals("subject", "200");
        assertFormElementEquals("subject_0", "2222");
    }

    private void setNoVoicemailPermission(int i) {
        SiteTestHelper.home(tester);
        clickLink("ManageUsers");

        clickLinkWithText(NEW_USER_USERNAME[i]);
        clickLinkWithText("permission");
        uncheckCheckbox("booleanField_8");
        clickButton("setting:ok");
    }

    private void addNewUser(int i) {
        SiteTestHelper.home(tester);
        clickLink("NewUser");
        setFormElement("userId", NEW_USER_USERNAME[i]);
        setFormElement("password", NEW_USER_PWORD);
        setFormElement("confirmPassword", NEW_USER_PWORD);
        setFormElement("aliases", NEW_USER_ALIASES[i]);
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.assertNoException(tester);
    }
}

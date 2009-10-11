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

public class UserSettingsTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(UserSettingsTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.seedUser(tester);
        clickLink("ManageUsers");
        clickLinkWithText(SiteTestHelper.TEST_USER);
    }

    public void testDisplay() {
        clickLink("link:permission.label");
        SiteTestHelper.assertNoException(tester);
    }

    public void testSaveSetting() {
        clickLink("link:permission.label");
        checkCheckbox("setting:900Dialing");

        clickButton("form:ok");

        // click ok, then navigate back.  apply would work but this is more thurough.
        clickLinkWithText(SiteTestHelper.TEST_USER);
        clickLink("link:permission.label");
        assertCheckboxSelected("setting:900Dialing");
    }

    public void testSaveImId() {
        //test save im id and im display name
        clickLink("link:im.label");
        setTextField("user:imId","openfire1");
        setTextField("user:imDisplayName","openfireName1");
        setTextField("cp:password", "1234");
        setTextField("cp:confirmPassword", "1235");
        //verify password confirmation
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        setTextField("cp:password", "1234");
        setTextField("cp:confirmPassword", "1234");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        clickLinkWithText(SiteTestHelper.TEST_USER);
        clickLink("link:im.label");
        assertTextFieldEquals("user:imId", "openfire1");
        assertTextFieldEquals("user:imDisplayName", "openfireName1");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        //try to create user with the same instant message id: openfire1
        clickLink("AddUser");
        setTextField("user:userId", "y");
        setTextField("cp:password", "1234");
        setTextField("cp:confirmPassword", "1234");
        clickButton("form:apply");
        // click Instant Messaging tab
        clickLink("link:im.label");
        //set an invalid im id value
        setTextField("user:imId","!@#$%^");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        //set a duplicate im id value
        setTextField("user:imId","openfire1");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        //set a different instant message id
        setTextField("user:imId","openfire2");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
    }
}

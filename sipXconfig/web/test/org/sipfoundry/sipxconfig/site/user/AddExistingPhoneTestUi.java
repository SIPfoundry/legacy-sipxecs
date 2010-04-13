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
import org.sipfoundry.sipxconfig.site.phone.PhoneTestHelper;

public class AddExistingPhoneTestUi extends WebTestCase {

    private PhoneTestHelper m_helper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(UserPhonesTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester(), true);
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
    }

    public void testAddExistingPhone() throws Exception {
        //create new phone
        m_helper.seedPhone(1);

        /*clickLink("ManagePhones");
        SiteTestHelper.assertNoUserError(tester);*/

        //create new user
        clickLink("ManageUsers");
        SiteTestHelper.assertNoUserError(tester);
        clickLink("addUser");
        SiteTestHelper.assertNoUserError(tester);
        setTextField("userId", "test_user");
        setTextField("cp:password", "123");
        setTextField("cp:confirmPassword", "123");
        submit("form:ok");

        //assign created phone to user
        SiteTestHelper.assertNoUserError(tester);
        clickLinkWithText("test_user");
        SiteTestHelper.assertNoUserError(tester);
        clickLinkWithText("Phones");
        SiteTestHelper.assertNoUserError(tester);
        clickLink("addPhones");
        SiteTestHelper.assertNoUserError(tester);
        assertEquals(2,SiteTestHelper.getRowCount(tester, "phone:list"));
        SiteTestHelper.selectRow(tester, 1, true);
        submit("select");
        SiteTestHelper.assertNoUserError(tester);
        assertEquals(2, SiteTestHelper.getRowCount(tester, "phone:list"));

        //cleanup phone
        SiteTestHelper.home(getTester(), true);
        clickLink("ManagePhones");
        SiteTestHelper.assertNoUserError(tester);
        assertEquals(2, SiteTestHelper.getRowCount(tester, "phone:list"));
        SiteTestHelper.selectRow(tester, 1, true);
        submit("phone:delete");
        SiteTestHelper.assertNoUserError(tester);

        //cleanup user
        SiteTestHelper.home(getTester(), true);
        clickLink("ManageUsers");
        SiteTestHelper.assertNoUserError(tester);
        assertEquals(2, SiteTestHelper.getRowCount(tester, "user:list"));
        SiteTestHelper.selectRow(tester, 1, true);
        submit("user:delete");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testGroupFilter() {
        //create new phone
        m_helper.seedPhone(1);

        //assign created phone to user
        SiteTestHelper.assertNoUserError(tester);
        clickLinkWithText("test_user");
        SiteTestHelper.assertNoUserError(tester);
        clickLinkWithText("Phones");
        SiteTestHelper.assertNoUserError(tester);
        clickLink("addPhones");
        SiteTestHelper.assertNoUserError(tester);

        //all phones
        assertEquals(2,SiteTestHelper.getRowCount(tester, "phone:list"));

        //only unasigned phones
        SiteTestHelper.selectOption(tester, "group:filter", "- unassigned -");
        int noLinesTableCount = SiteTestHelper.getRowCount(tester, "phone:list");
        assertEquals(1, noLinesTableCount);

    }
}

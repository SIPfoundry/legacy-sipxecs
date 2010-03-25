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
import org.sipfoundry.sipxconfig.site.branch.BranchesPageTestUi;
import org.sipfoundry.sipxconfig.site.phone.PhoneTestHelper;


public class ManageUsersTestUi extends WebTestCase {

    private PhoneTestHelper m_helper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManageUsersTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
    }

    public void testDisplay() throws Exception {
        clickLink("ManageUsers");
        SiteTestHelper.assertNoException(tester);
    }

    public void testAddUser() throws Exception {
        SiteTestHelper.seedUser(tester);
        clickLink("ManageUsers");
        clickLink("AddUser");
        clickButton("form:cancel");
        SiteTestHelper.assertNoException(tester);
    }

    public void testUserInheritsGroupBranch() {
        //create two new branches
        BranchesPageTestUi.seedBranch(tester, 2);

        //create a group with branch: seedBranch0
        SiteTestHelper.home(tester);
        clickLink("UserGroups");
        clickLink("AddGroup");
        tester.setTextField("item:name", "group1");
        tester.selectOption("branchSelection", "seedBranch0");
        tester.clickButton("form:ok");

        //ceate a user with group group1
        SiteTestHelper.home(tester);
        clickLink("ManageUsers");
        clickLink("AddUser");
        tester.setTextField("user:userId", "x");
        tester.setTextField("cp:password", "1234");
        tester.setTextField("cp:confirmPassword", "1234");
        tester.setTextField("gms:groups", "group1");
        tester.clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);

        //ceate a user with group group1 and with a branch different than inherited group branch
        SiteTestHelper.home(tester);
        clickLink("ManageUsers");
        clickLink("AddUser");
        tester.setTextField("user:userId", "x");
        tester.setTextField("cp:password", "1234");
        tester.setTextField("cp:confirmPassword", "1234");
        tester.setTextField("gms:groups", "group1");
        tester.selectOption("branchSelection", "seedBranch1");
        tester.clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
    }

    public void testGroupFilter() throws Exception {
        SiteTestHelper.seedUser(tester);
        SiteTestHelper.seedGroup(tester, "NewUserGroup", 1);
        clickLink("ManageUsers");

        // all users
        int allTableCount = SiteTestHelper.getRowCount(tester, "user:list");

        // empty group, no users
        SiteTestHelper.selectOption(tester, "group:filter", "seedGroup0");
        SiteTestHelper.assertNoException(tester);
        int emptyTableCount = SiteTestHelper.getRowCount(tester, "user:list");
        assertTrue(allTableCount > emptyTableCount);

        // back to all users
        SiteTestHelper.selectOption(tester, "group:filter", "- all -");
        int allTableCountAgain = SiteTestHelper.getRowCount(tester, "user:list");
        assertEquals(allTableCount, allTableCountAgain);
    }

}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.setting;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.branch.BranchesPageTestUi;

public class GroupSettingsTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(GroupSettingsTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("resetCoreContext");
    }

    public void testDisplay() {
        seedGroup(1);
        clickLink("UserGroups");
        clickLinkWithText("seedGroup0");
        SiteTestHelper.assertNoException(getTester());
    }

    public void seedGroup(int count) {
        SiteTestHelper.seedGroup(getTester(), "NewUserGroup", count);
    }

    public void testGroupWithBranch() {
        BranchesPageTestUi.seedBranch(tester, 1);
        SiteTestHelper.home(tester);
        clickLink("UserGroups");
        clickLink("AddGroup");
        SiteTestHelper.assertNoUserError(tester);

        setWorkingForm("groupForm");
        assertTextFieldEquals("item:name", "");
        assertTextFieldEquals("item:description", "");
        assertElementPresent("branchSelection");
        assertSelectOptionPresent("branchSelection", "seedBranch0");
        assertButtonPresent("form:ok");

        setTextField("item:name", "groupWithBranch");
        setTextField("item:description", "description group with branch");
        SiteTestHelper.setScriptingEnabled(tester, true);
        selectOption("branchSelection", "seedBranch0");
        assertTextFieldEquals("item:name", "groupWithBranch");
        assertTextFieldEquals("item:description", "description group with branch");
        SiteTestHelper.assertNoUserError(tester);

        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);

        clickLinkWithText("groupWithBranch");
        SiteTestHelper.assertNoUserError(tester);
        assertTextFieldEquals("item:name", "groupWithBranch");
        assertTextFieldEquals("item:description", "description group with branch");
        assertSelectedOptionEquals("branchSelection", "seedBranch0");
        // reset branch
        SiteTestHelper.setScriptingEnabled(tester, true);
        selectOptionByValue("branchSelection", "");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        clickLinkWithText("groupWithBranch");
        assertSelectedOptionValueEquals("branchSelection", "");
    }

    public void testChangeGroupBranch() {
        //create two branches
        BranchesPageTestUi.seedBranch(tester, 2);
        //create a group with no branch
        SiteTestHelper.home(tester);
        clickLink("UserGroups");
        clickLink("AddGroup");
        tester.setTextField("item:name", "group1");
        tester.clickButton("form:ok");

        //ceate a user with group group1 and branch seedBranch0
        SiteTestHelper.home(tester);
        clickLink("ManageUsers");
        clickLink("AddUser");
        tester.setTextField("user:userId", "x");
        tester.setTextField("cp:password", "1234");
        tester.setTextField("cp:confirmPassword", "1234");
        tester.setTextField("gms:groups", "group1");
        tester.selectOption("branchSelection", "seedBranch0");
        tester.clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);

        //try to change the group branch - it fails
        SiteTestHelper.home(tester);
        tester.clickLink("UserGroups");
        tester.clickLinkWithExactText("group1");
        tester.selectOption("branchSelection", "seedBranch1");
        tester.clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
    }

    public void testEditGroup() {
        seedGroup(1);
        clickLink("UserGroups");
        clickLinkWithText("seedGroup0");
        clickLink("link:configure");
        assertTextFieldEquals("item:name", "seedGroup0");
        // Pick a group name that is very unlikely to collide with any previous names
        setTextField("item:name", "edit-seed-test-" + System.currentTimeMillis());
        clickButton("form:apply");
        SiteTestHelper.assertNoException(getTester());
        assertLinkPresent("link:configure");
    }

    public void testAddSchedules() {
        seedGroup(1);
        clickLink("UserGroups");
        clickLinkWithText("seedGroup0");
        assertLinkPresent("link:schedules");
        clickLink("link:schedules");
        assertLinkPresent("group:addSchedules");
        clickLink("group:addSchedules");
        assertFormElementPresent("item:name");
        assertFormElementPresent("item:description");
        assertLinkPresent("addPeriod");
        assertButtonPresent("form:ok");
        assertButtonPresent("form:apply");
        clickButton("form:cancel");
        SiteTestHelper.assertNoUserError(tester);
    }
}

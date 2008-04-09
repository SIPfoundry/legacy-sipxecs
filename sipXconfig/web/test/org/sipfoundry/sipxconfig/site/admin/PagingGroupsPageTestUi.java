/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.dialplan.EditAutoAttendantTestUi;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class PagingGroupsPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(PagingGroupsPageTestUi.class);
    }

    public void setUp() {
        reloadPage();
    }

    public void testPagingGroups() throws Exception {
        deleteAllGroups();
        reloadPage();
        createGroup();
        reloadPage();
        editGroup();
        reloadPage();
        deleteAllGroups();
    }

    private void editGroup() {
        assertLinkPresentWithText("1");
        clickLinkWithText("1");
        assertElementPresent("enableGroup");
        assertCheckboxSelected("enableGroup");
        assertElementPresent("number");
        assertTextFieldEquals("number", "1");
        assertElementPresent("description");
        assertTextFieldEquals("description", "test group");
        assertButtonPresent("form:cancel");
        clickButton("form:cancel");
    }

    private void createGroup() {
        assertLinkPresent("link.addPagingGroup");
        clickLink("link.addPagingGroup");
        assertElementPresent("enableGroup");
        checkCheckbox("enableGroup");
        assertElementPresent("number");
        setTextField("number", "1");
        assertElementPresent("description");
        setTextField("description", "test group");
        assertElementPresent("enableGroup");
        assertCheckboxSelected("enableGroup");
        assertElementPresent("number");
        assertTextFieldEquals("number", "1");
        assertElementPresent("description");
        assertTextFieldEquals("description", "test group");
        assertFormElementPresent("prompt");
        SiteTestHelper.initUploadFieldsWithFile(tester, TestUtil
                .getTestSourceDirectory(EditAutoAttendantTestUi.class)
                + "/" + EditAutoAttendantTestUi.PROMPT_TEST_FILE);
        assertButtonPresent("form:ok");
        clickButton("form:ok");
    }

    private void reloadPage() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("toggleNavigation");
        clickLink("menu.pagingGroups");
    }

    private void deleteAllGroups() {
        int rowCount = SiteTestHelper.getRowCount(getTester(), "pagingGroups:list");
        if (rowCount > 1) {
            for (int i = 1; i < rowCount; i++) {
                SiteTestHelper.selectRow(getTester(), i - 1, true);
            }
            clickButton("pagingGroups:delete");
            assertEquals(SiteTestHelper.getRowCount(getTester(), "pagingGroups:list"), 1);
        }
    }

    public void testAddPagingGroupWithSameNumber() throws Exception {
        SiteTestHelper.assertNoUserError(tester);
        assertLinkPresent("link.addPagingGroup");
        clickLink("link.addPagingGroup");
        setTextField("number", "1");
        SiteTestHelper.initUploadFieldsWithFile(tester, TestUtil
                .getTestSourceDirectory(EditAutoAttendantTestUi.class)
                + "/" + EditAutoAttendantTestUi.PROMPT_TEST_FILE);
        clickButton("form:ok");

        SiteTestHelper.assertNoUserError(tester);
        clickLink("link.addPagingGroup");
        setTextField("number", "1");
        SiteTestHelper.initUploadFieldsWithFile(tester, TestUtil
                .getTestSourceDirectory(EditAutoAttendantTestUi.class)
                + "/" + EditAutoAttendantTestUi.PROMPT_TEST_FILE);
        clickButton("form:ok");

        SiteTestHelper.assertUserError(tester);
    }

    public void testPagingDescriptionMax255Characters() throws Exception {
        StringBuilder description = new StringBuilder();
        for (int i=0; i< 256; i++) {
            description.append("a");
        }

        SiteTestHelper.assertNoUserError(tester);
        assertLinkPresent("link.addPagingGroup");
        clickLink("link.addPagingGroup");
        setTextField("number", "2");
        setTextField("description", description.toString());
        SiteTestHelper.initUploadFieldsWithFile(tester, TestUtil
                .getTestSourceDirectory(EditAutoAttendantTestUi.class)
                + "/" + EditAutoAttendantTestUi.PROMPT_TEST_FILE);
        clickButton("form:ok");

        SiteTestHelper.assertUserError(tester);
    }
}

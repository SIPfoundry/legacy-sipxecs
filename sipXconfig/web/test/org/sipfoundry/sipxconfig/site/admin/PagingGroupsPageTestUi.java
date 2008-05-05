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
    private String m_fileName = TestUtil.getTestSourceDirectory(EditAutoAttendantTestUi.class)
            + "/" + EditAutoAttendantTestUi.PROMPT_TEST_FILE;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(PagingGroupsPageTestUi.class);
    }

    public void setUp() {
        reloadPage();
        deleteAllGroups();
    }

    public void testPagingGroups() throws Exception {
        assertElementNotPresent("prefix");
        assertButtonNotPresent("pagingGroups:save");
        
        createGroup();
        reloadPage();
        
        assertElementPresent("prefix");
        assertTextFieldEquals("prefix", "*77");
        assertButtonPresent("pagingGroups:save");        
        
        editGroup();
        reloadPage();
        deleteAllGroups();
        
        assertElementNotPresent("prefix");
        assertButtonNotPresent("pagingGroups:save");        
    }

    private void editGroup() {
        assertLinkPresentWithText("1");
        clickLink("editRowLink");
        setWorkingForm("Form");
        assertCheckboxSelected("enableGroup");
        assertTextFieldEquals("number", "1");
        assertTextFieldEquals("description", "test group");
        assertTextFieldEquals("timeout", "60");
        assertButtonPresent("form:cancel");
        clickButton("form:cancel");
    }

    private void createGroup() {
        clickLink("link.addPagingGroup");
        SiteTestHelper.assertNoUserError(tester);
        setWorkingForm("Form");
        checkCheckbox("enableGroup");
        setTextField("number", "1");
        setTextField("description", "test group");
        setTextField("timeout", "60");
        setTextField("promptUpload", m_fileName);
        assertButtonPresent("form:ok");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
    }

    private void reloadPage() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("toggleNavigation");
        clickLink("menu.pagingGroups");
    }

    private void deleteAllGroups() {
        setWorkingForm("pagingGroups");
        int rowCount = SiteTestHelper.getRowCount(tester, "pagingGroups:list");
        if (rowCount <= 1) {
            return;
        }
        for (int i = 0; i < rowCount - 1; i++) {
            SiteTestHelper.selectRow(tester, i, true);
        }
        clickButton("pagingGroups:delete");
        assertEquals(1, SiteTestHelper.getRowCount(tester, "pagingGroups:list"));
    }

    public void testAddPagingGroupWithSameNumber() throws Exception {
        SiteTestHelper.assertNoUserError(tester);
        assertLinkPresent("link.addPagingGroup");
        clickLink("link.addPagingGroup");
        setTextField("number", "1");
        setTextField("timeout", "60");
        setTextField("promptUpload", m_fileName);
        clickButton("form:ok");

        SiteTestHelper.assertNoUserError(tester);
        clickLink("link.addPagingGroup");
        setTextField("number", "1");
        setTextField("timeout", "60");
        setTextField("promptUpload", m_fileName);
        clickButton("form:ok");

        SiteTestHelper.assertUserError(tester);
    }

    public void testPagingDescriptionMax255Characters() throws Exception {
        StringBuilder description = new StringBuilder();
        for (int i = 0; i < 256; i++) {
            description.append("a");
        }

        SiteTestHelper.assertNoUserError(tester);
        assertLinkPresent("link.addPagingGroup");
        clickLink("link.addPagingGroup");
        setTextField("number", "2");
        setTextField("description", description.toString());
        setTextField("timeout", "60");
        setTextField("promptUpload", m_fileName);
        clickButton("form:ok");

        SiteTestHelper.assertUserError(tester);
    }
}

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
import net.sourceforge.jwebunit.WebTestCase;

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
        assertElementPresent("prefix");
        assertFormElementEquals("prefix", "*77");
        assertLinkPresentWithText("1");
        clickLinkWithText("1");
        assertElementPresent("enableGroup");
        assertCheckboxSelected("enableGroup");
        assertElementPresent("number");
        assertFormElementEquals("number", "1");
        assertElementPresent("description");
        assertFormElementEquals("description", "test group");
        assertButtonPresent("form:cancel");
        clickButton("form:cancel");
    }

    private void createGroup() {
        assertLinkPresent("link.addPagingGroup");
        clickLink("link.addPagingGroup");
        assertElementPresent("prefix");
        setFormElement("prefix", "*77");
        assertElementPresent("enableGroup");
        checkCheckbox("enableGroup");
        assertElementPresent("number");
        setFormElement("number", "1");
        assertElementPresent("description");
        setFormElement("description", "test group");
        assertElementPresent("prefix");
        assertFormElementEquals("prefix", "*77");
        assertElementPresent("enableGroup");
        assertCheckboxSelected("enableGroup");
        assertElementPresent("number");
        assertFormElementEquals("number", "1");
        assertElementPresent("description");
        assertFormElementEquals("description", "test group");
        assertFormElementPresent("prompt");
        SiteTestHelper.initUploadFieldsWithFile(getDialog().getForm(), TestUtil
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
                SiteTestHelper.selectRow(getTester(), i-1, true);
            }
            clickButton("pagingGroups:delete");
            assertEquals(SiteTestHelper.getRowCount(getTester(), "pagingGroups:list"), 1);
        }
    }
}

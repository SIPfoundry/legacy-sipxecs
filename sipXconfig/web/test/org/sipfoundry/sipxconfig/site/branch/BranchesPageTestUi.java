/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.branch;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTester;
import org.sipfoundry.sipxconfig.site.ListWebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import static org.apache.commons.lang.StringUtils.EMPTY;

public class BranchesPageTestUi extends ListWebTestCase {

    public BranchesPageTestUi() {
        super("link:branches", "resetBranches", "branch");
        setPager(true);
        setHasDuplicate(false);
        setExactCheck(false);
    }

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(BranchesPageTestUi.class);
    }

    @Override
    protected String[] getParamNames() {
        return new String[] {
            "item:name", "item:description", "street", "city", "state", "country", "zip", "phoneNumber",
            "faxNumber", "designation"
        };
    }

    @Override
    protected String[] getParamValues(int i) {
        return new String[] {
            "branch" + i, "branch description" + i, "street" + i, "city" + i, "state" + i, "country" + i, "zip" + i,
            "number" + i, "number" + i, "1000" + i
        };
    }

    @Override
    protected Object[] getExpectedTableRow(String[] paramValues) {
        return new String[] {
            "unchecked", paramValues[0], EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, paramValues[1]
        };
    }

    public void testSearch() throws Exception {
        BranchesPageTestUi.seedBranch(tester, 2);

        // all users
        int allTableCount = SiteTestHelper.getRowCount(tester, "branch:list");

        SiteTestHelper.selectOptionByValue(tester, "group:filter", "label.search");
        SiteTestHelper.assertNoException(tester);
        setTextField("group:query", "XXX");
        clickButton("group:submit");

        int tableCount = SiteTestHelper.getRowCount(tester, "branch:list");
        // header + footer + 0 rows visible...
        assertEquals(2, tableCount);

        // back to all branches
        SiteTestHelper.selectOptionByValue(tester, "group:filter", "label.all");
        int allTableCountAgain = SiteTestHelper.getRowCount(tester, "branch:list");
        assertEquals(allTableCount, allTableCountAgain);
    }

    /**
     * Create a new branch
     *
     * @param pageLinkId From the TestPage, what link to click to get to new branch page
     */
    public static void seedBranch(WebTester tester, int count) {
        for (int i = 0; i < count; i++) {
            SiteTestHelper.home(tester);
            tester.clickLink("link:branches");
            tester.clickLink("branch:add");
            tester.setWorkingForm("branchForm");
            tester.setTextField("item:name", "seedBranch" + i);
            tester.clickButton("form:ok");
        }
    }

    public void testBranchCrud() {
        // create new branch
        SiteTestHelper.home(tester);
        SiteTestHelper.setScriptingEnabled(getTester(), true);
        tester.clickLink("link:branches");
        clickLink("branch:add");
        setWorkingForm("branchForm");
        // name should not be empty
        clickButton("form:ok");
        SiteTestHelper.assertUserError(getTester());
        setTextField("item:name", "Earth");
        clickButton("form:ok");

        // create 2nd branch
        clickLink("branch:add");
        setWorkingForm("branchForm");
        // should fail if an existing name provided
        setTextField("item:name", "Earth");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(getTester());
        setTextField("item:name", "Moon");
        clickButton("form:ok");

        // modify branch name
        clickLinkWithExactText("Moon");
        // should fail if an existing name provided
        setWorkingForm("branchForm");
        setTextField("item:name", "Earth");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(getTester());
        // change name
        setTextField("item:name", "Mars");
        clickButton("form:ok");

        // modify branch properties other than name
        clickLinkWithExactText("Mars");
        setWorkingForm("branchForm");
        setTextField("item:description", "Planet");
        clickButton("form:ok");

        // delete all branches
        setExpectedJavaScriptConfirm("Are you sure you want to delete the selected branches?", true);
        clickElementByXPath("//input[@onclick='setCheckboxGroup(this.checked)']");
        clickButton("branch:delete");
        SiteTestHelper.assertNoUserError(getTester());

    }
}

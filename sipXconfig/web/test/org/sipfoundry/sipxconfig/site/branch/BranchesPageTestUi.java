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
    }

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(BranchesPageTestUi.class);
    }

    @Override
    protected String[] getParamNames() {
        return new String[] {
            "item:name", "item:description", "street", "city", "state", "country", "zip",
            "item:phoneNumber", "item:faxNumber"
        };
     }

    @Override
    protected String[] getParamValues(int i) {
        return new String[] {
            "branch" + i, "branch description" + i, "street" + i, "city" + i, "state" + i,
             "country" + i, "zip" + i, "number" + i, "number" + i
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
        SiteTestHelper.selectOptionByValue(tester, "group:filter",  "label.all");
        int allTableCountAgain = SiteTestHelper.getRowCount(tester, "branch:list");
        assertEquals(allTableCount, allTableCountAgain);
    }

    /**
     * Create a new branch
     *
     * @param pageLinkId From the TestPage, what link to click to get to new branch page
     */
    public static void seedBranch(WebTester tester, int count) {
        SiteTestHelper.home(tester);
        tester.clickLink("link:branches");
        for (int i = 0; i < count; i++) {
            tester.clickLink("branch:add");
            tester.setWorkingForm("branchForm");
            tester.setTextField("item:name", "seedBranch" + i);
            tester.clickButton("form:ok");
        }
    }
}

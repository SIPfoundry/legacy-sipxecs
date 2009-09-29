/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class ManageAttendantsTestUi extends WebTestCase {

    private static final String SEED_DESCRIPTION = "ManageAttendantsTestUi seed description";

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManageAttendantsTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("resetDialPlans");
        clickLink("ManageAttendants");
        SiteTestHelper.assertNoException(tester);
    }

    public void testAddAttendants() {
        seedAttendants(3);
        String[][] expectedData = {
            // Name Ext Description
            {
                "unchecked", "ManageAttendantsTestUi 0", SEED_DESCRIPTION
            }, {
                "unchecked", "ManageAttendantsTestUi 1", SEED_DESCRIPTION
            }, {
                "unchecked", "ManageAttendantsTestUi 2", SEED_DESCRIPTION
            }
        };
        assertTableRowsEqual("list:attendant", 1, expectedData);
    }

    public void testEditAttendants() {
        seedAttendants(2);
        clickLinkWithText("ManageAttendantsTestUi 1");
        assertElementPresent("attendant:menuItems");
        setTextField("item:name", "Name edited");
        String file = TestUtil.getTestSourceDirectory(EditAutoAttendantTestUi.class) + "/"
                + EditAutoAttendantTestUi.PROMPT_TEST_FILE;
        setTextField("promptUpload", file);
        clickButton("form:ok");
        String[][] expectedData = {
            // Name Ext Description
            {
                "unchecked", "ManageAttendantsTestUi 0", SEED_DESCRIPTION
            }, {
                "unchecked", "Name edited", SEED_DESCRIPTION
            }
        };
        assertTableRowsEqual("list:attendant", 1, expectedData);
    }

    public void testDeleteAttendants() {
        seedAttendants(4);
        // delete 2nd and last for no brilliant reason
        SiteTestHelper.selectRow(tester, 1, true);
        SiteTestHelper.selectRow(tester, 3, true);
        clickButton("list:attendant:delete");
        String[][] expectedData = {
            // Name Ext Description
            {
                "unchecked", "ManageAttendantsTestUi 0", SEED_DESCRIPTION
            }, {
                "unchecked", "ManageAttendantsTestUi 2", SEED_DESCRIPTION
            }
        };
        assertTableRowsEqual("list:attendant", 1, expectedData);
    }

    private void seedAttendants(int count) {
        for (int i = 0; i < count; i++) {
            clickLink("addAttendant");
            setTextField("item:name", "ManageAttendantsTestUi " + i);
            setTextField("item:description", SEED_DESCRIPTION);
            String file = TestUtil.getTestSourceDirectory(EditAutoAttendantTestUi.class) + "/"
                    + EditAutoAttendantTestUi.PROMPT_TEST_FILE;
            setTextField("promptUpload", file);
            clickButton("form:ok");
            SiteTestHelper.assertNoUserError(tester);
        }
    }
}

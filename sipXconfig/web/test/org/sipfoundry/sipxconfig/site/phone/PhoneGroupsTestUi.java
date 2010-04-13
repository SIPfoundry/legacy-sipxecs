/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class PhoneGroupsTestUi extends WebTestCase {

    private PhoneTestHelper m_phoneTester;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(PhoneGroupsTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        m_phoneTester = new PhoneTestHelper(getTester());
    }

    public void testDisplay() {
        m_phoneTester.reset();
        m_phoneTester.seedGroup(2);
        SiteTestHelper.home(getTester());
        clickLink("PhoneGroups");
        SiteTestHelper.assertNoException(getTester());
        String[][] table = new String[][] {
            {
                "unchecked", "1. seedGroup0", ""
            }, {
                "unchecked", "2. seedGroup1", ""
            },
        };
        assertTableRowsEqual("group:list", 2, table);
    }

    public void testMoveGroups() {
        m_phoneTester.reset();
        m_phoneTester.seedGroup(3);
        SiteTestHelper.home(getTester());
        clickLink("PhoneGroups");
        SiteTestHelper.selectRow(getTester(), 1, true);

        // move up
        clickButton("group:moveUp");
        SiteTestHelper.assertNoException(getTester());
        String[][] tableUp = new String[][] {
            {
                "checked", "1. seedGroup1", ""
            }, {
                "unchecked", "2. seedGroup0", ""
            }, {
                "unchecked", "3. seedGroup2", ""
            },
        };
        assertTableRowsEqual("group:list", 2, tableUp);

        // move down
        clickButton("group:moveDown");
        SiteTestHelper.assertNoException(getTester());
        String[][] tableDown = new String[][] {
            {
                "unchecked", "1. seedGroup0", ""
            }, {
                "checked", "2. seedGroup1", ""
            }, {
                "unchecked", "3. seedGroup2", ""
            },
        };
        assertTableRowsEqual("group:list", 2, tableDown);
    }

    public void testDelete() {
        m_phoneTester.reset();
        m_phoneTester.seedGroup(3);
        SiteTestHelper.home(getTester());
        clickLink("PhoneGroups");
        SiteTestHelper.selectRow(getTester(), 1, true);
        clickButton("group:delete");
        SiteTestHelper.assertNoException(getTester());
        String[][] table = new String[][] {
            {
                "unchecked", "1. seedGroup0", ""
            }, {
                "unchecked", "2. seedGroup2", ""
            },
        };
        assertTableRowsEqual("group:list", 2, table);
    }
}

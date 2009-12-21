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

public class ManagePhonesTestUi extends WebTestCase {

    private PhoneTestHelper m_helper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManagePhonesTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
    }

    public void testGenerateProfiles() {
        m_helper.seedPhone(1);

        clickLink("ManagePhones");
        SiteTestHelper.enableCheckbox(tester, "checkbox", 0, true);
        checkCheckbox("checkbox");
        clickButton("phone:sendProfiles");
        SiteTestHelper.assertNoException(tester);
        // test if profiles were actually generated
    }

    public void testRestart() {
        m_helper.seedPhone(1);

        clickLink("ManagePhones");
        SiteTestHelper.enableCheckbox(tester, "checkbox", 0, true);
        clickButton("phone:restart");
        SiteTestHelper.assertNoException(tester);
        // test if SIP messages were sent
    }

    public void testDelete() {
        m_helper.seedPhone(1);

        clickLink("ManagePhones");
        SiteTestHelper.enableCheckbox(tester, "checkbox", 0, true);
        clickButton("phone:delete");
        // 2 = 1 thead (columns) + 1 tfoot (pager)
        assertEquals(2, SiteTestHelper.getRowCount(tester, "phone:list"));

        SiteTestHelper.assertNoException(tester);
    }

    public void testGroupFilter() throws Exception {
        m_helper.seedPhone(1);
        SiteTestHelper.seedGroup(tester, "NewPhoneGroup", 1);
        clickLink("ManagePhones");

        // all users
        int allTableCount = SiteTestHelper.getRowCount(tester, "phone:list");

        // empty group, no users
        SiteTestHelper.selectOption(tester, "group:filter", "seedGroup0");
        SiteTestHelper.assertNoException(tester);
        int emptyTableCount = SiteTestHelper.getRowCount(tester, "phone:list");
        assertTrue(allTableCount > emptyTableCount);

        // back to all users
        SiteTestHelper.selectOption(tester, "group:filter", "- all -");
        int allTableCountAgain = SiteTestHelper.getRowCount(tester, "phone:list");
        assertEquals(allTableCount, allTableCountAgain);

        // add a line registered to the registered phone
        m_helper.seedLine(1);
        clickLink("ManagePhones");
        SiteTestHelper.selectOption(tester, "group:filter", "- unassigned -");
        int noLinesTableCount = SiteTestHelper.getRowCount(tester, "phone:list");
        assertEquals(2, noLinesTableCount);
    }
}

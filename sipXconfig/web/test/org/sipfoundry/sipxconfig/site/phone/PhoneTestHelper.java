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


import net.sourceforge.jwebunit.junit.WebTester;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.acme.AcmePhone;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

/**
 * Helper methods for phone base unittesting.
 *
 * Thought about making this a helper w/static functions but webunit methods are protected, as
 * they probably should be.
 */
public class PhoneTestHelper {

    public Phone[] endpoint;

    public Line[] line;

    private WebTester m_tester;

    public PhoneTestHelper(WebTester tester) {
        m_tester = tester;
    }

    public void reset() {
        SiteTestHelper.home(m_tester);
        m_tester.clickLink("resetPhoneContext");
        SiteTestHelper.assertNoException(m_tester);
    }

    public void seedPhone(int count) {
	    SiteTestHelper.home(m_tester);
	    endpoint = new Phone[count];
	    for (int i = 0; i < endpoint.length; i++) {
            endpoint[i] = new AcmePhone();
            String serNum = "000000000000" + i;
            endpoint[i].setSerialNumber(serNum.substring(serNum.length() - 12));
            m_tester.clickLink("NewPhone");
            m_tester.setTextField("phone:serialNumber", endpoint[i].getSerialNumber());
            m_tester.clickButton("form:ok");
            SiteTestHelper.home(m_tester);
	    }
    }

    public void seedLine(int count) {
        SiteTestHelper.seedUser(m_tester);
        seedPhone(1);
        SiteTestHelper.home(m_tester);
        m_tester.clickLink("ManagePhones");
        m_tester.clickLinkWithText(endpoint[0].getSerialNumber());
        m_tester.clickLinkWithText("Lines");
        line = new Line[count];
        for (int i = 0; i < line.length; i++) {
            line[0] = endpoint[0].createLine();
            User testUser = new User();
            testUser.setUserName(SiteTestHelper.TEST_USER);
            line[0].setUser(testUser);
            m_tester.clickLink("AddUser");
            m_tester.clickButton("user:search");
            // first (should be only?) row
            SiteTestHelper.assertNoException(m_tester);
            SiteTestHelper.selectRow(m_tester, 0, true);
            m_tester.clickButton("user:select");
        }
        SiteTestHelper.home(m_tester);
    }

    public void seedGroup(int count) {
        SiteTestHelper.seedGroup(m_tester, "NewPhoneGroup", count);
    }
}

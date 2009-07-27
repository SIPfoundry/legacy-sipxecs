/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.conference;

import net.sourceforge.jwebunit.junit.WebTester;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.TestPage;

/**
 * Helper class for tests that require bridges or conferences to be created.
 */
public class ConferenceTestHelper {

    private final WebTester m_tester;

    public ConferenceTestHelper(WebTester tester) {
        m_tester = tester;
    }

    public void createBridge() {
        SiteTestHelper.home(m_tester);
        m_tester.clickLink("createTestBridge");
    }

    public void groupConferenceAutomation(String seedGroup) {
	    SiteTestHelper.home(m_tester);
        m_tester.clickLink("UserGroups");
        m_tester.clickLinkWithText(seedGroup);
        m_tester.clickLink("link:conference");
        m_tester.checkCheckbox("conferences:enable");
        m_tester.setTextField("conferences:prefix", "1000");
        m_tester.selectOption("bridgeSelect", TestPage.TEST_LOCATION_FQDN);
        m_tester.submit("submit:ok");
	    SiteTestHelper.home(m_tester);

    }

    public void addUserToGroup(int count, String seedGroup, int offset) {
	SiteTestHelper.home(m_tester);
	    for (int i = 0; i < count; i++) {
            //Create a new user and assign it to the group.
            m_tester.clickLink("NewUser");
            m_tester.setTextField("user:userId", Integer.toString(offset+i));
            m_tester.setTextField("cp:password", "12345");
            m_tester.setTextField("cp:confirmPassword", "12345");
            m_tester.setTextField("gms:groups", seedGroup);
            m_tester.submit("form:apply");

            SiteTestHelper.home(m_tester);
	    }
    }
}

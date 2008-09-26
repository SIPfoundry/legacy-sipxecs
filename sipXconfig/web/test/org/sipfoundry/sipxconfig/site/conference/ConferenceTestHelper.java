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

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import net.sourceforge.jwebunit.junit.WebTester;

/**
 * Helper class for tests that require bridges or conferences to be created.
 */
public class ConferenceTestHelper {

    private WebTester m_tester;
    
    public ConferenceTestHelper(WebTester tester) {
        m_tester = tester;
    }
    
    public void createBridge(String name) {
        SiteTestHelper.home(m_tester);
        m_tester.clickLink("resetConferenceBridgeContext");               
        m_tester.clickLink("EditBridge");
        
        m_tester.setWorkingForm("form");
        m_tester.setTextField("item:name", name);
        m_tester.checkCheckbox("item:enabled");
        m_tester.clickButton("form:apply");        
    }

    public void groupConferenceAutomation(String seedGroup, String bridge) {
	    SiteTestHelper.home(m_tester);
        m_tester.clickLink("UserGroups");
        m_tester.clickLinkWithText(seedGroup);
        m_tester.clickLink("link:conferences");
        m_tester.checkCheckbox("conferences:enable");
        m_tester.setTextField("conferences:offset", "1000");
        m_tester.selectOption("bridgeSelect", bridge);
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

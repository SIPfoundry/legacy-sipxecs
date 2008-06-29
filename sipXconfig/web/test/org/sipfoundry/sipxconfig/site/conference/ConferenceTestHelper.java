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
}

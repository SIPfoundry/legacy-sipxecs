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

import static org.sipfoundry.sipxconfig.site.SiteTestHelper.getBaseUrl;
import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditConferenceTestUi extends WebTestCase {

    private static final String TEST_CONFERENCE_NAME = "200-test-conf";
    
    private ConferenceTestHelper m_helper;    
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditConferenceTestUi.class);
    }
    
    public void setUp() {
        m_helper = new ConferenceTestHelper(tester);
        getTestContext().setBaseUrl(getBaseUrl());
        SiteTestHelper.home(tester);
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("resetConferenceBridgeContext");
        m_helper.createBridge("testbridge");        
    }
    
    /**
     * Tests adding a conference from a User Conferences page.
     * This Edit Conference page should have an owner assigned, and should
     * prompt for a conference bridge.
     */
    public void testAddUserConference() {
        SiteTestHelper.home(tester);
        clickLink("resetCoreContext");
        SiteTestHelper.home(tester);
        clickLink("ManageUsers");
        clickLinkWithText("testuser");
        clickLink("userConferencesLink");
        SiteTestHelper.clickSubmitLink(tester, "conference:add");
        assertElementPresent("bridgeSelect");
        assertTextPresent("testuser");
        assertTextNotPresent("(none)");
    }
    
    /**
     * Tests adding a conference from a Conference Bridge page.
     * This Edit Conference page should not prompt for a conference bridge.
     */    
    public void testAddConferenceFromBridge() {
        SiteTestHelper.home(tester);
        clickLink("ListBridges");
        clickLinkWithText("testbridge");
        clickLink("link:conferences");
        SiteTestHelper.clickSubmitLink(tester, "conference:add");
        assertElementNotPresent("bridgeSelect");
        assertTextPresent("(none)");
    }
    
}

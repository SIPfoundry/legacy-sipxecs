/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class UserSettingsTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(UserSettingsTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        SiteTestHelper.seedUser(tester);
        clickLink("ManageUsers");
        clickLinkWithText(SiteTestHelper.TEST_USER);        
    }
    
    public void testDisplay() {
        clickLink("link:permission.label");        
        SiteTestHelper.assertNoException(tester);        
    }

    public void testSaveSetting() {
        clickLink("link:permission.label");
        SiteTestHelper.enableCheckbox(tester, "booleanField", 0, true);
        
        clickButton("setting:ok");
        
        // click ok, then navigate back.  apply would work but this is more thurough.
        clickLinkWithText(SiteTestHelper.TEST_USER);        
        clickLink("link:permission.label");
        assertCheckboxSelected("booleanField");
    }    
}

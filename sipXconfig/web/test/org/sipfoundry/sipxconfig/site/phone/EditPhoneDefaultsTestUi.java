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

public class EditPhoneDefaultsTestUi extends WebTestCase {
    
    PhoneTestHelper tester;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditPhoneDefaultsTestUi.class);
    }
    
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        tester = new PhoneTestHelper(getTester());
    }
    
    public void testBooleanField() {
        tester.reset();
        tester.seedGroup(1);
        clickLink("PhoneGroups");
        clickLinkWithText("seedGroup0");
        clickLinkWithText("Polycom SoundPoint IP 300/301");
        setWorkingForm("settingsForm");
        checkCheckbox("setting:24HourClock");
        clickButton("setting:ok");
        clickLinkWithText("Polycom SoundPoint IP 300/301");        
        setWorkingForm("settingsForm");
        assertCheckboxSelected("setting:24HourClock");
    }
}

/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class RestartReminderTestUi extends WebTestCase {

    public static final String RESTART_REMINDER_LINK = "RestartReminder";

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(RestartReminderTestUi.class);
    }
    
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink(RESTART_REMINDER_LINK);
    }
    
    public void testRestartNow() {
        SiteTestHelper.assertNoException(tester);
        assertElementPresent("restart:form");
        
        uncheckCheckbox("restart:later");
        
        clickButton("restart:save");

        // make sure that you get user error about not being able to access server
        SiteTestHelper.assertUserError(tester);
    }
    
    public void testRestartLater() {
        SiteTestHelper.assertNoException(tester);
        assertElementPresent("restart:form");

        checkCheckbox("restart:later");
        
        clickButton("restart:save");
        
        // should be back at the test page
        assertLinkPresent(RESTART_REMINDER_LINK);
    }

}

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
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class RestartReminderTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(RestartReminderTestUi.class);
    }
    
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink(RestartReminder.PAGE);
    }
    
    public void testRestartNow() {
        SiteTestHelper.assertNoException(tester);
        assertElementPresent("restart:form");
        
        uncheckCheckbox("checkRestartLater");
        
        clickButton("restart:save");

        // should be back at the test page
        assertLinkPresent(RestartReminder.PAGE);
        // there will be some exceptions in the log - topology file is not found
    }
    
    public void testRestartLater() {
        SiteTestHelper.assertNoException(tester);
        assertElementPresent("restart:form");

        checkCheckbox("checkRestartLater");
        
        clickButton("restart:save");
        
        // should be back at the test page
        assertLinkPresent(RestartReminder.PAGE);
    }

}

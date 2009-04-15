/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.vm;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class MailboxPreferencesTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(MailboxPreferencesTestUi.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        SiteTestHelper.home(getTester());    
    }
    
    public void testDisabledVoicemail() throws Exception {
        clickLink("disableVoicemail");
        clickLink("NewUser");
        assertElementPresent("mailbox:disabled");
    }

    public void testEnabledVoicemail() throws Exception {
        clickLink("resetVoicemail");
        clickLink("NewUser");
        SiteTestHelper.assertNoException(tester);
        assertElementNotPresent("mailbox:disabled");
    }
}

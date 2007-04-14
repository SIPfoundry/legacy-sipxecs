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

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.WebTestCase;

public class SupervisorPermissionTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(SupervisorPermissionTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        SiteTestHelper.seedUser(tester);
        clickLink("ManageUsers");
        clickLinkWithText(SiteTestHelper.TEST_USER);        
    }

    public void testDisplay() throws Exception {
        clickLink("groupSupervisorLink");        
        SiteTestHelper.assertNoException(tester);                
    }
}

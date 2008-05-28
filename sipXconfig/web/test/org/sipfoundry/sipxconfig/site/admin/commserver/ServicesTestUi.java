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

public class ServicesTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ServicesTestUi.class);
    }
    
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("seedLocationsManager");
        clickLink("Services");
    }
    
    public void testDisplay() {
        // there will be user error - we cannot contact the host.example.org
        SiteTestHelper.dumpPage(tester);
        SiteTestHelper.assertUserError(tester);
    }
}

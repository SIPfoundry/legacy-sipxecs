/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.acd;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class AcdServerPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AcdServerPageTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("acdServerPage");
        SiteTestHelper.assertNoException(tester);
    }

    public void testDisplay() {
        assertLinkNotPresent("link:config");
        assertLinkNotPresent("link:lines");
        assertLinkNotPresent("link:queues");
        assertFormPresent();
        assertButtonPresent("form:apply");
    }
    
    public void testLinks() {
        assertButtonPresent("form:apply");

        clickButton("form:apply");
        assertLinkPresent("link:config");
        assertLinkPresent("link:lines");
        assertLinkPresent("link:queues");
        
        clickLink("link:lines");
        SiteTestHelper.assertNoException(tester);
        clickLink("link:queues");
        SiteTestHelper.assertNoException(tester);
        clickLink("link:config");
        SiteTestHelper.assertNoException(tester);        
    }   
}

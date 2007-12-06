/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditMyInformationTestUi extends WebTestCase {
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditMyInformationTestUi.class);
    }
    
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("seedTestUser");
        clickLink("loginFirstTestUser");
        clickLink("toggleNavigation");
    }
    
    /**
     * Test that the My Information page displays correctly.  The selected tab
     * when the page is first displayed should be the voicemail tab
     */
    public void testDisplay() {
        clickLink("menu.myInformation");
        SiteTestHelper.assertNoUserError(tester);
        assertTextPresent("Active greeting");
    }
    
    public void testTabNavigation() {
        clickLink("menu.myInformation");
        clickLink("link:menu");
        SiteTestHelper.assertNoUserError(tester);
        assertTextPresent("Override default AutoAttendant language");
        
        clickLink("link:distributionLists");
        SiteTestHelper.assertNoUserError(tester);
        assertTextPresent("Dialpad");
    }
}

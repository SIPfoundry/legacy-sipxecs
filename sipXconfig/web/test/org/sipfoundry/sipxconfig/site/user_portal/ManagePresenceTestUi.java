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

import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ManagePresenceTestUi extends WebTestCase {

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.seedUser(getTester());
        tester.clickLink("loginFirstTestUser");
        SiteTestHelper.home(getTester());
        tester.clickLink("managePresence");
    }

    public void testNotEnabled() {
        assertElementNotPresent("menu.managePresence");

        // not enabled by in unit test env.
        // system test should run tests
        assertElementPresent("presence-disabled");
    }
}

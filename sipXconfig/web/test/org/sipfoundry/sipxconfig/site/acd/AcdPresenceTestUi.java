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

import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class AcdPresenceTestUi extends WebTestCase {
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("acdPresenceServer");
        SiteTestHelper.assertNoException(tester);
    }

    public void testDisplay() {
        assertSubmitButtonPresent("signInButton");
        assertSubmitButtonPresent("signOutButton");
        assertSubmitButtonPresent("refresh");
    }

    public void testRefresh() {
        submit("refresh");
    }
}

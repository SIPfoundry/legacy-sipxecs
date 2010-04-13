/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class BorderTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(BorderTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
    }

    /**
     * Checks if navigation disappears/re-appears when we click the toggle
     */
    public void testToggleNavigation() {
        SiteTestHelper.assertNoException(getTester());
        assertElementNotPresent("navigation");
        assertElementPresent("content");

        clickLink("toggleNavigation");

        SiteTestHelper.assertNoException(getTester());
        assertElementPresent("navigation");
        assertElementPresent("content");

        clickLink("toggleNavigation");

        SiteTestHelper.assertNoException(getTester());
        assertElementNotPresent("navigation");
        assertElementPresent("content");
    }

    public void testLogout() {
        // display navigation and click logout link
        clickLink("toggleNavigation");
        clickLink("link.logout");

        // login form should be visible
        SiteTestHelper.assertNoException(getTester());
        assertElementPresent("loginForm");
        SiteTestHelper.assertNoUserError(getTester());
    }

    public void testHelp() {
        clickLink("toggleNavigation");
        assertLinkPresent("link.help");
    }
}

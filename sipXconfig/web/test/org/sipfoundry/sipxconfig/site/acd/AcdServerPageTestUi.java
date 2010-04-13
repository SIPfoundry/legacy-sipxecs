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
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class AcdServerPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AcdServerPageTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("seedAcdServer");
        clickLink("listAcdServers");
        SiteTestHelper.assertNoException(tester);
    }

    public void testLinks() {
        assertLinkPresentWithText("server.example.com");
        clickLink("editRowLink");

        assertLinkPresent("link:config");
        assertLinkPresent("link:lines");
        assertLinkPresent("link:queues");

        clickLink("link:lines");
        SiteTestHelper.assertNoException(tester);
        clickLink("link:queues");
        SiteTestHelper.assertNoException(tester);
        clickLink("link:config");
        SiteTestHelper.assertNoException(tester);

        resetAcdContext();
    }

    public void resetAcdContext() {
        SiteTestHelper.home(getTester());
        clickLink("resetAcdContext");
    }
}

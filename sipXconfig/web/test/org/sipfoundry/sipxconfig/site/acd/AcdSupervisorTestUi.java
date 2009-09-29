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

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.junit.WebTestCase;

public class AcdSupervisorTestUi extends WebTestCase {
    private static String[] NAVIGATION_LINKS = {
        "link:selectServer",
        "link:agentPresence",
        "link:agentsStats",
        "link:callsStats",
        "link:queuesStats"
    };

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AcdSupervisorTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
    }

    public void testDisplay() {
        clickLink("acdSupervisorConsole");
        SiteTestHelper.assertNoException(tester);
    }

    public void testNoServersMessage() {
        clickLink("acdSupervisorConsole");

        for (String link : NAVIGATION_LINKS) {
            clickLink(link);
            assertElementPresent("message.noServers");
        }
    }

    public void testOneServer() {
        clickLink("seedAcdServer");

        SiteTestHelper.home(getTester());
        clickLink("acdSupervisorConsole");
        SiteTestHelper.assertNoException(tester);
        for (String link : NAVIGATION_LINKS) {
            clickLink(link);
            if (link != "link:selectServer") {
                assertElementNotPresent("message.selectServer");
            }
        }

        assertStatisticsDisplay();
    }

    private void assertStatisticsDisplay() {
        clickLink(NAVIGATION_LINKS[1]);
        assertElementPresent("list.presence");
        clickLink(NAVIGATION_LINKS[2]);
        assertElementPresent("agent:stats:list");
        clickLink(NAVIGATION_LINKS[3]);
        assertElementPresent("calls:stats:list");
        clickLink(NAVIGATION_LINKS[4]);
        assertElementPresent("queues:stats:list");
    }
}

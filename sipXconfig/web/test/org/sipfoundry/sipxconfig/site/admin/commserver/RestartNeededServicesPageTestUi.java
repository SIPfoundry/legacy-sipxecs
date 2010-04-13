/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.junit.WebTestCase;

public class RestartNeededServicesPageTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(RestartNeededServicesPageTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
    }

    public void testDisplay() {
        SiteTestHelper.assertNoUserError(tester);
        clickLink("link:clearRestartNeeded");
        assertLinkNotPresent("restartNeededLink");

        clickLink("link:markForRestart");
        assertLinkPresent("restartNeededLink");
        clickLink("restartNeededLink");
        SiteTestHelper.assertNoUserError(tester);

        assertLinkNotPresent("restartNeededLink");
        assertTablePresent("restartNeededServices:list");
        assertButtonPresent("services:restart");
        assertButtonPresent("services:ignore");

        int rowCount = SiteTestHelper.getRowCount(tester, "restartNeededServices:list");
        // at least one service restart is needed
        assertTrue(1 < rowCount);

        SiteTestHelper.selectRow(tester, 0, true);
        submit("services:restart");
        SiteTestHelper.assertNoException(tester);
    }
}

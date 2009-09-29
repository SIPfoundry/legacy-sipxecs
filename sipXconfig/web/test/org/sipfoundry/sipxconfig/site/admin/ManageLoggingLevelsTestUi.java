/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ManageLoggingLevelsTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManageLoggingLevelsTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        tester.clickLink("toggleNavigation");
        clickLink("menu.loggingLevels");
    }

    public void testDisplay() {
        SiteTestHelper.assertNoException(tester);
        assertElementPresent("loggingLevel:generalLevel");
    }

    public void testShowAdvanced() {
        SiteTestHelper.assertNoException(tester);
        assertElementPresent("setting:toggle");
        clickLink("setting:toggle");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
    }
}

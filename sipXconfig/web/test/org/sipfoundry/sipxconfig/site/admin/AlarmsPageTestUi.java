/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class AlarmsPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AlarmsPageTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.setScriptingEnabled(tester, true);
        SiteTestHelper.home(tester);
        clickLink("toggleNavigation");
        clickLink("menu.Alarms");
    }

    public void testDisplayEdit() throws Exception {
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);

        assertLinkPresent("setting:toggle");
        assertElementPresent("enableEmailNotification");
        assertLinkPresent("addLink");
        assertElementPresent("TextField");
        assertTextFieldEquals("TextField", "sipxpbxuser@localhost");
        assertLinkPresent("deleteLink");
        assertButtonPresent("form:apply");

        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);

        assertTableNotPresent("alarm:list");
        assertButtonNotPresent("alarms:enableEmail");
    }
}

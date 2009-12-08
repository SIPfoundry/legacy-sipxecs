/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class LoginHistoryPageTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(LoginHistoryPageTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("toggleNavigation");
        clickLink("menu.loginhistory");
        SiteTestHelper.assertNoException(getTester());
    }

    public void testLoginHistoryPage() throws Exception {
        SiteTestHelper.assertNoException(getTester());
        assertFormElementPresent("type");
        assertElementPresent("datetimeDate");
        assertElementPresent("datetime:time");
        assertElementPresent("datetimeDate_0");
        assertElementPresent("datetime:time_0");
        assertFormElementPresent("user");
        assertFormElementPresent("remoteip");
        assertElementPresent("logs");
        SiteTestHelper.assertNoException(getTester());
        // FIXME: requires enabled javascript and dojo - incompatible with httpunit we are using
        //clickButton("Submit");
        SiteTestHelper.assertNoUserError(getTester());
    }

}

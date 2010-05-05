/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.io.IOException;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class IntercomPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(IntercomPageTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.setScriptingEnabled(tester, true);
        SiteTestHelper.home(tester);
        clickLink("seedLocationsManager");
        clickLink("toggleNavigation");
        clickLink("menu.intercom");
    }

    public void testIntercomPage() throws Exception {
        SiteTestHelper.assertNoException(tester);

        assertElementPresent("intercom:enable");
        assertElementPresent("intercom:prefix");
        setTextField("groups", "test");
        assertElementPresent("intercom:timeout");
        setTextField("intercom:timeout", "-1");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        setTextField("intercom:timeout", "20abc");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
    }

}

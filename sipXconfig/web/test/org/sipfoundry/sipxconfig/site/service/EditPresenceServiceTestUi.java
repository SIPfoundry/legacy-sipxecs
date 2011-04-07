/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.service;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditPresenceServiceTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditPresenceServiceTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("seedPresenceService");
        clickLink("toggleNavigation");
        clickLink("menu.locations");
    }

    public void testDisplay() {
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        clickLink("menu.locations");
        assertLinkPresent("editLocationLink");
        clickLink("editLocationLink");
        assertLinkPresent("link:listServices");
        clickLink("link:listServices");
        assertTablePresent("servicesTable");
        assertEquals(2, getTable("servicesTable").getRowCount());
        assertLinkPresent("editSipxService");
        clickLink("editSipxService");
        SiteTestHelper.assertNoException(tester);
        assertTextPresent("SIP_PRESENCE_SIGN_IN_CODE");
        assertTextNotPresent("SIP_PRESENCE_LOG_LEVEL");
    }
}

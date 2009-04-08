/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.service;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditCallResolverServiceTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditCallResolverServiceTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("seedCallResolverService");
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
        assertElementPresent("setting:SIP_CALLRESOLVER_PURGE_AGE_CDR");
    }
}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class RegistrationsTestUi extends WebTestCase {

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("Registrations");
    }

    public void testRefresh() {
        SiteTestHelper.assertNoException(tester);
        setWorkingForm("refreshForm");
        assertButtonPresent("refresh");
        assertTablePresent("registrations:list");
        clickButton("refresh");
        SiteTestHelper.assertNoException(tester);
        setWorkingForm("refreshForm");
        assertButtonPresent("refresh");
        assertTablePresent("registrations:list");
    }

}

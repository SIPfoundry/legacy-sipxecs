/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.user;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class UserRegistrationsTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(UserRegistrationsTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
    }

    public void testRefresh() {
        SiteTestHelper.home(tester, true);
        clickLink("UserRegistrations");
        SiteTestHelper.assertNoUserError(tester);
        setWorkingForm("refreshForm");
        assertButtonPresent("refresh");
        clickButton("refresh");
        SiteTestHelper.assertNoException(tester);
        setWorkingForm("refreshForm");
        assertButtonPresent("refresh");
    }
}

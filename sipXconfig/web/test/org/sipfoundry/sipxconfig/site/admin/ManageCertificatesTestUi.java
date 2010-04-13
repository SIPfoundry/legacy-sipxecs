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

import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ManageCertificatesTestUi extends WebTestCase {

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("toggleNavigation");
        clickLink("menu.certificates");
        SiteTestHelper.assertNoException(getTester());
    }

    public void testCsrTab() throws Exception {
        SiteTestHelper.assertNoException(getTester());
        clickLink("link:generate");
        setWorkingForm("generateForm");
        assertFormElementPresent("country");
        assertFormElementPresent("state");
        assertFormElementPresent("locality");
        assertFormElementPresent("organization");
        assertFormElementPresent("server:id");
        assertFormElementPresent("email");
        //TODO enhanced testing
        // e.g. leave out some form fields, press Generate button, and check that error message is displayed
        // e.g. fill in all form fields, press Generate button, and check that certificate text is displayed
    }

    public void testCrtTab() throws Exception {
        SiteTestHelper.assertNoException(getTester());
        clickLink("link:import");
        setWorkingForm("importForm");
        assertFormElementPresent("crtFile");
        assertFormElementPresent("keyFile");

        assertElementPresent("importMethod0");
        assertElementPresent("importMethod2");

        //TODO enhanced testing
    }

    public void testAuthoritiesTab() throws Exception {
        SiteTestHelper.assertNoException(getTester());
        clickLink("link:authorities");
        setWorkingForm("importForm");
        assertFormElementPresent("certificateFile");
        assertFormElementPresent("import");
        assertElementPresent("certificates:list");
        assertElementPresent("deleteCert");
        clickButton("import");
        SiteTestHelper.assertUserError(getTester());
    }

}

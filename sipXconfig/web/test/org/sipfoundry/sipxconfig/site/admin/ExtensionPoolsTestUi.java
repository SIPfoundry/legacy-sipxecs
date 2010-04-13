/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ExtensionPoolsTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ExtensionPoolsTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("ExtensionPools");
    }

    public void testDisplayExtensionPool() throws Exception {
        SiteTestHelper.assertNoUserError(tester);
        assertElementPresent("extPool:start");
        assertElementPresent("extPool:end");
        assertElementPresent("extPool:enable");
    }

    public void testChangeExtensionPool() throws Exception {
        final String start = "100";
        final String end = "199";
        checkCheckbox("extPool:enable");
        setTextField("extPool:start", start);
        setTextField("extPool:end", end);
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);

        // Go away and back and verify that our changes took effect
        SiteTestHelper.home(getTester());
        clickLink("ExtensionPools");
        assertCheckboxSelected("extPool:enable");
        assertTextFieldEquals("extPool:start", start);
        assertTextFieldEquals("extPool:end", end);
    }

    public void testValidation() {
        setTextField("extPool:start", "");
        setTextField("extPool:end", "444");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
    }
}

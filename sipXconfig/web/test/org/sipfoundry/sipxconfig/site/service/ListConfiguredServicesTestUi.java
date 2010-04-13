/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.service;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ListConfiguredServicesTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ListConfiguredServicesTestUi.class);
    }

    public void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, false);
        clickLink("resetConfiguredServices");
        clickLink("link:configuredServices");
    }

    public void testDisplay() {
        assertElementPresent("service:list");
        SiteTestHelper.assertNoException(getTester());
    }

    public void testNewService() {
        seedNtpService("new ntp service");
        assertElementPresent("service:list");
        String[][] expected = new String[][] {
                {"unchecked", "new ntp service", "Enabled", "1.1.1.1", "NTP" }
        };
        assertTableRowsEqual("service:list", 1, expected);
    }

    private void seedNtpService(String name) {
        SiteTestHelper.selectOption(tester, "newService", "NTP");
        assertElementPresent("server:form");
        setTextField("item:name", name);
        setTextField("service:address", "1.1.1.1");
        clickButton("form:ok");
    }

    public void testEditService() {
        seedNtpService("edit test");
        clickLinkWithText("edit test");
        assertElementPresent("server:form");
    }

    public void testDeleteService() {
        seedNtpService("delete test");
        assertTextInTable("service:list", "delete test");
        checkCheckbox("checkbox");
        clickButton("service:delete");
        assertTextNotInTable("service:list", "delete test");
    }
}

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

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class DnsTestPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(DnsTestPageTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickDnsTestPage();
    }

    private void clickDnsTestPage() {
        SiteTestHelper.home(tester);
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("toggleNavigation");
        clickLink("menu.dnsTest");
    }

    private void assertNeedToRunTest() {
        setWorkingForm("dnsTestForm");
        assertElementPresent("showPrompt");
        assertElementPresent("provideDns");
        assertElementPresent("runTest");
        // assert help
        setWorkingForm("detailedHelpForm");
        assertElementPresent("setting:toggle");
        assertElementNotPresent("detailedHelp");
        clickLink("setting:toggle");
        assertElementPresent("detailedHelp");
        setWorkingForm("detailedHelpForm");
        clickLink("setting:toggle");
        assertElementNotPresent("detailedHelp");
    }

    private void assertTestRan() {
        setWorkingForm("dnsTestForm");
        assertElementNotPresent("showPrompt");
        assertElementPresent("provideDns");
        assertElementPresent("runTest");
        // assert results / help
        assertElementNotPresent("dnsTestStatus");
        clickLink("setting:toggle");
        assertElementPresent("dnsTestStatus");
        setWorkingForm("dnsTestForm");
        submit();
        clickLink("setting:toggle");
        assertElementNotPresent("dnsTestStatus");
        setWorkingForm("detailedHelpForm");
        clickLinkWithExactText("Show Detailed Help");
        assertElementPresent("detailedHelp");
        setWorkingForm("detailedHelpForm");
        clickLinkWithExactText("Hide Detailed Help");
        assertElementNotPresent("detailedHelp");
    }

    public void testRunDns() throws Exception {
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);

        assertNeedToRunTest();
        setWorkingForm("dnsTestForm");
        submit();
        assertTestRan();

        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testChangeSystemConfiguration() throws Exception {
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        setWorkingForm("dnsTestForm");
        submit("runTest");
        assertTestRan();

        // add location
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("seedLocationsManager");
        clickLink("toggleNavigation");
        clickLink("menu.locations");
        clickLink("locations:add");
        setTextField("location:description", "newLocation");
        setTextField("location:address", "192.168.1.2");
        setTextField("location:fqdn", "another.example.org");
        setTextField("location:password", "123");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);

        clickDnsTestPage();
        assertNeedToRunTest();

        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
    }
}

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
        clickLink("toggleNavigation");
        clickLink("menu.DnsTest");
    }

    private void assertNeedToRunTest() {
        assertElementPresent("showPrompt");
        assertElementPresent("provideDns");
        assertElementPresent("runTest");
        assertElementNotPresent("testResults_0");
    }

    private void assertTestRan() {
        assertElementNotPresent("showPrompt");
        assertElementPresent("provideDns");
        assertElementPresent("runTest");
        assertElementPresent("testResults_0");
    }

    public void testRunDns() throws Exception {
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);

        assertNeedToRunTest();
        clickButton("runTest");
        assertTestRan();

        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testChangeSystemConfiguration() throws Exception {
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        clickButton("runTest");
        assertTestRan();

        //add location
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("seedLocationsManager");
        clickLink("toggleNavigation");
        clickLink("menu.locations");
        clickLink("locations:add");
        setTextField("location:description", "newLocation");
        setTextField("location:address", "192.168.1.2");
        setTextField("location:fqdn", "another.example.org");
        setTextField("location:password","123");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);

        clickDnsTestPage();
        assertNeedToRunTest();

        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
    }
}

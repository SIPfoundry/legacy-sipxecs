/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditLocationPageTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditLocationPageTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("seedLocationsManager");
        clickLink("toggleNavigation");
        clickLink("menu.locations");
        clickLink("locations:add");
    }

    public void testDisplay() {
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddWithValidInput() {
        SiteTestHelper.assertNoUserError(tester);
        setTextField("location:description", "newLocation");
        setTextField("location:address", "192.168.1.2");
        setTextField("location:fqdn", "another.example.org");
        setTextField("location:password","123");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddWithInvalidInput() {
        SiteTestHelper.assertNoUserError(tester);
        setTextField("location:address", "another.example.org");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);
    }

    public void testDisplayNatLocationPanel() {
        tester.setScriptingEnabled(true);
        SiteTestHelper.assertNoUserError(tester);
        clickLink("menu.locations");
        assertLinkPresent("editLocationLink");
        clickLink("editLocationLink");

        assertLinkPresent("link:natLocation");
        clickLink("link:natLocation");
        SiteTestHelper.assertNoUserError(tester);
        assertLinkPresent("setting:toggle");

        assertFormElementPresent("type");
        // 0 - public address, 1 - use stun
        assertSelectOptionValuePresent("type", "0");
        assertSelectOptionValuePresent("type", "1");

        selectOptionByValue("type", "0");
        assertElementNotPresent("stunAddress");
        assertElementNotPresent("stunInterval");
        assertElementPresent("publicAddress");

        selectOptionByValue("type", "1");
        SiteTestHelper.assertNoUserError(tester);
        assertElementPresent("stunAddress");
        assertElementPresent("stunInterval");
        assertTextFieldEquals("stunAddress", "stun01.sipphone.com");
        assertTextFieldEquals("stunInterval", "60");
        assertElementNotPresent("publicAddress");

        assertElementPresent("publicPort");
        assertTextFieldEquals("publicPort", "5060");
        assertElementPresent("publicTlsPort");
        assertTextFieldEquals("publicTlsPort", "5061");
        assertElementNotPresent("startRtpPort");
        assertElementNotPresent("stopRtpPort");
        clickLink("setting:toggle"); // advanced
        assertElementPresent("startRtpPort");
        assertElementPresent("stopRtpPort");
        assertTextFieldEquals("startRtpPort", "30000");
        assertTextFieldEquals("stopRtpPort", "31000");
    }
}

/*
 *
 *
 * Copyright (C) 2009 Nortel., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.openfire;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class InstantMessagingPanelTestUi extends WebTestCase {

    /*
     * **************************** TESTS DISABLED ************************************
     * ================================================================================
     * The openfire plugin is currently not loaded on UI unit tests. Therefore, we do
     * not have access to the sipxOpenfireService reference, which is required for
     * this test. Hence, the tests are disabled for now.
     * ================================================================================
     */

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(InstantMessagingPanelTestUi.class);
    }


    public void disabled_setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("seedOpenfireService");
        clickLink("link:instantMessaging");
        SiteTestHelper.assertNoException(tester);
    }

    public void disabled_testServerToServerApplySettings() {

        SiteTestHelper.assertNoException(tester);
        assertButtonPresent("serverToServerForm:apply");

        assertCheckboxPresent("setting:enabled");
        assertElementPresent("setting:port");
        assertCheckboxPresent("setting:disconnect-on-idle");
        assertElementPresent("setting:idle-timeout");
        assertCheckboxPresent("setting:any-can-connect");
        assertElementPresent("setting:allowed-servers");
        assertElementPresent("setting:disallowed-servers");

        checkCheckbox("setting:enabled");
        setTextField("setting:port", "1234");
        clickButton("serverToServerForm:apply");
        SiteTestHelper.assertNoUserError(tester);
        tester.assertCheckboxSelected("setting:enabled");
        tester.assertTextFieldEquals("setting:port", "1234");

        setTextField("setting:idle-timeout", "time");
        clickButton("serverToServerForm:apply");
        SiteTestHelper.assertUserError(tester);

        setTextField("setting:allowed-servers", "http://server1.org");
        clickButton("serverToServerForm:apply");
        SiteTestHelper.assertUserError(tester);

        setTextField("setting:allowed-servers", "http://server1.org:1234,server2address:12345");
        clickButton("serverToServerForm:apply");
        SiteTestHelper.assertNoUserError(tester);
        tester.assertTextFieldEquals("setting:allowed-servers", "http://server1.org:1234,server2address:12345");
    }

    public void disabled_testLoggingApplySettings() {
        clickLink("link:logging");
        SiteTestHelper.assertNoException(tester);
        assertButtonPresent("loggingForm:apply");
        assertCheckboxPresent("setting:enabled");
        checkCheckbox("setting:enabled");
        clickButton("loggingForm:apply");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testDisabled() {
    }
}

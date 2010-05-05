/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

/**
 * UserCallForwardingTestUi
 */
public class UserCallForwardingTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(UserCallForwardingTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
    }

    public void testAdminDisplay() throws Exception {
        SiteTestHelper.home(getTester(), true);
        clickLink("UserCallForwarding");
        assertTextNotPresent("An exception has occurred.");
        assertTextPresent("Call Forwarding");
        assertButtonPresent("form:ok");
        assertButtonPresent("form:cancel");
        assertButtonPresent("form:apply");
    }

    public void _testNonAdminDisplay() throws Exception {
        SiteTestHelper.home(getTester());
        tester.clickLink("loginFirstTestUser");
        clickLink("menu.callForwarding");
        clickLink("");
        SiteTestHelper.assertNoException(getTester());

        // at the time, no where to go to, e.g. callback null, so
        // apply is only option
        assertButtonNotPresent("form:ok");
        assertButtonNotPresent("form:cancel");
        assertButtonPresent("form:apply");
    }

    public void _testAddAddress() throws Exception {
        SiteTestHelper.home(getTester());
        tester.clickLink("resetCallForwarding");
        SiteTestHelper.setScriptingEnabled(tester, true);
        tester.clickLink("loginFirstTestUser");
        clickLink("UserCallForwarding");

        // Javascript submit link
        clickLink("addRingLink");
        setTextField("number", "123");
        clickButton("form:apply");
        assertElementPresent("user:success");

        setTextField("number", "@.%^&");
        clickButton("form:apply");
        assertElementPresent("user:error");

        setTextField("number", "john@example.com");
        clickButton("form:apply");
        assertElementPresent("user:success");

        setTextField("number", "john@example@bogus.com");
        clickButton("form:apply");
        assertElementPresent("user:error");

        clickLinkWithText("Delete");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void _testAddSchedule() throws Exception {
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("loginFirstTestUser");
        clickLink("toggleNavigation");
        clickLink("menu.callForwarding");
        clickLink("link:forwarding");
        clickLink("addRingLink");

        assertSelectedOptionEquals("schedule", "Always");
        setTextField("number", "222");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        clickLinkWithText("Delete");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);

        clickLink("menu.callForwarding");
        clickLink("link:schedules");
        clickLinkWithText("Add Schedule");
        setTextField("name", "schedule");
        clickLink("addPeriod");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);

        clickLink("menu.callForwarding");
        clickLink("link:forwarding");
        clickLink("addRingLink");
        selectOption("schedule", "schedule");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);

        clickLink("menu.callForwarding");
        clickLink("link:forwarding");
        clickLink("addRingLink");
        setTextField("number", "@#$%");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);

        setTextField("number", "222@q.c");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);

        setTextField("number", "222");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);

        clickLinkWithText("Delete");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);

        clickLink("menu.callForwarding");
        clickLink("link:schedules");
        checkCheckbox("checkbox");
        clickButton("schedule:delete");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testRingingTime() throws Exception {
        SiteTestHelper.home(getTester());
        clickLink("UserCallForwarding");

        setWorkingForm("callForwardingForm");
        setTextField("delay", "2");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);

        setWorkingForm("callForwardingForm");
        setTextField("delay", "2abc");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);

        setWorkingForm("callForwardingForm");
        setTextField("delay", "12345678910");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
    }
}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.acd;

import junit.framework.Test;

import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ListAcdServersTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ListAcdServersTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("seedAcdServer");
        clickLink("listAcdServers");
        SiteTestHelper.assertNoException(tester);
    }

    public void testDisplayEdit() throws Exception {
        SiteTestHelper.assertNoException(tester);
        assertLinkPresentWithText("server.example.com");
        clickLink("editRowLink");

        assertFormElementPresent("hostField");
        assertFormElementPresent("portField");
        assertFormElementPresent("form:ok");
        assertFormElementPresent("form:cancel");

        resetAcdContext();
    }

    public void testPresenceServerLink() throws Exception {
        clickLink("link:presence");
        assertElementPresent("setting:SIP_PRESENCE_SIGN_IN_CODE");
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
    }

    public void resetAcdContext() {
        SiteTestHelper.home(getTester());
        clickLink("resetAcdContext");
    }
}

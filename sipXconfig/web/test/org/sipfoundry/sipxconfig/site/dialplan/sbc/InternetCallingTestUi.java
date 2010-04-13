/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan.sbc;

import junit.framework.Test;
import net.sourceforge.jwebunit.html.Table;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;


public class InternetCallingTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(InternetCallingTestUi.class);
    }

    public InternetCallingTestUi() {
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());

        // assumption true for list bridges at least
        SiteTestHelper.setScriptingEnabled(tester, true);

        clickLink("resetInternetCalling");
        clickLink("InternetCalling");
        clickLink("link:internetCalling");
    }

    public void testDisplayEdit() throws Exception {
        SiteTestHelper.assertNoException(tester);
        assertCheckboxPresent("sbc:enabled");
        assertButtonPresent("internetCalling:apply");
        assertLinkNotPresent("sbc:add");
        assertElementNotPresent("sbc:list");

        // try toglling
        clickLink("setting:toggle");

        assertLinkPresent("sbc:add");
        assertElementPresent("sbc:list");
        clickButton("internetCalling:apply");
        SiteTestHelper.assertNoUserError(tester);

        checkCheckbox("sbc:enabled");
        clickButton("internetCalling:apply");

        // it's going to fail since "Enable Internet Calling" is checked and SBC address is needed
        SiteTestHelper.assertUserError(tester);

        Table auxSbcsTable = tester.getTable("sbc:list");
        assertEquals(1, auxSbcsTable.getRowCount());

    }
}

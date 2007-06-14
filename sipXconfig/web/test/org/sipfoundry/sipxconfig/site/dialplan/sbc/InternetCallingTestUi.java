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
import net.sourceforge.jwebunit.WebTestCase;

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
        SiteTestHelper.setScriptingEnabled(true);

        clickLink("resetInternetCalling");
        clickLink("InternetCalling");
    }

    public void testDisplayEdit() throws Exception {
        SiteTestHelper.assertNoException(tester);
        assertButtonPresent("form:apply");

        assertLinkNotPresent("add:sbc");

        // try toglling
        clickLink("setting:toggle");

        // it's going to fails since SBC address is needed
        SiteTestHelper.assertUserError(tester);

        setFormElement("sbcAddress", "sbc.example.org");

        // add aditional SBC
        clickLink("sbc:add");
        SiteTestHelper.assertNoUserError(tester);
        setFormElement("sbcAddress", "sbc.example.net");
        clickButton("form:ok");

        // check number of rows in the table after SBC added
        assertTextInTable("sbc:list", "sbc.example.net");

        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("sbc:delete");

        assertTextNotInTable("sbc:list", "sbc.example.net");
    }
}

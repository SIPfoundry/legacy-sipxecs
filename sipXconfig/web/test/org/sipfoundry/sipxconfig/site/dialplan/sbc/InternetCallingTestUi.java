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
        assertButtonPresent("internetCalling:apply");
        assertLinkNotPresent("sbc:add");
        assertElementNotPresent("sbc:list");

        // try toglling
        SiteTestHelper.clickSubmitLink(tester, "setting:toggle");

        assertLinkPresent("sbc:add");
        assertElementPresent("sbc:list");
        clickButton("internetCalling:apply");

        // it's going to fails since SBC address is needed
        SiteTestHelper.assertUserError(tester);

        SiteTestHelper.selectOption(tester, "sbcDeviceSelect", "Unmanaged SBC");
        SiteTestHelper.assertNoUserError(tester);
        setTextField("sbcDevice:name", "sbcDevice1");
        setTextField("sbcDevice:address", "sbc.example.org");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.selectOption(tester, "sbcDeviceSelect", "sbcDevice1");

        Table auxSbcsTable = tester.getTable("sbc:list");
        assertEquals(1, auxSbcsTable.getRowCount());

        // add aditional SBC
        clickLink("sbc:add");
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.selectOption(tester, "sbcDeviceSelect", "Unmanaged SBC");
        setTextField("sbcDevice:name", "sbcDevice2");
        setTextField("sbcDevice:address", "sbc.example.net");
        clickButton("form:ok");
        SiteTestHelper.selectOption(tester, "sbcDeviceSelect", "sbcDevice2");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);

        // check number of rows in the table after SBC added
        auxSbcsTable = tester.getTable("sbc:list");
        assertEquals(2, auxSbcsTable.getRowCount());
        assertTextInTable("sbc:list", "sbc.example.net");

        // check number of rows in the table after SBC is deleted
        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("sbc:delete");
        auxSbcsTable = tester.getTable("sbc:list");
        assertEquals(1, auxSbcsTable.getRowCount());
        assertTextNotInTable("sbc:list", "sbc.example.net");
    }
}

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

import com.meterware.httpunit.WebTable;

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
        assertLinkNotPresent("sbc:add");
        assertElementNotPresent("sbc:list");

        // try toglling
        clickLink("setting:toggle");

        assertLinkPresent("sbc:add");
        assertElementPresent("sbc:list");
        clickButton("form:apply");

        // it's going to fails since SBC address is needed
        SiteTestHelper.assertUserError(tester);

        selectOption("common_FlexiblePropertySelection", "Unmanaged SBC");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        setFormElement("sbcDeviceName", "sbcDevice1");
        setFormElement("sbcDeviceAddress", "sbc.example.org");
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        selectOption("common_FlexiblePropertySelection", "sbcDevice1");

        WebTable auxSbcsTable = getDialog().getWebTableBySummaryOrId("sbc:list");
        assertEquals(1, auxSbcsTable.getRowCount());

        // add aditional SBC
        clickLink("sbc:add");
        SiteTestHelper.assertNoUserError(tester);
        selectOption("common_FlexiblePropertySelection", "Unmanaged SBC");
        setFormElement("sbcDeviceName", "sbcDevice2");
        setFormElement("sbcDeviceAddress", "sbc.example.net");
        clickButton("form:ok");
        selectOption("common_FlexiblePropertySelection", "sbcDevice2");
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);

        // check number of rows in the table after SBC added
        auxSbcsTable = getDialog().getWebTableBySummaryOrId("sbc:list");
        assertEquals(2, auxSbcsTable.getRowCount());
        assertTextInTable("sbc:list", "sbc.example.net");

        // check number of rows in the table after SBC is deleted
        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("sbc:delete");
        auxSbcsTable = getDialog().getWebTableBySummaryOrId("sbc:list");
        assertEquals(1, auxSbcsTable.getRowCount());
        assertTextNotInTable("sbc:list", "sbc.example.net");
    }
}

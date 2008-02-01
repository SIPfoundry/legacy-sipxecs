/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.site.sbc;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import com.meterware.httpunit.WebTable;

import net.sourceforge.jwebunit.WebTestCase;

public class EditSbcDeviceTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditSbcDeviceTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("toggleNavigation");
        clickLink("menu.sbcs");
    }

    public void testAddDeleteSbc() throws Exception {
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        assertTablePresent("list:sbc");
        WebTable sbcsTable = getDialog().getWebTableBySummaryOrId("list:sbc");
        assertEquals(1, sbcsTable.getRowCount());

        selectOption("PropertySelection", "Unmanaged SBC");
        setFormElement("sbcDeviceName", "sbc1");
        setFormElement("sbcDeviceAddress", "10.1.1.1");
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        sbcsTable = getDialog().getWebTableBySummaryOrId("list:sbc");
        assertEquals(2, sbcsTable.getRowCount());

        checkCheckbox("checkbox");
        clickButton("list:sbc:delete");
        sbcsTable = getDialog().getWebTableBySummaryOrId("list:sbc");
        assertEquals(1, sbcsTable.getRowCount());
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddSbcWithSameName() throws Exception {
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);

        selectOption("PropertySelection", "Unmanaged SBC");
        setFormElement("sbcDeviceName", "uniqueSbcName");
        setFormElement("sbcDeviceAddress", "10.1.1.2");
        clickButton("form:ok");

        selectOption("PropertySelection", "Unmanaged SBC");
        setFormElement("sbcDeviceName", "uniqueSbcName");
        setFormElement("sbcDeviceAddress", "10.1.1.3");
        clickButton("form:ok");

        SiteTestHelper.assertUserError(tester);
    }
}

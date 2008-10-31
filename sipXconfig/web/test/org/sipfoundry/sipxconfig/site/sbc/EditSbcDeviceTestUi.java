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

import net.sourceforge.jwebunit.html.Table;
import net.sourceforge.jwebunit.junit.WebTestCase;

public class EditSbcDeviceTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditSbcDeviceTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("resetSbcDevices");
        clickLink("toggleNavigation");
        clickLink("menu.sbcs");
    }

    public void testAddDeleteSbc() throws Exception {
        SiteTestHelper.assertNoUserError(tester);
        assertTablePresent("list:sbc");
        Table sbcsTable = getTable("list:sbc");
        assertEquals(1, sbcsTable.getRowCount());

        SiteTestHelper.selectOption(tester, "PropertySelection", "Unmanaged SBC");
        setTextField("sbcDevice:name", "sbc1");
        setTextField("sbcDevice:address", "10.1.1.1");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        sbcsTable = getTable("list:sbc");
        assertEquals(2, sbcsTable.getRowCount());

        //local address automatically configured
        SiteTestHelper.selectOption(tester, "PropertySelection", "Unmanaged SBC");
        setTextField("sbcDevice:name", "sbc2");
        assertTextFieldEquals("sbcDevice:address", SiteTestHelper.getSbcDeviceLocalIp());
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        sbcsTable = getTable("list:sbc");
        assertEquals(3, sbcsTable.getRowCount());

        //add bridge
        SiteTestHelper.selectOption(tester, "PropertySelection", "Internal SBC");
        setTextField("sbcDevice:name", "bridge");
        assertTextFieldEquals("sbcDevice:address", SiteTestHelper.getSbcDeviceLocalIp());
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        sbcsTable = getTable("list:sbc");
        assertEquals(4, sbcsTable.getRowCount());

        //creation of maximum one bridge is allowed
        SiteTestHelper.selectOption(tester, "PropertySelection", "Internal SBC");
        SiteTestHelper.assertUserError(getTester());

        //delete one sbc
        setWorkingForm("Form");
        checkCheckbox("checkbox");
        clickButton("list:sbc:delete");
        SiteTestHelper.assertNoUserError(tester);
        sbcsTable = getTable("list:sbc");
        assertEquals(3, sbcsTable.getRowCount());

        //delete all sbcs
        deleteAllSbcs();

    }

    private void deleteAllSbcs() {
        setWorkingForm("Form");
        int rowCount = SiteTestHelper.getRowCount(getTester(), "list:sbc");
        if (rowCount <= 1) {
            return;
        }
        for (int i = 0; i < rowCount - 1; i++) {
            SiteTestHelper.selectRow(getTester(), i, true);
        }
        clickButton("list:sbc:delete");
        assertEquals(1, SiteTestHelper.getRowCount(getTester(), "list:sbc"));
    }

    public void testAddSbcWithSameName() throws Exception {
        SiteTestHelper.assertNoUserError(tester);

        SiteTestHelper.selectOption(tester, "PropertySelection", "Unmanaged SBC");
        setTextField("sbcDevice:name", "uniqueSbcName");
        setTextField("sbcDevice:address", "10.1.1.2");
        clickButton("form:ok");

        SiteTestHelper.selectOption(tester, "PropertySelection", "Unmanaged SBC");
        setTextField("sbcDevice:name", "uniqueSbcName");
        setTextField("sbcDevice:address", "10.1.1.3");
        clickButton("form:ok");

        SiteTestHelper.assertUserError(tester);
    }

    public void testNoProfilePreviewOnNewSbc() throws Exception {
        SiteTestHelper.assertNoUserError(tester);

        SiteTestHelper.selectOption(tester, "PropertySelection", "Internal SBC");

        SiteTestHelper.assertNoUserError(tester);
        assertTextNotPresent("sipxbridge.xml");
    }
}

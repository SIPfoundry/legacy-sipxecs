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

        selectOption("PropertySelection", "Unmanaged SBC");
        setTextField("sbcDevice:name", "sbc1");
        setTextField("sbcDevice:address", "22");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);
        setTextField("sbcDevice:address", "10.1.1.1");
        setTextField("sbcDevice:port", "20ab");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);
        setTextField("sbcDevice:port", "-1");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);
        setTextField("sbcDevice:port", "100");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        sbcsTable = getTable("list:sbc");
        assertEquals(2, sbcsTable.getRowCount());

        // local address automatically configured
        selectOption("PropertySelection", "Unmanaged SBC");
        setTextField("sbcDevice:name", "sbc2");
        assertTextFieldEquals("sbcDevice:address", "");
        setTextField("sbcDevice:address", "10.0.0.1");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        sbcsTable = getTable("list:sbc");
        assertEquals(3, sbcsTable.getRowCount());

        // delete one sbc
        setWorkingForm("Form");
        checkCheckbox("checkbox");
        setExpectedJavaScriptConfirm("Are you sure you want to remove selected SBC Routes?", true);
        clickButton("list:sbc:delete");
        SiteTestHelper.assertNoUserError(tester);
        sbcsTable = getTable("list:sbc");
        assertEquals(2, sbcsTable.getRowCount());

        // delete all sbcs
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
        setExpectedJavaScriptConfirm("Are you sure you want to remove selected SBC Routes?", true);
        clickButton("list:sbc:delete");
        assertEquals(1, SiteTestHelper.getRowCount(getTester(), "list:sbc"));
    }

    public void testAddSbcWithSameName() throws Exception {
        SiteTestHelper.assertNoUserError(tester);

        selectOption("PropertySelection", "Unmanaged SBC");
        setTextField("sbcDevice:name", "uniqueSbcName");
        setTextField("sbcDevice:address", "10.1.1.2");
        clickButton("form:ok");

        selectOption("PropertySelection", "Unmanaged SBC");
        setTextField("sbcDevice:name", "uniqueSbcName");
        setTextField("sbcDevice:address", "10.1.1.3");
        clickButton("form:ok");

        SiteTestHelper.assertUserError(tester);
    }

    public void testAddSbcWithoutAddress() {
        SiteTestHelper.assertNoUserError(tester);
        int rowCount = SiteTestHelper.getRowCount(getTester(), "list:sbc");

        selectOption("PropertySelection", "Unmanaged SBC");
        setTextField("sbcDevice:name", "sbcWithoutIp");
        clickButton("form:ok");

        SiteTestHelper.assertUserError(tester);
        clickButton("form:cancel");
        assertTableRowCountEquals("list:sbc", rowCount);

    }

    public void testSbcPort() {
        selectOption("PropertySelection", "Unmanaged SBC");

        setWorkingForm("Form");
        setTextField("sbcDevice:name", "uniqueSbcName");
        setTextField("sbcDevice:address", "10.1.1.2");
        setTextField("sbcDevice:port", "2a");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
    }
}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.gateway;

import java.util.Arrays;

import junit.framework.Test;
import net.sourceforge.jwebunit.html.Table;
import net.sourceforge.jwebunit.junit.WebTestCase;
import net.sourceforge.jwebunit.junit.WebTester;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

/**
 * GatewaysTestUi
 */
public class GatewaysTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(GatewaysTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("resetDialPlans");
    }

    public void testAddGateways() throws Exception {
        clickLink("ListGateways");

        assertTablePresent("list:gateway");
        Table gatewaysTable = getTable("list:gateway");
        // make sure it's sorted by name
        clickLinkWithText("Name");
        int lastColumn = SiteTestHelper.getColumnCount(gatewaysTable) - 1;
        assertEquals(4, lastColumn);

        SiteTestHelper.selectOption(tester, "selectGatewayModel", "Unmanaged gateway");

        addGateway(null);
        SiteTestHelper.assertUserError(tester);

        addGateway("bongo");
        SiteTestHelper.assertNoUserError(tester);
        assertTablePresent("list:gateway");
        gatewaysTable = getTable("list:gateway");
        // we should have 2 gateway now
        assertEquals(2, gatewaysTable.getRowCount());
        assertEquals("bongoDescription", SiteTestHelper.getCellAsText(gatewaysTable, 1,
                lastColumn));

        SiteTestHelper.selectOption(tester, "selectGatewayModel", "Unmanaged gateway");

        addGateway("kuku");

        gatewaysTable = getTable("list:gateway");
        // we should have 2 gateway now
        assertEquals(3, gatewaysTable.getRowCount());
        assertEquals("kukuDescription", SiteTestHelper
                .getCellAsText(gatewaysTable, 2, lastColumn));
    }

    /**
     * Tests that only a SIP Trunk gateway has the additional "Route" field on its Configuration tab,
     * and no other gateway type has it.
     */
    public void testSipTrunkRouteField() throws Exception {
        clickLink("ListGateways");

        String[] nonRouteGateways = {"Acme 1000", "AudioCodes MP114 FXO",
                "AudioCodes MP118 FXO", "AudioCodes Mediant",
                "AudioCodes TP260", "Unmanaged gateway"};

        for (String gatewayType : nonRouteGateways) {
            SiteTestHelper.selectOption(tester, "selectGatewayModel", gatewayType);
            assertElementNotPresent("common_FlexiblePropertySelection");
            clickButton("form:cancel");
        }

        SiteTestHelper.selectOption(tester, "selectGatewayModel", "SIP trunk");        
        assertElementPresent("common_FlexiblePropertySelection");
        tester.setTextField("gateway:name", "SipTrunkRouteTest");
        tester.setTextField("gateway:address", "1.2.3.4");
        SiteTestHelper.selectOption(tester, "common_FlexiblePropertySelection", "Unmanaged SBC");
        tester.setTextField("sbcDevice:name", "sbcDeviceForSipTrunk");
        tester.setTextField("sbcDevice:address", "sbc.example.org");
        tester.clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.selectOption(tester, "common_FlexiblePropertySelection", "sbcDeviceForSipTrunk");
        tester.clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddGatewaysDuplicateName() throws Exception {
        clickLink("ListGateways");
        SiteTestHelper.selectOption(tester, "selectGatewayModel", "Unmanaged gateway");
        addGateway(getTester(), "dupname");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.selectOption(tester, "selectGatewayModel", "Unmanaged gateway");
        addGateway(getTester(), "dupname");
        SiteTestHelper.assertUserError(tester);
    }

    public void testDeleteGateways() throws Exception {
        addTestGateways(getTester(), 10);

        assertTablePresent("list:gateway");
        Table gatewaysTable = getTable("list:gateway");
        assertEquals(11, gatewaysTable.getRowCount());

        SiteTestHelper.selectRow(tester, 0, true);
        SiteTestHelper.selectRow(tester, 1, true);
        clickButton("list:gateway:delete");

        SiteTestHelper.assertNoUserError(tester);
        assertTablePresent("list:gateway");
        gatewaysTable = getTable("list:gateway");
        assertEquals(9, gatewaysTable.getRowCount());

        for (int i = 0; i < 8; i++) {
            SiteTestHelper.selectRow(tester, i, true);
        }
        clickButton("list:gateway:delete");

        assertTablePresent("list:gateway");
        gatewaysTable = getTable("list:gateway");
        assertEquals(1, gatewaysTable.getRowCount());
    }

    public void testValidateDescription() {
        clickLink("ListGateways");
        SiteTestHelper.selectOption(tester, "selectGatewayModel", "Unmanaged gateway");
        
        addGateway("bongo");
        clickLinkWithText("bongo");
        int limit = 255; // postgres database field size
        char[] descriptionToLong = new char[limit + 1];
        Arrays.fill(descriptionToLong, 'x');
        char[] descriptionOk = new char[limit];
        Arrays.fill(descriptionOk, 'x');
        setTextField("gateway:description", new String(descriptionToLong));
        tester.clickButton("form:ok");
        // there should be an error now
        assertTextPresent("Enter at most");
        setTextField("gateway:description", new String(descriptionOk));
        tester.clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
        // we should not get error this time
        assertTablePresent("list:gateway");
    }

    /**
     * Fills and submits edit gateway form
     *
     * @param name response after clicking submit button
     */
    private void addGateway(String name) {
        addGateway(getTester(), name);
    }

    /**
     * Static version to be called from other tests
     *
     * @param name response after clicking submit button
     */
    public static String[] addGateway(WebTester tester, String name) {
        String[] row = new String[] {
            "unchecked", name + "Name", name + "Address", "Unmanaged gateway", name + "Description"
        };

        if (null != name) {
            tester.setTextField("gateway:name", row[1]);
            tester.setTextField("gateway:address", row[2]);
            tester.setTextField("gateway:description", row[4]);
        }
        tester.submit("form:ok");
        return row;
    }

    /**
     * Adds number of test gateways to test
     *
     * @param counter number of gateways to add - names gateway0..gateway'count-1'
     */
    public static String[] addTestGateways(WebTester tester, int counter) {
        boolean scripting = SiteTestHelper.setScriptingEnabled(tester, true);
        String[] names = new String[counter];

        tester.clickLink("ListGateways");

        for (int i = 0; i < counter; i++) {
            SiteTestHelper.selectOption(tester, "selectGatewayModel", "Unmanaged gateway");

            // Give the new gateway a name that is extremely unlikely to collide
            // with any existing gateway names
            String name = "gateway" + i + Long.toString(System.currentTimeMillis());

            names[i] = addGateway(tester, name)[1];
        }
        SiteTestHelper.setScriptingEnabled(tester, scripting);
        return names;
    }
}

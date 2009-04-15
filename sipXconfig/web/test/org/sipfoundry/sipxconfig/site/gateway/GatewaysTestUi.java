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

import static org.sipfoundry.sipxconfig.site.SiteTestHelper.assertNoException;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.assertNoUserError;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.assertUserError;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.getBaseUrl;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.getCellAsText;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.getColumnCount;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.home;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.selectRow;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.webTestSuite;

/**
 * GatewaysTestUi
 */
public class GatewaysTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return webTestSuite(GatewaysTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(getBaseUrl());
        home(tester);
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("resetDialPlans");
    }

    public void testLinks() {
        clickLink("ListGateways");
        clickLink("link:dialPlans");
        assertNoException(tester);
        assertTablePresent("dialplan:list");
        clickLink("link:gateways");
        assertTablePresent("list:gateway");
    }

    public void testAddGateways() throws Exception {
        clickLink("ListGateways");

        assertTablePresent("list:gateway");
        Table gatewaysTable = getTable("list:gateway");

        int lastColumn = getColumnCount(gatewaysTable) - 1;
        assertEquals(4, lastColumn);

        SiteTestHelper.selectOption(tester, "selectGatewayModel", "Unmanaged gateway");

        addGateway(null);
        assertUserError(tester);

        addGateway("bongo");
        assertNoUserError(tester);
        assertTablePresent("list:gateway");
        gatewaysTable = getTable("list:gateway");
        // we should have 2 gateway now
        assertEquals(2, gatewaysTable.getRowCount());
        assertEquals("bongoDescription", getCellAsText(gatewaysTable, 1, lastColumn));

        SiteTestHelper.selectOption(tester, "selectGatewayModel", "Unmanaged gateway");

        addGateway("kuku");

        SiteTestHelper.selectOption(tester, "selectGatewayModel", "Unmanaged gateway");

        addGateway("sharedG", true);

        gatewaysTable = getTable("list:gateway");
        // we should have 3 gateway now
        assertEquals(4, gatewaysTable.getRowCount());
        assertTextInTable("list:gateway", "kukuDescription");
        assertTextInTable("list:gateway", "bongoDescription");
        assertTextInTable("list:gateway", "sharedGDescription");
    }

    public void testEditGatewaySettings() throws Exception {
        clickLink("ListGateways");
        SiteTestHelper.selectOption(tester, "selectGatewayModel", "AudioCodes MP114 FXO");
        setTextField("gateway:name", "EditGatewaySettingsTest");
        setTextField("gateway:address", "1.2.3.4");
        setTextField("gateway:serial", "123456654321");
        clickButton("form:apply");
        assertNoUserError(tester);

        clickLink("link:Network.label");
        assertNoUserError(tester);
        setTextField("setting:DNSPriServerIP", "4.3.2.1");
        clickButton("form:apply");
        assertNoUserError(tester);
        assertTextFieldEquals("setting:DNSPriServerIP", "4.3.2.1");
    }

    /**
     * Tests that only a SIP Trunk gateway has the additional "Route" field on its Configuration
     * tab, and no other gateway type has it.
     */
    public void testSipTrunkRouteField() throws Exception {
        clickLink("ListGateways");

        String[] nonRouteGateways = {
            "Acme 1000", "AudioCodes MP114 FXO", "AudioCodes MP118 FXO", "AudioCodes Mediant 1000",
            "AudioCodes Mediant 2000", "AudioCodes Mediant 3000", "AudioCodes TP260", "Unmanaged gateway"
        };

        for (String gatewayType : nonRouteGateways) {
            SiteTestHelper.selectOption(tester, "selectGatewayModel", gatewayType);
            assertElementNotPresent("sbcDeviceSelect");
            clickButton("form:cancel");
        }

        SiteTestHelper.selectOption(tester, "selectGatewayModel", "SIP trunk");
        setTextField("gateway:name", "SipTrunkRouteTest");
        setTextField("gateway:address", "1.2.3.4");
//        // FIXME: apply should not be necessary see: XCF-2444
        clickButton("form:apply");
        SiteTestHelper.selectOption(tester, "sbcDeviceSelect", "Unmanaged SBC");
        setTextField("sbcDevice:name", "sbcDeviceForSipTrunk");
        setTextField("sbcDevice:address", "sbc.example.org");
        clickButton("form:ok");
        assertNoUserError(tester);
        assertSelectedOptionEquals("sbcDeviceSelect", "sbcDeviceForSipTrunk");
        clickButton("form:ok");        
        assertNoUserError(tester);
    }

    public void testAddGatewaysDuplicateName() throws Exception {
        clickLink("ListGateways");
        SiteTestHelper.selectOption(tester, "selectGatewayModel", "Unmanaged gateway");
        addGateway(tester, "dupname");
        assertNoException(tester);
        SiteTestHelper.selectOption(tester, "selectGatewayModel", "Unmanaged gateway");
        addGateway(tester, "dupname");
        assertUserError(tester);
    }

    public void testDeleteGateways() throws Exception {
        addTestGateways(tester, 10);

        assertTablePresent("list:gateway");
        Table gatewaysTable = getTable("list:gateway");
        assertEquals(11, gatewaysTable.getRowCount());

        selectRow(tester, 0, true);
        selectRow(tester, 1, true);
        clickButton("list:gateway:delete");

        assertNoUserError(tester);
        assertTablePresent("list:gateway");
        gatewaysTable = getTable("list:gateway");
        assertEquals(9, gatewaysTable.getRowCount());

        for (int i = 0; i < 8; i++) {
            selectRow(tester, i, true);
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
        assertNoException(tester);
        // we should not get error this time
        assertTablePresent("list:gateway");
    }

    public void testNoPreviewProfileOnNewGateway() {
        clickLink("ListGateways");
        SiteTestHelper.selectOption(tester, "selectGatewayModel", "Acme 1000");
        assertTextNotPresent("null.ini");
    }

    /**
     * Fills and submits edit gateway form
     *
     * @param name response after clicking submit button
     */
    private void addGateway(String name) {
        addGateway(tester, name);
    }

    private void addGateway(String name, boolean shared) {
        addGateway(tester, name, shared);
    }

    private static String[] addGateway(WebTester tester, String name, boolean shared) {
        String[] row = new String[] {
                "unchecked", name + "Name", name + "Address" + ".localdomain",
            "Unmanaged gateway", name + "Description"
            };

            if (null != name) {
                tester.setTextField("gateway:name", row[1]);
                tester.setTextField("gateway:address", row[2]);
                tester.setTextField("gateway:description", row[4]);
                if (shared) {
                    tester.checkCheckbox("gateway:shared");
                }
            }
            tester.submit("form:ok");
            return row;
    }

    /**
     * Static version to be called from other tests
     *
     * @param name response after clicking submit button
     */
    public static String[] addGateway(WebTester tester, String name) {
        return addGateway(tester, name, false);
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

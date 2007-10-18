/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.gateway.GatewaysTestUi;

import com.meterware.httpunit.WebTable;

/**
 * DialPlanEditTestUi
 */
public class DialPlanEditTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(DialPlanEditTestUi.class);
    }

    private static final String[][] NAMES = {
        {
            "kukuName", "false", "kuku description", "Always"
        }, {
            "bongoName", "false", "bongoDescription", "Always"
        },
    };

    private static final String[][] DEFAULTS = {
        {
            "Emergency", "Disabled", "Emergency", "Emergency dialing plan", "Always"
        },
        {
            "International", "Disabled", "Long Distance", "International dialing", "Always"
        },
        {
            "Local", "Disabled", "Long Distance", "Local dialing", "Always"
        },
        {
            "Long Distance", "Disabled", "Long Distance", "Long distance dialing plan", "Always"
        },
        {
            "Restricted", "Disabled", "Long Distance", "Restricted dialing", "Always"
        },
        {
            "Toll free", "Disabled", "Long Distance", "Toll free dialing", "Always"
        },
        {
            "AutoAttendant", "Enabled", "Attendant", "Default autoattendant dialing plan",
            "Always"
        }, {
            "Internal", "Enabled", "Internal", "Default internal dialing plan", "Always"
        },
    };

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("resetDialPlans");
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("FlexibleDialPlan");
    }

    public void testDisplayAndClean() {
        assertTableRowsEqual("dialplan:list", 1, DEFAULTS);
        // remove all
        for (int i = 0; i < DEFAULTS.length; i++) {
            SiteTestHelper.selectRow(tester, i, true);
        }
        clickButton("dialplan:delete");
        // should be empty now
        WebTable rulesTable = getDialog().getWebTableBySummaryOrId("dialplan:list");
        assertEquals(1, rulesTable.getRowCount());
        // test revert
        clickLink("dialplan:revert");
        SiteTestHelper.assertNoException(tester);
        clickButton("form:ok");
        assertTableRowsEqual("dialplan:list", 1, DEFAULTS);
    }

    public void testViewRules() throws Exception {
        for (int i = 0; i < DEFAULTS.length; i++) {
            String name = DEFAULTS[i][0];
            clickLinkWithText(name);
            assertFormElementEquals("name", name);
            assertElementPresent("item:name");
            assertElementPresent("item:enabled");
            assertElementPresent("item:description");
            // all rules except "Attendant" have schedule
            if (!name.startsWith("AutoAttendant")) {
                assertElementPresent("schedule");
            }
            // all rules except "internal" have gateways panel
            if (!name.startsWith("Internal") && !name.startsWith("AutoAttendant")) {
                checkGateways();
            }
            setFormElement("name", "");
            clickButton("form:ok");
            // if validation kicks in we are on the same page
            SiteTestHelper.assertNoException(tester);
            SiteTestHelper.assertUserError(tester);
            setFormElement("name", name + "changed");
            clickButton("form:ok");
            // a link corresponding to new name should be in now
            SiteTestHelper.assertNoException(tester);
            SiteTestHelper.assertNoUserError(tester);
            assertLinkPresentWithText(name + "changed");
        }
    }

    public void testCustomRuleAdd() throws Exception {
        for (int i = 0; i < NAMES.length; i++) {
            String[] row = NAMES[i];
            selectOption("ruleTypeSelection", "Custom");

            SiteTestHelper.assertNoException(tester);

            assertLinkNotPresent("pattern:delete");
            assertLinkPresent("pattern:add");
            assertElementPresent("schedule");

            setFormElement("name", row[0]);
            setFormElement("description", row[2]);
            // dial pattern prefix
            setFormElement(SiteTestHelper.getIndexedId("prefix", 0), "333");
            // call pattern prefix
            setFormElement(SiteTestHelper.getIndexedId("prefix", 1), "444");

            checkAddDeletePattern();

            checkGateways();

            clickButton("form:ok");
            assertTextInTable("dialplan:list", row[2]);
            assertTextInTable("dialplan:list", row[3]);
            assertLinkPresentWithText(row[0]);
        }
    }

    public void testInternationalRuleAdd() {
        for (int i = 0; i < 4; i++) {
            String name = "international" + i;
            String description = "international description" + i;
            selectOption("ruleTypeSelection", "International");

            setFormElement("name", name);
            setFormElement("description", description);
            // dial pattern prefix
            setFormElement("longDistancePrefix", "100" + i);

            clickButton("form:ok");

            assertTextInTable("dialplan:list", description);
            assertLinkPresentWithText(name);
        }
    }

    public void testAttendantRuleAdd() throws Exception {
        for (int i = 0; i < NAMES.length; i++) {
            String[] row = NAMES[i];
            selectOption("ruleTypeSelection", "Attendant");

            setFormElement("name", row[0]);
            setFormElement("description", row[2]);
            // dial pattern prefix
            setFormElement("extension", "33344" + i);
            setFormElement("autoAttendantAliases", "");

            clickButton("form:ok");
            SiteTestHelper.assertNoException(tester);
            SiteTestHelper.assertNoUserError(tester);

            assertTextInTable("dialplan:list", row[2]);
            assertLinkPresentWithText(row[0]);
        }
    }

    public void testMove() {
        clickLink("dialplan:revert");
        SiteTestHelper.assertNoException(tester);
        clickButton("form:ok");
        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("dialplan:move:up");
        // no changes
        SiteTestHelper.assertNoException(getTester());
        assertTableRowsEqual("dialplan:list", 1, DEFAULTS);

        // move first row down
        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("dialplan:move:down");
        SiteTestHelper.assertNoException(getTester());
        WebTable rulesTable = getTester().getDialog().getWebTableBySummaryOrId("dialplan:list");
        assertEquals(DEFAULTS[0][0], rulesTable.getCellAsText(2, 1));
        assertEquals(DEFAULTS[1][0], rulesTable.getCellAsText(1, 1));
        assertEquals(DEFAULTS[2][0], rulesTable.getCellAsText(3, 1));
    }

    private void checkAddDeletePattern() throws Exception {
        // no delete link
        assertLinkNotPresent("pattern:delete");
        // add 2 more
        clickLink("pattern:add");
        SiteTestHelper.assertNoException(tester);
        clickLink("pattern:add");

        // delete 2

        clickLink("pattern:delete");
        clickLink("pattern:delete");
        // no delete link again
        assertLinkNotPresent("pattern:delete");
    }

    private void checkGateways() throws Exception {
        assertFormElementPresent("actionSelection");

        assertButtonPresent("gateway:remove");
        assertButtonPresent("gateway:moveUp");
        assertButtonPresent("gateway:moveDown");

        assertTablePresent("list:gateway");

        // add gateways

        final int gatewayCount = 4;
        String[][] gateways = new String[gatewayCount][];

        for (int i = 0; i < gatewayCount; i++) {
            selectOption("actionSelection", "Unmanaged gateway");
            SiteTestHelper.assertNoException(tester);
            SiteTestHelper.assertNoUserError(tester);

            // Give the new gateway a name that is extremely unlikely to collide
            // with any existing gateway names
            String gatewayName = "gateway" + i + Long.toString(System.currentTimeMillis());

            gateways[i] = GatewaysTestUi.addGateway(tester, gatewayName);
            SiteTestHelper.assertNoException(tester);
        }

        assertEquals(gatewayCount + 1, SiteTestHelper.getRowCount(tester, "list:gateway"));
        assertTableRowsEqual("list:gateway", 1, gateways);

        // test moving up/down
        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("gateway:moveUp");
        // no changes expected - cannot move up
        SiteTestHelper.assertNoException(getTester());
        assertTableRowsEqual("list:gateway", 1, gateways);

        // move down one row - no other changes expected
        clickButton("gateway:moveDown");
        WebTable gatewayTable = getTester().getDialog().getWebTableBySummaryOrId("list:gateway");
        assertEquals(gateways[0][0], gatewayTable.getCellAsText(2, 1));
        assertEquals(gateways[1][0], gatewayTable.getCellAsText(1, 1));
        assertEquals(gateways[2][0], gatewayTable.getCellAsText(3, 1));

        // click the gateway link - and then click cancel
        clickLinkWithText(gateways[0][0]);
        SiteTestHelper.assertNoException(tester);
        clickButton("form:cancel");
        SiteTestHelper.assertNoException(tester);

        // test removal
        for (int i = 0; i < gatewayCount; i++) {
            SiteTestHelper.selectRow(tester, i, true);
        }
        clickButton("gateway:remove");
        // only header in the table
        assertEquals(1, SiteTestHelper.getRowCount(tester, "list:gateway"));

        // test adding existing gateways
        selectOption("actionSelection", gateways[0][0]);
        SiteTestHelper.assertNoException(tester);
        assertEquals(2, SiteTestHelper.getRowCount(tester, "list:gateway"));
        gatewayTable = getTester().getDialog().getWebTableBySummaryOrId("list:gateway");
        assertEquals(gateways[0][0], gatewayTable.getCellAsText(1, 1));
    }

    public void testAddDeleteCustomSchedule() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLinkWithText("Schedules");
        clickLinkWithText("Add Schedule");
        setFormElement("name", "customRuleSchedule");
        setFormElement("description", "Schedule for a custom rule");
        clickLink("addPeriod");
        clickButton("form:ok");
        checkCheckbox("checkbox");
        clickButton("schedule:delete");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddGeneralScheduleWithSameName() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLinkWithText("Schedules");
        clickLinkWithText("Add Schedule");
        setFormElement("name", "generalSchedule");
        setFormElement("description", "Schedule for a rule");
        clickLink("addPeriod");
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        clickLinkWithText("Add Schedule");
        setFormElement("name", "generalSchedule");
        clickLink("addPeriod");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);
    }
}

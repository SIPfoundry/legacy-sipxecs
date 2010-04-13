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
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.gateway.GatewaysTestUi;

/**
 * DialingRuleEditTestUi
 */
public class DialingRuleTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(DialingRuleTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("resetDialPlans");
    }

    // FIXME: check if setting requests headers does not work or if we head problem
    public void _testScheduleTimeFormat() {
        clickLink("FlexibleDialPlan");
        SiteTestHelper.assertNoException(getTester());
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("link:schedules");
        clickLink("addSchedule");
        setTextField("item:name", "TestSchedule1");
        SiteTestHelper.clickSubmitLink(tester, "addPeriod");
        setTextField("from", "9:15 AM");
        setTextField("to", "6:15 PM");
        clickButton("form:ok");

        getTestContext().addRequestHeader("Accept-Language", "de");
        clickLinkWithText("TestSchedule1");
        assertTextFieldEquals("from", "09:15");
        assertTextFieldEquals("to", "18:15");
        clickButton("form:ok");

        getTestContext().addRequestHeader("Accept-Language", "en");
        clickLinkWithText("TestSchedule1");
        assertTextFieldEquals("from", "9:15 AM");
        assertTextFieldEquals("to", "6:15 PM");
    }

    public void testAddExistingGatewayNewRule() {
        String[] gatewayNames = GatewaysTestUi.addTestGateways(getTester(), 1);
        SiteTestHelper.home(getTester());
        boolean scripting = SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("FlexibleDialPlan");
        SiteTestHelper.assertNoException(getTester());
        SiteTestHelper.selectOption(tester, "rule:type", "Custom");
        SiteTestHelper.assertNoException(getTester());
        assertEquals(1, SiteTestHelper.getRowCount(tester, "list:gateway"));
        setTextField("item:name", "DialingRulesTestUi_custom");
        // try adding existing gateways
        SiteTestHelper.selectOption(tester, "actionSelection", gatewayNames[0]);
        SiteTestHelper.assertNoException(tester);
        assertEquals(2, SiteTestHelper.getRowCount(tester, "list:gateway"));
        SiteTestHelper.setScriptingEnabled(tester, scripting);
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
    }

    public void testAddExistingGatewayExistingRule() {
        String[] gatewayNames = GatewaysTestUi.addTestGateways(getTester(), 1);
        SiteTestHelper.home(getTester());
        boolean scripting = SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("FlexibleDialPlan");
        SiteTestHelper.assertNoException(getTester());
        clickLinkWithText("Emergency");
        SiteTestHelper.assertNoException(tester);
        assertEquals(1, SiteTestHelper.getRowCount(tester, "list:gateway"));
        // try adding existing gateways
        SiteTestHelper.selectOption(tester, "actionSelection", gatewayNames[0]);
        SiteTestHelper.assertNoException(tester);
        assertEquals(2, SiteTestHelper.getRowCount(tester, "list:gateway"));
        SiteTestHelper.setScriptingEnabled(tester, scripting);
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
    }

    public void testInvalidName() {
        GatewaysTestUi.addTestGateways(getTester(), 3);
        SiteTestHelper.home(getTester());
        clickLink("FlexibleDialPlan");
        SiteTestHelper.assertNoException(getTester());
        clickLinkWithText("Voicemail");
        SiteTestHelper.assertNoException(tester);
        // it's a submit link: uses java script, does not have id
        setTextField("item:name", "");
        clickButton("form:ok");
        // should fail with the error message
        SiteTestHelper.assertUserError(tester);
    }
}

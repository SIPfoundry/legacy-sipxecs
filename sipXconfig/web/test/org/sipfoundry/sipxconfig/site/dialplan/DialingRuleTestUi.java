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

    public void testAddExistingGatewayNewRule() {
        String[] gatewayNames = GatewaysTestUi.addTestGateways(getTester(), 1);
        SiteTestHelper.home(getTester());
        boolean scripting = SiteTestHelper.setScriptingEnabled(true);
        clickLink("FlexibleDialPlan");
        SiteTestHelper.assertNoException(getTester());
        selectOption("ruleTypeSelection", "Custom");
        assertEquals(1, SiteTestHelper.getRowCount(tester, "list:gateway"));

        SiteTestHelper.assertNoException(tester);
        setFormElement("name", "DialingRulesTestUi_custom");
        // try adding existing gateways
        selectOption("actionSelection", gatewayNames[0]);
        SiteTestHelper.assertNoException(tester);
        assertEquals(2, SiteTestHelper.getRowCount(tester, "list:gateway"));
        SiteTestHelper.setScriptingEnabled(scripting);
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
    }

    public void testAddExistingGatewayExistingRule() {
        String[] gatewayNames = GatewaysTestUi.addTestGateways(getTester(), 1);
        SiteTestHelper.home(getTester());
        boolean scripting = SiteTestHelper.setScriptingEnabled(true);
        clickLink("FlexibleDialPlan");
        SiteTestHelper.assertNoException(getTester());
        clickLinkWithText("Emergency");
        SiteTestHelper.assertNoException(tester);
        assertEquals(1, SiteTestHelper.getRowCount(tester, "list:gateway"));
        // try adding existing gateways
        selectOption("actionSelection", gatewayNames[0]);
        SiteTestHelper.assertNoException(tester);
        assertEquals(2, SiteTestHelper.getRowCount(tester, "list:gateway"));
        SiteTestHelper.setScriptingEnabled(scripting);
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
    }
    
    public void testInvalidName() {
        GatewaysTestUi.addTestGateways(getTester(), 3);
        SiteTestHelper.home(getTester());
        clickLink("FlexibleDialPlan");
        SiteTestHelper.assertNoException(getTester());
        clickLinkWithText("Internal");
        SiteTestHelper.assertNoException(tester);
        // it's a submit link: uses java script, does not have id
        setFormElement("name", "");
        clickButton("form:ok");
        // should fail with the error message
        SiteTestHelper.assertUserError(tester);
    }
}

package org.sipfoundry.sipxconfig.site.dialplan;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.WebTestCase;

public class EditInternalDialRuleTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditInternalDialRuleTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("resetDialPlans");
    }
    
    public void testHostnameRequiredForExternalVoicemailServer() {
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("FlexibleDialPlan");
        SiteTestHelper.assertNoException(getTester());
        selectOption("ruleTypeSelection", "Internal");
        SiteTestHelper.assertNoException(tester);
        setFormElement("name", "EditInternalDialRuleTest_HostnameMissing");
        setFormElement("voiceMail", "111");
        selectOption("mediaServer", "Exchange Voice Mail");
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertUserError(tester);
        dumpResponse(System.out);
        
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("FlexibleDialPlan");
        SiteTestHelper.assertNoException(getTester());
        selectOption("ruleTypeSelection", "Internal");
        SiteTestHelper.assertNoException(tester);
        setFormElement("name", "EditInternalDialRuleTest_HostnamePresent");
        setFormElement("voiceMail", "112");
        selectOption("mediaServer", "Exchange Voice Mail");
        setFormElement("mediaServerHostname", "!@#$%");
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertUserError(tester);
        
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("FlexibleDialPlan");
        SiteTestHelper.assertNoException(getTester());
        selectOption("ruleTypeSelection", "Internal");
        SiteTestHelper.assertNoException(tester);
        setFormElement("name", "EditInternalDialRuleTest_InternalServer");
        setFormElement("voiceMail", "113");
        selectOption("mediaServer", "Exchange Voice Mail");
        setFormElement("mediaServerHostname", "exchange.test.com");
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
    }
    
    public void testHostnameNotRequiredForInternalVoicemailServer() {
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("FlexibleDialPlan");
        SiteTestHelper.assertNoException(getTester());
        selectOption("ruleTypeSelection", "Internal");
        SiteTestHelper.assertNoException(tester);
        setFormElement("name", "EditInternalDialRuleTest_HostnameMissing");
        setFormElement("voiceMail", "111");
        selectOption("mediaServer", "sipX Voice Mail");
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        dumpResponse(System.out);
    }
}

package org.sipfoundry.sipxconfig.site.dialplan;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.WebTestCase;

public class EditInternalDialRuleTestUi extends WebTestCase {
    private static final String SIPX_VOICEMAIL_NAME = "Internal Voicemail Server";
    private static final String EXCHANGE_VOICEMAIL_NAME = "Exchange Voicemail Server";

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditInternalDialRuleTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("resetDialPlans");
    }
    
    public void testHostnameRequiredForExternalVoicemailServer() {
        navigateToVoicemailRule();
        setFormElement("name", "EditInternalDialRuleTest_HostnameMissing");
        setFormElement("voiceMail", "111");
        selectOption("mediaServer", EXCHANGE_VOICEMAIL_NAME);
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertUserError(tester);
        dumpResponse(System.out);
        
        navigateToVoicemailRule();
        setFormElement("name", "EditInternalDialRuleTest_HostnamePresent");
        setFormElement("voiceMail", "112");
        selectOption("mediaServer", EXCHANGE_VOICEMAIL_NAME);
        setFormElement("mediaServerHostname", "!@#$%");
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertUserError(tester);
        
        navigateToVoicemailRule();
        setFormElement("name", "EditInternalDialRuleTest_InternalServer");
        setFormElement("voiceMail", "113");
        selectOption("mediaServer", EXCHANGE_VOICEMAIL_NAME);
        setFormElement("mediaServerHostname", "exchange.test.com");
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
    }
    
    public void testHostnameNotRequiredForInternalVoicemailServer() {
        navigateToVoicemailRule();
        setFormElement("name", "EditInternalDialRuleTest_HostnameMissing");
        setFormElement("voiceMail", "111");
        selectOption("mediaServer", SIPX_VOICEMAIL_NAME);
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        dumpResponse(System.out);
    }
    
    private void navigateToVoicemailRule() {
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("FlexibleDialPlan");
        SiteTestHelper.assertNoException(getTester());
        selectOption("ruleTypeSelection", "Voicemail");
        SiteTestHelper.assertNoException(tester);
    }
}

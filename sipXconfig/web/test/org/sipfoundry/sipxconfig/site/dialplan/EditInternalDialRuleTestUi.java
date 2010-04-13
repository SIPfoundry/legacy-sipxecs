/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.junit.WebTestCase;

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
        setTextField("item:name", "EditInternalDialRuleTest_HostnameMissing");
        setTextField("dialplan:voiceMail", "111");
        selectOption("dialplan:mediaServer", EXCHANGE_VOICEMAIL_NAME);
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertUserError(tester);

        navigateToVoicemailRule();
        setTextField("item:name", "EditInternalDialRuleTest_HostnamePresent");
        setTextField("dialplan:voiceMail", "112");
        selectOption("dialplan:mediaServer", EXCHANGE_VOICEMAIL_NAME);
        setTextField("dialplan:externalVoicemailServerAddress", "!@#$%");
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertUserError(tester);

        navigateToVoicemailRule();
        setTextField("item:name", "EditInternalDialRuleTest_InternalServer");
        setTextField("dialplan:voiceMail", "113");
        selectOption("dialplan:mediaServer", EXCHANGE_VOICEMAIL_NAME);
        setTextField("dialplan:externalVoicemailServerAddress", "exchange.test.com");
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testHostnameNotRequiredForInternalVoicemailServer() {
        navigateToVoicemailRule();
        setTextField("item:name", "EditInternalDialRuleTest_HostnameMissing");
        setTextField("dialplan:voiceMail", "111");
        selectOption("dialplan:mediaServer", SIPX_VOICEMAIL_NAME);
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
    }

    private void navigateToVoicemailRule() {
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("FlexibleDialPlan");
        SiteTestHelper.assertNoException(getTester());
        SiteTestHelper.selectOption(tester, "rule:type", "Voicemail");
        SiteTestHelper.assertNoException(tester);
    }
}

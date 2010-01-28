/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.vm;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class MailboxPreferencesPageTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(MailboxPreferencesPageTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("UnifiedMessaging");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.setScriptingEnabled(tester, true);
    }

    public void testEmailNotification() {
        assertElementPresent("user:activeGreeting");
        assertElementNotPresent("user:voicemailTui");
        assertElementNotPresent("user:busyPrompt");
        assertElementPresent("user:emailAddress");
        assertElementPresent("user:voicemailProperties");

        assertElementNotPresent("user:emailFormat");
        assertElementNotPresent("user:includeAudioAttachment");
        assertElementNotPresent("user:imapHost");
        assertElementNotPresent("user:imapPort");
        assertElementNotPresent("user:imapTLS");
        assertElementNotPresent("user:imapAccount");
        assertElementNotPresent("user:imapPassword");

        assertElementPresent("user:alternateEmailAddress");
        assertElementPresent("user:voicemailToAlternateEmailNotification");
        assertElementNotPresent("user:alternateEmailFormat");
        assertElementNotPresent("user:includeAudioAttachmentAlternateEmail");

    }

    public void testVoicemailServer() {
        SiteTestHelper.assertNoException(tester);
        assertElementPresent("voicemail:server");
    }
}

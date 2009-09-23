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
        return SiteTestHelper.webTestSuite(EditVoicemailTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, true);
    }

    public void testDisabledVoicemail() {
        clickLink("disableVoicemail");
        clickLink("loginFirstTestUser");
        assertElementPresent("mailbox:disabled");
    }

    public void testEnabledVoicemail() {
        clickLink("resetVoicemail");
        clickLink("loginFirstTestUser");
        SiteTestHelper.assertNoException(tester);
        assertElementNotPresent("mailbox:disabled");
    }
}
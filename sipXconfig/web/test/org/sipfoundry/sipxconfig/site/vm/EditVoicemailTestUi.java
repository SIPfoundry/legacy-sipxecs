/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.vm;

import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditVoicemailTestUi extends WebTestCase {

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("resetVoicemail");
        clickLink("loginFirstTestUser");
        clickLink("ManageVoicemail");
        clickLinkWithText("Voice Message 00000002");
    }

    public void testSave() throws Exception {
        setTextField("vm:subject", "edit test");
        clickButton("form:ok");
        assertTextInTable("voicemail:list", "edit test");
    }
}

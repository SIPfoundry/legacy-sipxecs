/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditPhoneDefaultsTestUi extends WebTestCase {

    PhoneTestHelper tester;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditPhoneDefaultsTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        tester = new PhoneTestHelper(getTester());
    }

    public void testBooleanField() {
        tester.reset();
        tester.seedGroup(1);
        clickLink("PhoneGroups");
        clickLinkWithText("seedGroup0");
        clickLinkWithText("Acme");
        setWorkingForm("settingsForm");
        setTextField("setting:outboundProxyPort", "5060");
        clickButton("setting:ok");
        clickLinkWithText("Acme");
        setWorkingForm("settingsForm");
        assertTextFieldEquals("setting:outboundProxyPort", "5060");
    }
}

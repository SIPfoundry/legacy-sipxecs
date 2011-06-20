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

import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditPhoneSettingsTestUi extends WebTestCase {

    private PhoneTestHelper m_helper;

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testEditSettting() {
        final String outboundProxyField = "setting:outboundProxyPort";

        m_helper.seedPhone(1);
        clickLink("ManagePhones");
        clickLinkWithText(m_helper.endpoint[0].getSerialNumber());
        // NOTE: AcmePhone only setting
        clickLinkWithText("Servers");
        String proxy = String.valueOf(System.currentTimeMillis());
        setTextField(outboundProxyField, proxy);
        clickButton("setting:ok");

        // verify setting sticks
        SiteTestHelper.home(tester);
        clickLink("ManagePhones");
        clickLinkWithText(m_helper.endpoint[0].getSerialNumber());
        clickLinkWithText("Servers");
        assertTextFieldEquals(outboundProxyField, proxy);
    }
}

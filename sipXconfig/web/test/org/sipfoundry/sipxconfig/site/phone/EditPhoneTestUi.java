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

import static org.sipfoundry.sipxconfig.site.SiteTestHelper.assertNoException;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.getBaseUrl;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.webTestSuite;


public class EditPhoneTestUi extends WebTestCase {

    private PhoneTestHelper m_helper;

    public static Test suite() throws Exception {
        return webTestSuite(EditPhoneTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(getBaseUrl());
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
        m_helper.seedPhone(1);
        clickLink("ManagePhones");
        clickLinkWithText(m_helper.endpoint[0].getSerialNumber());
        setTextField("phone:serialNumber", "a00000000000");
    }

    public void testEditPhone() {
        setTextField("phone:serialNumber", "a00000000001");
        clickButton("form:ok");
        String[][] table = new String[][] {
            { "a00000000001", "", "Acme v1" },
        };
        assertTextInTable("phone:list", table[0]);
    }

    public void testAddLine() {
        clickLink("AddLine");
        assertElementPresent("user:list");
    }

    // FIXME: javascript needs to be enabled to test this
    public void _testGenerateProfiles() {
        clickButton("generateProfile");

        // check for confirm screen
        assertCheckboxSelected("restart:checkbox");
        assertFormElementPresent("datetimeDate");
        assertFormElementPresent("datetime:time");
        clickButton("generate:ok");

        assertNoException(tester);
        assertElementPresent("user:success");
    }
}

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

public class NewPhoneTestUi extends WebTestCase {

    private PhoneTestHelper m_helper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(NewPhoneTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testAddPhone() {
        clickLink("NewPhone");
        setTextField("phone:serialNumber", "000000000000");
        clickButton("form:ok");
        String[][] table = new String[][] {
            {
                "000000000000", "", "Acme v1"
            },
        };
        assertTextInTable("phone:list", table[0]);
    }

    public void testSaveAndStay() {
        clickLink("NewPhone");
        setTextField("phone:serialNumber", "000000000000");
        checkCheckbox("phone:stay");
        clickButton("form:ok");
        assertCheckboxSelected("phone:stay");
        // should clear the form
        assertTextFieldEquals("phone:serialNumber", "");

        clickButton("form:cancel");
        String[][] table = new String[][] {
            {
                "000000000000", "", "Acme v1"
            },
        };
        assertTextInTable("phone:list", table[0]);
    }

    public void testInvalidSerialNumber() {
        clickLink("NewPhone");

        // no digits
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);

        // wrong chars and wrong number
        setTextField("phone:serialNumber", "x");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);

        // 12 digits, but not valid chars
        setTextField("phone:serialNumber", "123456789abx");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);

        // 16 correct digits - not OK we only accept 12
        setTextField("phone:serialNumber", "123456789abcdef");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);

        SiteTestHelper.home(getTester());
        clickLink("NewPhone");
        // finally got it right
        setTextField("phone:serialNumber", "123456789abc");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
    }
}

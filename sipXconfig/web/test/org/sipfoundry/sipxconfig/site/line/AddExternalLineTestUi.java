/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.line;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.phone.PhoneTestHelper;

public class AddExternalLineTestUi extends WebTestCase {

    private PhoneTestHelper m_helper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AddExternalLineTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
        SiteTestHelper.seedUser(tester);
        m_helper.seedPhone(1);
        clickLink("ManagePhones");
        clickLinkWithText(m_helper.endpoint[0].getSerialNumber());
        clickLinkWithText("Lines");
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testDisplay() {
        clickLink("AddExternalLine");
        SiteTestHelper.assertNoException(tester);
        assertElementPresent("form:externalLine");
    }

    public void testAddExternalLine() {
        clickLink("AddExternalLine");
        setTextField("line:displayName", "Dil Bert");
        setTextField("line:userId", "dilbert");
        setTextField("cp:password", "1234");
        setTextField("cp:confirmPassword", "1234");
        setTextField("line:registrationServer", "frakenberry.org");
        setTextField("line:voiceMail", "2000");
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        assertTablePresent("line:list");
        assertEquals(SiteTestHelper.getRowCount(tester, "line:list"), 2); // 1 plus header
        assertLinkPresentWithText("\"Dil Bert\"<sip:dilbert@frakenberry.org>", 0);
    }

    public void testAddExternalLineWithError() {
        assertEquals(SiteTestHelper.getRowCount(tester, "line:list"), 1); // header

        clickLink("AddExternalLine");

        // empty
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);

        setTextField("line:userId", "dilbert");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);

        setTextField("cp:password", "1");
        setTextField("cp:confirmPassword", "1");
        setTextField("line:registrationServer", "fwd.org");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);

        assertTablePresent("line:list");
        assertEquals(SiteTestHelper.getRowCount(tester, "line:list"), 2); // 1 plus header

    }
}

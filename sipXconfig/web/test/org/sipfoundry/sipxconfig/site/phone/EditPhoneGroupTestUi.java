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

public class EditPhoneGroupTestUi extends WebTestCase {

    private PhoneTestHelper m_phoneTester;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditPhoneGroupTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        m_phoneTester = new PhoneTestHelper(getTester());
    }

    public void testNewGroup() {
        m_phoneTester.reset();
        clickLink("NewPhoneGroup");
        setTextField("item:name", "editPhoneGroupTestUi");
        setTextField("item:description", "test description text");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        clickLink("PhoneGroups");
        assertLinkPresentWithExactText("editPhoneGroupTestUi");
    }

    public void testDuplicateNameUserError() {
        m_phoneTester.reset();
        m_phoneTester.seedGroup(1);
        clickLink("NewPhoneGroup");
        setTextField("item:name", "seedGroup0");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);
        assertTextPresent("A group with name: seedGroup0 already exists");
    }

    public void testWhitespaceIllegal() {
        m_phoneTester.reset();
        clickLink("NewPhoneGroup");
        setTextField("item:name", "seed group");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);
        assertTextPresent("Cannot contain spaces");
    }
}

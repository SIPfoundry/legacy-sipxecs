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

public class PhoneModelsTestUi extends WebTestCase {
    PhoneTestHelper tester;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(PhoneModelsTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        tester = new PhoneTestHelper(getTester());
    }

    public void testDisplay() {
        tester.reset();
        tester.seedGroup(1);
        clickLink("PhoneGroups");
        clickLinkWithText("seedGroup0");
        SiteTestHelper.assertNoException(getTester());
        clickLinkWithText("Acme");
        SiteTestHelper.assertNoException(getTester());
    }

    public void testEditGroup() {
        tester.reset();
        tester.seedGroup(1);
        clickLink("PhoneGroups");
        clickLinkWithText("seedGroup0");
        SiteTestHelper.assertNoException(getTester());
        clickLink("group:edit");
        SiteTestHelper.assertNoException(getTester());
        assertTextFieldEquals("item:name", "seedGroup0");
        clickButton("form:ok");
        assertLinkPresentWithText("Acme");
    }

}

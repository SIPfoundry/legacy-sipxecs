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
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class PhoneModelsTestUi extends WebTestCase {
    PhoneTestHelper tester;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(PhoneModelsTestUi.class);
    }

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
        clickLinkWithText("Polycom SoundPoint IP 300");
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
        assertFormElementEquals("name", "seedGroup0");
        clickButton("form:ok");
        assertLinkPresentWithText("Polycom SoundPoint IP 300");
    }

}

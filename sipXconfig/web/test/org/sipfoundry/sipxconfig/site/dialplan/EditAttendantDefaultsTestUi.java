/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditAttendantDefaultsTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditAttendantDefaultsTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("resetDialPlans");
        clickLink("ManageAttendants");
        SiteTestHelper.assertNoException(tester);
    }

    public void testEditSetting() {
        clickLink("defaultAttendantGroup");
        SiteTestHelper.assertNoException(tester);
        setTextField("setting:interDigitTimeout", "5");
        clickButton("setting:apply");
        SiteTestHelper.assertNoException(tester);
        assertTextFieldEquals("setting:interDigitTimeout", "5");
        clickButton("setting:cancel");
        assertTablePresent("list:attendant");
    }
}

/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.device;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.junit.WebTestCase;

public class TimeZonePageTestUI extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(TimeZonePageTestUI.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("DefaultDeviceTimeZone");
        clickLink("toggleNavigation");
        clickLink("menu.timezone");
    }

    public void testTimeZonePage() throws Exception {
        assertTextFieldEquals("gmtOffset", "120");
        assertCheckboxSelected("enableDSTSettings");

        clickLink("setting:toggle"); // advanced

        assertTextFieldEquals("dstSavings", "60");

        assertTextFieldEquals("startTime", "3:00 AM");
        assertSelectedOptionEquals("startDayOfWeek", "Sunday");
        assertTextFieldEquals("startWeek", "Last Week");
        assertTextFieldEquals("startMonth", "March");

        assertTextFieldEquals("stopTime", "4:00 AM");
        assertSelectedOptionEquals("stopDayOfWeek", "Sunday");
        assertTextFieldEquals("stopWeek", "Last Week");
        assertTextFieldEquals("stopMonth", "October");
    }
}

/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.setting;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class GroupSettingsTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(GroupSettingsTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("resetCoreContext");
    }

    public void testDisplay() {
        seedGroup(1);
        clickLink("UserGroups");
        clickLinkWithText("seedGroup0");
        SiteTestHelper.assertNoException(getTester());
    }

    public void seedGroup(int count) {
        SiteTestHelper.seedGroup(getTester(), "NewUserGroup", count);
    }

    public void testEditGroup() {
        seedGroup(1);
        clickLink("UserGroups");
        clickLinkWithText("seedGroup0");
        clickLink("link:configure");
        assertTextFieldEquals("item:name", "seedGroup0");
        // Pick a group name that is very unlikely to collide with any previous names
        setTextField("item:name", "edit-seed-test-" + System.currentTimeMillis());
        clickButton("form:apply");
        SiteTestHelper.assertNoException(getTester());
        assertLinkPresent("link:configure");
    }

    public void testAddSchedules() {
        seedGroup(1);
        clickLink("UserGroups");
        clickLinkWithText("seedGroup0");
        assertLinkPresent("link:schedules");
        clickLink("link:schedules");
        assertLinkPresent("group:addSchedules");
        clickLink("group:addSchedules");
        assertFormElementPresent("item:name");
        assertFormElementPresent("item:description");
        assertLinkPresent("addPeriod");
        assertButtonPresent("form:ok");
        assertButtonPresent("form:apply");
        clickButton("form:cancel");
        SiteTestHelper.assertNoUserError(tester);
    }
}

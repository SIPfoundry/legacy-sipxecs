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
import net.sourceforge.jwebunit.WebTestCase;

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
        clickLink("group:edit");
        assertFormElementEquals("name", "seedGroup0");
        // Pick a group name that is very unlikely to collide with any previous names
        setFormElement("name", "edit-seed-test-" + System.currentTimeMillis());
        clickButton("form:ok");
        SiteTestHelper.assertNoException(getTester());
        assertLinkPresent("group:edit");
    }    
}

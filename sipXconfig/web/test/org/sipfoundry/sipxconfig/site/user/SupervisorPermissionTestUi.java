/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.junit.WebTestCase;

public class SupervisorPermissionTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(SupervisorPermissionTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        SiteTestHelper.seedUser(tester);
    }

    public void testDisplay() throws Exception {
        gotoTestUserPermissionPage();
        SiteTestHelper.assertNoException(tester);
    }

    private void gotoTestUserPermissionPage() {
        SiteTestHelper.home(tester);
        clickLink("ManageUsers");
        clickLink("userNameLink");
        clickLink("groupSupervisorLink");
    }

    public void testSetGroups() throws Exception {
        gotoTestUserPermissionPage();
        setTextField("supervisorForGroups", "group1");
        clickButton("form:apply");
        gotoTestUserPermissionPage();
        setTextField("supervisorForGroups", "group2");
        clickButton("form:apply");
        gotoTestUserPermissionPage();
        assertTextFieldEquals("supervisorForGroups", "group2");
    }
}

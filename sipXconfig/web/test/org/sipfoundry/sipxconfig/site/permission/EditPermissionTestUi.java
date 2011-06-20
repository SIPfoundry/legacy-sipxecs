/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.permission;

import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditPermissionTestUi extends WebTestCase {

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        tester.clickLink("toggleNavigation");
        clickLink("menu.permissions");
        setWorkingForm("Form");
    }

    public void testAddDuplicateCustomPermission() throws Exception {
        SiteTestHelper.assertNoException(tester);
        clickLink("permissions:add");
        setTextField("label", "duplicate");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        clickLink("permissions:add");
        setTextField("label", "duplicate");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
    }

    public void testDeleteCustomPermission() throws Exception {
        SiteTestHelper.assertNoException(tester);
        checkCheckbox("checkbox");
        clickButton("permissions:delete");
        SiteTestHelper.assertNoException(tester);
    }
}

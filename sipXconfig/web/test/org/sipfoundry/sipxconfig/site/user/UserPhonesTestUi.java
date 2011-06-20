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

import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class UserPhonesTestUi extends WebTestCase {

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester(), true);
    }

    public void testDisplay() throws Exception {
        clickLink("UserPhonesPage");
        SiteTestHelper.assertNoUserError(tester);
    }
    
    public void testShared() {
        clickLink("UserPhonesPage");
        checkCheckbox("isShared");

        clickButton("isShared:apply");

        // click apply, then navigate back.
        SiteTestHelper.home(getTester(), true);
        clickLink("UserPhonesPage");
        assertCheckboxSelected("isShared");
    }

}

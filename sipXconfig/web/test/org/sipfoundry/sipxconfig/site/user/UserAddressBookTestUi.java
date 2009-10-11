/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class UserAddressBookTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(UserAddressBookTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester(), true);
    }

    public void testDisplay() throws Exception {
        clickLink("ExtendedUserInfoPage");
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.assertNoException(tester);

        assertFormPresent("extendedUserInfoForm");
        setWorkingForm("extendedUserInfoForm");

        assertElementPresent("jobTitle");
        assertElementPresent("jobDept");
        assertElementPresent("companyName");
        assertElementPresent("assistantName");
        assertElementPresent("cellPhoneNumber");
        assertElementPresent("homePhoneNumber");
        assertElementPresent("assistantPhoneNumber");
        assertElementPresent("faxNumber");
        assertElementPresent("alternateImId");
        assertElementPresent("location");

        assertElementPresent("street");
        assertElementPresent("zip");
        assertElementPresent("city");
        assertElementPresent("country");
        assertElementPresent("state");

        assertButtonPresent("form:ok");
        assertButtonPresent("form:apply");
        assertButtonPresent("form:cancel");
    }
}

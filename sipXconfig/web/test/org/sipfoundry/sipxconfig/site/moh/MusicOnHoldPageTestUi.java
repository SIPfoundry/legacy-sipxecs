/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.moh;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;


public class MusicOnHoldPageTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(MusicOnHoldPageTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester(), true);
    }

    public void testDisplay() throws Exception {
        clickLink("MusicOnHoldPage");
        SiteTestHelper.assertNoUserError(tester);
        assertButtonPresent("form:ok");
        assertButtonPresent("form:cancel");
        assertButtonPresent("form:apply");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testUpdatePhones() throws Exception {
        // just exercises page for error
        clickLink("MusicOnHoldPage");
        clickButton("form:updatePhones");
        SiteTestHelper.assertNoUserError(tester);
    }
}

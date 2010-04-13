/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.acd;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class AcdHistoryPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AcdHistoryPageTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        //create one and only one location (primary)
        clickLink("seedLocationsManager");
        clickLink("toggleNavigation");
        clickLink("menu.locations");
        SiteTestHelper.assertNoUserError(tester);
        //make sure acd call center is not attached to primary location
        clickLink("editLocationLink");
        clickLink("link:configureBundle");
        uncheckCheckbox("MultiplePropertySelection", "0");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testNoReports() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        //test with no call center - no report is available
        clickLink("acdReports");
        SiteTestHelper.assertUserError(tester);
    }
     public void testReports() {
         getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
         SiteTestHelper.home(getTester());
         //create one and only one location (primary): Acd bundle
         //is enabled by default
         clickLink("seedLocationsManager");

         //all acd reports are available
         getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
         SiteTestHelper.home(getTester());
         clickLink("acdReports");
         clickLink("report0");
         clickLink("report1");
         clickLink("report2");
         clickLink("report3");
         clickLink("report4");
         clickLink("report5");
         clickLink("report6");
     }
}

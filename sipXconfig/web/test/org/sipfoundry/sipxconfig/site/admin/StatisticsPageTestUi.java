/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class StatisticsPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(StatisticsPageTestUi.class);
    }

    @Override
    public void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("seedLocationsManager");
        clickLink("toggleNavigation");
        clickLink("menu.statistics");
    }

    public void testStatisticsPage() throws Exception {
        assertLinkPresent("link.configureTargets");
        clickLink("link.configureTargets");
        setWorkingForm("targets");

        // select targets
        SiteTestHelper.selectRow(tester, 0, true);
        SiteTestHelper.selectRow(tester, 1, true);
        SiteTestHelper.selectRow(tester, 2, true);
        SiteTestHelper.selectRow(tester, 3, true);
        SiteTestHelper.selectRow(tester, 4, true);
        clickButton("form:ok");

        SiteTestHelper.assertNoException(tester);

        clickLink("menu.statistics");
        selectOption("PropertySelection", "host.example.org");

        assertLinkPresent("link.configureTargets");
        assertLinkPresent("report0");// summary report
        assertLinkPresent("report1");
        assertLinkPresent("report2");
        assertLinkPresent("report3");
        assertLinkPresent("report4");
        assertLinkPresent("report5");

        assertLinkPresent("image0");
        assertLinkPresent("image1");
        assertLinkPresent("image2");
        assertLinkPresent("image3");
        assertLinkPresent("image4");

        // test empty community string
        clickLink("link.configureTargets");

        // remove first and second targets to monitor
        SiteTestHelper.selectRow(tester, 1, false);
        SiteTestHelper.selectRow(tester, 2, false);
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);

        clickLink("menu.statistics");
        selectOption("PropertySelection", "host.example.org");
        assertLinkPresent("report0");// summary report
        assertLinkPresent("report1");
        assertLinkPresent("report2");
        assertLinkPresent("report3");

        assertLinkPresent("image0");
        assertLinkPresent("image1");
        assertLinkPresent("image2");

        // remove all targets to monitor
        clickLink("menu.statistics");
        selectOption("PropertySelection", "host.example.org");
        clickLink("link.configureTargets");
        SiteTestHelper.selectRow(tester, 2, false);
        SiteTestHelper.selectRow(tester, 3, false);
        SiteTestHelper.selectRow(tester, 4, false);
        clickButton("form:ok");

        SiteTestHelper.assertNoException(tester);
    }
}

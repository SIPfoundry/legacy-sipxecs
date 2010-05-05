/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class LocationsPageTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(LocationsPageTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("seedLocationsManager");
        clickLink("toggleNavigation");
        clickLink("menu.locations");
    }

    public void testDisplay() {
        SiteTestHelper.assertNoUserError(tester);
        assertTextPresent("host.example.org");
        assertEquals("Registered", SiteTestHelper.getCellAsText(getTable("locations:list"), 1, 4));
        clickLink("editLocationLink");
        assertLinkPresent("link:configureLocation");
        assertLinkPresent("link:configureBundle");
        assertLinkPresent("link:listServices");
        assertLinkPresent("link:natLocation");
        assertLinkPresent("link:monitorTarget");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddLocation() {
        SiteTestHelper.assertNoUserError(tester);
        clickLink("locations:add");
        SiteTestHelper.assertNoUserError(tester);
        setTextField("location:description", "newLocation");
        setTextField("location:address", "192.168.1.1");
        setTextField("location:fqdn", "another.example.org");
        setTextField("location:password", "123");
        clickButton("form:ok");

        SiteTestHelper.assertNoUserError(tester);
        assertTextPresent("another.example.org");
        boolean uninitialized1 = StringUtils.equals("Uninitialized", SiteTestHelper.getCellAsText(getTable("locations:list"), 1, 4));
        boolean uninitialized2 = StringUtils.equals("Uninitialized", SiteTestHelper.getCellAsText(getTable("locations:list"), 2, 4));
        assertTrue(uninitialized1 || uninitialized2);
        if (uninitialized1) {
            clickLink("editLocationLink");
        } else if (uninitialized2) {
            clickLink("editLocationLink_0");
        }
        assertLinkNotPresent("link:configureLocation");
        assertLinkNotPresent("link:listServices");
        assertLinkNotPresent("link:monitorTarget");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testDeleteLocation() throws Exception {
        setUp();
        SiteTestHelper.assertNoUserError(tester);
        clickLink("locations:add");
        SiteTestHelper.assertNoUserError(tester);
        setTextField("location:description", "newLocation");
        setTextField("location:address", "192.168.1.1");
        setTextField("location:fqdn", "another.example.org");
        setTextField("location:password", "123");
        clickButton("form:ok");
        setWorkingForm("Form");
        SiteTestHelper.selectRow(tester, 0, true);
        SiteTestHelper.selectRow(tester, 1, true);
        clickButton("locations:delete");
        //exception thrown when tried to delete the primary location
        SiteTestHelper.assertUserError(tester);
        //added secondary locations deleted
        assertTextNotPresent("another.example.org");
        //primary location not deleted
        assertTextPresent("host.example.org");
    }

    public void testSendProfiles() {
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        clickLink("locations:add");
        setTextField("location:description", "wrong location");
        setTextField("location:address", "1.1.1.1");
        setTextField("location:fqdn", "test.wrong.org");
        setTextField("location:password", "123");
        clickButton("form:ok");

        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        setWorkingForm("Form");
        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("locations:sendProfiles");

        // does not throw exceptions any more since replication happens in the background
        SiteTestHelper.assertNoException(tester);
    }

    public void testNatPanel() {
        SiteTestHelper.assertNoUserError(tester);
        assertTextPresent("host.example.org");
        assertEquals("Registered", SiteTestHelper.getCellAsText(getTable("locations:list"), 1, 4));
        clickLink("editLocationLink");
        clickLink("link:natLocation");
        setTextField("stunInterval", "abc");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);
        setTextField("stunInterval", "1025");
        setTextField("publicPort", "77777");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);
        setTextField("publicPort", "20abc");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);
    }
}

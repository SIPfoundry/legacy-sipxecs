package org.sipfoundry.sipxconfig.site.admin.commserver;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

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
    }

    public void testAddLocation() {
        SiteTestHelper.assertNoUserError(tester);
        clickLink("locations:add");
        SiteTestHelper.assertNoUserError(tester);
        setTextField("location:description", "newLocation");
        setTextField("location:address", "192.168.1.1");
        setTextField("location:fqdn", "another.example.org");
        clickButton("form:ok");

        SiteTestHelper.assertNoUserError(tester);
        assertTextPresent("another.example.org");
    }

    public void testDeleteLocation() {
        SiteTestHelper.assertNoUserError(tester);
        setWorkingForm("Form");
        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("locations:delete");
        SiteTestHelper.assertNoUserError(tester);
        assertTextNotPresent("host.example.org");
    }
}

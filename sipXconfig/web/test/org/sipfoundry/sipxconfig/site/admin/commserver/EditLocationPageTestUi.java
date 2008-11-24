package org.sipfoundry.sipxconfig.site.admin.commserver;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditLocationPageTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditLocationPageTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("seedLocationsManager");
        clickLink("toggleNavigation");
        clickLink("menu.locations");
        clickLink("locations:add");
    }

    public void testDisplay() {
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddWithValidInput() {
        SiteTestHelper.assertNoUserError(tester);
        setTextField("location:description", "newLocation");
        setTextField("location:address", "192.168.1.2");
        setTextField("location:fqdn", "another.example.org");
        setTextField("location:password","123");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testAddWithInvalidInput() {
        SiteTestHelper.assertNoUserError(tester);
        setTextField("location:address", "another.example.org");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);
    }
}

package org.sipfoundry.sipxconfig.site.service;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.junit.WebTestCase;

public class EditPresenceServiceTestUi extends WebTestCase {
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditPresenceServiceTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("seedPresenceService");
        clickLink("toggleNavigation");
        clickLink("menu.locations");
    }

    public void testDisplay() {
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        clickLink("menu.locations");
        assertLinkPresent("editLocationLink");
        clickLink("editLocationLink");
        assertLinkPresent("link:listServices");
        clickLink("link:listServices");
        assertTablePresent("servicesTable");
        assertEquals(2, getTable("servicesTable").getRowCount());
        assertLinkPresent("editSipxService");
        clickLink("editSipxService");
        SiteTestHelper.assertNoException(tester);
        assertTextPresent("SIP_PRESENCE_SIGN_IN_CODE");
    }
}

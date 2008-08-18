package org.sipfoundry.sipxconfig.site.service;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.junit.WebTestCase;

public class EditPresenceServiceTestUi extends WebTestCase {
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditPresenceServiceTestUi.class);
    }
    
    public void testDisplay() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("editPresenceService");
        SiteTestHelper.assertNoException(tester);
        assertTextPresent("SIP_PRESENCE_SIGN_IN_CODE");
    }
}

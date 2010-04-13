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

import static org.sipfoundry.sipxconfig.site.SiteTestHelper.getBaseUrl;
import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.TestPage;
import org.sipfoundry.sipxconfig.site.conference.ConferenceTestHelper;

public class UserGroupConferenceSettingsTestUi extends WebTestCase {

    private ConferenceTestHelper m_helper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(UserGroupConferenceSettingsTestUi.class);
    }

    @Override
    protected void setUp() {
        m_helper = new ConferenceTestHelper(getTester());
        getTestContext().setBaseUrl(getBaseUrl());
        SiteTestHelper.home(tester);
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("resetCoreContext");
    }

    /**
     * Tests that an invalid bridge ID is handled gracefully.
     * Normally this would be handled at the database level, but because the user group's configuration options are
     * all in settings, and settings can only store primitives, we have to store the bridge ID in the settings.
     */
    public void testBridgeRefIntegrity() {
        // Create a bridge and a group.
        m_helper.createBridge();
        SiteTestHelper.seedGroup(getTester(), "NewUserGroup", 1);
        clickLink("UserGroups");
        clickLinkWithText("seedGroup0");
        clickLink("link:conference");

        // Enable conference creation and select the previously created bridge.
        checkCheckbox("conferences:enable");
        setTextField("conferences:prefix", "prefix");
        selectOption("bridgeSelect", TestPage.TEST_LOCATION_FQDN);
        submit("submit:ok");

        // Go back and delete the bridge.
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        //besides deleting all locations, we need to reset primary location as well (it has the bridge attached)
        clickLink("seedLocationsManager");

        // Now go back to the group - make sure there is no exception page, and that the default option
        // gets selected.
        SiteTestHelper.home(tester);
        clickLink("UserGroups");
        clickLinkWithText("seedGroup0");
        clickLink("link:conference");
        //seeding location manager does not remove conference role since it is now enabled
        //by default.  Still, check no exception
        SiteTestHelper.assertNoException(tester);
    }

    /**
     * Tests to make sure the conference creation UI requires a bridge and prefix
     * to be selected when conference creation is enabled, and does not require a bridge and prefix
     * when creation is disabled.
     */
    public void testValidation() {
        m_helper.createBridge();
        SiteTestHelper.seedGroup(getTester(), "NewUserGroup", 1);
        clickLink("UserGroups");
        clickLinkWithText("seedGroup0");
        clickLink("link:conference");
        checkCheckbox("conferences:enable");
        selectOptionByValue("bridgeSelect", "");
        setTextField("conferences:prefix", "");
        submit("submit:apply");
        SiteTestHelper.assertUserError(tester);

        setTextField("conferences:prefix", "prefix");
        selectOptionByValue("bridgeSelect", "");
        submit("submit:apply");
        SiteTestHelper.assertUserError(tester);

        setTextField("conferences:prefix", "");
        selectOption("bridgeSelect", TestPage.TEST_LOCATION_FQDN);
        submit("submit:apply");
        SiteTestHelper.assertUserError(tester);

        selectOption("bridgeSelect", TestPage.TEST_LOCATION_FQDN);
        setTextField("conferences:prefix", "prefix");
        submit("submit:apply");
        SiteTestHelper.assertNoUserError(tester);

        setTextField("conferences:prefix", "");
        selectOptionByValue("bridgeSelect", "");
        uncheckCheckbox("conferences:enable");
        submit("submit:apply");
        SiteTestHelper.assertNoUserError(tester);
    }

}

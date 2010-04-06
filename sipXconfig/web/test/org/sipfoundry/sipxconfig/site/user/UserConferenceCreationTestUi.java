/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.user;

import static org.sipfoundry.sipxconfig.site.SiteTestHelper.getBaseUrl;
import junit.framework.Test;
import net.sourceforge.jwebunit.html.Table;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.TestPage;
import org.sipfoundry.sipxconfig.site.conference.ConferenceTestHelper;

public class UserConferenceCreationTestUi extends WebTestCase {

    private ConferenceTestHelper m_helper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(UserConferenceCreationTestUi.class);
    }

    @Override
    protected void setUp() {
        m_helper = new ConferenceTestHelper(tester);
        getTestContext().setBaseUrl(getBaseUrl());
        SiteTestHelper.home(tester);
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("resetCoreContext");
        clickLink("resetConferenceBridgeContext");
    }

    /*
     * Disabling unit test
     * For more info, see: http://track.sipfoundry.org/browse/XX-7630
     */
    public void test_DISABLED() {
        return;
    }

    public void DISABLED_testConferenceCreation() {
        // Create the test bridge and a user group for conference creation.
        m_helper.createBridge();
        SiteTestHelper.seedGroup(getTester(), "NewUserGroup", 1);

        // Enable conference creation and select the previously created bridge.
        clickLink("UserGroups");
        clickLinkWithText("seedGroup0");
        clickLink("link:conference");
        checkCheckbox("conferences:enable");
        setTextField("conferences:prefix", "1000");
        selectOption("bridgeSelect", TestPage.TEST_LOCATION_FQDN);
        submit("submit:ok");

        // Create a new user and assign it to the group.
        SiteTestHelper.home(tester);
        clickLink("NewUser");
        setTextField("user:userId", "300");
        setTextField("cp:password", "12345");
        setTextField("cp:confirmPassword", "12345");
        setTextField("gms:groups", "seedGroup0");
        submit("form:apply");

        // Lastly, verify that a conference was created for this user.
        SiteTestHelper.home(tester);
        clickLink("ListBridges");
        clickLinkWithText(TestPage.TEST_LOCATION_FQDN);
        clickLink("link:conferences");
        Table conferenceTable = getTable("conference:list");
        assertEquals(3, conferenceTable.getRowCount()); // the header, the pager and the one conference
        assertEquals("300-conference", SiteTestHelper.getCellAsText(conferenceTable, 2, 1)); // conference name
        assertEquals("300", SiteTestHelper.getCellAsText(conferenceTable, 2, 2)); // owner username
        assertEquals("1000300", SiteTestHelper.getCellAsText(conferenceTable, 2, 4)); // prefix + extension
    }

}

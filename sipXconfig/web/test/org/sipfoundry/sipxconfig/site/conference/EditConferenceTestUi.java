/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.conference;

import static org.sipfoundry.sipxconfig.site.SiteTestHelper.getBaseUrl;
import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.TestPage;

public class EditConferenceTestUi extends WebTestCase {

    private ConferenceTestHelper m_helper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditConferenceTestUi.class);
    }

    @Override
    public void setUp() {
        m_helper = new ConferenceTestHelper(tester);
        getTestContext().setBaseUrl(getBaseUrl());
        SiteTestHelper.home(tester);
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("resetConferenceBridgeContext");
        m_helper.createBridge();
    }

    public void testSetOwner() {
        SiteTestHelper.home(tester);
        clickLink("ListBridges");
        clickLinkWithText(TestPage.TEST_LOCATION_FQDN);
        clickLink("link:conferences");
        tester.setWorkingForm("refreshForm");
        SiteTestHelper.clickSubmitLink(tester, "conference:add");
        checkCheckbox("item:enabled");
        setTextField("item:name", "OwnerTestConf");
        setTextField("item:extension", "9999");
        setTextField("item:description", "test description 123");
        setTextField("setting:participant-code", "12345");
        clickButton("assign");
        clickButton("user:search");
        checkCheckbox("checkbox");
        clickButton("user:select");
        checkCheckbox("conference:autorecord");

        // Make sure we didn't lose all the settings
        assertTextFieldEquals("item:name", "OwnerTestConf");
        assertTextFieldEquals("item:extension", "9999");
        assertTextFieldEquals("item:description", "test description 123");
        assertTextFieldEquals("setting:participant-code", "12345");
    }

    /**
     * Tests adding a conference from a User Conferences page.
     * This Edit Conference page should have an owner assigned, and should
     * prompt for a conference bridge.
     */
    public void testAddUserConference() {
        SiteTestHelper.home(tester);
        clickLink("resetCoreContext");
        SiteTestHelper.home(tester);
        clickLink("ManageUsers");
        clickLinkWithText("testuser");
        clickLink("userConferencesLink");
        tester.setWorkingForm("refreshForm");
        SiteTestHelper.clickSubmitLink(tester, "conference:add");
        assertElementPresent("bridgeSelect");
        assertTextPresent("testuser");
        assertTextNotPresent("(none)");
    }

    /**
     * Tests adding a conference from a Conference Bridge page.
     * This Edit Conference page should not prompt for a conference bridge.
     */
    public void testAddConferenceFromBridge() {
        SiteTestHelper.home(tester);
        clickLink("ListBridges");
        clickLinkWithText(TestPage.TEST_LOCATION_FQDN);
        clickLink("link:conferences");
        tester.setWorkingForm("refreshForm");
        SiteTestHelper.clickSubmitLink(tester, "conference:add");
        assertElementNotPresent("bridgeSelect");
        assertTextPresent("(none)");
    }

    public void testConferenceValidation() {
        SiteTestHelper.home(tester);
        clickLink("ListBridges");
        clickLinkWithText(TestPage.TEST_LOCATION_FQDN);
        clickLink("link:conferences");
        tester.setWorkingForm("refreshForm");
        SiteTestHelper.clickSubmitLink(tester, "conference:add");

        // Test empty fields
        submit("form:ok");
        SiteTestHelper.assertUserError(tester);

        setTextField("item:extension", "4444");

        // Test invalid conference names (must be a valid SIP user part)
        setTextField("item:name", "conference 123");
        submit("form:ok");
        SiteTestHelper.assertUserError(tester);

        setTextField("item:name", "conf@$)(*@#");
        submit("form:ok");
        SiteTestHelper.assertUserError(tester);

        setTextField("item:name", "conference123");
        submit("form:apply");
        SiteTestHelper.assertNoUserError(tester);

        // Test invalid extensions
        setTextField("item:extension", "testextension");
        submit("form:apply");
        SiteTestHelper.assertUserError(tester);

        setTextField("item:extension", "");
        submit("form:apply");
        SiteTestHelper.assertUserError(tester);

        setTextField("item:extension", "4444");
        submit("form:apply");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testWebConference() {
        SiteTestHelper.home(tester);
        clickLink("ListBridges");
        clickLinkWithText(TestPage.TEST_LOCATION_FQDN);
        clickLink("link:conferences");
        tester.setWorkingForm("refreshForm");
        SiteTestHelper.clickSubmitLink(tester, "conference:add");
        setTextField("item:name", "dimdimConference");
        setTextField("item:extension", "123456");
        setTextField("setting:participant-code","1122");

        assertElementNotPresent("link:dimdim");
        submit("form:apply");
        assertElementPresent("link:dimdim");
        clickLink("link:dimdim");
        assertTextFieldEquals("setting:dimdim-host", "webmeeting.dimdim.com");
        setTextField("setting:user", "test");
        setTextField("setting:password", "test");
        setTextField("setting:did", "22222");
        assertElementNotPresent("startWebConference");
        assertElementNotPresent("inviteEmail");
        submit("form:apply");
        assertElementPresent("startWebConference");
        assertElementPresent("inviteEmail");
        tester.setWorkingForm("dimdimForm");
        assertHiddenFieldPresent("account", "test");
        assertHiddenFieldPresent("password", "test");
        assertHiddenFieldPresent("meetingName","dimdimConference");
        assertHiddenFieldPresent("internationalTollNumber", "22222");
        assertHiddenFieldPresent("attendeePhonePassCode", "1122");
        assertHiddenFieldPresent("attendeeKey", "1122");
    }

    /**
     * Tests to ensure a validation error is displayed when no conference bridge is selected.
     */
    public void testBridgeValidation() {
       SiteTestHelper.home(tester);
       clickLink("resetCoreContext");
       SiteTestHelper.home(tester);
       clickLink("ManageUsers");
       clickLinkWithText("testuser");
       clickLink("userConferencesLink");
       tester.setWorkingForm("refreshForm");
       SiteTestHelper.clickSubmitLink(tester, "conference:add");
       setTextField("item:name", "test101");
       setTextField("item:extension", "1101");
       checkCheckbox("item:enabled");

       // Deselect the conference bridge
       selectOptionByValue("bridgeSelect", "");
       submit("form:ok");
       SiteTestHelper.assertUserError(tester);
    }

}

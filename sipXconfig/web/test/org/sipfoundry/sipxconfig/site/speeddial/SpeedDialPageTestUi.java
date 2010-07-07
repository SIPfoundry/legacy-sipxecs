/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.speeddial;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class SpeedDialPageTestUi extends WebTestCase {
    private static String USE_GROUP_SPEED_DIALS = "speeddial:groupSync";
    private static String ADD_NUMBER_LINK = "addNumberLink";
    private static String PRESENCE_PERMISSION = "setting:subscribe-to-presence";

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(SpeedDialPageTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester(), true);
    }

    public void testDisplay() throws Exception {
        clickLink("SpeedDialPage");
        SiteTestHelper.assertNoUserError(tester);
        assertButtonPresent("form:ok");
        assertButtonPresent("form:cancel");
        assertButtonPresent("form:apply");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testUpdatePhones() throws Exception {
        // just exercises page for error
        clickLink("SpeedDialPage");
        clickButton("form:updatePhones");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testUseGroupSpeedDials() throws Exception {
        clickLink("SpeedDialPage");
        //test default value
        assertCheckboxPresent(USE_GROUP_SPEED_DIALS);
        assertCheckboxSelected(USE_GROUP_SPEED_DIALS);
        assertLinkNotPresent(ADD_NUMBER_LINK);
        //test use group speed dials checkbox functionality
        uncheckCheckbox(USE_GROUP_SPEED_DIALS);
        SiteTestHelper.assertNoUserError(tester);
        assertCheckboxNotSelected(USE_GROUP_SPEED_DIALS);
        clickButton("form:apply");
        assertLinkPresent(ADD_NUMBER_LINK);
        //test hide add number link
        checkCheckbox(USE_GROUP_SPEED_DIALS);
        clickButton("form:apply");
        assertCheckboxSelected(USE_GROUP_SPEED_DIALS);
        assertLinkNotPresent(ADD_NUMBER_LINK);
        SiteTestHelper.assertNoUserError(tester);
    }

    private void gotoTestUserPermissionPage() {
        SiteTestHelper.home(tester);
        clickLink("ManageUsers");
        clickLinkWithExactText("testuser");
        clickLink("link:permission.label");
    }

    public void testDisableSubscribeToPresence() throws Exception {
        //go to user's permission page and
        //test that the permission subscribe to presence is enabled by default
        gotoTestUserPermissionPage();
        assertCheckboxSelected(PRESENCE_PERMISSION);
        //go to Speed Dial Page
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("speedDialLink");
        //uncheck the group speed dials checkbox
        assertCheckboxPresent(USE_GROUP_SPEED_DIALS);
        uncheckCheckbox(USE_GROUP_SPEED_DIALS);
        SiteTestHelper.assertNoUserError(tester);
        assertCheckboxNotSelected(USE_GROUP_SPEED_DIALS);
        clickButton("form:apply");
        //add one button and set blf to TRUE
        assertLinkPresent(ADD_NUMBER_LINK);
        clickLink(ADD_NUMBER_LINK);
        setTextField("label", "aaa");
        setTextField("number", "123");
        checkCheckbox("blf");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.setScriptingEnabled(tester, false);
        //go to user permission page and disable the subscribe to presence permission
        gotoTestUserPermissionPage();
        uncheckCheckbox(PRESENCE_PERMISSION);
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        //go back to Speed Dial Page
        clickLink("speedDialLink");
        //check that the blf checkbox's button is disabled and set to FALSE
        assertElementPresent("label");
        assertElementPresent("number");
        assertCheckboxNotSelected("disabledBlf");
        assertElementPresentByXPath("//input[@type = 'checkbox' and @id='disabledBlf' and @disabled='disabled']");
    }
}

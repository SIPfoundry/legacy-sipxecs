/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.io.IOException;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class AlarmsPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AlarmsPageTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.setScriptingEnabled(tester, true);
        SiteTestHelper.home(tester);
        clickLink("seedLocationsManager");
        clickLink("toggleNavigation");
        clickLink("menu.alarms");
    }

    public void testDisplayEdit() throws Exception {
        SiteTestHelper.assertNoException(tester);

        clickLink("link:configureAlarms");
        assertLinkPresent("setting:toggle");
        assertElementPresent("enableEmailNotification");
        assertButtonPresent("form:apply");
        assertTableNotPresent("alarm:list");

        clickLink("link:notificationGroups");
        SiteTestHelper.assertNoException(tester);

        assertTablePresent("alarmGroups:list");
        assertButtonPresent("alarmGroups:delete");
    }

    public void testAlarmGroups() throws Exception {
        reloadPage();
        deleteAllGroups();
        createGroup();

        reloadPage();
        editGroup();

        reloadPage();
        deleteAllGroups();
    }

    private void editGroup() throws IOException {
        clickLinkWithExactText("one");
        SiteTestHelper.assertNoException(tester);

        setWorkingForm("alarmGroupForm");
        assertCheckboxSelected("item:enabled");
        assertTextFieldEquals("item:name", "one");
        assertTextFieldEquals("item:description", "test group");
        assertButtonPresent("form:cancel");
        clickButton("form:cancel");
    }

    private void createGroup() {
        clickLink("link.addAlarmGroup");
        SiteTestHelper.assertNoUserError(tester);

        setWorkingForm("alarmGroupForm");
        setTextField("item:name", "one");
        setTextField("item:description", "test group");
        assertButtonPresent("form:ok");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
    }

    private void reloadPage() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("toggleNavigation");
        clickLink("menu.alarms");
        clickLink("link:notificationGroups");
    }

    private void deleteAllGroups() {
        SiteTestHelper.assertNoException(tester);
        setWorkingForm("alarmGroupsForm");
        int rowCount = SiteTestHelper.getRowCount(tester, "alarmGroups:list");
        if (rowCount <= 1) {
            return;
        }
        for (int i = 0; i < rowCount - 1; i++) {
            SiteTestHelper.selectRow(tester, i, true);
        }
        clickButton("alarmGroups:delete");

        // The column header row, plus the "default" group that cannot be deleted.
        assertEquals(2, SiteTestHelper.getRowCount(tester, "alarmGroups:list"));
    }

    public void testDisplayHistory() throws Exception {
        assertLinkPresent("link:historyAlarms");
        clickLink("link:historyAlarms");
        SiteTestHelper.assertNoException(tester);

        assertElementPresent("hosts");
        SiteTestHelper.selectOption(tester, "hosts", "host.example.org");
        assertElementPresent("datetimeDate");
        assertElementPresent("datetime:time");
        assertElementPresent("datetimeDate_0");
        assertElementPresent("datetime:time_0");
        assertButtonPresent("showAlarms");
        assertTablePresent("alarmEvent:list");
    }
}

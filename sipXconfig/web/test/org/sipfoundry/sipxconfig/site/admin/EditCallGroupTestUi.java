/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditCallGroupTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditCallGroupTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.seedUser(getTester());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("resetCallGroupContext");
        clickLink("NewCallGroup");
        setFormData();
    }

    public void testDisplay() {
        SiteTestHelper.assertNoException(getTester());
        assertElementPresent("item:name");
        assertElementPresent("item:extension");
        assertElementPresent("item:description");
        assertElementPresent("item:enabled");
        assertElementPresent("callgroup:voicemailFallback");
        assertElementPresent("callgroup:userForward");

        assertLinkPresent("callgroup:addRing");
        assertEquals(1, SiteTestHelper.getRowCount(tester, "userring:list"));
        assertButtonPresent("userring:delete");
        assertButtonPresent("userring:moveUp");
        assertButtonPresent("userring:moveDown");
    }

    public void testAddDeleteRing() throws Exception {
        addUser();

        assertEquals(2, SiteTestHelper.getRowCount(tester, "userring:list"));

        reopenScreen();

        assertEquals(2, SiteTestHelper.getRowCount(tester, "userring:list"));

        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("userring:delete");

        assertEquals(1, SiteTestHelper.getRowCount(tester, "userring:list"));

        reopenScreen();

        assertEquals(1, SiteTestHelper.getRowCount(tester, "userring:list"));
    }

    private void reopenScreen() {
        // leave the screen and get back to it again
        SiteTestHelper.home(getTester());
        clickLink("ListCallGroups");
        clickLinkWithText("testName");
    }

    public void testMoveRing() throws Exception {
        for (int i = 0; i < 3; i++) {
            addUser();
        }

        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("userring:moveDown");
        SiteTestHelper.assertRowNotSelected(tester, 0);
        SiteTestHelper.assertRowSelected(tester, 1);
        clickButton("userring:moveUp");
        SiteTestHelper.assertRowSelected(tester, 0);
        SiteTestHelper.assertRowNotSelected(tester, 1);

        for (int i = 0; i < 3; i++) {
            SiteTestHelper.selectRow(tester, i, true);
        }
    }

    private void addUser() throws Exception {
        SiteTestHelper.clickSubmitLink(tester, "callgroup:addRing");

        clickButton("user:search");
        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("user:select");
    }

    private void setFormData() {
        setWorkingForm("form");
        setTextField("item:name", "testName");
        setTextField("item:extension", "123");
    }

    /** Make sure that one cannot enable 2 huntgroups with the same name */
    public void testEnableDuplicate() throws Exception {
        checkCheckbox("item:enabled");
        setTextField("item:name", "xxxx");
        setTextField("item:extension", "123");
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);

        clickLink("callgroups:add");
        checkCheckbox("item:enabled");
        setTextField("item:name", "xxxx");
        setTextField("item:extension", "123");
        clickButton("form:ok");
        // this time we expect page will complain
        SiteTestHelper.assertUserError(tester);
    }
}

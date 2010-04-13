/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.acd;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditAcdAgentTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditAcdAgentTestUi.class);
    }

    public void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("seedTestUser");
        clickLink("seedAcdServer");
        clickLink("listAcdServers");
        clickLink("editRowLink");

        clickLink("link:queues");
        clickLink("queue:add");
        setFormData();
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        clickLink("link:agents");
    }

    public void testDisplay() {
        SiteTestHelper.assertNoException(getTester());
        assertLinkPresent("agent:add");
        assertTablePresent("agent:list");
        assertEquals(1, SiteTestHelper.getRowCount(tester, "agent:list"));
        assertButtonPresent("agent:delete");
        assertButtonPresent("agent:moveUp");
        assertButtonPresent("agent:moveDown");
    }

    public void testAddDelete() throws Exception {
        addAgent(0);

        assertTablePresent("agent:list");
        assertEquals(2, SiteTestHelper.getRowCount(tester, "agent:list"));

        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("agent:delete");

        SiteTestHelper.assertNoException(tester);
        assertTablePresent("agent:list");
        assertEquals(1, SiteTestHelper.getRowCount(tester, "agent:list"));
    }

    public void testMove() throws Exception {
        int count = 2;
        for (int i = 0; i < count; i++) {
            addAgent(i);
        }
        assertEquals(count + 1, SiteTestHelper.getRowCount(tester, "agent:list"));

        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("agent:moveDown");
        SiteTestHelper.assertRowNotSelected(tester, 0);
        SiteTestHelper.assertRowSelected(tester, 1);
        clickButton("agent:moveUp");
        SiteTestHelper.assertRowSelected(tester, 0);
        SiteTestHelper.assertRowNotSelected(tester, 1);

        for (int i = 0; i < count; i++) {
            SiteTestHelper.selectRow(tester, i, true);
        }
    }

    private void addAgent(int i) throws Exception {
        setWorkingForm("agentsForm");
        clickLink("agent:add");
        SiteTestHelper.assertNoUserError(tester);
        submit("user:search");
        SiteTestHelper.selectRow(tester, i, true);
        submit("user:select");
        SiteTestHelper.assertNoException(tester);
    }

    private void setFormData() {
        setTextField("item:name", "testName");
    }
}

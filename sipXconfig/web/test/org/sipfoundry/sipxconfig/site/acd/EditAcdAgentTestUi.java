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
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditAcdAgentTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditAcdAgentTestUi.class);
    }

    public void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("seedTestUser");
        clickLink("resetAcdContext");
        clickLink("acdServerPage");
        clickButton("form:apply");
        clickLink("link:queues");
        clickLink("queue:add");
        setFormData();
        clickButton("form:apply");
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
        SiteTestHelper.initUploadFields(getDialog().getForm(), "EditAcdAgent");
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
        SiteTestHelper.initUploadFields(getDialog().getForm(), "EditAcdAgent");        
        clickButton("agent:moveDown");
        SiteTestHelper.assertRowNotSelected(tester, 0);
        SiteTestHelper.assertRowSelected(tester, 1);
        SiteTestHelper.initUploadFields(getDialog().getForm(), "EditAcdAgent");
        clickButton("agent:moveUp");
        SiteTestHelper.assertRowSelected(tester, 0);
        SiteTestHelper.assertRowNotSelected(tester, 1);

        for (int i = 0; i < count; i++) {
            SiteTestHelper.selectRow(tester, i, true);
        }
    }

    private void addAgent(int i) throws Exception {
        SiteTestHelper.initUploadFields(getDialog().getForm(), "EditAcdAgent");
        clickLink("agent:add");
        SiteTestHelper.assertNoException(tester);
        clickButton("user:search");
        SiteTestHelper.selectRow(tester, i, true);
        clickButton("user:select");
        SiteTestHelper.assertNoException(tester);
    }

    private void setFormData() {
        setFormElement("name", "testName");
        SiteTestHelper.initUploadFields(getDialog().getForm(), "EditAcdAgent");
    }
}

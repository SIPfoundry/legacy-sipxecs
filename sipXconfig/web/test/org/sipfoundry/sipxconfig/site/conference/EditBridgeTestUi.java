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

import junit.framework.Test;

import org.apache.commons.lang.ArrayUtils;
import org.sipfoundry.sipxconfig.site.ListWebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditBridgeTestUi extends ListWebTestCase {

	private ConferenceTestHelper m_conferenceHelper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditBridgeTestUi.class);
    }

    public EditBridgeTestUi() {
        super("EditBridge", "resetConferenceBridgeContext", "conference");
        setPager(true);
        setHasDuplicate(false);
        setExactCheck(false);
        setAddLinkSubmit(true);
    }

    public void testTabNames() {
        SiteTestHelper.home(tester);
        clickLink("EditBridge");
        assertLinkPresent("link:config");
        assertLinkNotPresent("link:conferences");
        m_conferenceHelper.createBridge("testCreateBridge");
        SiteTestHelper.home(tester);
        clickLink("ListBridges");
        clickLinkWithText("testCreateBridge");
        assertLinkPresent("link:config");
        assertLinkPresent("link:conferences");
    }
    
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("resetConferenceBridgeContext");
        clickLink("EditBridge");
        setWorkingForm("form");
        setTextField("item:name", "bridge_test");
        clickButton("form:apply");
        clickLink("link:conferences");
        m_conferenceHelper = new ConferenceTestHelper(tester);
        SiteTestHelper.assertNoUserError(tester);
    }

    protected String[] getParamNames() {
        return new String[] {
            "item:name", "item:extension", "item:description"
        };
    }

    protected String[] getParamValues(int i) {
        return new String[] {
            "conference" + i, "444" + i, "Description" + i
        };
    }

    protected Object[] getExpectedTableRow(String[] paramValues) {
        Object[] expected = super.getExpectedTableRow(paramValues);
        expected = ArrayUtils.add(expected, 2, "Disabled");
        return ArrayUtils.add(expected, "");
    }

    // 2 = 1 thead (columns) + 1 tfoot (pager)
    //Table row counting will be the true value+2
    public void testGroupFilter() throws Exception {
	SiteTestHelper.home(getTester());
	clickLink("resetConferenceBridgeContext");
        SiteTestHelper.seedGroup(tester, "NewUserGroup", 2);
        m_conferenceHelper.createBridge("groupFilterTestBridge");
        m_conferenceHelper.groupConferenceAutomation("seedGroup0", "groupFilterTestBridge");
        m_conferenceHelper.groupConferenceAutomation("seedGroup1", "groupFilterTestBridge");
        m_conferenceHelper.addUserToGroup(3, "seedGroup1",12200);
        m_conferenceHelper.addUserToGroup(2, "seedGroup0",13300);

        SiteTestHelper.home(tester);
        clickLink("ListBridges");
        clickLinkWithText("groupFilterTestBridge");
        clickLink("link:conferences");

        // all conferences
        SiteTestHelper.selectOption(tester, "group:filter", "- all -");
        int tableCount = SiteTestHelper.getRowCount(tester, "conference:list");
        assertEquals(7, tableCount);

        //filter User Group seedGroup0 conferences
        SiteTestHelper.selectOption(tester, "group:filter", "seedGroup0");
        SiteTestHelper.assertNoException(tester);
        tableCount = SiteTestHelper.getRowCount(tester, "conference:list");
        assertEquals(4, tableCount);

        //filter User Group seedGroup1 conferences
        SiteTestHelper.selectOption(tester, "group:filter", "seedGroup1");
        SiteTestHelper.assertNoException(tester);
        tableCount = SiteTestHelper.getRowCount(tester, "conference:list");
        assertEquals(5, tableCount);

        // back to all conferences
        SiteTestHelper.selectOption(tester, "group:filter", "- all -");
        tableCount = SiteTestHelper.getRowCount(tester, "conference:list");
        assertEquals(7, tableCount);
    }

}

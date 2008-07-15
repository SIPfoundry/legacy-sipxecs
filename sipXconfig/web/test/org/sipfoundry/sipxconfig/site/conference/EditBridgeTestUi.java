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

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditBridgeTestUi.class);
    }

    public EditBridgeTestUi() {
        super("EditBridge", "resetConferenceBridgeContext", "conference");
        setHasDuplicate(false);
        setExactCheck(false);
        setAddLinkSubmit(true);
    }

    public void testTabNames() {
        SiteTestHelper.home(tester);
        clickLink("EditBridge");
        assertLinkPresent("link:config");
        assertLinkNotPresent("link:conferences");
        
        new ConferenceTestHelper(tester).createBridge("testCreateBridge");
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
}

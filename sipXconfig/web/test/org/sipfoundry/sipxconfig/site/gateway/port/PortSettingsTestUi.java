/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.gateway.port;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class PortSettingsTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(PortSettingsTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, false);
        clickLink("NewFxo");
    }

    public void testAddPortAndEditSettings() {
        SiteTestHelper.assertNoException(tester);
        clickLink("tab:ports");
        assertEquals(1, SiteTestHelper.getRowCount(tester, "line:list"));  // 1 is header
        setWorkingForm("Form");
        SiteTestHelper.clickSubmitLink(tester, "addPort");
        setWorkingForm("settingsForm");
        assertElementPresent("setting:ProtocolType");  // TP260 port setting
        setTextField("setting:MaxChannel", "22");
        clickButton("setting:ok");
        SiteTestHelper.assertNoException(tester);
        clickLink("tab:ports");
        assertEquals(2, SiteTestHelper.getRowCount(tester, "line:list"));
    }

    public void testDeletePort() {
        SiteTestHelper.assertNoException(tester);
        clickLink("tab:ports");
        setWorkingForm("Form");
        SiteTestHelper.clickSubmitLink(tester, "addPort");
        clickButton("setting:ok");
        clickLink("tab:ports");
        assertEquals(2, SiteTestHelper.getRowCount(tester, "line:list"));
        checkCheckbox("checkbox");
        clickButton("port:delete");
        assertEquals(1, SiteTestHelper.getRowCount(tester, "line:list"));
    }
}

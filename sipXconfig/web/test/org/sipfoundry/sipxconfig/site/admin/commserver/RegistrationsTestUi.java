/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import com.meterware.httpunit.WebTable;

public class RegistrationsTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(RegistrationsTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        //SiteTestHelper.setScriptingEnabled(true);
        clickLink("Registrations");
    }

    public void testRefresh() {
        SiteTestHelper.assertNoException(tester);
        setWorkingForm("refreshForm");
        assertButtonPresent("refresh");
        assertElementPresent("registrations:page");
        clickButton("refresh");
        SiteTestHelper.assertNoException(tester);
        setWorkingForm("refreshForm");
        assertButtonPresent("refresh");
        assertElementPresent("registrations:page");
    }

    /**
     * DISABLED because this requires javascript and javascript has to be disabled now that 
     * page uses dojo which is incompatible with the version of httpunit we are using  
     */
    public void DISABLED_testShowPrimary() {
        SiteTestHelper.assertNoException(tester);
        
        // display primary column
        clickLink("setting:toggle");
        SiteTestHelper.assertNoException(tester);
        assertTablePresent("registrations:list");
        WebTable table = getDialog().getWebTableBySummaryOrId("registrations:list");
        assertEquals(4, table.getColumnCount());
        
        // hide primary column
        clickLink("setting:toggle");
        SiteTestHelper.assertNoException(tester);
        assertTablePresent("registrations:list");
        table = getDialog().getWebTableBySummaryOrId("registrations:list");
        assertEquals(3, table.getColumnCount());
    }
}

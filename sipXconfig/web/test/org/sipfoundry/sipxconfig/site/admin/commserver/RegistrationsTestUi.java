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
import net.sourceforge.jwebunit.html.Table;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

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
        assertTablePresent("registrations:list");
        clickButton("refresh");
        SiteTestHelper.assertNoException(tester);
        setWorkingForm("refreshForm");
        assertButtonPresent("refresh");
        assertTablePresent("registrations:list");
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
        Table table = getTable("registrations:list");
        assertEquals(4, SiteTestHelper.getColumnCount(table));

        // hide primary column
        clickLink("setting:toggle");
        SiteTestHelper.assertNoException(tester);
        assertTablePresent("registrations:list");
        table = getTable("registrations:list");
        assertEquals(3, SiteTestHelper.getColumnCount(table));
    }
}

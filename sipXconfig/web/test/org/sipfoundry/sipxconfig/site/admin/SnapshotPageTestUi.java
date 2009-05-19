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

public class SnapshotPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(SnapshotPageTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("SnapshotPage");
    }

    /**
     * Does not check if operation was successful - just checks if no Tapestry exceptions show up
     * would have to send mock backup shell script to artificial root.
     */
    public void testDisplay() {
        SiteTestHelper.assertNoException(getTester());
        assertFormPresent("snapshotForm");
        setWorkingForm("snapshotForm");
        assertCheckboxSelected("snapshot:logs");
        assertCheckboxNotSelected("snapshot:credentials");
        assertCheckboxNotSelected("snapshot:cdr");
        assertCheckboxNotSelected("snapshot:profiles");
        assertCheckboxSelected("snapshot:filterTime");
        assertElementPresent("datetimeDate");
        assertElementPresent("datetime:time");
        assertElementPresent("datetimeDate_0");
        assertElementPresent("datetime:time_0");
        clickButton("form:apply");
        SiteTestHelper.assertNoException(getTester());
        SiteTestHelper.assertNoUserError(getTester());
        assertFormPresent("refreshForm");
    }
}

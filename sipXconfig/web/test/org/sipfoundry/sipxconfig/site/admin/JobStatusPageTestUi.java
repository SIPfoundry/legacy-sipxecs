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
import net.sourceforge.jwebunit.html.Table;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class JobStatusPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(JobStatusPageTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
    }

    protected void tearDown() throws Exception {
    }

    public void testDisplay() throws Exception {
        clickLink("jobs:populate");
        clickLink("JobStatusPage");
        SiteTestHelper.assertNoException(tester);
        Table table = tester.getTable("jobs:list");
        assertEquals(5, table.getRowCount());

        // refresh table
        submit("refresh");
        // clickButton("Refresh");
        table = tester.getTable("jobs:list");
        assertEquals(5, table.getRowCount());

        // remove finished jobs
        submit("jobs:remove");
        table = tester.getTable("jobs:list");
        assertEquals(4, table.getRowCount());
    }

    public void testClear() throws Exception {
        clickLink("jobs:populate");
        clickLink("JobStatusPage");
        SiteTestHelper.assertNoException(tester);
        Table table = tester.getTable("jobs:list");
        assertEquals(5, table.getRowCount());

        // remove finished jobs
        submit("jobs:clear");
        table = tester.getTable("jobs:list");
        assertEquals(1, table.getRowCount());
    }
}

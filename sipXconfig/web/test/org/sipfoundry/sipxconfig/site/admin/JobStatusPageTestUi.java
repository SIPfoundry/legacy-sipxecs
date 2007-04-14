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
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import com.meterware.httpunit.WebTable;

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
        WebTable table = tester.getDialog().getWebTableBySummaryOrId("jobs:list");
        assertEquals(5, table.getRowCount());

        // refresh table
        submit("refresh");
        // clickButton("Refresh");
        table = tester.getDialog().getWebTableBySummaryOrId("jobs:list");
        assertEquals(5, table.getRowCount());

        // remove finishedjobs
        submit("remove");
        table = tester.getDialog().getWebTableBySummaryOrId("jobs:list");
        assertEquals(4, table.getRowCount());
    }

    public void testClear() throws Exception {
        clickLink("jobs:populate");
        clickLink("JobStatusPage");
        SiteTestHelper.assertNoException(tester);
        WebTable table = tester.getDialog().getWebTableBySummaryOrId("jobs:list");
        assertEquals(5, table.getRowCount());

        // remove finishedjobs
        submit("clear");
        table = tester.getDialog().getWebTableBySummaryOrId("jobs:list");
        assertEquals(1, table.getRowCount());
    }
}

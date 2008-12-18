/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.configdiag;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ConfigurationDiagnosticPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ConfigurationDiagnosticPageTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("ConfigurationDiagnosticPage");
    }

    public void testConfigDiagsLoaded() throws Exception {
        SiteTestHelper.assertNoException(tester);
        int diagCount = SiteTestHelper.getRowCount(tester, "configdiag:list");

        // expect header row plus 2 rows per test
        // this assert needs to be changed as tests are added or removed
        int testCounter = 16;
        assertEquals(2 * testCounter + 1, diagCount);

    }

    public void testDownloadLinkPresent() throws Exception {
        SiteTestHelper.assertNoException(tester);
        assertLinkPresent("downloadLink");
    }
}

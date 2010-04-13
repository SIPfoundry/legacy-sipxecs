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

public class BulkImportTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(BulkImportTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("link:import");
    }

    /**
     * Basic display test
     */
    public void testDisplay() {
        SiteTestHelper.assertNoException(getTester());
        assertElementPresent("import:form");
        assertElementPresent("import:upload");
        assertButtonPresent("import:ok");
    }

    public void testExport() {
        clickLink("link:export");
        submit("export:now");

        SiteTestHelper.assertNoException(getTester());
        assertLinkPresent("exportFile");

    }
}

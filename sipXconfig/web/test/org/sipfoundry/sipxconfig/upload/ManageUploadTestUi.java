/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.upload;

import java.io.File;

import junit.framework.Test;
import net.sourceforge.jwebunit.html.Table;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ManageUploadTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManageUploadTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("link:resetUpload");
    }

    public void testDisplay() {
        clickLink("link:upload");
        SiteTestHelper.assertNoException(tester);
    }

    public void testDelete() throws Exception {
        seedUpload();
        SiteTestHelper.home(tester);
        clickLink("link:upload");
        int tableCount = SiteTestHelper.getRowCount(tester, "upload:list");
        assertEquals(2, tableCount);
        SiteTestHelper.enableCheckbox(tester, "checkbox", 0, true);
        clickButton("upload:delete");
        int nextTableCount = SiteTestHelper.getRowCount(tester, "upload:list");
        assertEquals(1, nextTableCount);
    }

    public void testEmptyActivation() throws Exception {
        seedUpload();
        SiteTestHelper.home(tester);
        clickLink("link:upload");
        clickButton("upload:activate");
        SiteTestHelper.assertNoException(tester);
    }

    public void testActivation() throws Exception {
        seedUpload();
        SiteTestHelper.home(tester);
        clickLink("link:upload");

        // activate
        SiteTestHelper.enableCheckbox(tester, "checkbox", 0, true);
        clickButton("upload:activate");
        Table table = getTable("upload:list");
        assertEquals("Active", SiteTestHelper.getCellAsText(table, 1, 2));

        // inactivate
        SiteTestHelper.enableCheckbox(tester, "checkbox", 0, true);
        clickButton("upload:inactivate");
        table = getTable("upload:list");
        assertEquals("Inactive", SiteTestHelper.getCellAsText(table, 1, 2));
    }

    public void testActivationOfActiveDeviceFiles() throws Exception {
        seedUpload();
        SiteTestHelper.home(tester);
        clickLink("link:upload");

        // activate once
        SiteTestHelper.enableCheckbox(tester, "checkbox", 0, true);
        clickButton("upload:activate");
        String actual = SiteTestHelper.getCellAsText(getTable("upload:list"), 1, 2);
        assertEquals("Active", actual);

        // activate a second time (should cause BySummaryOrIduser error but no exception)
        SiteTestHelper.enableCheckbox(tester, "checkbox", 0, true);
        clickButton("upload:activate");
        SiteTestHelper.assertNoException(tester);
    }

    private void seedUpload() throws Exception {
        clickLink("link:newUpload");
        SiteTestHelper.assertNoException(tester);
        setTextField("item:name", "manage uploads seed");
        clickButton("form:apply");
        File f = File.createTempFile("manage-upload", ".dat");
        setTextField("promptUpload", f.getAbsolutePath());
        clickButton("form:ok");
    }
}

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
import net.sourceforge.jwebunit.WebTestCase;

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
        String[][] sparseTableCellValues = getDialog().getSparseTableBySummaryOrId("upload:list");
        assertEquals("Active", sparseTableCellValues[1][1]);

        // inactivate
        SiteTestHelper.enableCheckbox(tester, "checkbox", 0, true);
        clickButton("upload:inactivate");
        sparseTableCellValues = getDialog().getSparseTableBySummaryOrId("upload:list");
        assertEquals("Inactive", sparseTableCellValues[1][1]);
    }
    
    private void seedUpload() throws Exception {
        clickLink("link:newUpload");
        setFormElement("name", "manage uploads seed");
        clickButton("form:apply");        
        File f = File.createTempFile("manage-upload", ".dat");
        assertTrue(getDialog().getForm().hasParameterNamed("promptUpload"));
        getDialog().getForm().setParameter("promptUpload", f);
        clickButton("form:ok");
    }
}

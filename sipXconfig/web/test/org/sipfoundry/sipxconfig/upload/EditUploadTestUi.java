/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *  $
 */
package org.sipfoundry.sipxconfig.upload;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditUploadTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditUploadTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("link:resetUpload");
        clickLink("link:newUpload");
    }

    public void testNewUpload() throws Exception {
        setFormElement("name", "new upload");
        assertFalse(getDialog().getForm().hasParameterNamed("promptUpload"));
        assertButtonNotPresent("upload:activate");
        assertButtonNotPresent("upload:inactivate");
        clickButton("form:apply");

        SiteTestHelper.initUploadFields(getDialog().getForm(), "new-upload-test");
        clickButton("form:apply");

        SiteTestHelper.assertNoException(tester);
        // download file link implies file uploaded ok
        assertLinkPresentWithText("Download");
    }

    public void testActivate() throws Exception {
        setFormElement("name", "new upload");
        assertFalse(getDialog().getForm().hasParameterNamed("promptUpload"));
        clickButton("form:apply");
        SiteTestHelper.initUploadFields(getDialog().getForm(), "new-upload-test");
        assertButtonNotPresent("upload:inactivate");
        assertButtonPresent("upload:activate");
        clickButton("upload:activate");
        assertButtonNotPresent("upload:activate");
        assertButtonPresent("upload:inactivate");
        clickButton("upload:inactivate");
    }
    
    public void testCancel() throws Exception {
        setFormElement("name", "cancelled");
        clickButton("form:cancel");
        assertTextNotPresent("cancelled");       
    }       
}

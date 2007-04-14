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

import java.io.File;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class MusicOnHoldTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(MusicOnHoldTestUi.class);
    }

    private File m_tempFile;

    public void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        m_tempFile = File.createTempFile("MusicOnHoldTestUi", null);
    }

    public void testDisplay() throws Exception {
        clickLink("MusicOnHold");
        SiteTestHelper.assertNoException(tester);
        getDialog().getForm().setParameter("promptUpload", m_tempFile);
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        assertOptionEquals("prompt", m_tempFile.getName());
    }
}

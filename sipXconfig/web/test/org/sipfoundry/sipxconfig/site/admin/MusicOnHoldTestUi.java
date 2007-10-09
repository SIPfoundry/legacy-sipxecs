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
import org.sipfoundry.sipxconfig.site.dialplan.EditAutoAttendantTestUi;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class MusicOnHoldTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(MusicOnHoldTestUi.class);
    }

    private File m_tempFile;
    private File m_wavFile;

    public void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        m_tempFile = File.createTempFile("MusicOnHoldTestUi", null);
        m_wavFile = new File(TestUtil.getTestSourceDirectory(EditAutoAttendantTestUi.class) + "/"
                + EditAutoAttendantTestUi.PROMPT_TEST_FILE );
    }

    public void testDisplay() throws Exception {
        clickLink("MusicOnHold");
        SiteTestHelper.assertNoException(tester);
        getDialog().getForm().setParameter("promptUpload", m_tempFile);
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        getDialog().getForm().setParameter("promptUpload", m_wavFile);
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        assertOptionEquals("prompt", m_wavFile.getName());
    }
}

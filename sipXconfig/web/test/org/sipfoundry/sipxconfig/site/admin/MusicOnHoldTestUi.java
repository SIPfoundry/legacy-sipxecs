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
import net.sourceforge.jwebunit.junit.WebTestCase;

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
                + EditAutoAttendantTestUi.PROMPT_TEST_FILE);
    }

    public void testDisplay() throws Exception {
        clickLink("MusicOnHold");
        SiteTestHelper.assertNoException(tester);
        assertElementPresent("setting:MOH_SOURCE");
        setTextField("promptUpload", m_tempFile.getAbsolutePath());
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        setTextField("promptUpload", m_wavFile.getAbsolutePath());
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        setWorkingForm("moh");
        assertSelectedOptionEquals("common_AssetSelectorMultiple_0", m_wavFile.getName());
    }
}

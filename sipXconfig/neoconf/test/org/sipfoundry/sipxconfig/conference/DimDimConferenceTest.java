/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.conference;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.TestHelper;

public class DimDimConferenceTest extends TestCase {
    private Conference m_conference;

    @Override
    protected void setUp() throws Exception {
        m_conference = new Conference();
        m_conference.setName("my_conf");
        m_conference.setExtension("12345");
        m_conference.setModelFilesContext(TestHelper.getModelFilesContext());
    }

    public void testNoDimDimConfigured() {
        DimDimConference dimDimConference = new DimDimConference(m_conference);
        assertFalse(dimDimConference.isConfigured());
    }

    public void testDimDimConfigured() {
        DimDimConference dimDimConference = new DimDimConference(m_conference);

        m_conference.setSettingTypedValue("web-meeting/dimdim-host", "my.dimdim.com");
        m_conference.setSettingTypedValue("web-meeting/did", "6131234567");
        m_conference.setSettingTypedValue("web-meeting/user", "dimUser");
        m_conference.setSettingTypedValue("web-meeting/password", "dimPass");

        assertTrue(dimDimConference.isConfigured());
    }
}

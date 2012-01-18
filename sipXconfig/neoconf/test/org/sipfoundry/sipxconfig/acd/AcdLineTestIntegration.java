/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class AcdLineTestIntegration extends IntegrationTestCase {
    private AcdLine m_acdLine;
    private AcdServer m_acdServer;

    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        m_acdLine.setAcdServer(m_acdServer);
    }

    public void testSetSettings() throws Exception {
        Setting settings = m_acdLine.getSettings();
        m_acdLine.setSettingValue(AcdLine.URI, "abc");
        assertEquals("abc", settings.getSetting("acd-line/uri").getValue());
        m_acdLine.setExtension("3333");
        m_acdLine.initialize();
        assertEquals("3333", settings.getSetting("acd-line/extension").getValue());
    }

    public void setAcdLine(AcdLine acdLine) {
        m_acdLine = acdLine;
    }

    public void setAcdServer(AcdServer acdServer) {
        m_acdServer = acdServer;
    }
}

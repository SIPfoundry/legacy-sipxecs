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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Setting;

public class AcdLineTestDb extends TestCase {

    private AcdLine m_line;
    private AcdServer m_server;

    protected void setUp() throws Exception {
        m_server = (AcdServer) TestHelper.getApplicationContext().getBean("acdServer");
        m_line = (AcdLine) TestHelper.getApplicationContext().getBean("acdLine");
        m_line.setAcdServer(m_server);
    }

    public void testSetSettings() throws Exception {
        Setting settings = m_line.getSettings();
        m_line.setSettingValue(AcdLine.URI, "abc");
        assertEquals("abc", settings.getSetting("acd-line/uri").getValue());
        m_line.setExtension("3333");
        m_line.initialize();
        assertEquals("3333", settings.getSetting("acd-line/extension").getValue());
    }
}

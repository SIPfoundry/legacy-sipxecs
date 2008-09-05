/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.InputStream;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxServerTest extends TestCase {

    private SipxServer m_server;

    protected void setUp() throws Exception {
        m_server = setUpSipxServer();
        InputStream configDefs = SipxServerTest.class.getResourceAsStream("config.defs");
        TestHelper
                .copyStreamToDirectory(configDefs, TestHelper.getTestDirectory(), "config.defs");
    }

    public void testGetSetting() {
        Setting settings = m_server.getSettings();
        assertNotNull(settings);
    }

    public void testGetMusicOnHoldUri() {
        m_server.setMohUser("~~mh~");
        assertEquals("sip:~~mh~@example.org", m_server.getMusicOnHoldUri("example.org"));
    }

    public static SipxServer setUpSipxServer() {
        SipxServer server = new SipxServer();
        server.setConfigDirectory(TestHelper.getTestDirectory());

        server.setModelFilesContext(TestHelper.getModelFilesContext());
        server.setMohUser("~~mh~");
        return server;
    }
}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.List;

import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class SipxServiceTestIntegration extends IntegrationTestCase {

    private SipxStatusService m_statusService;

    public void testGetConfigurations() {
        List configurations = m_statusService.getConfigurations();
        assertEquals(2, configurations.size());
    }

    public void setSipxStatusService(SipxStatusService sipxStatusService) {
        m_statusService = sipxStatusService;
    }
    
    public void testGetSetLogLevel() {
        String level = m_statusService.getLogLevel();
        assertEquals("NOTICE", level);
        level = "DEBUG";
        m_statusService.setLogLevel(level);
        level = m_statusService.getLogLevel();
        assertEquals("DEBUG", level);
    }
}

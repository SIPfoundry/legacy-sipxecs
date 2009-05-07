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

import java.util.ArrayList;
import java.util.List;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.LoggingManager;

public class SipxServiceTestIntegration extends IntegrationTestCase {

    private SipxStatusService m_statusService;

    public void testGetConfigurations() {
        List configurations = m_statusService.getConfigurations();
        assertEquals(3, configurations.size());
    }

    public void setSipxStatusService(SipxStatusService sipxStatusService) {
        m_statusService = sipxStatusService;
    }

    public void testGetSetLogLevel() {
        List<LoggingEntity> entities = new ArrayList<LoggingEntity>();
        LoggingManager manager = EasyMock.createMock(LoggingManager.class);
        manager.getEntitiesToProcess();
        EasyMock.expectLastCall().andReturn(entities).anyTimes();
        EasyMock.replay(manager);
        m_statusService.setLoggingManager(manager);
        String level = m_statusService.getLogLevel();
        assertEquals("NOTICE", level);
        level = "DEBUG";
        m_statusService.setLogLevel(level);
        level = m_statusService.getLogLevel();
        assertEquals("DEBUG", level);
    }
}

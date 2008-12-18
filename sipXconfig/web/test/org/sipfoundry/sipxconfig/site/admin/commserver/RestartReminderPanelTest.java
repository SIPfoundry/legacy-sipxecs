/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.Arrays;
import java.util.List;

import junit.framework.TestCase;

import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.test.Creator;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.service.SipxPresenceService;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class RestartReminderPanelTest extends TestCase {
    private SipxProxyService m_sipxProxyService;
    private SipxPresenceService m_sipxPresenceService;

    private final Creator m_pageMaker = new Creator();
    private RestartReminderPanel m_restartReminder;
    private Object[] m_services;

    @Override
    protected void setUp() throws Exception {
        m_sipxPresenceService = new SipxPresenceService();
        m_sipxPresenceService.setProcessName("PresenceServer");
        m_sipxProxyService = new SipxProxyService();
        m_sipxProxyService.setProcessName("SIPXProxy");
        m_services = new Object[] {
            m_sipxPresenceService, m_sipxProxyService
        };
        m_restartReminder = (RestartReminderPanel) m_pageMaker.newInstance(RestartReminderPanel.class);
    }

    public void testGetProcessesToRestartLater() {
        m_restartReminder.setRestartLater(true);

        m_restartReminder.setProcesses(null);
        assertNull(m_restartReminder.getProcessesToRestart());

        m_restartReminder.setProcesses(m_services);
        assertNull(m_restartReminder.getProcessesToRestart());
    }

    public void testGetProcessesToRestartNow() {
        SipxServiceManager serviceManager = createMock(SipxServiceManager.class);
        serviceManager.getRestartable();
        expectLastCall().andReturn(Arrays.asList(m_services));

        replay(serviceManager);

        m_restartReminder.setRestartLater(false);
        PropertyUtils.write(m_restartReminder, "sipxServiceManager", serviceManager);

        m_restartReminder.setProcesses(null);
        List processesToRestart = m_restartReminder.getProcessesToRestart();
        assertEquals(m_services.length, processesToRestart.size());

        m_restartReminder.setProcesses(m_services);
        processesToRestart = m_restartReminder.getProcessesToRestart();
        assertEquals(m_services.length, processesToRestart.size());
        for (int i = 0; i < m_services.length; i++) {
            assertEquals(m_services[i], processesToRestart.get(i));
        }

        verify(serviceManager);
    }

    public void testRestartLater() throws Exception {
        SipxProcessContext context = createMock(SipxProcessContext.class);
        replay(context);

        m_restartReminder.setRestartLater(true);
        PropertyUtils.write(m_restartReminder, "sipxProcessContext", context);

        m_restartReminder.restart();

        verify(context);
    }

    public void testRestartNow() throws Exception {
        SipxService sipxService = new SipxService() {
            @Override
            public String getBeanId() {
                return "DummyService";
            }
        };

        List l = Arrays.asList(sipxService);

        SipxServiceManager serviceManager = createMock(SipxServiceManager.class);
        serviceManager.getRestartable();
        expectLastCall().andReturn(l);

        SipxProcessContext context = createMock(SipxProcessContext.class);
        context.manageServices(l, Command.RESTART);

        replay(context, serviceManager);

        m_restartReminder.setRestartLater(false);
        PropertyUtils.write(m_restartReminder, "sipxProcessContext", context);
        PropertyUtils.write(m_restartReminder, "sipxServiceManager", serviceManager);

        m_restartReminder.restart();

        verify(context, serviceManager);
    }
}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.acd;

import java.util.ArrayList;
import java.util.Collection;

import junit.framework.TestCase;

import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.acd.AcdContext;

public class AcdAgentsPanelTest extends TestCase {
    private Creator m_pageMaker = new Creator();
    private AcdAgentsPanel m_panel;

    protected void setUp() throws Exception {
        m_panel = (AcdAgentsPanel) m_pageMaker.newInstance(AcdAgentsPanel.class);
    }

    public void testMoveUp() {
        Collection ids = new ArrayList();
        ids.add(new Integer(2));
        ids.add(new Integer(5));

        Integer queueId = new Integer(10);

        IMocksControl control = EasyMock.createControl();
        AcdContext context = control.createMock(AcdContext.class);
        context.moveAgentsInQueue(queueId, ids, -1);
        control.replay();

        IMocksControl mc = EasyMock.createControl();
        IRequestCycle rc = mc.createMock(IRequestCycle.class);
        mc.replay();

        PropertyUtils.write(m_panel, "acdContext", context);
        m_panel.setAcdQueueId(queueId);
        m_panel.setRowsToMoveUp(ids);

        assertTrue(m_panel.onFormSubmit(rc));

        control.verify();
        mc.verify();
    }

    public void testDownUp() {
        Collection ids = new ArrayList();
        ids.add(new Integer(2));
        ids.add(new Integer(5));

        Integer queueId = new Integer(10);

        IMocksControl control = EasyMock.createControl();
        AcdContext context = control.createMock(AcdContext.class);
        context.moveAgentsInQueue(queueId, ids, 1);
        control.replay();

        IMocksControl mc = EasyMock.createControl();
        IRequestCycle rc = mc.createMock(IRequestCycle.class);
        mc.replay();

        PropertyUtils.write(m_panel, "acdContext", context);
        m_panel.setAcdQueueId(queueId);
        m_panel.setRowsToMoveDown(ids);

        assertTrue(m_panel.onFormSubmit(rc));

        control.verify();
        mc.verify();
    }

    public void testDelete() {
        Collection ids = new ArrayList();
        ids.add(new Integer(2));
        ids.add(new Integer(5));

        Integer queueId = new Integer(10);

        IMocksControl control = EasyMock.createControl();
        AcdContext context = control.createMock(AcdContext.class);
        context.removeAgents(queueId, ids);
        control.replay();

        IMocksControl mc = EasyMock.createControl();
        IRequestCycle rc = mc.createMock(IRequestCycle.class);
        mc.replay();

        PropertyUtils.write(m_panel, "acdContext", context);
        m_panel.setAcdQueueId(queueId);
        m_panel.setRowsToDelete(ids);

        assertTrue(m_panel.onFormSubmit(rc));

        control.verify();
        mc.verify();
    }

    public void testNoAction() {
        IMocksControl mc = EasyMock.createControl();
        IRequestCycle rc = mc.createMock(IRequestCycle.class);
        mc.replay();

        assertFalse(m_panel.onFormSubmit(rc));

        mc.verify();
    }
}

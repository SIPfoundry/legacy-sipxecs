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

import junit.framework.TestCase;

import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdLine;
import org.sipfoundry.sipxconfig.acd.AcdQueue;
import org.sipfoundry.sipxconfig.acd.AcdServer;

public class EditAcdLineTest extends TestCase {
    private Creator m_pageMaker = new Creator();
    private EditAcdLine m_page;

    protected void setUp() throws Exception {
        m_page = (EditAcdLine) m_pageMaker.newInstance(EditAcdLine.class);
    }

    public void testPageBeginRender() {
        AcdServer server = new AcdServer();
        server.setUniqueId();

        AcdLine line = new AcdLine();
        line.setUniqueId();

        server.insertLine(line);

        AcdQueue queue = new AcdQueue();
        queue.setUniqueId();

        line.associateQueue(queue);

        IMocksControl control = EasyMock.createControl();
        AcdContext context = control.createMock(AcdContext.class);
        context.loadLine(line.getId());
        control.andReturn(line);
        context.loadServer(server.getId());
        control.andReturn(server);
        control.replay();

        m_page.setAcdLineId(line.getId());
        PropertyUtils.write(m_page, "acdContext", context);

        m_page.pageBeginRender(null);

        assertEquals(queue.getId(), m_page.getAcdQueueId());
        assertEquals(line, m_page.getAcdLine());

        control.verify();
    }

    public void testPageBeginRenderEmpty() {
        AcdServer server = new AcdServer();
        server.setUniqueId();

        AcdLine line = new AcdLine();
        line.setUniqueId();

        IMocksControl control = EasyMock.createControl();
        AcdContext context = control.createMock(AcdContext.class);
        context.newLine();
        control.andReturn(line);
        context.loadServer(server.getId());
        control.andReturn(server);
        control.replay();

        PropertyUtils.write(m_page, "acdContext", context);
        m_page.setAcdServerId(server.getId());
        m_page.pageBeginRender(null);

        assertNull(m_page.getAcdQueueId());
        assertEquals(line, m_page.getAcdLine());

        control.verify();
    }

    public void testSaveValid() {
        AcdLine line = new AcdLine();
        line.setUniqueId();

        AcdQueue queue = new AcdQueue();
        queue.setUniqueId();

        line.associateQueue(queue);

        IMocksControl control = EasyMock.createControl();
        AcdContext context = control.createMock(AcdContext.class);
        context.store(line);
        context.associate(line.getId(), queue.getId());
        control.replay();

        PropertyUtils.write(m_page, "acdContext", context);
        m_page.setAcdLine(line);
        m_page.setAcdQueueId(queue.getId());
        m_page.saveValid();

        control.verify();
    }

    public void testSaveNew() {
        AcdLine line = new AcdLine();

        AcdQueue queue = new AcdQueue();
        queue.setUniqueId();

        line.associateQueue(queue);

        AcdServer server = new AcdServer();
        server.setUniqueId();

        IMocksControl control = EasyMock.createControl();
        AcdContext context = control.createMock(AcdContext.class);
        context.loadServer(server.getId());
        control.andReturn(server);
        context.store(line);
        context.associate(line.getId(), queue.getId());
        control.replay();

        PropertyUtils.write(m_page, "acdContext", context);
        m_page.setAcdLine(line);
        m_page.setAcdServerId(server.getId());
        m_page.setAcdQueueId(queue.getId());
        m_page.saveValid();

        assertEquals(1, server.getLines().size());
        assertEquals(line, server.getLines().toArray()[0]);

        control.verify();
    }
}

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

import java.util.Collections;
import java.util.List;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public class AcdQueueTest extends BeanWithSettingsTestCase {
    private AcdQueue m_queue;

    protected void setUp() throws Exception {
        super.setUp();
        m_queue = new AcdQueue();
        initializeBeanWithSettings(m_queue);
    }

    public void testGetSettingsModel() throws Exception {
        assertNotNull(m_queue.getSettings());
    }

    public void testAudioFiles() throws Exception {
        List audioFiles = m_queue.getAudioFiles();
        assertEquals(0, audioFiles.size());

        m_queue.setSettingValue(AcdQueue.WELCOME_AUDIO, "a.wav");
        m_queue.setSettingValue(AcdQueue.BACKGROUND_AUDIO, "b.wav");
        m_queue.setSettingValue(AcdQueue.CALL_TERMINATION_AUDIO, "c.wav");

        audioFiles = m_queue.getAudioFiles();
        assertEquals(3, audioFiles.size());
        assertTrue(audioFiles.contains("a.wav"));
        assertTrue(audioFiles.contains("b.wav"));
        assertTrue(audioFiles.contains("c.wav"));

        m_queue.setSettingValue(AcdQueue.QUEUE_AUDIO, "d.wav");
        audioFiles = m_queue.getAudioFiles();
        assertEquals(4, audioFiles.size());
        assertTrue(audioFiles.contains("d.wav"));
    }

    public void testPrepareSettings() {
        IMocksControl mc = EasyMock.createControl();
        CoreContext coreContext = mc.createMock(CoreContext.class);
        mc.replay();

        AcdServer server = new AcdServer();
        Location location = new Location();
        location.setFqdn("host.mydomain.org");
        server.setLocation(location);

        m_queue.setAcdServer(server);

        m_queue.setCoreContext(coreContext);
        m_queue.setName("testqueue");
        m_queue.setDescription("queue description");

        m_queue.initialize();
        assertEquals("queue description", m_queue.getSettingValue(AcdQueue.QUEUE_NAME));
        assertEquals("sip:testqueue@host.mydomain.org", m_queue.getSettingValue(AcdQueue.URI));

        assertEquals("", m_queue.getSettingValue(AcdQueue.AGENT_LIST));
        assertEquals("", m_queue.getSettingValue(AcdQueue.OVERFLOW_QUEUE));

        mc.verify();
    }

    public void testPrepareSettingsWithOverflowQueue() {
        IMocksControl mc = EasyMock.createControl();
        CoreContext coreContext = mc.createMock(CoreContext.class);
        mc.replay();

        AcdServer server = new AcdServer();
        Location location = new Location();
        location.setFqdn("host.mydomain.org");
        server.setLocation(location);

        m_queue.setAcdServer(server);
        m_queue.setCoreContext(coreContext);
        m_queue.setName("testqueue");
        m_queue.setDescription("queue description");

        AcdQueue overflowQueue = new AcdQueue();
        initializeBeanWithSettings(overflowQueue);
        overflowQueue.setAcdServer(server);
        overflowQueue.setCoreContext(coreContext);
        overflowQueue.setName("overflow");
        m_queue.setOverflowQueue(overflowQueue);

        m_queue.initialize();
        assertEquals("queue description", m_queue.getSettingValue(AcdQueue.QUEUE_NAME));
        assertEquals("sip:testqueue@host.mydomain.org", m_queue.getSettingValue(AcdQueue.URI));
        assertEquals("sip:overflow@host.mydomain.org", m_queue
                .getSettingValue(AcdQueue.OVERFLOW_QUEUE));

        assertEquals("", m_queue.getSettingValue(AcdQueue.AGENT_LIST));

        mc.verify();
    }

    public void testCleanLines() {
        AcdLine[] lines = new AcdLine[4];

        for (int i = 0; i < lines.length; i++) {
            lines[i] = new AcdLine();
            lines[i].setUniqueId();
            assertNull(lines[i].getAcdQueue());
        }

        AcdQueue q = new AcdQueue();

        for (int i = 0; i < lines.length; i++) {
            lines[i].associateQueue(q);
        }

        assertEquals(lines.length, q.getLines().size());

        for (int i = 0; i < lines.length; i++) {
            assertSame(q, lines[i].getAcdQueue());
        }

        q.cleanLines();
        assertEquals(0, q.getLines().size());
        for (int i = 0; i < lines.length; i++) {
            assertNull(lines[i].getAcdQueue());
        }
    }

    public void testCleanAgents() {
        AcdAgent[] agents = new AcdAgent[4];

        for (int i = 0; i < agents.length; i++) {
            agents[i] = new AcdAgent();
            agents[i].setUniqueId();
            assertTrue(agents[i].getQueues().isEmpty());
        }

        AcdQueue q = new AcdQueue();

        for (int i = 0; i < agents.length; i++) {
            q.insertAgent(agents[i]);
        }

        assertEquals(agents.length, q.getAgents().size());

        for (int i = 0; i < agents.length; i++) {
            assertSame(q, agents[i].getQueues().get(0));
        }

        q.cleanAgents();
        assertEquals(0, q.getLines().size());
        for (int i = 0; i < agents.length; i++) {
            assertTrue(agents[i].getQueues().isEmpty());
        }
    }

    public void testPrepareSettingsWithAgents() {
        IMocksControl mc = EasyMock.createControl();
        CoreContext coreContext = mc.createMock(CoreContext.class);
        coreContext.getDomainName();
        mc.andReturn("mydomain.org").atLeastOnce();
        mc.replay();

        User user = new User();
        user.setUniqueId();
        user.setUserName("testuser1");

        AcdAgent agent1 = new AcdAgent();
        agent1.setUniqueId();
        agent1.setUser(user);
        agent1.setCoreContext(coreContext);
        m_queue.insertAgent(agent1);

        User user2 = new User();
        user2.setUniqueId();
        user2.setUserName("testuser2");

        AcdAgent agent2 = new AcdAgent();
        agent2.setUniqueId();
        agent2.setUser(user2);
        agent2.setCoreContext(coreContext);

        AcdServer server = new AcdServer();
        Location location = new Location();
        location.setFqdn("host.mydomain.org");
        server.setLocation(location);

        m_queue.setAcdServer(server);
        m_queue.insertAgent(agent2);

        m_queue.setCoreContext(coreContext);
        m_queue.setName("testqueue");
        m_queue.setDescription("queue description");

        m_queue.initialize();
        assertEquals("queue description", m_queue.getSettingValue(AcdQueue.QUEUE_NAME));
        assertEquals("sip:testqueue@host.mydomain.org", m_queue.getSettingValue(AcdQueue.URI));

        assertEquals("sip:testuser1@mydomain.org,sip:testuser2@mydomain.org", m_queue
                .getSettingValue(AcdQueue.AGENT_LIST));

        mc.verify();
    }

    public void testMoveAgents() {
        User user = new User();
        user.setUniqueId();
        user.setUserName("testuser1");

        AcdAgent agent1 = new AcdAgent();
        agent1.setUniqueId();
        agent1.setUser(user);
        m_queue.insertAgent(agent1);

        User user2 = new User();
        user2.setUniqueId();
        user2.setUserName("testuser2");

        AcdAgent agent2 = new AcdAgent();
        agent2.setUniqueId();
        agent2.setUser(user2);
        m_queue.insertAgent(agent2);

        assertEquals(agent1, m_queue.getAgents().get(0));
        m_queue.moveAgents(Collections.singletonList(agent2.getId()), -1);
        assertEquals(agent2, m_queue.getAgents().get(0));
        m_queue.moveAgents(Collections.singletonList(agent2.getId()), 1);
        assertEquals(agent1, m_queue.getAgents().get(0));
    }
}

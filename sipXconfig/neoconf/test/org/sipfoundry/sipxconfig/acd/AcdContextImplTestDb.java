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

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.dbunit.Assertion;
import org.dbunit.database.IDatabaseConnection;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.service.SipxPresenceService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.setting.Setting;


public class AcdContextImplTestDb extends SipxDatabaseTestCase {
    private final static Integer SERVER_ID = new Integer(1001);

    private AcdContext m_context;
    private CoreContext m_coreContext;
    private LocationsManager m_locationsManager;

    @Override
    protected void setUp() throws Exception {
        m_context = (AcdContext) TestHelper.getApplicationContext().getBean(
                AcdContext.CONTEXT_BEAN_NAME);
        m_coreContext = (CoreContext) TestHelper.getApplicationContext().getBean(
                CoreContext.CONTEXT_BEAN_NAME);
        m_locationsManager = (LocationsManager) TestHelper.getApplicationContext().getBean(
                LocationsManager.CONTEXT_BEAN_NAME);

        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testGetUsersWithAgents() throws Exception {
        TestHelper.insertFlat("acd/same_user_on_two_servers.db.xml");
        User[] users = (User[]) m_context.getUsersWithAgents().toArray(new User[0]);
        assertEquals(2, users.length);
        assertEquals("testuser1", users[0].getUserName());
        assertEquals("testuser2", users[1].getUserName());
    }

    public void testNewLine() {
        AcdLine line = m_context.newLine();
        line.setName("kuku");
        assertNotNull(line.getSettings());
        AcdServer acdServer = m_context.newServer();
        Location location = new Location();
        location.setFqdn("localhost");
        location.setAddress("127.0.0.1");
        location.setName("localhost");
        m_locationsManager.storeLocation(location);
        acdServer.setLocation(location);
        assertNotNull(acdServer.getLines());
        assertEquals(0, acdServer.getLines().size());
        acdServer.insertLine(line);
        m_context.store(acdServer);
        assertEquals(1, acdServer.getLines().size());
        assertSame(acdServer, line.getAcdServer());
    }

    public void testLoadLine() throws Exception {
        Integer id = new Integer(2001);
        TestHelper.insertFlat("acd/lines.db.xml");
        AcdLine line = m_context.loadLine(id);
        assertEquals(id, line.getId());
    }

    public void testLoadQueue() throws Exception {
        Integer id = new Integer(2001);
        TestHelper.insertFlat("acd/queues.db.xml");
        AcdQueue queue = m_context.loadQueue(id);
        assertEquals(id, queue.getId());
    }

    public void testLoadQueueWithAgents() throws Exception {
        TestHelper.insertFlat("acd/agents.db.xml");
        AcdQueue queue = m_context.loadQueue(new Integer(2002));
        assertEquals(3, queue.getAgents().size());
        queue = m_context.loadQueue(new Integer(2001));
        assertEquals(1, queue.getAgents().size());
    }

    public void testLoadServer() throws Exception {
        TestHelper.insertFlat("acd/agents.db.xml");
        AcdServer server = m_context.loadServer(SERVER_ID);
        assertEquals(2, server.getQueues().size());
        assertEquals(0, server.getLines().size());
    }

    public void testRemoveServers() throws Exception {
        TestHelper.insertFlat("acd/agents.db.xml");

        m_context.removeServers(Collections.singletonList(SERVER_ID));
        IDataSet dataSet = TestHelper.getConnection().createDataSet();
        assertEquals(0, dataSet.getTable("acd_server").getRowCount());
        assertEquals(0, dataSet.getTable("acd_queue").getRowCount());
        assertEquals(0, dataSet.getTable("acd_line").getRowCount());
    }

    public void testNewQueue() {
        AcdQueue queue = m_context.newQueue();
        queue.setName("kuku");
        assertNotNull(queue.getSettings());
        AcdServer acdServer = m_context.newServer();
        Location location = new Location();
        location.setFqdn("localhost");
        location.setAddress("127.0.0.1");
        location.setName("localhost");
        m_locationsManager.storeLocation(location);
        acdServer.setLocation(location);
        assertNotNull(acdServer.getQueues());
        assertEquals(0, acdServer.getQueues().size());
        acdServer.insertQueue(queue);
        m_context.store(acdServer);
        assertEquals(1, acdServer.getQueues().size());
        assertSame(acdServer, queue.getAcdServer());
    }

    public void testStoreServer() throws Exception {
        AcdServer acdServer = m_context.newServer();
        Location location = new Location();
        location.setFqdn("localhost");
        location.setAddress("127.0.0.1");
        location.setName("localhost");
        m_locationsManager.storeLocation(location);
        acdServer.setLocation(location);

        SipxPresenceService presenceService = org.easymock.classextension.EasyMock.createMock(SipxPresenceService.class);
        presenceService.getPresenceServerPort();
        EasyMock.expectLastCall().andReturn(5130).atLeastOnce();
        presenceService.getPresenceApiPort();
        EasyMock.expectLastCall().andReturn(8111).atLeastOnce();
        org.easymock.classextension.EasyMock.replay(presenceService);

        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxPresenceService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(presenceService).atLeastOnce();
        EasyMock.replay(sipxServiceManager);

        acdServer.setSipxServiceManager(sipxServiceManager);

        assertNotNull(acdServer);
        Setting settingRoot = acdServer.getSettings();
        Setting setting = settingRoot.getSetting("acd-server/log-level");
        assertNotNull(setting);
        setting.setValue("INFO");
        m_context.store(acdServer);
        ITable acdServerTable = TestHelper.getConnection().createDataSet().getTable("acd_server");

        assertEquals(1, acdServerTable.getRowCount());
        assertNotNull(acdServerTable.getValue(0, "value_storage_id"));

        ITable valuesTable = TestHelper.getConnection().createDataSet().getTable("setting_value");
        assertEquals(1, valuesTable.getRowCount());
        assertEquals("INFO", valuesTable.getValue(0, "value"));
    }

    public void testStoreLine() throws Exception {
        TestHelper.insertFlat("acd/lines.db.xml");
        TestHelper.insertFlat("acd/lines_and_conferences.db.xml");

        AcdServer acdServer = m_context.loadServer(new Integer(1001));

        AcdLine line = m_context.newLine();
        line.setAcdServer(acdServer);
        line.setName("l1");
        try {
            m_context.store(line);
            fail("Should fail");
        } catch (UserException e) {
            // ok
        }

        line.setName("l4");
        line.setExtension("102");
        try {
            m_context.store(line);
            fail("Should fail");
        } catch (UserException e) {
            // ok
        }

        line.setName("l5");
        line.setExtension("7777");
        try {
            m_context.store(line);
            fail("Should fail. Extension used by a conference");
        } catch (UserException e) {
            // ok
        }

        line.setExtension("112");
        m_context.store(line);

        // check if I can store the same line more than once
        m_context.store(line);
    }

    public void testStoreQueue() throws Exception {
        TestHelper.insertFlat("acd/queues.db.xml");

        AcdServer acdServer = m_context.loadServer(new Integer(1001));

        AcdQueue queue = m_context.newQueue();
        queue.setAcdServer(acdServer);
        queue.setName("q1");
        try {
            m_context.store(queue);
            fail("Should fail");
        } catch (UserException e) {
            // ok
        }

        queue.setName("q4");
        m_context.store(queue);
    }

    public void testRemoveLines() throws Exception {
        TestHelper.insertFlat("acd/lines.db.xml");
        Set lines = m_context.loadServer(SERVER_ID).getLines();
        assertEquals(2, lines.size());

        m_context.removeLines(Collections.singletonList(new Integer(2002)));
        lines = m_context.loadServer(SERVER_ID).getLines();
        assertEquals(1, lines.size());
        for (Iterator i = lines.iterator(); i.hasNext();) {
            AcdLine line = (AcdLine) i.next();
            assertEquals(new Integer(2001), line.getId());
        }
    }

    public void testRemoveLinesWithQueues() throws Exception {
        TestHelper.insertFlat("acd/assoc.db.xml");
        Set lines = m_context.loadServer(SERVER_ID).getLines();
        assertEquals(2, lines.size());

        m_context.removeLines(Collections.singletonList(new Integer(2001)));
        lines = m_context.loadServer(SERVER_ID).getLines();
        assertEquals(1, lines.size());
        for (Iterator i = lines.iterator(); i.hasNext();) {
            AcdLine line = (AcdLine) i.next();
            assertEquals(new Integer(2002), line.getId());
        }
    }

    public void testRemoveQueues() throws Exception {
        TestHelper.insertFlat("acd/queues.db.xml");

        ITable acdQueueTable = TestHelper.getConnection().createDataSet().getTable("acd_queue");

        assertEquals(2, acdQueueTable.getRowCount());

        Set queues = m_context.loadServer(SERVER_ID).getQueues();
        Set lines = m_context.loadServer(SERVER_ID).getLines();
        assertEquals(0, lines.size());
        assertEquals(2, queues.size());

        m_context.removeQueues(Collections.singletonList(new Integer(2002)));
        queues = m_context.loadServer(SERVER_ID).getQueues();
        assertEquals(1, queues.size());
        for (Iterator i = queues.iterator(); i.hasNext();) {
            AcdQueue line = (AcdQueue) i.next();
            assertEquals(new Integer(2001), line.getId());
        }
    }

    public void testRemoveOveflowQueue() throws Exception {
        TestHelper.insertFlat("acd/overflow_queues.db.xml");

        IDatabaseConnection dbConnection = TestHelper.getConnection();
        IDataSet dataSet = dbConnection.createDataSet();
        ITable acdQueueTable = dataSet.getTable("acd_queue");

        assertEquals(3, acdQueueTable.getRowCount());
        assertEquals(2, dbConnection.getRowCount("acd_queue", "where overflow_queue_id = 2003"));

        Set queues = m_context.loadServer(SERVER_ID).getQueues();
        assertEquals(3, queues.size());

        AcdQueue overflowQueue = m_context.loadQueue(new Integer(2003));
        AcdQueue queue = m_context.loadQueue(new Integer(2002));

        assertEquals(queue.getOverflowQueue(), overflowQueue);

        m_context.removeQueues(Collections.singletonList(overflowQueue.getId()));

        acdQueueTable = dataSet.getTable("acd_queue");
        assertEquals(2, acdQueueTable.getRowCount());

        queue = m_context.loadQueue(new Integer(2002));
        assertNull(queue.getOverflowQueue());
        assertEquals(0, dbConnection.getRowCount("acd_queue", "where overflow_queue_id = 2003"));
    }

    public void testRemoveQueuesWithLines() throws Exception {
        TestHelper.insertFlat("acd/assoc.db.xml");

        ITable acdQueueTable = TestHelper.getConnection().createDataSet().getTable("acd_queue");

        assertEquals(2, acdQueueTable.getRowCount());

        Set queues = m_context.loadServer(SERVER_ID).getQueues();
        Set lines = m_context.loadServer(SERVER_ID).getLines();
        assertEquals(2, lines.size());
        assertEquals(2, queues.size());

        m_context.removeQueues(Collections.singletonList(new Integer(3001)));
        queues = m_context.loadServer(SERVER_ID).getQueues();
        assertEquals(1, queues.size());
        for (Iterator i = queues.iterator(); i.hasNext();) {
            AcdQueue line = (AcdQueue) i.next();
            assertEquals(new Integer(3002), line.getId());
        }
    }

    public void testRemoveQueuesWithAgents() throws Exception {
        TestHelper.insertFlat("acd/agents.db.xml");

        ITable acdQueueTable = TestHelper.getConnection().createDataSet().getTable("acd_queue");
        assertEquals(2, acdQueueTable.getRowCount());
        ITable acdAgentTable = TestHelper.getConnection().createDataSet().getTable("acd_agent");
        assertEquals(4, acdAgentTable.getRowCount());

        Set queues = m_context.loadServer(SERVER_ID).getQueues();
        assertEquals(2, queues.size());

        m_context.removeQueues(Collections.singletonList(new Integer(2002)));
        queues = m_context.loadServer(SERVER_ID).getQueues();
        assertEquals(1, queues.size());
        for (Iterator i = queues.iterator(); i.hasNext();) {
            AcdQueue line = (AcdQueue) i.next();
            assertEquals(new Integer(2001), line.getId());
        }
        acdQueueTable = TestHelper.getConnection().createDataSet().getTable("acd_queue");
        assertEquals(1, acdQueueTable.getRowCount());

        // agents that were only on 2002 should be removed as well
        acdAgentTable = TestHelper.getConnection().createDataSet().getTable("acd_agent");
        assertEquals(2, acdAgentTable.getRowCount());
    }

    public void testRemoveQueuesWithSharedAgents() throws Exception {
        TestHelper.insertFlat("acd/agents.db.xml");

        ITable acdQueueTable = TestHelper.getConnection().createDataSet().getTable("acd_queue");

        assertEquals(2, acdQueueTable.getRowCount());

        Set queues = m_context.loadServer(SERVER_ID).getQueues();
        assertEquals(2, queues.size());

        List queuesToBeDeleted = new ArrayList();
        queuesToBeDeleted.add(new Integer(2001));
        queuesToBeDeleted.add(new Integer(2002));

        m_context.removeQueues(queuesToBeDeleted);
        queues = m_context.loadServer(SERVER_ID).getQueues();
        assertEquals(0, queues.size());
        acdQueueTable = TestHelper.getConnection().createDataSet().getTable("acd_queue");
        assertEquals(0, acdQueueTable.getRowCount());
    }

    public void testOnDeleteUser() throws Exception {
        TestHelper.insertFlat("acd/agents.db.xml");

        ITable acdAgentTable = TestHelper.getConnection().createDataSet().getTable("acd_agent");
        assertEquals(4, acdAgentTable.getRowCount());

        m_coreContext.deleteUsers(Collections.singleton(new Integer(1000)));

        acdAgentTable = TestHelper.getConnection().createDataSet().getTable("acd_agent");
        assertEquals(0, acdAgentTable.getRowCount());
    }

    public void testAddUsersToQueue() throws Exception {
        User testUser1 = new User();
        testUser1.setUserName("testUser1");
        m_coreContext.saveUser(testUser1);

        User testUser2 = new User();
        testUser2.setUserName("testUser2");
        m_coreContext.saveUser(testUser2);

        User testUser3 = new User();
        testUser3.setUserName("testUser3");
        m_coreContext.saveUser(testUser3);

        List agentList = new ArrayList();

        agentList.add(testUser1.getId());
        agentList.add(testUser2.getId());
        agentList.add(testUser3.getId());

        TestHelper.insertFlat("acd/queues.db.xml");
        final Serializable queueId = new Integer(2001);

        m_context.addUsersToQueue(queueId, Collections.unmodifiableCollection(agentList));

        ITable agentTable = TestHelper.getConnection().createDataSet().getTable("acd_agent");
        assertEquals(3, agentTable.getRowCount());
        assertEquals(testUser1.getId(), agentTable.getValue(0, "user_id"));
        assertEquals(SERVER_ID, agentTable.getValue(0, "acd_server_id"));

        ITable queues2agentsTable = TestHelper.getConnection().createDataSet().getTable(
                "acd_queue_agent");
        assertEquals(3, queues2agentsTable.getRowCount());
        assertEquals(queueId, queues2agentsTable.getValue(0, "acd_queue_id"));

        // caling this again should not change anything
        m_context.addUsersToQueue(queueId, Collections.singletonList(testUser1.getId()));

        agentTable = TestHelper.getConnection().createDataSet().getTable("acd_agent");
        assertEquals(3, agentTable.getRowCount());
        assertEquals(testUser1.getId(), agentTable.getValue(0, "user_id"));

        queues2agentsTable = TestHelper.getConnection().createDataSet().getTable(
                "acd_queue_agent");
        assertEquals(3, queues2agentsTable.getRowCount());
        assertEquals(queueId, queues2agentsTable.getValue(0, "acd_queue_id"));

    }

    public void testRemoveAgent() throws Exception {
        TestHelper.insertFlat("acd/agents.db.xml");

        List removeList = new ArrayList();
        removeList.add(new Integer(3001));
        removeList.add(new Integer(3003));

        final Serializable queueId = new Integer(2002);

        m_context.removeAgents(queueId, removeList);
        List agents = m_context.loadQueue(queueId).getAgents();
        assertEquals(1, agents.size());

        ITable agentTable = TestHelper.getConnection().createDataSet().getTable("acd_agent");
        // one agent remove, another one still belongs to the second queue
        assertEquals(3, agentTable.getRowCount());
    }

    public void testLoadAssociate() throws Exception {
        TestHelper.insertFlat("acd/assoc.db.xml");

        AcdLine line = m_context.loadLine(new Integer(2001));
        assertEquals(new Integer(3001), line.getAcdQueue().getId());
    }

    public void testAssociate() throws Exception {
        TestHelper.insertFlat("acd/assoc.db.xml");

        m_context.associate(new Integer(2002), new Integer(3002));

        ITable assocTable = TestHelper.getConnection().createDataSet().getTable("acd_line_queue");
        // 2 since there is one more in another queue
        assertEquals(2, assocTable.getRowCount());
        for (int i = 0; i < 2; i++) {
            Integer lineId = (Integer) assocTable.getValue(i, "acd_line_id");
            Integer queueId = (Integer) assocTable.getValue(i, "acd_queue_id");
            assertEquals(lineId.intValue() + 1000, queueId.intValue());
        }
    }

    public void testUnAssociate() throws Exception {
        TestHelper.insertFlat("acd/assoc.db.xml");

        m_context.associate(new Integer(2001), null);
        ITable assocTable = TestHelper.getConnection().createDataSet().getTable("acd_line_queue");
        // empty - we removed the only existing association
        assertEquals(0, assocTable.getRowCount());
    }

    public void testMoveQueues() throws Exception {
        TestHelper.insertFlat("acd/agents.db.xml");

        Integer agentId = new Integer(3001);
        Collection queuesIds = Collections.singletonList(new Integer(2002));

        m_context.moveQueuesInAgent(agentId, queuesIds, -1);

        ITable actual = TestHelper.getConnection().createDataSet().getTable("acd_agent_queue");

        IDataSet expectedDs = TestHelper.loadDataSetFlat("acd/moved_queues.db.xml");
        ITable expected = expectedDs.getTable("acd_agent_queue");

        Assertion.assertEquals(expected, actual);
    }

    public void testMoveAgents() throws Exception {
        TestHelper.insertFlat("acd/agents.db.xml");

        Integer queueId = new Integer(2002);
        Collection agentsIds = Collections.singletonList(new Integer(3001));

        m_context.moveAgentsInQueue(queueId, agentsIds, 2);

        ITable actual = TestHelper.getConnection().createDataSet().getTable("acd_queue_agent");

        IDataSet expectedDs = TestHelper.loadDataSetFlat("acd/moved_agents.db.xml");
        ITable expected = expectedDs.getTable("acd_queue_agent");

        Assertion.assertEquals(expected, actual);
    }

    public void testClear() throws Exception {
        TestHelper.insertFlat("acd/agents.db.xml");
        m_context.clear();

        IDataSet dataSet = TestHelper.getConnection().createDataSet();
        assertEquals(0, dataSet.getTable("acd_server").getRowCount());
        assertEquals(0, dataSet.getTable("acd_queue").getRowCount());
        assertEquals(0, dataSet.getTable("acd_line").getRowCount());
        assertEquals(0, dataSet.getTable("acd_agent_queue").getRowCount());
        assertEquals(0, dataSet.getTable("acd_queue_agent").getRowCount());
        assertEquals(0, dataSet.getTable("acd_line_queue").getRowCount());
    }

    public void testGetQueuesForUsers() throws Exception {
        TestHelper.cleanInsertFlat("acd/queues_and_agents.db.xml");
        List<User> agents = new ArrayList();
        agents.add(m_coreContext.loadUser(1001));
        agents.add(m_coreContext.loadUser(1003));
        AcdServer server = m_context.loadServer(1001);

        Collection<AcdQueue> queues = m_context.getQueuesForUsers(server, agents);
        assertEquals(2, queues.size());
        Iterator<AcdQueue> iqueues = queues.iterator();
        assertEquals("q1", iqueues.next().getName());
        assertEquals("q2", iqueues.next().getName());
    }

    public void testGetQueuesForEmptyUsers() throws Exception {
        // just need 1 acd server
        TestHelper.cleanInsertFlat("acd/queues.db.xml");
        AcdServer server = m_context.loadServer(1001);
        Collection empty = m_context.getQueuesForUsers(server, new ArrayList());
        assertTrue(empty.isEmpty());
    }
}

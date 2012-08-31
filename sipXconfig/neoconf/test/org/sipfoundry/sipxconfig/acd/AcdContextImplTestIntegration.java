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


import static org.junit.Assert.assertArrayEquals;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.dbunit.database.IDatabaseConnection;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.ResultDataGrid;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class AcdContextImplTestIntegration extends IntegrationTestCase {
    private final static Integer SERVER_ID = new Integer(1001);
    private AcdContext m_acdContext;
    private CoreContext m_coreContext;
    private LocationsManager m_locationsManager;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        //disableDaoEventPublishing();
        clear();
        sql("domain/DomainSeed.sql");
        sql("acd/location.sql");
    }
    
    public void testGetUsersWithAgents() throws Exception {
        TestHelper.insertFlat("acd/same_user_on_two_servers.db.xml");
        User[] users = (User[]) m_acdContext.getUsersWithAgents().toArray(new User[0]);
        assertEquals(2, users.length);
        assertEquals("testuser1", users[0].getUserName());
        assertEquals("testuser2", users[1].getUserName());
    }

    public void testNewLine() {
        AcdLine line = m_acdContext.newLine();
        line.setName("kuku");
        assertNotNull(line.getSettings());
        AcdServer acdServer = m_acdContext.newServer();
        Location location = new Location();
        location.setFqdn("host.example.org");
        location.setAddress("127.0.0.1");
        location.setName("localhost");
        m_locationsManager.saveLocation(location);
        acdServer.setLocation(location);
        assertNotNull(acdServer.getLines());
        assertEquals(0, acdServer.getLines().size());
        acdServer.insertLine(line);
        m_acdContext.saveComponent(acdServer);
        assertEquals(1, acdServer.getLines().size());
        assertSame(acdServer, line.getAcdServer());
    }

    public void testLoadLine() throws Exception {
        Integer id = new Integer(2001);
        sql("acd/lines.sql");
        AcdLine line = m_acdContext.loadLine(id);
        assertEquals(id, line.getId());
    }

    public void testLoadQueue() throws Exception {
        Integer id = new Integer(2001);
        sql("acd/queues.sql");
        AcdQueue queue = m_acdContext.loadQueue(id);
        assertEquals(id, queue.getId());
    }

    public void testLoadQueueWithAgents() throws Exception {
        sql("acd/agents.sql");
        AcdQueue queue = m_acdContext.loadQueue(new Integer(2002));
        assertEquals(3, queue.getAgents().size());
        queue = m_acdContext.loadQueue(new Integer(2001));
        assertEquals(1, queue.getAgents().size());
    }

    public void testLoadServer() throws Exception {
        sql("acd/agents.sql");
        AcdServer server = m_acdContext.loadServer(SERVER_ID);
        assertEquals(2, server.getQueues().size());
        assertEquals(0, server.getLines().size());
    }

    public void testRemoveServers() throws Exception {
        sql("acd/agents.sql");
        AcdServer server = m_acdContext.loadServer(SERVER_ID);
        m_acdContext.removeServers(Collections.singletonList(server));   
        commit();
        assertEquals(0, countRowsInTable("acd_server"));
        assertEquals(0, countRowsInTable("acd_queue"));
        assertEquals(0, countRowsInTable("acd_line"));
    }

    public void testNewQueue() {
        AcdQueue queue = m_acdContext.newQueue();
        queue.setName("kuku");
        assertNotNull(queue.getSettings());
        AcdServer acdServer = m_acdContext.newServer();
        Location location = new Location();
        location.setFqdn("host.example.org");
        location.setAddress("127.0.0.1");
        location.setName("localhost");
        m_locationsManager.saveLocation(location);
        acdServer.setLocation(location);
        assertNotNull(acdServer.getQueues());
        assertEquals(0, acdServer.getQueues().size());
        acdServer.insertQueue(queue);
        m_acdContext.saveComponent(acdServer);
        assertEquals(1, acdServer.getQueues().size());
        assertSame(acdServer, queue.getAcdServer());
    }

    public void testsaveComponentServer() throws Exception {
        AcdServer acdServer = m_acdContext.newServer();
        Location location = new Location();
        location.setFqdn("host.example.org");
        location.setAddress("127.0.0.1");
        location.setName("localhost");
        m_locationsManager.saveLocation(location);
        acdServer.setLocation(location);

        assertNotNull(acdServer);
        Setting settingRoot = acdServer.getSettings();
        Setting setting = settingRoot.getSetting("acd-server/log-level");
        assertNotNull(setting);
        setting.setValue("INFO");
        m_acdContext.saveComponent(acdServer);
        commit();
        assertEquals(1, countRowsInTable("acd_server"));
        assertNotNull(db().queryForObject("select value_storage_id from acd_server", Integer.class));

        assertEquals(1, countRowsInTable("setting_value"));
        assertEquals("INFO", db().queryForObject("select value from setting_value", String.class));
    }

    public void testsaveComponentLine() throws Exception {
        sql("acd/lines.sql");
        sql("acd/lines_and_conferences.sql");

        AcdServer acdServer = m_acdContext.loadServer(new Integer(1001));

        AcdLine line = m_acdContext.newLine();
        line.setAcdServer(acdServer);
        line.setName("l1");
        try {
            m_acdContext.saveComponent(line);
            fail("Should fail");
        } catch (UserException e) {
            // ok
        }

        line.setName("l4");
        line.setExtension("102");
        try {
            m_acdContext.saveComponent(line);
            fail("Should fail");
        } catch (UserException e) {
            // ok
        }

        line.setName("l5");
        line.setExtension("7777");
        try {
            m_acdContext.saveComponent(line);
            fail("Should fail. Extension used by a conference");
        } catch (UserException e) {
            // ok
        }

        line.setName("*88");
        try {
            m_acdContext.saveComponent(line);
            fail("Should fail");
        } catch (Exception e) {
            // ok
        }

        line.setName("*86");
        try {
            m_acdContext.saveComponent(line);
            fail("Should fail");
        } catch (Exception e) {
            // ok
        }
        line.setName("l5");
        line.setExtension("112");
        m_acdContext.saveComponent(line);

        // check if I can saveComponent the same line more than once
        m_acdContext.saveComponent(line);

    }

    public void testsaveComponentQueue() throws Exception {
        sql("acd/queues.sql");
        AcdServer acdServer = m_acdContext.loadServer(new Integer(1001));
        AcdQueue queue = m_acdContext.newQueue();
        queue.setAcdServer(acdServer);
        queue.setName("q1");
        try {
            m_acdContext.saveComponent(queue);
            fail("Should fail");
        } catch (UserException e) {
            // ok
        }

        queue.setName("q4");
        m_acdContext.saveComponent(queue);
    }

    public void testRemoveLines() throws Exception {
        sql("acd/lines.sql");
        Set lines = m_acdContext.loadServer(SERVER_ID).getLines();
        assertEquals(2, lines.size());

        m_acdContext.removeLines(Collections.singletonList(new Integer(2002)));
        lines = m_acdContext.loadServer(SERVER_ID).getLines();
        assertEquals(1, lines.size());
        for (Iterator i = lines.iterator(); i.hasNext();) {
            AcdLine line = (AcdLine) i.next();
            assertEquals(new Integer(2001), line.getId());
        }
    }

    public void testRemoveLinesWithQueues() throws Exception {
        sql("acd/assoc.sql");
        Set lines = m_acdContext.loadServer(SERVER_ID).getLines();
        assertEquals(2, lines.size());

        m_acdContext.removeLines(Collections.singletonList(new Integer(2001)));
        lines = m_acdContext.loadServer(SERVER_ID).getLines();
        assertEquals(1, lines.size());
        for (Iterator i = lines.iterator(); i.hasNext();) {
            AcdLine line = (AcdLine) i.next();
            assertEquals(new Integer(2002), line.getId());
        }
    }

    public void testRemoveQueues() throws Exception {
        sql("acd/queues.sql");
        assertEquals(2, countRowsInTable("acd_queue"));
        Set queues = m_acdContext.loadServer(SERVER_ID).getQueues();
        Set lines = m_acdContext.loadServer(SERVER_ID).getLines();
        assertEquals(0, lines.size());
        assertEquals(2, queues.size());

        m_acdContext.removeQueues(Collections.singletonList(new Integer(2002)));
        queues = m_acdContext.loadServer(SERVER_ID).getQueues();
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

        Set queues = m_acdContext.loadServer(SERVER_ID).getQueues();
        assertEquals(3, queues.size());

        AcdQueue overflowQueue = m_acdContext.loadQueue(new Integer(2003));
        AcdQueue queue = m_acdContext.loadQueue(new Integer(2002));

        assertEquals(queue.getOverflowQueue(), overflowQueue);

        m_acdContext.removeQueues(Collections.singletonList(overflowQueue.getId()));
        commit();

        assertEquals(2, countRowsInTable("acd_queue"));

        queue = m_acdContext.loadQueue(new Integer(2002));
        assertNull(queue.getOverflowQueue());
        assertEquals(0, db().queryForLong("select count(*) from acd_queue where overflow_queue_id = 2003"));
    }

    public void testRemoveQueuesWithLines() throws Exception {
        sql("acd/assoc.sql");
        assertEquals(2, countRowsInTable("acd_queue"));

        Set queues = m_acdContext.loadServer(SERVER_ID).getQueues();
        Set lines = m_acdContext.loadServer(SERVER_ID).getLines();
        assertEquals(2, lines.size());
        assertEquals(2, queues.size());

        m_acdContext.removeQueues(Collections.singletonList(new Integer(3001)));
        queues = m_acdContext.loadServer(SERVER_ID).getQueues();
        assertEquals(1, queues.size());
        for (Iterator i = queues.iterator(); i.hasNext();) {
            AcdQueue line = (AcdQueue) i.next();
            assertEquals(new Integer(3002), line.getId());
        }
    }

    public void testRemoveQueuesWithAgents() throws Exception {
        sql("acd/agents.sql");
        assertEquals(2, countRowsInTable("acd_queue"));
        assertEquals(4, countRowsInTable("acd_agent"));

        Set queues = m_acdContext.loadServer(SERVER_ID).getQueues();
        assertEquals(2, queues.size());

        m_acdContext.removeQueues(Collections.singletonList(new Integer(2002)));
        queues = m_acdContext.loadServer(SERVER_ID).getQueues();
        assertEquals(1, queues.size());
        for (Iterator i = queues.iterator(); i.hasNext();) {
            AcdQueue line = (AcdQueue) i.next();
            assertEquals(new Integer(2001), line.getId());
        }
        commit();

        assertEquals(1, countRowsInTable("acd_queue"));

        // agents that were only on 2002 should be removed as well
        assertEquals(2, countRowsInTable("acd_agent"));
    }

    public void testRemoveQueuesWithSharedAgents() throws Exception {
        sql("acd/agents.sql");
        assertEquals(2, countRowsInTable("acd_queue"));
        Set queues = m_acdContext.loadServer(SERVER_ID).getQueues();
        assertEquals(2, queues.size());
        List<Integer> queuesToBeDeleted = new ArrayList<Integer>();
        queuesToBeDeleted.add(new Integer(2001));
        queuesToBeDeleted.add(new Integer(2002));
        m_acdContext.removeQueues(queuesToBeDeleted);
        queues = m_acdContext.loadServer(SERVER_ID).getQueues();
        commit();
        assertEquals(0, queues.size());
        assertEquals(0, countRowsInTable("acd_queue"));
    }

    public void testOnDeleteUser() throws Exception {
        sql("acd/agents.sql");
        assertEquals(4, countRowsInTable("acd_agent"));
        commit();
        getDaoEventPublisher().resetListeners();
        m_coreContext.deleteUsers(Collections.singleton(new Integer(1000)));
        assertEquals(0, countRowsInTable("acd_agent"));
    }

    public void testAddUsersToQueue() throws Exception {
        sql("acd/queues.sql");
        User testUser1 = m_coreContext.newUser();
        testUser1.setUserName("testUser1");
        m_coreContext.saveUser(testUser1);

        User testUser2 = m_coreContext.newUser();
        testUser2.setUserName("testUser2");
        m_coreContext.saveUser(testUser2);

        User testUser3 = m_coreContext.newUser();
        testUser3.setUserName("testUser3");
        m_coreContext.saveUser(testUser3);

        List agentList = new ArrayList();

        agentList.add(testUser1.getId());
        agentList.add(testUser2.getId());
        agentList.add(testUser3.getId());
        
        final Serializable queueId = new Integer(2001);

        m_acdContext.addUsersToQueue(queueId, Collections.unmodifiableCollection(agentList));
        commit();

        assertEquals(3, countRowsInTable("acd_agent"));
        Map<String, Object> agentTable = db().queryForMap("select * from acd_agent limit 1");
        assertEquals(testUser1.getId(), agentTable.get("user_id"));
        assertEquals(SERVER_ID, agentTable.get("acd_server_id"));

        assertEquals(3, countRowsInTable("acd_queue_agent"));
        assertEquals(queueId, db().queryForInt("select acd_queue_id from acd_queue_agent limit 1"));

        // caling this again should not change anything
        m_acdContext.addUsersToQueue(queueId, Collections.singletonList(testUser1.getId()));

        assertEquals(3, countRowsInTable("acd_agent"));
        assertEquals(testUser1.getId().intValue(), db().queryForInt("select user_id from acd_agent limit 1"));

        assertEquals(3, countRowsInTable("acd_queue_agent"));
        assertEquals(queueId, db().queryForInt("select acd_queue_id from acd_queue_agent limit 1"));
    }

    public void testRemoveAgent() throws Exception {
        sql("acd/agents.sql");
        List removeList = new ArrayList();
        removeList.add(new Integer(3001));
        removeList.add(new Integer(3003));
        final Serializable queueId = new Integer(2002);
        m_acdContext.removeAgents(queueId, removeList);
        List agents = m_acdContext.loadQueue(queueId).getAgents();
        assertEquals(1, agents.size());
        commit();
        // one agent remove, another one still belongs to the second queue
        assertEquals(3, countRowsInTable("acd_agent"));
    }

    public void testLoadAssociate() throws Exception {
        sql("acd/assoc.sql");
        AcdLine line = m_acdContext.loadLine(new Integer(2001));
        assertEquals(new Integer(3001), line.getAcdQueue().getId());
    }

    public void testAssociate() throws Exception {
        sql("acd/assoc.sql");
        m_acdContext.associate(new Integer(2002), new Integer(3002));
        // 2 since there is one more in another queue
        commit();
        assertEquals(2, countRowsInTable("acd_line_queue"));
        ResultDataGrid actual = new ResultDataGrid();
        db().query("select acd_line_id, acd_queue_id from acd_line_queue", actual);
        Object[][] expected = new Object[][] {
                {2001, 3001},
                {2002, 3002}
        };
        assertArrayEquals(expected, actual.toArray());
    }

    public void testUnAssociate() throws Exception {
        sql("acd/assoc.sql");
        m_acdContext.associate(new Integer(2001), null);
        commit();
        
        // empty - we removed the only existing association
        assertEquals(0, countRowsInTable("acd_line_queue"));
    }

    public void testMoveQueues() throws Exception {
        sql("acd/agents.sql");
        Integer agentId = new Integer(3001);
        Collection queuesIds = Collections.singletonList(new Integer(2002));

        m_acdContext.moveQueuesInAgent(agentId, queuesIds, -1);
        commit();
        ResultDataGrid actual = new ResultDataGrid();
        db().query("select acd_agent_id, acd_queue_id, queue_position from acd_agent_queue order by acd_agent_id, acd_queue_id, queue_position", actual);
        Object[][] expected = new Object[][] {
                new Object[] { 3001, 2001, 1 },
                new Object[] { 3001, 2002, 0 },
                new Object[] { 3002, 2002, 0 },
                new Object[] { 3003, 2002, 0 }
        };
        assertArrayEquals(expected, actual.toArray());
    }

    public void testMoveAgents() throws Exception {
        sql("acd/agents.sql");
        Integer queueId = new Integer(2002);
        Collection agentsIds = Collections.singletonList(new Integer(3001));
        m_acdContext.moveAgentsInQueue(queueId, agentsIds, 2);
        commit();
        ResultDataGrid actual = new ResultDataGrid();
        db().query("select acd_agent_id, acd_queue_id, agent_position from acd_queue_agent", actual);
        Object[][] expected = new Object[][] {
                new Object[] { 3001, 2001, 0 },
                new Object[] { 3002, 2002, 0 },
                new Object[] { 3003, 2002, 0 },
                new Object[] { 3001, 2002, 0 }
        };
    }

    public void testClear() throws Exception {
        sql("acd/agents.sql");
        m_acdContext.clear();
        commit();
        assertEquals(0, countRowsInTable("acd_server"));
        assertEquals(0, countRowsInTable("acd_queue"));
        assertEquals(0, countRowsInTable("acd_line"));
        assertEquals(0, countRowsInTable("acd_agent_queue"));
        assertEquals(0, countRowsInTable("acd_queue_agent"));
        assertEquals(0, countRowsInTable("acd_line_queue"));
    }

    public void testGetQueuesForUsers() throws Exception {
        TestHelper.cleanInsertFlat("acd/queues_and_agents.db.xml");
        List<User> agents = new ArrayList();
        agents.add(m_coreContext.loadUser(1001));
        agents.add(m_coreContext.loadUser(1003));
        AcdServer server = m_acdContext.loadServer(1001);

        Collection<AcdQueue> queues = m_acdContext.getQueuesForUsers(server, agents);
        assertEquals(2, queues.size());
        Iterator<AcdQueue> iqueues = queues.iterator();
        assertEquals("q1", iqueues.next().getName());
        assertEquals("q2", iqueues.next().getName());
    }

    public void testGetQueuesForEmptyUsers() throws Exception {
        // just need 1 acd server
        sql("acd/queues.sql");
        AcdServer server = m_acdContext.loadServer(1001);
        Collection empty = m_acdContext.getQueuesForUsers(server, new ArrayList());
        assertTrue(empty.isEmpty());
    }

    public void testAliases() throws Exception {
        sql("acd/lines.sql");
        assertTrue(m_acdContext.isAliasInUse("101"));
        assertTrue(m_acdContext.isAliasInUse("123456789"));
    }

    public void setAcdContext(AcdContext acdContext) {
        m_acdContext = acdContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}

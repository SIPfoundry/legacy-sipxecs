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

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public class AcdAgentTest extends BeanWithSettingsTestCase {

    private AcdAgent m_agent;

    protected void setUp() throws Exception {
        super.setUp();
        m_agent = new AcdAgent();
        initializeBeanWithSettings(m_agent);
    }

    public void testCalculateUri() {
        IMocksControl mc = EasyMock.createControl();
        CoreContext coreContext = mc.createMock(CoreContext.class);
        coreContext.getDomainName();
        mc.andReturn("mydomain.org");
        mc.replay();

        m_agent.setCoreContext(coreContext);

        User user = new User();
        user.setUserName("testuser");
        m_agent.setUser(user);

        assertEquals("sip:testuser@mydomain.org", m_agent.calculateUri());
    }

    public void testSettings() {
        IMocksControl mc = EasyMock.createControl();
        CoreContext coreContext = mc.createMock(CoreContext.class);
        coreContext.getDomainName();
        mc.andReturn("mydomain.org").atLeastOnce();
        mc.replay();

        m_agent.setCoreContext(coreContext);

        User user = new User();
        user.setUserName("testuser");
        m_agent.setUser(user);

        AcdServer server = new AcdServer();
        Location location = new Location();
        location.setFqdn("host.mydomain.org");
        server.setLocation(location);

        AcdQueue queue = new AcdQueue();
        queue.setCoreContext(coreContext);
        queue.setName("testqueue");
        queue.insertAgent(m_agent);
        queue.setAcdServer(server);

        m_agent.initialize();
        assertEquals("testuser", m_agent.getSettingValue(AcdAgent.NAME));
        assertEquals("sip:testuser@mydomain.org", m_agent.getSettingValue(AcdAgent.URI));
        assertEquals("sip:testqueue@host.mydomain.org", m_agent.getSettingValue(AcdAgent.QUEUE_LIST));

        user.setFirstName("aaa");
        user.setLastName("bbb");
        assertEquals("aaa bbb", m_agent.getSettingValue(AcdAgent.NAME));

        mc.verify();
    }

    public void testSettingsForMultipleQueues() {
        IMocksControl mc = EasyMock.createControl();
        CoreContext coreContext = mc.createMock(CoreContext.class);
        mc.replay();

        m_agent.setCoreContext(coreContext);

        User user = new User();
        user.setUserName("testuser");
        m_agent.setUser(user);

        AcdServer server = new AcdServer();
        Location location = new Location();
        location.setFqdn("host.mydomain.org");
        server.setLocation(location);

        AcdQueue queue1 = new AcdQueue();
        queue1.setUniqueId();
        queue1.setCoreContext(coreContext);
        queue1.setName("testqueue1");
        queue1.insertAgent(m_agent);
        queue1.setAcdServer(server);

        AcdQueue queue2 = new AcdQueue();
        queue2.setUniqueId();
        queue2.setCoreContext(coreContext);
        queue2.setName("testqueue2");
        queue2.insertAgent(m_agent);
        queue2.setAcdServer(server);

        m_agent.initialize();
        assertEquals("sip:testqueue1@host.mydomain.org,sip:testqueue2@host.mydomain.org", m_agent
                .getSettingValue(AcdAgent.QUEUE_LIST));

        mc.verify();
    }

    public void testMoveQueues() {
        AcdQueue queue1 = new AcdQueue();
        queue1.setUniqueId();
        queue1.setName("testqueue");
        queue1.insertAgent(m_agent);

        AcdQueue queue2 = new AcdQueue();
        queue2.setUniqueId();
        queue2.setName("testqueue");
        queue2.insertAgent(m_agent);

        assertEquals(queue1, m_agent.getQueues().get(0));
        m_agent.moveQueues(Collections.singletonList(queue2.getId()), -1);
        assertEquals(queue2, m_agent.getQueues().get(0));
        m_agent.moveQueues(Collections.singletonList(queue2.getId()), 1);
        assertEquals(queue1, m_agent.getQueues().get(0));
    }
}

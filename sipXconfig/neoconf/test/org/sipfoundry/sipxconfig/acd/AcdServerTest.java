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

import java.util.Set;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.service.SipxPresenceService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

import static org.easymock.EasyMock.expectLastCall;

public class AcdServerTest extends BeanWithSettingsTestCase {

    private AcdServer m_server;

    private Location m_location;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        m_server = new AcdServer();
        m_location = new Location();
        m_location.setFqdn("localhost");
        m_server.setLocation(m_location);
        m_server.setPort(8110);
        initializeBeanWithSettings(m_server);

        SipxPresenceService presenceService = org.easymock.classextension.EasyMock.createMock(SipxPresenceService.class);
        presenceService.getPresenceServerPort();
        expectLastCall().andReturn(5130).atLeastOnce();
        presenceService.getPresenceApiPort();
        expectLastCall().andReturn(8111).atLeastOnce();
        org.easymock.classextension.EasyMock.replay(presenceService);

        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxPresenceService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(presenceService).atLeastOnce();
        EasyMock.replay(sipxServiceManager);

        m_server.setSipxServiceManager(sipxServiceManager);
    }

    public void testGetSettingsModel() throws Exception {
        assertNotNull(m_server.getSettings());
    }

    /**
     * It can be only enabled temporarily when ACD server is up.
     *
     */
    public void _testAcdXmlRpc() {
        XmlRpcSettings xmlRpc = new XmlRpcSettings(null);

        Set servers = m_server.getAll(xmlRpc, "acd-server", "uri");
        assertEquals(1, servers.size());
        Set lines = m_server.getAll(xmlRpc, "acd-line", "uri");
        assertFalse(lines.isEmpty());
        Set queues = m_server.getAll(xmlRpc, "acd-queue", "uri");
        assertFalse(queues.isEmpty());
        Set agents = m_server.getAll(xmlRpc, "acd-agent", "uri");
        assertFalse(agents.isEmpty());
        xmlRpc.deleteAll("acd-agent", agents, "uri");
        agents = m_server.getAll(xmlRpc, "acd-agent", "uri");
        assertTrue(agents.isEmpty());
    }

    public void testAdminState() {
        m_server.setAdminState(AcdServer.State.DOWN);
        assertEquals("3", m_server.getSettingValue(AcdServer.ADMIN_STATE));
        m_server.setAdminState(AcdServer.State.STANDBY);
        assertEquals("2", m_server.getSettingValue(AcdServer.ADMIN_STATE));
        m_server.setAdminState(AcdServer.State.ACTIVE);
        assertEquals("1", m_server.getSettingValue(AcdServer.ADMIN_STATE));
    }

    public void testAddRemoveQueue() {
        AcdQueue queue = new AcdQueue();
        AcdServer server = new AcdServer();
        assertNull(queue.getAcdServer());
        assertTrue(server.getQueues().isEmpty());

        server.insertQueue(queue);
        assertSame(server, queue.getAcdServer());
        assertSame(queue, server.getQueues().iterator().next());

        server.removeQueue(queue);
        assertNull(queue.getAcdServer());
        assertTrue(server.getQueues().isEmpty());
    }

    public void testAddRemoveLine() {
        AcdLine line = new AcdLine();
        AcdServer server = new AcdServer();
        assertNull(line.getAcdServer());
        assertTrue(server.getLines().isEmpty());

        server.insertLine(line);
        assertSame(server, line.getAcdServer());
        assertSame(line, server.getLines().iterator().next());

        server.removeLine(line);
        assertNull(line.getAcdServer());
        assertTrue(server.getLines().isEmpty());
    }

    public void testGerServiceUri() {
        assertEquals("http://localhost:8110/RPC2", m_server.getServiceUri());

        m_server.setPort(7777);
        m_location = new Location();
        m_location.setFqdn("bongo.sipfoudry.org");
        m_server.setLocation(m_location);
        assertEquals("http://bongo.sipfoudry.org:7777/RPC2", m_server.getServiceUri());

    }

    public void testPrepareSettings() {
        IMocksControl mc = EasyMock.createControl();
        CoreContext coreContext = mc.createMock(CoreContext.class);
        coreContext.getDomainName();
        mc.andReturn("mydomain.org").atLeastOnce();
        mc.replay();

        m_server.setCoreContext(coreContext);
        m_location = new Location();
        m_location.setFqdn("presence.com");
        m_server.setLocation(m_location);

        assertEquals("mydomain.org", m_server.getSettingValue("acd-server/domain"));
        assertEquals("presence.com", m_server.getSettingValue("acd-server/fqdn"));
        assertEquals("sip:presence.com:5130", m_server
                .getSettingValue("acd-server/presence-server-uri"));
        assertEquals("http://presence.com:8111/RPC2", m_server
                .getSettingValue("acd-server/presence-service-uri"));

        mc.verify();
    }

    public void testGetPresenceServiceUri() {
        assertEquals("http://localhost:8111/RPC2", m_server.getPresenceServiceUri());

        m_location = new Location();
        m_location.setFqdn("presence.com");
        m_server.setLocation(m_location);
        assertEquals("http://presence.com:8111/RPC2", m_server.getPresenceServiceUri());
    }

    public void testGetPresenceServerUri() {
        assertEquals("sip:localhost:5130", m_server.getPresenceServerUri());

        m_location = new Location();
        m_location.setFqdn("presence.com");
        m_server.setLocation(m_location);
        assertEquals("sip:presence.com:5130", m_server.getPresenceServerUri());
    }
}

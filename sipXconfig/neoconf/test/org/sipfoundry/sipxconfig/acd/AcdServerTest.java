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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.util.Set;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.presence.PresenceServer;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsTestCase;

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
        
        AddressManager addressManager = createMock(AddressManager.class);
        addressManager.getSingleAddress(PresenceServer.HTTP_ADDRESS);
        expectLastCall().andReturn(new Address(PresenceServer.HTTP_ADDRESS, "presence-api.example.org", 100)).anyTimes();
        addressManager.getSingleAddress(PresenceServer.SIP_TCP_ADDRESS);
        expectLastCall().andReturn(new Address(PresenceServer.SIP_TCP_ADDRESS, "presence-sip.example.org", 101)).anyTimes();
        replay(addressManager);
        m_server.setAddressManager(addressManager);
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
        assertEquals("presence-sip.example.org:101", m_server
                .getSettingValue("acd-server/presence-server-uri"));
        assertEquals("presence-api.example.org:100", m_server
                .getSettingValue("acd-server/presence-service-uri"));

        mc.verify();
    }

    public void testGetPresenceServiceUri() {
        assertEquals("presence-api.example.org:100", m_server.getPresenceServiceUri());
    }

    public void testGetPresenceServerUri() {
        assertEquals("presence-sip.example.org:101", m_server.getPresenceServerUri());
    }
}

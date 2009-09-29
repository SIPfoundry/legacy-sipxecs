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

import java.util.ArrayList;
import java.util.List;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.CoreContext;

public class AcdLineTest extends BeanWithSettingsTestCase {

    private AcdLine m_line;

    protected void setUp() throws Exception {
        super.setUp();
        m_line = new AcdLine();
        initializeBeanWithSettings(m_line);
    }

    public void testGetSettingsModel() throws Exception {
        assertNotNull(m_line.getSettings());
    }

    public void testAssociateQueue() {
        AcdLine line = new AcdLine();
        assertNull(line.getAcdQueue());
        AcdQueue q1 = new AcdQueue();
        assertTrue(q1.getLines().isEmpty());
        AcdQueue q2 = new AcdQueue();
        assertTrue(q2.getLines().isEmpty());

        AcdQueue old = line.associateQueue(q1);
        assertNull(old);
        assertEquals(1, q1.getLines().size());
        assertTrue(q2.getLines().isEmpty());
        assertSame(line, q1.getLines().iterator().next());

        old = line.associateQueue(q1);
        assertSame(old, q1);
        assertEquals(1, q1.getLines().size());
        assertTrue(q2.getLines().isEmpty());

        old = line.associateQueue(q2);
        assertSame(old, q1);
        assertEquals(1, q2.getLines().size());
        assertTrue(q1.getLines().isEmpty());
        assertSame(line, q2.getLines().iterator().next());

        old = line.associateQueue(null);
        assertSame(old, q2);
        assertTrue(q1.getLines().isEmpty());
        assertTrue(q2.getLines().isEmpty());
    }

    public void testPrepareSettings() {
        IMocksControl mc = EasyMock.createControl();
        CoreContext coreContext = mc.createMock(CoreContext.class);
        mc.replay();

        AcdServer server = new AcdServer();
        Location location = new Location();
        location.setFqdn("host.mydomain.org");
        server.setLocation(location);
        m_line.setAcdServer(server);
        m_line.setCoreContext(coreContext);
        m_line.setName("myline");

        AcdQueue queue = new AcdQueue();
        queue.setAcdServer(server);
        queue.setName("testqueue");
        queue.setCoreContext(coreContext);
        m_line.associateQueue(queue);

        m_line.initialize();
        assertEquals("myline", m_line.getSettingValue(AcdLine.LINE_NAME));
        assertEquals("sip:myline@host.mydomain.org", m_line.getSettingValue(AcdLine.URI));
        assertEquals("sip:testqueue@host.mydomain.org", m_line.getSettingValue(AcdLine.ACD_QUEUE));

        mc.verify();
    }

    public void testAppendAliases() {
        IMocksControl mc = EasyMock.createControl();
        CoreContext coreContext = mc.createMock(CoreContext.class);
        coreContext.getDomainName();
        mc.andReturn("mydomain.org").atLeastOnce();
        mc.replay();

        IMocksControl mcs = org.easymock.classextension.EasyMock.createControl();
        AcdServer server = mcs.createMock(AcdServer.class);
        Location location = new Location();
        location.setFqdn("localhost");
        server.getLocation();
        mcs.andReturn(location);
        server.getSipPort();
        mcs.andReturn(100);
        Location location2 = new Location();
        location2.setFqdn("somehost.domain.org");
        server.getLocation();
        mcs.andReturn(location2);
        server.getSipPort();
        mcs.andReturn(101);
        mcs.replay();

        m_line.setCoreContext(coreContext);
        m_line.setModelFilesContext(TestHelper.getModelFilesContext());
        m_line.setName("myline");
        m_line.setExtension("555");
        m_line.setAcdServer(server);

        List aliases = new ArrayList();
        m_line.appendAliases(aliases);
        assertEquals(1, aliases.size());
        AliasMapping alias = (AliasMapping) aliases.get(0);
        assertEquals("555@mydomain.org", alias.getIdentity());
        assertEquals("sip:myline@localhost:100", alias.getContact());

        aliases.clear();
        m_line.appendAliases(aliases);
        assertEquals(1, aliases.size());
        alias = (AliasMapping) aliases.get(0);
        assertEquals("555@mydomain.org", alias.getIdentity());
        assertEquals("sip:myline@somehost.domain.org:101", alias.getContact());

        mcs.verify();
        mc.verify();
    }
}

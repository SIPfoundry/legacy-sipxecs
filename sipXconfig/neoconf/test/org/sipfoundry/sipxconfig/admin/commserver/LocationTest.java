package org.sipfoundry.sipxconfig.admin.commserver;

import junit.framework.TestCase;

public class LocationTest extends TestCase {
    public void testGetProcessMonitorUrl() {
        Location out = new Location();
        out.setFqdn("localhost");
        assertEquals("https://localhost:8092/RPC2", out.getProcessMonitorUrl());
        Location out1 = new Location();
        out1.setFqdn("mysystem.europe.pmd.com");
        assertEquals("https://mysystem.europe.pmd.com:8092/RPC2", out1.getProcessMonitorUrl());
    }

    @SuppressWarnings("deprecation")
    public void testParseAddress() {
        // it tests setUrl - which is deprecated, because it's only used when migrating from topology.xml
        Location out = new Location();
        out.setUrl("https://localhost:8091/cgi-bin/replication/replication.cgi");
        assertEquals("localhost", out.getFqdn());
        assertNull(out.getAddress());

        Location out1 = new Location();
        out1.setUrl("https://192.168.1.10:8091/cgi-bin/replication/replication.cgi");
        assertEquals("192.168.1.10", out1.getFqdn());
        assertNull(out.getAddress());
    }
}

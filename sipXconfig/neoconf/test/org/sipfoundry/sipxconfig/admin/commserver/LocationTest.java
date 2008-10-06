package org.sipfoundry.sipxconfig.admin.commserver;

import junit.framework.TestCase;

public class LocationTest extends TestCase {
    public void testGetProcessMonitorUrl() {
        Location out = new Location();
        out.setFqdn("localhost");
        assertEquals("https://localhost:8092/RPC2", out.getProcessMonitorUrl());
        Location out1 = new Location();
        out1.setAddress("192.168.1.2");
        assertEquals("https://192.168.1.2:8092/RPC2", out1.getProcessMonitorUrl());
        out1.setFqdn("mysystem.europe.pmd.com");
        assertEquals("mysystem", out1.getHostname());
        assertEquals("europe.pmd.com", out1.getDomainname());

    }
    
    public void testParseAddress() {
        Location out = new Location();
        out.setUrl("https://localhost:8091/cgi-bin/replication/replication.cgi");
        assertEquals("localhost", out.getFqdn());
        assertNull(out.getAddress());
        
        Location out1 = new Location();
        out1.setUrl("https://192.168.1.10:8091/cgi-bin/replication/replication.cgi");
        assertEquals("192.168.1.10", out1.getAddress());
        assertNull(out1.getFqdn());
    }
}

package org.sipfoundry.sipxconfig.admin.commserver;

import junit.framework.TestCase;

public class LocationTest extends TestCase {
    public void testGetReplicationUrl() {
        Location out = new Location();
        out.setAddress("localhost");
        assertEquals("https://localhost:8091/cgi-bin/replication/replication.cgi", out.getReplicationUrl());
    }
    
    public void testGetProcessMonitorUrl() {
        Location out = new Location();
        out.setAddress("localhost");
        assertEquals("https://localhost:8092/RPC2", out.getProcessMonitorUrl());
    }
    
    public void testParseAddress() {
        Location out = new Location();
        out.setUrl("https://localhost:8091/cgi-bin/replication/replication.cgi");
        assertEquals("localhost", out.getAddress());
        
        out.setUrl("https://192.168.1.10:8091/cgi-bin/replication/replication.cgi");
        assertEquals("192.168.1.10", out.getAddress());
    }
}

/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.InputStream;

import junit.framework.TestCase;


public class SipxReplicationContextImplTest extends TestCase {
    SipxReplicationContextImpl m_processContextImpl;

    protected void setUp() throws Exception {
        m_processContextImpl = new SipxReplicationContextImpl() {
            protected InputStream getTopologyAsStream() {
                return SipxReplicationContextImplTest.class
                        .getResourceAsStream("topology.test.xml");
            }
        };
    }

    public void testRetrieveReplicationUrls() throws Exception {
        Location[] locations = m_processContextImpl.getLocations();
        assertEquals(2, locations.length);
        assertEquals("https://localhost:8091/cgi-bin/replication/replication.cgi", locations[0]
                .getReplicationUrl());
        assertEquals("h1.sipfoundry.org", locations[0].getSipDomain());
        
        assertEquals("https://192.168.0.27:8091/cgi-bin/replication/replication.cgi",
                locations[1].getReplicationUrl());
        assertEquals("h2.sipfoundry.org", locations[1].getSipDomain());
    }
}

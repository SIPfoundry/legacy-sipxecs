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

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.TestHelper;

public class LocationsManagerImplTestIntegration extends IntegrationTestCase {
    private LocationsManager m_out;

    public void testGetLocations() throws Exception {
        TestHelper.cleanInsert("admin/commserver/clearLocations.xml");
        Location[] emptyLocations = m_out.getLocations();
        assertEquals(0, emptyLocations.length);
        
        TestHelper.cleanInsert("admin/commserver/seedLocations.xml");
        Location[] locations = m_out.getLocations();
        assertEquals(2, locations.length);
        assertEquals("https://localhost:8091/cgi-bin/replication/replication.cgi", locations[0].getReplicationUrl());
        assertEquals("h1.example.org", locations[0].getSipDomain());
        assertEquals("https://192.168.0.27:8091/cgi-bin/replication/replication.cgi", locations[1].getReplicationUrl());
        assertEquals("h2.example.org", locations[1].getSipDomain());
    }
    
    public void testStore() throws Exception {
        TestHelper.cleanInsert("admin/commserver/clearLocations.xml");
        Location location = new Location();
        location.setName("test location");
        location.setSipDomain("example.org");
        
        m_out.storeLocation(location);
        
        Location[] dbLocations = m_out.getLocations();
        assertEquals(1, dbLocations.length);
        assertEquals("test location", dbLocations[0].getName());
        assertEquals("example.org", dbLocations[0].getSipDomain());
        
        location.setSipDomain("newdomain.org");
        m_out.storeLocation(location);
        
        dbLocations = m_out.getLocations();
        assertEquals(1, dbLocations.length);
        assertEquals("newdomain.org", dbLocations[0].getSipDomain());
    }
    
    public void testDelete() throws Exception {
        TestHelper.cleanInsert("admin/commserver/seedLocations.xml");
        Location[] locationsBeforeDelete = m_out.getLocations();
        assertEquals(2, locationsBeforeDelete.length);
        
        m_out.deleteLocation(locationsBeforeDelete[0]);
        
        Location[] locationsAfterDelete = m_out.getLocations();
        assertEquals(1, locationsAfterDelete.length);
        assertEquals("h2.example.org", locationsAfterDelete[0].getSipDomain());
    }
    
    public void setLocationsManager(LocationsManager locationsManager) {
        m_out = locationsManager;
    }
}

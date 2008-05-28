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
    private LocationsManager m_locationsManager;

    public void testRetrieveReplicationUrls() throws Exception {
        TestHelper.cleanInsert("admin/commserver/seedLocations.xml");
        
        Location[] locations = m_locationsManager.getLocations();
        assertEquals(2, locations.length);
        assertEquals("https://localhost:8091/cgi-bin/replication/replication.cgi", locations[0].getReplicationUrl());
        assertEquals("h1.example.org", locations[0].getSipDomain());

        assertEquals("https://192.168.0.27:8091/cgi-bin/replication/replication.cgi", locations[1].getReplicationUrl());
        assertEquals("h2.example.org", locations[1].getSipDomain());
    }
    
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}

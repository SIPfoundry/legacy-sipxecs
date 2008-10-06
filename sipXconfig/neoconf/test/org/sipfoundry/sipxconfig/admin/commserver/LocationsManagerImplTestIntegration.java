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

public class LocationsManagerImplTestIntegration extends IntegrationTestCase {
    private LocationsManager m_out;

    public void testGetLocations() throws Exception {
        loadDataSetXml("admin/commserver/clearLocations.xml");
        Location[] emptyLocations = m_out.getLocations();
        assertEquals(0, emptyLocations.length);

        loadDataSetXml("admin/commserver/seedLocations.xml");
        Location[] locations = m_out.getLocations();
        assertEquals(2, locations.length);
        assertEquals("https://localhost:8092/RPC2", locations[0].getProcessMonitorUrl());
        assertEquals("https://remotehost.example.com:8092/RPC2", locations[1].getProcessMonitorUrl());
    }

    public void testFindById() throws Exception {
        loadDataSetXml("admin/commserver/seedLocations.xml");
        Location[] locations = m_out.getLocations();

        Location firstLocation = locations[0];
        int locationId = firstLocation.getId();

        Location locationById = m_out.getLocation(locationId);
        assertNotNull(locationById);
        assertEquals(firstLocation.getName(), locationById.getName());
    }

    public void testStore() throws Exception {
        loadDataSetXml("admin/commserver/clearLocations.xml");
        Location location = new Location();
        location.setName("test location");
        location.setAddress("192.168.1.2");
        location.setFqdn("localhost");

        m_out.storeLocation(location);

        Location[] dbLocations = m_out.getLocations();
        assertEquals(1, dbLocations.length);
        assertEquals("test location", dbLocations[0].getName());
        assertEquals("192.168.1.2", dbLocations[0].getAddress());
        assertEquals("localhost", dbLocations[0].getFqdn());
    }

    public void testDelete() throws Exception {
        loadDataSetXml("admin/commserver/seedLocations.xml");
        Location[] locationsBeforeDelete = m_out.getLocations();
        assertEquals(2, locationsBeforeDelete.length);

        m_out.deleteLocation(locationsBeforeDelete[0]);

        Location[] locationsAfterDelete = m_out.getLocations();
        assertEquals(1, locationsAfterDelete.length);
        assertEquals("remotehost.example.com", locationsAfterDelete[0].getFqdn());
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_out = locationsManager;
    }
}

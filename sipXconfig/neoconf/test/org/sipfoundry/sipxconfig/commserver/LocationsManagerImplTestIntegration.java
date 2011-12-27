/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver;

import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventPublisher;
import org.sipfoundry.sipxconfig.commserver.imdb.ImdbTestCase;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.springframework.data.mongodb.core.MongoTemplate;

public class LocationsManagerImplTestIntegration extends ImdbTestCase {
    private LocationsManager m_out;
    private AcdContext m_acdContext;
    private ConferenceBridgeContext m_conferenceBridgeContext;

    public void testGetLocations() throws Exception {
        loadDataSetXml("commserver/clearLocations.xml");
        Location[] emptyLocations = m_out.getLocations();
        assertEquals(0, emptyLocations.length);

        loadDataSetXml("commserver/seedLocationsAndServices.xml");
        Location[] locations = m_out.getLocations();
        assertEquals(3, locations.length);
        assertEquals("https://localhost:8092/RPC2", locations[0].getProcessMonitorUrl());
        assertEquals("https://secondary.example.org:8092/RPC2", locations[1].getProcessMonitorUrl());
    }

    public void testGetLocationByFqdn() throws Exception {
        loadDataSetXml("commserver/clearLocations.xml");
        loadDataSetXml("commserver/seedLocationsAndServices.xml");

        Location l = m_out.getLocationByFqdn("remotehost.example.org");
        assertEquals(new Integer(102), l.getId());
    }

    public void testGetLocationByAddress() throws Exception {
        loadDataSetXml("commserver/clearLocations.xml");
        loadDataSetXml("commserver/seedLocationsAndServices.xml");

        Location l = m_out.getLocationByAddress("10.1.1.1");
        assertEquals(new Integer(101), l.getId());
    }

    public void testFindById() throws Exception {
        loadDataSetXml("commserver/seedLocationsAndServices.xml");
        Location[] locations = m_out.getLocations();

        Location secondLocation = locations[1];
        int locationId = secondLocation.getId();

        Location locationById = m_out.getLocation(locationId);
        assertNotNull(locationById);
        assertEquals(secondLocation.getName(), locationById.getName());
    }

    public void testGetPrimaryLocation() throws Exception {
        loadDataSetXml("commserver/seedLocations.xml");
        Location location = m_out.getPrimaryLocation();
        assertNotNull(location);
        assertEquals(101, (int) location.getId());
    }

    public void testStore() throws Exception {
        loadDataSetXml("commserver/clearLocations.xml");
        Location location = new Location();
        location.setName("test location");
        location.setPrimary(true);
        location.setFqdn("location.example.org");
        location.setAddress("192.168.1.2");
        location.setFqdn("localhost");
        location.setRegistered(true);

        Location location2 = new Location();
        location2.setName("test location");
        location2.setFqdn("location2.example.org");
        location2.setAddress("192.168.1.3");
        location2.setFqdn("localhost1");
        location2.setRegistered(false);

        m_out.saveLocation(location);
        m_out.saveLocation(location2);

        Location[] dbLocations = m_out.getLocations();
        assertEquals(2, dbLocations.length);
        assertEquals("test location", dbLocations[0].getName());
        assertEquals("192.168.1.2", dbLocations[0].getAddress());
        assertEquals("localhost", dbLocations[0].getFqdn());
        assertTrue(dbLocations[0].isCallTraffic());

        dbLocations[1].setCallTraffic(false);
        m_out.saveLocation(dbLocations[1]);

        dbLocations[1].setCallTraffic(true);
        m_out.saveLocation(dbLocations[1]);
    }

    public void testsaveLocationWithDuplicateFqdnOrIp() throws Exception {
        loadDataSetXml("commserver/clearLocations.xml");
        Location location = new Location();
        location.setName("test location");
        location.setAddress("10.1.1.1");
        location.setFqdn("localhost");
        m_out.saveLocation(location);

        Location[] dbLocations = m_out.getLocations();
        assertEquals(1, dbLocations.length);
        assertEquals("test location", dbLocations[0].getName());
        assertEquals("10.1.1.1", dbLocations[0].getAddress());
        assertEquals("localhost", dbLocations[0].getFqdn());

        location = new Location();
        location.setName("test location");
        location.setAddress("10.1.1.2");
        // Same FQDN
        location.setFqdn("localhost");
        try {
            m_out.saveLocation(location);
            assertTrue(false);
        } catch (UserException ex) {
            assertTrue(true);
        }

        location = new Location();
        location.setName("test location");
        // Same ip address
        location.setAddress("10.1.1.1");
        location.setFqdn("localhost.localdomain");
        try {
            m_out.saveLocation(location);
            assertTrue(false);
        } catch (UserException ex) {
            assertTrue(true);
        }
    }

    public void testStoreNatLocation() throws Exception {
        loadDataSetXml("commserver/seedLocations.xml");

        Location nat = m_out.getPrimaryLocation();
        assertNotNull(nat);

        nat.setStunAddress("stun.com");
        nat.setStunInterval(30);
        nat.setPublicPort(5160);
        nat.setPublicTlsPort(5161);
        nat.setStartRtpPort(30000);
        nat.setStopRtpPort(30100);

        m_out.saveLocation(nat);

        Location natDB = m_out.getLocation(nat.getId());
        assertNotNull(natDB);
        assertEquals("stun.com", natDB.getStunAddress());
        assertEquals(30, natDB.getStunInterval());
        assertEquals(5160, natDB.getPublicPort());
        assertEquals(5161, natDB.getPublicTlsPort());
        assertEquals(30000, natDB.getStartRtpPort());
        assertEquals(30100, natDB.getStopRtpPort());
    }

    public void testGetNatLocation() throws Exception {
        loadDataSet("nattraversal/nat_location.db.xml");

        Location natLocation = m_out.getLocation(111);
        assertEquals("stun.ezuce.com", natLocation.getStunAddress());
        assertEquals(60, natLocation.getStunInterval());
        assertEquals(5060, natLocation.getPublicPort());
        assertEquals(5061, natLocation.getPublicTlsPort());
        assertEquals(30000, natLocation.getStartRtpPort());
        assertEquals(31000, natLocation.getStopRtpPort());
    }


    public void setLocationsManager(LocationsManager locationsManager) {
        m_out = locationsManager;
    }

    public void setAcdContext(AcdContext acdContext) {
        m_acdContext = acdContext;
    }

    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }
}

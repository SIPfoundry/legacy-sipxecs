/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.parkorbit;

import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class ParkOrbitTestIntegration extends IntegrationTestCase {
    private ParkOrbitContext m_parkOrbitContext;
    private LocationsManager m_locationsManager;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("commserver/SeedLocations.sql");
    }
    
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        loadDataSetXml("parkorbit/OrbitSeed.xml");
    }

    public void testLoadParkOrbit() throws Exception {
        ParkOrbit orbit = m_parkOrbitContext.loadParkOrbit(new Integer(1001));
        assertEquals("sales", orbit.getName());
        assertTrue(orbit.isEnabled());
        assertEquals("sales", orbit.getName());
        assertEquals("501", orbit.getExtension());
        assertEquals("something.wav", orbit.getMusic());
        assertEquals("primary.example.org", orbit.getLocation().getFqdn());
    }

    public void testGetParkOrbits() throws Exception {
        Collection<ParkOrbit> orbits = m_parkOrbitContext.getParkOrbits();
        assertEquals(2, orbits.size());
        Iterator<ParkOrbit> i = orbits.iterator();
        ParkOrbit orbit1 = i.next();
        ParkOrbit orbit2 = i.next();
        assertFalse(orbit1.getName().equals(orbit2.getName()));
    }

    public void testGetParkOrbitsByServer() throws Exception {
        Location primary = m_locationsManager.getPrimaryLocation();
        Collection<ParkOrbit> orbits = m_parkOrbitContext.getParkOrbits(primary.getId());
        assertEquals(2, orbits.size());
        Location secondary = m_locationsManager.getLocationByFqdn("remote.example.org");
        orbits = m_parkOrbitContext.getParkOrbits(secondary.getId());
        assertEquals(0, orbits.size());
    }

    public void testStoreParkOrbit() throws Exception {
        Location primary = m_locationsManager.getPrimaryLocation();
        ParkOrbit orbit = new ParkOrbit();
        orbit.setName("kuku");
        orbit.setDescription("kukuLine");
        orbit.setExtension("202");
        orbit.setEnabled(true);
        orbit.setMusic("tango.wav");
        orbit.setLocation(primary);
        m_parkOrbitContext.storeParkOrbit(orbit);
        // table will have 4 rows - 3 park orbits + 1 music on hold
        commit();
        assertEquals(4, countRowsInTable("park_orbit"));
    }

    public void testRemoveParkOrbit() throws Exception {
        List ids = Arrays.asList(new Integer[] {
            new Integer(1001), new Integer(1002)
        });
        m_parkOrbitContext.removeParkOrbits(ids);
        // table should be empty now - except for 1 music on hold orbit
        assertEquals(1, countRowsInTable("park_orbit"));
    }

    public void testClear() throws Exception {
        m_parkOrbitContext.clear();
        commit();
        // table should be empty now - except for 1 music on hold orbit
        assertEquals(1, countRowsInTable("park_orbit"));
    }

    public void testDefaultMusicOnHold() throws Exception {
        final String newMusic = "new.wav";
        assertEquals("default.wav", m_parkOrbitContext.getDefaultMusicOnHold());
        m_parkOrbitContext.setDefaultMusicOnHold(newMusic);
        commit();
        assertEquals(newMusic, m_parkOrbitContext.getDefaultMusicOnHold());
        db().queryForInt("select 1 from park_orbit where music = ?", newMusic);
    }

    public void testIsAliasInUse() {
        assertTrue(m_parkOrbitContext.isAliasInUse("sales"));
        assertTrue(m_parkOrbitContext.isAliasInUse("501"));
        assertTrue(m_parkOrbitContext.isAliasInUse("502"));
        assertFalse(m_parkOrbitContext.isAliasInUse("911"));
    }

    public void testGetBeanIdsOfObjectsWithAlias() {
        assertTrue(m_parkOrbitContext.getBeanIdsOfObjectsWithAlias("sales").size() == 1);
        assertTrue(m_parkOrbitContext.getBeanIdsOfObjectsWithAlias("501").size() == 1);
        assertTrue(m_parkOrbitContext.getBeanIdsOfObjectsWithAlias("502").size() == 1);
        assertTrue(m_parkOrbitContext.getBeanIdsOfObjectsWithAlias("911").size() == 0);
    }

    public void setParkOrbitContext(ParkOrbitContext parkOrbitContext) {
        m_parkOrbitContext = parkOrbitContext;
    }

    public void setLocationsManager(LocationsManager manager) {
        m_locationsManager = manager;
    }
}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.parkorbit;

import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.dbunit.dataset.ITable;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class ParkOrbitTestDb extends SipxDatabaseTestCase {

    private ParkOrbitContext m_context;

    @Override
    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_context = (ParkOrbitContext) appContext.getBean(ParkOrbitContext.CONTEXT_BEAN_NAME);

        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsert("admin/parkorbit/OrbitSeed.xml");
    }

    public void testLoadParkOrbit() throws Exception {
        ParkOrbit orbit = m_context.loadParkOrbit(new Integer(1001));
        assertEquals("sales", orbit.getName());
        assertTrue(orbit.isEnabled());
        assertEquals("sales", orbit.getName());
        assertEquals("501", orbit.getExtension());
        assertEquals("something.wav", orbit.getMusic());
    }

    public void testGetParkOrbits() throws Exception {
        Collection<ParkOrbit> orbits = m_context.getParkOrbits();
        assertEquals(2, orbits.size());
        Iterator<ParkOrbit> i = orbits.iterator();
        ParkOrbit orbit1 = i.next();
        ParkOrbit orbit2 = i.next();
        assertFalse(orbit1.getName().equals(orbit2.getName()));
    }

    public void testStoreParkOrbit() throws Exception {
        ParkOrbit orbit = new ParkOrbit();
        orbit.setName("kuku");
        orbit.setDescription("kukuLine");
        orbit.setExtension("202");
        orbit.setEnabled(true);
        orbit.setMusic("tango.wav");
        m_context.storeParkOrbit(orbit);
        // table will have 4 rows - 3 park orbits + 1 music on hold
        ITable orbitTable = TestHelper.getConnection().createDataSet().getTable("park_orbit");
        assertEquals(4, orbitTable.getRowCount());
    }

    public void testRemoveParkOrbit() throws Exception {
        List ids = Arrays.asList(new Integer[] {
            new Integer(1001), new Integer(1002)
        });
        m_context.removeParkOrbits(ids);
        // table should be empty now - except for 1 music on hold orbit
        ITable orbitTable = TestHelper.getConnection().createDataSet().getTable("park_orbit");
        assertEquals(1, orbitTable.getRowCount());
    }

    public void testClear() throws Exception {
        m_context.clear();
        // table should be empty now - except for 1 music on hold orbit
        ITable orbitTable = TestHelper.getConnection().createDataSet().getTable("park_orbit");
        assertEquals(1, orbitTable.getRowCount());
    }

    public void testGenerateAliases() throws Exception {
        // park orbits do not generate any aliases
        Collection aliases = m_context.getAliasMappings();
        assertNotNull(aliases);
        assertEquals(0, aliases.size());
    }

    public void testDefaultMusicOnHold() throws Exception {
        final String newMusic = "new.wav";
        assertEquals("default.wav", m_context.getDefaultMusicOnHold());
        m_context.setDefaultMusicOnHold(newMusic);
        assertEquals(newMusic, m_context.getDefaultMusicOnHold());

        ITable orbitTable = TestHelper.getConnection().createDataSet().getTable("park_orbit");
        assertEquals(newMusic, orbitTable.getValue(2, "music"));
    }

    public void testIsAliasInUse() {
        assertTrue(m_context.isAliasInUse("sales"));
        assertTrue(m_context.isAliasInUse("501"));
        assertTrue(m_context.isAliasInUse("502"));
        assertFalse(m_context.isAliasInUse("911"));
    }

    public void testGetBeanIdsOfObjectsWithAlias() {
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("sales").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("501").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("502").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("911").size() == 0);
    }
}

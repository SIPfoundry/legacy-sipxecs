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

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class LocationsMigrationTriggerTestIntegration extends IntegrationTestCase {

    private static final String TOPOLOGY_TEST_XML = "topology.test.xml";
    private static final String TOPOLOGY_TEST_XML_ORIG = "topology.test.xml.orig";
    private LocationsMigrationTrigger m_out;
    private LocationsManager m_locationsManager;

    public void testOnInitTaskWithNoLocationsInDatabase() throws Exception {
        loadDataSetXml("admin/commserver/clearLocations.xml");
        Location[] locationsBeforeMigration = m_locationsManager.getLocations();
        assertEquals(0, locationsBeforeMigration.length);

        File testTopologyFile = createTopologyFile();

        m_out.setConfigDirectory(getConfigDirectory());
        m_out.setTopologyFilename(TOPOLOGY_TEST_XML);
        m_out.onInitTask("migrate_locations");

        Location[] locationsAfterMigration = m_locationsManager.getLocations();
        assertEquals(2, locationsAfterMigration.length);
        assertEquals("https://localhost:8092/RPC2", locationsAfterMigration[0].getProcessMonitorUrl());
        assertEquals("localhost", locationsAfterMigration[0].getFqdn());
        assertEquals("https://192.168.0.27:8092/RPC2", locationsAfterMigration[1].getProcessMonitorUrl());
        assertEquals("192.168.0.27", locationsAfterMigration[1].getFqdn());

        assertFalse(testTopologyFile.exists());
    }

    public void testOnInitTaskExistingLocationsInDatabase() throws Exception {
        loadDataSetXml("admin/commserver/seedLocations.xml");
        Location[] locationsBeforeMigration = m_locationsManager.getLocations();
        assertEquals(2, locationsBeforeMigration.length);

        m_out.setConfigDirectory(getConfigDirectory());
        m_out.onInitTask("migrate_locations");

        Location[] locationsAfterMigration = m_locationsManager.getLocations();
        assertEquals(2, locationsAfterMigration.length);
        assertEquals("https://localhost:8092/RPC2", locationsAfterMigration[0].getProcessMonitorUrl());
        assertEquals("localhost", locationsAfterMigration[0].getFqdn());
        assertEquals("https://remotehost.example.com:8092/RPC2", locationsAfterMigration[1].getProcessMonitorUrl());
        assertEquals("remotehost.example.com", locationsAfterMigration[1].getFqdn());
    }

    public void testOnInitTaskWithMissingTopologyFile() throws Exception {
        loadDataSetXml("admin/commserver/clearLocations.xml");
        Location[] locationsBeforeMigration = m_locationsManager.getLocations();
        assertEquals(0, locationsBeforeMigration.length);

        // skip step of creating topology file

        m_out.setConfigDirectory(getConfigDirectory());
        m_out.setLocalIpAddress("192.168.1.2");
        m_out.setHostname("my.full.hostname");
        m_out.onInitTask("migrate_locations");

        Location[] locationsAfterMigration = m_locationsManager.getLocations();
        assertEquals(1, locationsAfterMigration.length);
        assertEquals("https://my.full.hostname:8092/RPC2", locationsAfterMigration[0].getProcessMonitorUrl());
    }

    public void setLocationsMigrationTrigger(LocationsMigrationTrigger locationMigrationTrigger) {
        m_out = locationMigrationTrigger;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    private String getConfigDirectory() {
        return new File(getClass().getResource(TOPOLOGY_TEST_XML_ORIG).getPath()).getParentFile().getAbsolutePath();
    }

    private File createTopologyFile() throws Exception {
        File origTopologyFile = new File(getConfigDirectory(), TOPOLOGY_TEST_XML_ORIG);
        File testTopologyFile = new File(getConfigDirectory(), TOPOLOGY_TEST_XML);

        InputStream in = new FileInputStream(origTopologyFile);
        OutputStream out = new FileOutputStream(testTopologyFile);

        byte[] buffer = new byte[1024];
        int len;
        while ((len = in.read(buffer)) > 0) {
            out.write(buffer, 0, len);
        }

        in.close();
        out.close();

        return testTopologyFile;
    }
}

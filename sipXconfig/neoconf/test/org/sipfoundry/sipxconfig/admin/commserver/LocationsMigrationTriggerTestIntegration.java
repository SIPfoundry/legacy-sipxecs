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
import java.util.Arrays;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class LocationsMigrationTriggerTestIntegration extends IntegrationTestCase {

    private static final String SIPXCONFIG_HOSTNAME = "sipxconfig.hostname";
    private static final String TOPOLOGY_TEST_XML = "topology.test.xml";
    private static final String TOPOLOGY_TEST_XML_ORIG = "topology.test.xml.orig";
    private LocationsMigrationTrigger m_out;
    private LocationsManager m_locationsManager;

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();

        System.setProperty(SIPXCONFIG_HOSTNAME, "sipx.example.org");

        m_out = new LocationsMigrationTrigger();
        m_out.setConfigDirectory(getConfigDirectory());
        m_out.setLocationsManager(m_locationsManager);
        m_out.setTopologyFilename("topology.xml");
        m_out.setNetworkPropertiesFilename("sipxconfig-netif");
        m_out.setTaskNames(Arrays.asList(new String[] {"migrate_locations"}));
    }

    @Override
    protected void onTearDownInTransaction() throws Exception {
        super.onTearDownInTransaction();

        System.clearProperty(SIPXCONFIG_HOSTNAME);
    }

    public void testOnInitTaskUpgrade() throws Exception {
        loadDataSetXml("admin/commserver/clearLocations.xml");
        Location[] locationsBeforeMigration = m_locationsManager.getLocations();
        assertEquals(0, locationsBeforeMigration.length);

        File testTopologyFile = createTopologyFile();

        m_out.setTopologyFilename(TOPOLOGY_TEST_XML);
        m_out.onInitTask("migrate_locations");

        Location[] locationsAfterMigration = m_locationsManager.getLocations();
        assertEquals(2, locationsAfterMigration.length);
        boolean foundPrimary = false;
        boolean foundSecondary = false;
        for (Location location : locationsAfterMigration) {
            if (location.isPrimary()) {
                assertEquals("https://localhost:8092/RPC2", location.getProcessMonitorUrl());
                assertEquals("localhost", location.getFqdn());
                assertFalse(StringUtils.isEmpty(location.getAddress()));
                assertTrue(location.getAddress().matches("([\\d]{1,3}\\.){3}[\\d]{1,3}"));
                foundPrimary = true;
            } else {
                assertEquals("https://192.168.0.27:8092/RPC2", location.getProcessMonitorUrl());
                assertEquals("192.168.0.27", location.getFqdn());
                foundSecondary = true;
            }
        }

        assertTrue("Primary location not found.", foundPrimary);
        assertTrue("Secondary location not found.", foundSecondary);
        assertFalse("Topology file not deleted.", testTopologyFile.exists());
    }

    public void testOnInitTaskExistingLocationsInDatabase() throws Exception {
        loadDataSetXml("admin/commserver/seedLocationsAndServices2.xml");
        Location[] locationsBeforeMigration = m_locationsManager.getLocations();
        assertEquals(2, locationsBeforeMigration.length);

        m_out.onInitTask("migrate_locations");

        Location[] locationsAfterMigration = m_locationsManager.getLocations();
        assertEquals(2, locationsAfterMigration.length);
        assertEquals("https://localhost:8092/RPC2", locationsAfterMigration[0].getProcessMonitorUrl());
        assertEquals("localhost", locationsAfterMigration[0].getFqdn());
        //Localhost should be primary location
        assertTrue(locationsAfterMigration[0].isPrimary());

        assertEquals("https://remotehost.example.org:8092/RPC2", locationsAfterMigration[1].getProcessMonitorUrl());
        assertEquals("remotehost.example.org", locationsAfterMigration[1].getFqdn());
        //all other hosts should be secondary
        assertFalse(locationsAfterMigration[1].isPrimary());
    }

    public void testOnInitTaskNewInstallation() throws Exception {
        loadDataSetXml("admin/commserver/clearLocations.xml");
        Location[] locationsBeforeMigration = m_locationsManager.getLocations();
        assertEquals(0, locationsBeforeMigration.length);

        m_out.setNetworkPropertiesFilename("sipxconfig-netif-test");
        m_out.onInitTask("migrate_locations");

        Location primaryLocation = m_locationsManager.getPrimaryLocation();
        assertNotNull(primaryLocation);
        assertEquals("https://sipx.example.org:8092/RPC2", primaryLocation.getProcessMonitorUrl());
        assertEquals("192.168.87.11", primaryLocation.getAddress());
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

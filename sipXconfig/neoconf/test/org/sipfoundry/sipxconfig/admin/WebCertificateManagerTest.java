/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.Properties;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class WebCertificateManagerTest extends TestCase {

    private WebCertificateManagerImpl m_manager;
    private LocationsManager m_locationsManager;
    private Location m_primaryLocation;

    @Override
    protected void setUp() {
        m_manager = new WebCertificateManagerImpl();
        m_manager.setCertDirectory(TestUtil.getTestSourceDirectory(this.getClass()));
        m_manager.setSslDirectory(TestUtil.getTestSourceDirectory(this.getClass()));

        m_primaryLocation = TestUtil.createDefaultLocation();

        m_locationsManager = EasyMock.createMock(LocationsManager.class);
        m_locationsManager.getPrimaryLocation();
        EasyMock.expectLastCall().andReturn(m_primaryLocation).anyTimes();
        EasyMock.replay(m_locationsManager);
        m_manager.setLocationsManager(m_locationsManager);
    }

    public void testWriteAndLoadCertPropertiesFile() {
        // write properties to file
        Properties prop1 = new Properties();
        prop1.put("countryName", "US");
        prop1.put("stateOrProvinceName", "Texas");
        prop1.put("localityName", "Austin");
        prop1.put("organizationName", "Test Organization");
        prop1.put("serverEmail", "test@test.org");
        m_manager.writeCertPropertiesFile(prop1);

        // load properties from file
        Properties prop2 = m_manager.loadCertPropertiesFile();
        assertEquals("US", prop2.get("countryName"));
        assertEquals("Texas", prop2.get("stateOrProvinceName"));
        assertEquals("Austin", prop2.get("localityName"));
        assertEquals("Test Organization", prop2.get("organizationName"));
        assertEquals("test@test.org", prop2.get("serverEmail"));

        // cleanup
        File propertiesFile = new File(TestUtil.getTestSourceDirectory(this.getClass()), "webCert.properties");
        assertTrue(propertiesFile.delete());
    }

    public void testGetCRTFilePath() {
        assertEquals(TestUtil.getTestSourceDirectory(this.getClass()) + File.separator
                + m_locationsManager.getPrimaryLocation().getFqdn() + "-web.crt", m_manager.getCRTFile().getPath());
    }

    public void testWriteCRTFile() throws Exception {
        String certificate = new String("TEST");
        m_manager.writeCRTFile(certificate);
        BufferedReader reader = new BufferedReader(new FileReader(m_manager.getCRTFile()));
        String line = reader.readLine();
        assertEquals("TEST", line);
        assertNull(reader.readLine());
    }
}

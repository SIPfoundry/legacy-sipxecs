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
import java.util.Set;

import junit.framework.TestCase;

import org.apache.commons.io.FileUtils;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class CertificateManagerTest extends TestCase {

    private CertificateManagerImpl m_manager;
    private LocationsManager m_locationsManager;
    private Location m_primaryLocation;

    @Override
    protected void setUp() {
        m_manager = new CertificateManagerImpl();
        m_manager.setCertDirectory(TestUtil.getTestSourceDirectory(this.getClass()));
        m_manager.setSslDirectory(TestUtil.getTestSourceDirectory(this.getClass()));
        m_manager.setBinCertDirectory(TestUtil.getTestSourceDirectory(this.getClass()));
        m_manager.setSslAuthDirectory(TestUtil.getTestSourceDirectory(this.getClass())
                + File.separator + "testAuthorities");

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
        prop1.put("serverName", "example.org");
        m_manager.writeCertPropertiesFile(prop1);

        // load properties from file
        Properties prop2 = m_manager.loadCertPropertiesFile();
        assertEquals("US", prop2.get("countryName"));
        assertEquals("Texas", prop2.get("stateOrProvinceName"));
        assertEquals("Austin", prop2.get("localityName"));
        assertEquals("Test Organization", prop2.get("organizationName"));
        assertEquals("test@test.org", prop2.get("serverEmail"));
        assertEquals("example.org", prop2.get("serverName"));

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

    public void testValid() throws Exception {
        boolean valid = m_manager.validateCertificate(new File("invalidCA.crt"));
        assertFalse(valid);
        valid = m_manager.validateCertificate(new File("validCA.crt"));
        assertTrue(valid);
    }

    public void testShow() throws Exception {
        String text = m_manager.showCertificate(new File("validCA.crt"));
        assertEquals("VALID", text);
    }

    public void testListCertificates() throws Exception {
        Set<CertificateDecorator> certs =  m_manager.listCertificates();
        assertEquals(1, certs.size());
        for (CertificateDecorator cert : certs) {
            assertEquals("valid1CA.crt", cert.getFileName());
        }
    }

    public void testSaveDeleteCert() throws Exception {
        File tmpCAFile = m_manager.getCATmpFile("validCA.crt");

        FileUtils.copyFile(new File(TestUtil.getTestSourceDirectory(this.getClass())
                + File.separator + "validCA.crt"), tmpCAFile);
        m_manager.copyCRTAuthority();
        m_manager.deleteCRTAuthorityTmpDirectory();
        Set<CertificateDecorator> certs =  m_manager.listCertificates();
        assertEquals(2, certs.size());

        CertificateDecorator cert = new CertificateDecorator();
        cert.setFileName("validCA.crt");
        m_manager.deleteCA(cert);
        certs =  m_manager.listCertificates();
        assertEquals(1, certs.size());
    }
}

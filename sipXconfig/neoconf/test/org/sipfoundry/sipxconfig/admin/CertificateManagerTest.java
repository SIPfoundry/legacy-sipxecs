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
import java.util.Collection;
import java.util.Properties;
import java.util.Set;

import junit.framework.TestCase;

import org.apache.commons.io.FileUtils;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.service.SipxBridgeService;
import org.sipfoundry.sipxconfig.service.SipxConfigService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class CertificateManagerTest extends TestCase {

    private CertificateManagerImpl m_manager;
    private Location m_primaryLocation;

    @Override
    protected void setUp() {
        m_manager = new CertificateManagerImpl();

        m_manager.setCertDirectory(TestUtil.getTestSourceDirectory(this.getClass()));
        m_manager.setSslDirectory(TestUtil.getTestSourceDirectory(this.getClass()));
        m_manager.setBinCertDirectory(TestUtil.getTestSourceDirectory(this.getClass()));
        m_manager.setSslAuthDirectory(TestUtil.getTestSourceDirectory(this.getClass())
                + File.separator + "testAuthorities");
        m_manager.setLibExecDirectory(TestUtil.getTestSourceDirectory(this.getClass()));

        m_primaryLocation = TestUtil.createDefaultLocation();
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
                + m_primaryLocation.getFqdn() + "-web.crt", m_manager.getCRTFile(
                        m_primaryLocation.getFqdn()).getPath());
    }

    public void testWriteCRTFile() throws Exception {
        String certificate = new String("TEST");
        m_manager.setCertDirectory(TestUtil.getTestOutputDirectory("neoconf") + File.separator + "certs");
        m_manager.writeCRTFile(certificate, m_primaryLocation.getFqdn());
        compareFileContents(m_manager.getCRTFile(m_primaryLocation.getFqdn()), certificate);
    }

    public void testGetExternalCRTFilePath() {
        assertEquals(TestUtil.getTestSourceDirectory(this.getClass()) + File.separator
                + "external-key-based-web.crt", m_manager.getExternalCRTFile().getAbsolutePath());
    }

    public void testWriteExternalCRTFile() throws Exception {
        String certificate = new String("TEST");
        m_manager.setCertDirectory(TestUtil.getTestOutputDirectory("neoconf") + File.separator + "certs");
        m_manager.writeExternalCRTFile(certificate);
        compareFileContents(m_manager.getExternalCRTFile(), certificate);
    }

    public void testGetKeyFilePath() {
        assertEquals(TestUtil.getTestSourceDirectory(this.getClass()) + File.separator
                + m_primaryLocation.getFqdn() + "-web.key", m_manager.getKeyFile(
                        m_primaryLocation.getFqdn()).getPath());
    }

    public void testGetExternalKeyFilePath() {
        assertEquals(TestUtil.getTestSourceDirectory(this.getClass()) + File.separator
                + "external-key-based-web.key", m_manager.getExternalKeyFile().getAbsolutePath());
    }

    public void testWriteKeyFile() throws Exception {
        String key = new String("TESTKEY");
        m_manager.setCertDirectory(TestUtil.getTestOutputDirectory("neoconf") + File.separator + "certs");
        m_manager.writeKeyFile(key);
        compareFileContents(m_manager.getExternalKeyFile(), key);
    }

    public void testValid() throws Exception {
        boolean valid = m_manager.validateCertificate(new File("invalidCRT.crt"));
        assertFalse(valid);
        valid = m_manager.validateCertificate(new File("validCRT.crt"));
        assertTrue(valid);
    }

    public void testValidCA() throws Exception {
        boolean valid = m_manager.validateCertificateAuthority(new File("invalidCA.crt"));
        assertFalse(valid);
        valid = m_manager.validateCertificateAuthority(new File("validCA.crt"));
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
        File caFile = m_manager.getCAFile("validCA.crt");

        FileUtils.copyFile(new File(TestUtil.getTestSourceDirectory(this.getClass())
                + File.separator + "validCA.crt"), tmpCAFile);
        FileUtils.copyFile(tmpCAFile, caFile);
        m_manager.deleteCRTAuthorityTmpDirectory();
        Set<CertificateDecorator> certs =  m_manager.listCertificates();
        assertEquals(2, certs.size());

        CertificateDecorator cert = new CertificateDecorator();
        cert.setFileName("validCA.crt");
        m_manager.deleteCA(cert);
        certs =  m_manager.listCertificates();
        assertEquals(1, certs.size());
    }

    public void testImportKeyAndCertificate() throws Exception {
        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxConfigService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(new SipxConfigService()).atLeastOnce();
        sipxServiceManager.getServiceByBeanId(SipxBridgeService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(new SipxBridgeService()).atLeastOnce();

        SipxProcessContext processContext = EasyMock.createMock(SipxProcessContext.class);
        processContext.markServicesForRestart((Collection) EasyMock.anyObject());
        EasyMock.expectLastCall().atLeastOnce();

        m_manager.setSipxServiceManager(sipxServiceManager);
        m_manager.setProcessContext(processContext);

        EasyMock.replay(sipxServiceManager, processContext);

        String sslDirectory = TestUtil.getTestOutputDirectory("neoconf") + File.separator + "certs" +
            File.separator + "ssl";

        File sslWebCert = new File(sslDirectory + File.separator + "ssl-web.crt");
        File sslWebKey = new File(sslDirectory + File.separator + "ssl-web.key");

        (new File(sslDirectory)).mkdirs();
        sslWebCert.createNewFile();
        sslWebKey.createNewFile();

        m_manager.setSslDirectory(sslDirectory);
        m_manager.importKeyAndCertificate(m_primaryLocation.getFqdn(), true);
        compareFileContents(sslWebCert, "CSR_BASED_CERT");
        compareFileContents(sslWebKey, "CSR_BASED_KEY");

        m_manager.setSslDirectory(sslDirectory);
        m_manager.importKeyAndCertificate(m_primaryLocation.getAddress(), false);
        compareFileContents(sslWebCert, "EXTERNAL_KEY_BASED_CERT");
        compareFileContents(sslWebKey, "EXTERNAL_KEY_BASED_KEY");

        m_manager.setSslDirectory(sslDirectory);
        m_manager.importKeyAndCertificate("random.domain.org", true);
        compareFileContents(sslWebCert, "CSR_BASED_CERT_DIFFERNET_DOMAIN");
        compareFileContents(sslWebKey, "CSR_BASED_KEY_DIFFERNET_DOMAIN");

        m_manager.setSslDirectory(sslDirectory);
        m_manager.importKeyAndCertificate("random.domain.org", false);
        compareFileContents(sslWebCert, "EXTERNAL_KEY_BASED_CERT");
        compareFileContents(sslWebKey, "EXTERNAL_KEY_BASED_KEY");
    }

    private void compareFileContents(File file, String expectedOutput) throws Exception {
        BufferedReader reader = new BufferedReader(new FileReader(file));
        String line = reader.readLine();
        assertEquals(expectedOutput, line);
        assertNull(reader.readLine());
    }
}

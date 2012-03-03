/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import static org.easymock.classextension.EasyMock.replay;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.Properties;
import java.util.Set;

import junit.framework.TestCase;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpMethod;
import org.apache.commons.httpclient.URI;
import org.apache.commons.httpclient.URIException;
import org.apache.commons.io.FileUtils;
import org.apache.commons.lang.StringUtils;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigCommands;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class CertificateManagerTest extends TestCase {

    private CertificateManagerImpl m_manager;
    private Location m_primaryLocation;
    private static String DELETE = "DELETE";
    private String m_srcDir;

    @Override
    protected void setUp() {
        m_manager = createCertificateManager();
        m_srcDir = TestHelper.getResourceAsFile(getClass(), "check-cert.sh").getParent();
        m_manager.setCertDirectory(m_srcDir);
        m_manager.setSslDirectory(m_srcDir);
        m_manager.setBinCertDirectory(m_srcDir);
        m_manager.setSslAuthDirectory(m_srcDir + "/testAuthorities");
        m_manager.setLibExecDirectory(m_srcDir);
        m_primaryLocation = TestHelper.createDefaultLocation();
    }

    private CertificateManagerImpl createCertificateManager() {
        return new CertificateManagerImpl() {
            @Override
            protected HttpClient getNewHttpClient() {
                return new HttpClient() {
                    @Override
                    public int executeMethod(HttpMethod method) {
                        if (StringUtils.equals(method.getName(), DELETE)) {
                            try{
                                URI reqURI = method.getURI();
                                FileUtils.deleteQuietly(new File(reqURI.getPath()));
                            } catch (URIException uex) {
                                //do nothing
                            }
                            return 200;
                        }
                        else {
                            return 501;
                        }
                    }
                };
            }
        };
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
        File propertiesFile = TestHelper.getResourceAsFile(getClass(), "webCert.properties");
        assertTrue(propertiesFile.delete());
    }

    public void testGetCRTFilePath() {
        assertEquals(m_srcDir + File.separator
                + m_primaryLocation.getFqdn() + "-web.crt", m_manager.getCRTFile(
                        m_primaryLocation.getFqdn()).getPath());
    }

    public void testWriteCRTFile() throws Exception {
        String certificate = new String("TEST");
        m_manager.setCertDirectory(TestHelper.getTestOutputDirectory() + File.separator + "certs");
        m_manager.writeCRTFile(certificate, m_primaryLocation.getFqdn());
        compareFileContents(m_manager.getCRTFile(m_primaryLocation.getFqdn()), certificate);
    }

    public void testGetExternalCRTFilePath() {
        assertEquals(m_srcDir + File.separator
                + "external-key-based-web.crt", m_manager.getExternalCRTFile().getAbsolutePath());
    }

    public void testWriteExternalCRTFile() throws Exception {
        String certificate = new String("TEST");
        m_manager.setCertDirectory(TestHelper.getTestOutputDirectory() + File.separator + "certs");
        m_manager.writeExternalCRTFile(certificate);
        compareFileContents(m_manager.getExternalCRTFile(), certificate);
    }

    public void testGetKeyFilePath() {
        assertEquals(m_srcDir + File.separator
                + m_primaryLocation.getFqdn() + "-web.key", m_manager.getKeyFile(
                        m_primaryLocation.getFqdn()).getPath());
    }

    public void testGetExternalKeyFilePath() {
        assertEquals(m_srcDir + File.separator
                + "external-key-based-web.key", m_manager.getExternalKeyFile().getAbsolutePath());
    }

    public void testWriteKeyFile() throws Exception {
        String key = new String("TESTKEY");
        m_manager.setCertDirectory(TestHelper.getTestOutputDirectory() + File.separator + "certs");
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

        FileUtils.copyFile(new File(m_srcDir
                + File.separator + "validCA.crt"), tmpCAFile);
        FileUtils.copyFile(tmpCAFile, caFile);
        m_manager.deleteCRTAuthorityTmpDirectory();

        LocationsManager locationsManager = EasyMock.createMock(LocationsManager.class);
        locationsManager.getLocations();
        EasyMock.expectLastCall().andReturn(new Location[] {m_primaryLocation}).anyTimes();

        m_primaryLocation.setFqdn("example.com");
        m_manager.setLocationsManager(locationsManager);

        replay(locationsManager);

        Set<CertificateDecorator> certs =  m_manager.listCertificates();
        assertEquals(2, certs.size());

        CertificateDecorator cert = new CertificateDecorator();
        cert.setFileName("validCA.crt");
        m_manager.deleteCA(cert);
        certs =  m_manager.listCertificates();
        assertEquals(1, certs.size());
    }

    public void testImportKeyAndCertificate() throws Exception {
//        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
//        sipxServiceManager.getServiceByBeanId(SipxConfigService.BEAN_ID);
//        EasyMock.expectLastCall().andReturn(new SipxConfigService()).atLeastOnce();
//        sipxServiceManager.getServiceByBeanId(SipxBridgeService.BEAN_ID);
//        EasyMock.expectLastCall().andReturn(new SipxBridgeService()).atLeastOnce();
//        sipxServiceManager.getServiceByBeanId(SipxIvrService.BEAN_ID);
//        EasyMock.expectLastCall().andReturn(new SipxIvrService()).atLeastOnce();
//        sipxServiceManager.getServiceByBeanId(SipxRecordingService.BEAN_ID);
//        EasyMock.expectLastCall().andReturn(new SipxRecordingService()).atLeastOnce();
//        sipxServiceManager.getServiceByBeanId(SipxImbotService.BEAN_ID);
//        EasyMock.expectLastCall().andReturn(new SipxImbotService()).atLeastOnce();
//        sipxServiceManager.getServiceByBeanId("sipxOpenfireService");
//        EasyMock.expectLastCall().andReturn(new SipxService() {}).atLeastOnce();
//        sipxServiceManager.getServiceByBeanId(SipxProvisionService.BEAN_ID);
//        EasyMock.expectLastCall().andReturn(new SipxProvisionService()).atLeastOnce();
//        sipxServiceManager.getServiceByBeanId(SipxRestService.BEAN_ID);
//        EasyMock.expectLastCall().andReturn(new SipxRestService()).atLeastOnce();
//
        LocationsManager locationsManager = EasyMock.createMock(LocationsManager.class);
        locationsManager.getPrimaryLocation();
        EasyMock.expectLastCall().andReturn(m_primaryLocation).atLeastOnce();
        
        m_manager.setConfigCommands(org.easymock.classextension.EasyMock.createNiceMock(ConfigCommands.class));
//
//        SipxProcessContext processContext = EasyMock.createMock(SipxProcessContext.class);
//        processContext.markServicesForRestart(EasyMock.eq(m_primaryLocation), (Collection) EasyMock.anyObject());
//        EasyMock.expectLastCall().atLeastOnce();
//
//        m_manager.setSipxServiceManager(sipxServiceManager);
//        m_manager.setProcessContext(processContext);
        m_manager.setLocationsManager(locationsManager);
//
//        EasyMock.replay(sipxServiceManager, locationsManager, processContext);

        String sslDirectory = TestHelper.getTestOutputDirectory() + File.separator + "certs" +
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

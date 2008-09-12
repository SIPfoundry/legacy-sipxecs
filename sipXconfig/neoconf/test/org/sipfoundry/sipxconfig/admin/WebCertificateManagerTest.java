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

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxServer;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainConfiguration;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.domain.DomainManagerImpl;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class WebCertificateManagerTest extends TestCase {

    private WebCertificateManagerImpl manager;

    private class DomainManagerMock extends DomainManagerImpl {
        public Domain getDomain() {
            return new DomainMock();
        }

        protected SipxServer getServer() {
            return null;
        }

        protected DomainConfiguration createDomainConfiguration() {
            return null;
        }

        protected SipxReplicationContext getReplicationContext() {
            return null;
        }

        protected SipxServiceManager getSipxServiceManager() {
            return null;
        }
    }

    private class DomainMock extends Domain {
        public String getName() {
            return "test.org";
        }
    }

    protected void setUp() {
        manager = new WebCertificateManagerImpl();
        manager.setCertDirectory(TestUtil.getTestSourceDirectory(this.getClass()));
        manager.setSslDirectory(TestUtil.getTestSourceDirectory(this.getClass()));
        DomainManager domainManager = new DomainManagerMock();
        manager.setDomainManager(domainManager);
    }

    public void testWriteAndLoadCertPropertiesFile() {
        //write properties to file
        Properties prop1 = new Properties();
        prop1.put("countryName", "US");
        prop1.put("stateOrProvinceName", "Texas");
        prop1.put("localityName", "Austin");
        prop1.put("organizationName", "Test Organization");
        prop1.put("serverEmail", "test@test.org");
        manager.writeCertPropertiesFile(prop1);

        //load properties from file
        Properties prop2 = manager.loadCertPropertiesFile();
        assertEquals("US", prop2.get("countryName"));
        assertEquals("Texas", prop2.get("stateOrProvinceName"));
        assertEquals("Austin", prop2.get("localityName"));
        assertEquals("Test Organization", prop2.get("organizationName"));
        assertEquals("test@test.org", prop2.get("serverEmail"));
    }

    public void testGetDomainName() {
        assertEquals("test.org", manager.getDomainName());
    }

    public void testGetCRTFilePath() {
        assertEquals(TestUtil.getTestSourceDirectory(this.getClass())+ File.separator + manager.getDomainName() + "-web.crt", manager.getCRTFilePath());
    }

    public void testWriteCRTFile() {
        try {
            String certificate = new String("TEST");
            manager.writeCRTFile(certificate);
            BufferedReader reader = new BufferedReader(new FileReader(manager.getCRTFilePath()));
            String line = reader.readLine();
            assertEquals("TEST", line);
            assertNull(reader.readLine());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}

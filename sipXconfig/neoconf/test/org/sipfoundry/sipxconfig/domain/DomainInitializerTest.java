/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import java.io.IOException;

import junit.framework.TestCase;

import org.apache.commons.codec.binary.Base64;
import org.sipfoundry.sipxconfig.common.InitializationTask;

public class DomainInitializerTest extends TestCase {
    private DomainInitializer m_domainInitializer;
    
    protected void setUp() {
        m_domainInitializer = new DomainInitializer();
    }

    public void testGetLocalFQDN() {
        assertNotNull(m_domainInitializer.getInitialDomainName());
        m_domainInitializer.setInitialDomain("bluejay");
        assertEquals("bluejay", m_domainInitializer.getInitialDomainName());
    }
    
    public void testOnInitTaskInitializeDomain() throws IOException {
        DomainManager mockDomainManager = new MockDomainManager();
        
        m_domainInitializer.setInitialDomain("sparrow");
        m_domainInitializer.setDomainManager(mockDomainManager);
        InitializationTask initTask = new InitializationTask("initialize-domain");
        m_domainInitializer.onApplicationEvent(initTask);

        assertEquals("sparrow", mockDomainManager.getDomain().getName());
    }
    
    public void testOnInitTaskInitializeDomainSecret() throws IOException {
        DomainManager mockDomainManager = new MockDomainManager();
        
        m_domainInitializer.setInitialDomain("sparrow");
        m_domainInitializer.setDomainManager(mockDomainManager);
        InitializationTask initTask = new InitializationTask("initialize-domain-secret");
        m_domainInitializer.onApplicationEvent(initTask);

        assertEquals("sparrow", mockDomainManager.getDomain().getName());
        
        String sharedSecret = mockDomainManager.getDomain().getSharedSecret();
        byte[] secretBytes = new Base64().decode(sharedSecret.getBytes());
        assertEquals(18, secretBytes.length);
        
        // test that on a subsequent call, after domain is originally saved, we
        // we don't regenerate the secret
        m_domainInitializer.onApplicationEvent(initTask);
        assertEquals(sharedSecret, mockDomainManager.getDomain().getSharedSecret());
    }
    
    private static class MockDomainManager extends DomainManagerImpl {
        private Domain m_domain;
        public Domain getDomain() {
            if (m_domain == null) {
                throw new DomainNotInitializedException();
            }
            return m_domain;
        }
        public void saveDomain(Domain domain) {
            m_domain = domain;
        }
        public boolean isDomainInitialized() {
            return m_domain != null;
        }
    }
}

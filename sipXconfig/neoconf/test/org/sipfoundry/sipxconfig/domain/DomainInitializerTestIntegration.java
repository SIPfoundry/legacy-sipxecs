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

import org.apache.commons.codec.binary.Base64;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.common.InitializationTask;

public class DomainInitializerTestIntegration extends IntegrationTestCase {
    private DomainInitializer m_domainInitializer;
    
    private DomainManager m_domainManager;
    
    public void setDomainInitializer(DomainInitializer domainInitializer) {
        m_domainInitializer = domainInitializer;
    }    
    
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void testOnInitTaskInitializeDomain() throws Exception {
        loadDataSetXml("domain/NoDomainSeed.xml");        
        InitializationTask initTask = new InitializationTask("initialize-domain");
        m_domainInitializer.onApplicationEvent(initTask);

        assertEquals("example.org", m_domainManager.getDomain().getName());
    }

    public void testOnInitTaskInitializeDomainSecret() throws Exception {
        loadDataSet("domain/missing-domain-secret.db.xml");        
        InitializationTask initTask = new InitializationTask("initialize-domain-secret");
        m_domainInitializer.onApplicationEvent(initTask);

        Domain domain = m_domainManager.getDomain();
        assertEquals("example.org", domain.getName());
        assertEquals(new Integer(2000), domain.getId());

        String sharedSecret = domain.getSharedSecret();
        byte[] secretBytes = new Base64().decode(sharedSecret.getBytes());
        assertEquals(18, secretBytes.length);

        // test that on a subsequent call, after domain is originally saved, we
        // we don't regenerate the secret
        m_domainInitializer.onApplicationEvent(initTask);
        assertEquals(sharedSecret, m_domainManager.getDomain().getSharedSecret());
    }
}

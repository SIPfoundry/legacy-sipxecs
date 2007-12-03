/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.util.Set;

import org.apache.commons.codec.binary.Base64;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;

// FIXME: it only test domain manager initialization for now
public class FirstRunTaskTestIntegration extends IntegrationTestCase {
    private FirstRunTask m_firstRunTask;

    private DomainManager m_domainManager;

    public void setFirstRun(FirstRunTask firstRunTask) {
        m_firstRunTask = firstRunTask;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void testOnInitTaskInitializeDomain() throws Exception {
        loadDataSetXml("domain/NoDomainSeed.xml");
        // InitializationTask initTask = new InitializationTask("first-run");
        // m_firstRunTask.onApplicationEvent(initTask);
        m_domainManager.initialize();

        assertEquals("example.org", m_domainManager.getDomain().getName());
        Set<String> aliases = m_domainManager.getDomain().getAliases();
        assertEquals(1, aliases.size());
        assertTrue(aliases.contains("alias.example.org"));
    }

    public void testOnInitTaskInitializeDomainSecret() throws Exception {
        loadDataSet("domain/missing-domain-secret.db.xml");
        // InitializationTask initTask = new InitializationTask("first-run");
        // m_firstRunTask.onApplicationEvent(initTask);
        m_domainManager.initialize();

        Domain domain = m_domainManager.getDomain();
        assertEquals("example.org", domain.getName());
        assertEquals(new Integer(2000), domain.getId());

        String sharedSecret = domain.getSharedSecret();
        byte[] secretBytes = new Base64().decode(sharedSecret.getBytes());
        assertEquals(18, secretBytes.length);

        // test that on a subsequent call, after domain is originally saved, we
        // we don't regenerate the secret
        // m_firstRunTask.onApplicationEvent(initTask);
        m_domainManager.initialize();
        assertEquals(sharedSecret, m_domainManager.getDomain().getSharedSecret());
    }
}

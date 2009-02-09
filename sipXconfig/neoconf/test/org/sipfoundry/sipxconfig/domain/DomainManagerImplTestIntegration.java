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

import org.dbunit.Assertion;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.domain.DomainManager.DomainNotInitializedException;

public class DomainManagerImplTestIntegration extends IntegrationTestCase {

    private DomainManager m_out;

    public void testGetEmptyDomain() throws Exception {
        try {
            m_out.getDomain();
            fail();
        } catch (DomainNotInitializedException expected) {
            assertTrue(true);
        }
    }

    public void testGetDomain() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        Domain d = m_out.getDomain();
        assertNotNull(d);
    }

    public void testSaveNewDomain() throws Exception {
        Domain d = new Domain();
        d.setName("robin");
        d.setSipRealm("realm");
        d.setSharedSecret("secret");
        m_out.saveDomain(d);
        ReplacementDataSet ds = loadReplaceableDataSetFlat("domain/DomainUpdateExpected.xml");
        ds.addReplacementObject("[domain_id]", d.getId());
        ITable actual = ds.getTable("domain");
        ITable expected = getConnection().createDataSet().getTable("domain");
        Assertion.assertEquals(expected, actual);
    }

    public void testUpdateDomain() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        Domain domain = m_out.getDomain();
        domain.setName("robin");
        domain.setSipRealm("realm");
        domain.setSharedSecret("secret");
        m_out.saveDomain(domain);

        ReplacementDataSet ds = loadReplaceableDataSetFlat("domain/DomainUpdateExpected.xml");
        ds.addReplacementObject("[domain_id]", domain.getId());
        ITable actual = ds.getTable("domain");
        ITable expected = getConnection().createDataSet().getTable("domain");
        Assertion.assertEquals(expected, actual);
    }

    public void testInitializeDomain() throws Exception {
        String domainConfigFilename = DomainManagerImplTestIntegration.class.getResource(
                "initial-domain-config").getFile();
        m_out.setDomainConfigFilename(domainConfigFilename);
        m_out.initializeDomain();

        Domain domain = m_out.getDomain();
        assertNotNull(domain);
        
        assertEquals("domain.example.org", domain.getName());
        assertEquals("realm.example.org", domain.getSipRealm());
        assertNotNull(domain.getSharedSecret());
    }

    public void setDomainManager(DomainManager domainManager) {
        m_out = domainManager;
    }
}

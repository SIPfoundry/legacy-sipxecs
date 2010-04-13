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
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.DomainManager.DomainNotInitializedException;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class DomainManagerImplTestIntegration extends IntegrationTestCase {

    private DomainManager m_out;
    private DomainManagerImpl m_domainManagerImpl;
    private LocationsManager m_originalLocationsManager;

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();

        LocationsManager locationsManager = EasyMock.createMock(LocationsManager.class);
        Location primaryLocation = TestUtil.createDefaultLocation();
        locationsManager.getPrimaryLocation();
        EasyMock.expectLastCall().andReturn(primaryLocation).anyTimes();
        locationsManager.getLocations();
        EasyMock.expectLastCall().andReturn(new Location[]{primaryLocation}).anyTimes();
        EasyMock.replay(locationsManager);

        modifyContext(m_domainManagerImpl, "locationsManager", m_originalLocationsManager, locationsManager);
    }

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
        assertNotNull(domain.getAliases());
        assertEquals("alias.example.org", domain.getAliases().iterator().next());
        assertFalse(domain.getAliases().contains("domain.example.org"));
        assertNotNull(domain.getSharedSecret());
    }

    public void setDomainManager(DomainManager domainManager) {
        m_out = domainManager;
    }

    public void setDomainManagerImpl(DomainManagerImpl domainManagerImpl) {
        m_domainManagerImpl = domainManagerImpl;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_originalLocationsManager = locationsManager;
    }
}

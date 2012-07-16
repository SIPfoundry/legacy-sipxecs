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
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.DomainManager.DomainNotInitializedException;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class DomainManagerImplTestIntegration extends IntegrationTestCase {

    private DomainManager m_out;
    private DomainManagerImpl m_domainManagerImpl;
    private LocationsManager m_originalLocationsManager;

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        clear();
        LocationsManager locationsManager = EasyMock.createMock(LocationsManager.class);
        Location primaryLocation = TestHelper.createDefaultLocation();
        locationsManager.getPrimaryLocation();
        EasyMock.expectLastCall().andReturn(primaryLocation).anyTimes();
        locationsManager.getLocations();
        EasyMock.expectLastCall().andReturn(new Location[]{primaryLocation}).anyTimes();
        EasyMock.replay(locationsManager);
        modifyContext(m_domainManagerImpl, "locationsManager", m_originalLocationsManager, locationsManager);
        m_out.setNullDomain();  // forces domain to reload from db on next call to getDomain
    }
    
    @Override
    protected void onTearDownAfterTransaction() throws Exception {
        // TODO Auto-generated method stub
        super.onTearDownAfterTransaction();
        m_out.setNullDomain();  // forces domain to reload from db on next call to getDomain
    }
    
    public void testChangeName() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        m_domainManagerImpl.changeDomainName("changeme.example.org");
        Domain d = m_out.getDomain();
        assertEquals("changeme.example.org", d.getName());
    }

    public void testGetDomain() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        Domain d = m_out.getDomain();
        assertNotNull(d);
    }

    public void testSaveNewDomain() throws Exception {
        Domain d = new Domain();
        try {
            m_out.saveDomain(d);
            fail("Should not be able to edit domain");
        } catch (IllegalStateException e) {
            assertTrue(true);
        }
        
        d = m_out.getDomain();
        try {
            m_out.saveDomain(d);
            fail("Should not be able to edit domain");
        } catch (IllegalStateException e) {
            assertTrue(true);
        }       
        
        d = m_out.getEditableDomain();
        d.setName("robin");
        d.setSipRealm("realm");
        d.setSharedSecret("secret");
        flush();
        d.setName("robin");
        d.setSipRealm("realm");
        d.setSharedSecret("secret");        
        m_out.saveDomain(d);
        d = m_out.getDomain();
        flush();
        
        ReplacementDataSet ds = loadReplaceableDataSetFlat("domain/DomainUpdateExpected.xml");
        ds.addReplacementObject("[domain_id]", d.getId());
        ITable actual = ds.getTable("domain");
        ITable expected = getConnection().createDataSet().getTable("domain");
        Assertion.assertEquals(expected, actual);
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

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

import junit.framework.TestCase;

import org.dbunit.Assertion;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.domain.DomainManager.DomainNotInitializedException;
import org.springframework.context.ApplicationContext;

public class DomainManagerTestDb extends TestCase {
    
    private DomainManager m_context;

    private ApplicationContext m_appContext;

    protected void setUp() throws Exception {
        m_appContext = TestHelper.getApplicationContext();
        m_context = (DomainManager) m_appContext.getBean(DomainManager.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");        
    }
    
    public void testGetDomain() throws Exception {
        Domain d = m_context.getDomain();
        assertNotNull(d);        
    }
    
    public void testGetEmptyDomain() throws Exception {
        TestHelper.cleanInsert("domain/NoDomainSeed.xml");        
        try {
            m_context.getDomain();
            fail();
        } catch (DomainNotInitializedException expected) {
            assertTrue(true);
        }
    }    

    public void testSaveNewDomain() throws Exception {
        Domain d = new Domain();
        d.setName("robin");
        m_context.saveDomain(d);
        ReplacementDataSet ds = TestHelper.loadReplaceableDataSetFlat("domain/DomainUpdateExpected.xml");
        ds.addReplacementObject("[domain_id]", d.getId());
        ITable actual = ds.getTable("domain");
        ITable expected = TestHelper.getConnection().createDataSet().getTable("domain");
        Assertion.assertEquals(expected, actual);
    }

    public void testUpdateDomain() throws Exception {
        Domain domain = m_context.getDomain();
        domain.setName("robin");
        m_context.saveDomain(domain);        
        
        ReplacementDataSet ds = TestHelper.loadReplaceableDataSetFlat("domain/DomainUpdateExpected.xml");
        ds.addReplacementObject("[domain_id]", domain.getId());
        ITable actual = ds.getTable("domain");
        ITable expected = TestHelper.getConnection().createDataSet().getTable("domain");
        Assertion.assertEquals(expected, actual);
    }
}

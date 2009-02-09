/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.admin.localization;

import org.apache.commons.beanutils.BeanUtils;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class LocalizationContextTestIntegration extends IntegrationTestCase {

    // Object Under Test
    private LocalizationContext m_out;
    private LocalizationContextImpl m_localizationContextImpl;
    private DomainManager m_originalDomainManager;
    private DomainManager m_mockDomainManager;

    
    
    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        
        m_mockDomainManager = EasyMock.createMock(DomainManager.class);
        m_mockDomainManager.replicateDomainConfig();
        EasyMock.expectLastCall().once();
        EasyMock.replay(m_mockDomainManager);
        
        m_localizationContextImpl.setDomainManager(m_mockDomainManager);
    }

    @Override
    protected void onTearDownInTransaction() throws Exception {
        super.onTearDownInTransaction();
        
        m_localizationContextImpl.setDomainManager(m_originalDomainManager);
    }

    public void testUpdateRegion() throws Exception {
        assertEquals(-1, m_out.updateRegion("PL"));
    }

    public void testUpdateLanguage() throws Exception {
        assertEquals(1, m_out.updateLanguage("stdprompts_pl"));
        flush();
        assertEquals(1, getConnection().getRowCount("localization", "where language = 'pl'"));
        
        EasyMock.verify(m_mockDomainManager);
    }

    public void testDefaults() {
        assertEquals("na", m_out.getCurrentRegionId());
        assertEquals("en", m_out.getCurrentLanguage());
    }

    public void setLocalizationContext(LocalizationContext localizationContext) {
        m_out = localizationContext;
    }
    
    public void setLocalizationContextImpl(LocalizationContextImpl localizationContextImpl) {
        m_localizationContextImpl = localizationContextImpl;
    }
    
    public void setDomainManager(DomainManager domainManager) {
        m_originalDomainManager = domainManager;
    }
}

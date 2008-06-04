/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class SipxRegistrarServiceInitializerTestIntegration extends IntegrationTestCase {

    private SipxRegistrarServiceInitializer m_out;
    private SipxServiceManager m_sipxServiceManager;

    public void testOnInitTask() throws Exception {
        SipxService registrarService = new SipxRegistrarService();
        registrarService.setSettings(TestHelper.loadSettings("sipxregistrar/sipxregistrar.xml"));
        registrarService.setFullHostname("sipx.example.org");
        registrarService.setIpAddress("1.1.1.1");
        registrarService.getSettings().getSetting("domain/SIP_REGISTRAR_DOMAIN_ALIASES").setValue(null);
        
        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxRegistrarService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(registrarService).anyTimes();
        sipxServiceManager.storeService(registrarService);
        EasyMock.replay(sipxServiceManager);
        m_out.setSipxServiceManager(sipxServiceManager);
        
        // provide a mock domain manager to give a predefined alias
        Domain domain = new Domain();
        domain.addAlias("alias.example.org");
        DomainManager domainManager = EasyMock.createStrictMock(DomainManager.class);
        domainManager.getDomain();
        EasyMock.expectLastCall().andReturn(domain).anyTimes();
        EasyMock.replay(domainManager);
        m_out.setDomainManager(domainManager);
        
        m_out.onInitTask("initialize_sipx_registrar_service");
        String domainAliasesAfterInitialization = registrarService.getSettingValue("domain/SIP_REGISTRAR_DOMAIN_ALIASES");
        assertEquals("sipx.example.org 1.1.1.1 alias.example.org", domainAliasesAfterInitialization);
        
        EasyMock.verify(sipxServiceManager);
    }
    
    public void setSipxRegistrarInitializer(SipxRegistrarServiceInitializer initializer) {
        m_out = initializer;
    }
}

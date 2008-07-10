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

import java.util.Arrays;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxRegistrarServiceTest extends TestCase {

    private static final String SIP_REGISTRAR_DOMAIN_ALIASES = "domain/SIP_REGISTRAR_DOMAIN_ALIASES";
    private SipxRegistrarService m_out;
    
    public void setUp() throws Exception {
        m_out = new SipxRegistrarService();
        m_out.setModelDir("sipxregistrar");
        m_out.setModelName("sipxregistrar.xml");
        m_out.setModelFilesContext(TestHelper.getModelFilesContext());
    }
    
    public void testSetFullHostname() {
        m_out.setFullHostname("sipx.example.org");
        String domainAliasesSettingValue = m_out.getSettings().getSetting(SIP_REGISTRAR_DOMAIN_ALIASES).getValue();
        assertEquals("sipx.example.org", domainAliasesSettingValue);
    }
    
    public void testSetIpAddress() {
        m_out.setIpAddress("2.2.2.2");
        String domainAliasesSettingValue = m_out.getSettings().getSetting(SIP_REGISTRAR_DOMAIN_ALIASES).getValue();
        assertEquals("2.2.2.2", domainAliasesSettingValue);
    }
    
    public void testSetFullHostnameAndIpAddress() {
        m_out.setFullHostname("sipx.example.org");
        m_out.setIpAddress("2.2.2.2");
        String domainAliasesSettingValue = m_out.getSettings().getSetting(SIP_REGISTRAR_DOMAIN_ALIASES).getValue();
        assertEquals("sipx.example.org 2.2.2.2", domainAliasesSettingValue);
        
        m_out.setRegistrarDomainAliases(Arrays.asList(new String[] {"my.example.org", "another.example.org"}));
        domainAliasesSettingValue = m_out.getSettings().getSetting(SIP_REGISTRAR_DOMAIN_ALIASES).getValue();
        assertEquals("sipx.example.org 2.2.2.2 my.example.org another.example.org", domainAliasesSettingValue);
    }
    
    public void testSetFullHostnameAndIpAddressBeforeSettingsLoaded() {
        SipxRegistrarService registrarService = new SipxRegistrarService();
        registrarService.setFullHostname("sipx.example.org");
        registrarService.setIpAddress("3.3.3.3");
        registrarService.setModelDir("sipxregistrar");
        registrarService.setModelName("sipxregistrar.xml");
        registrarService.setModelFilesContext(TestHelper.getModelFilesContext());
        
        registrarService.setRegistrarDomainAliases(Arrays.asList(new String[] {"my.example.org", "another.example.org"}));
        String domainAliasesSettingValue = registrarService.getSettings().getSetting(SIP_REGISTRAR_DOMAIN_ALIASES).getValue();
        assertEquals("sipx.example.org 3.3.3.3 my.example.org another.example.org", domainAliasesSettingValue);
    }
    
    public void testSetRegistrarDomainAliases() {
        m_out.setFullHostname("sipx.example.org");
        m_out.setIpAddress("1.1.1.1");
        m_out.setRegistrarDomainAliases(Arrays.asList(new String[] {"my.example.org", "another.example.org"}));
        String domainAliasesSettingValue = m_out.getSettings().getSetting(SIP_REGISTRAR_DOMAIN_ALIASES).getValue();
        assertEquals("sipx.example.org 1.1.1.1 my.example.org another.example.org", domainAliasesSettingValue);
        
        m_out.setRegistrarDomainAliases(Arrays.asList(new String[] {"yours.example.org"}));
        domainAliasesSettingValue = m_out.getSettings().getSetting(SIP_REGISTRAR_DOMAIN_ALIASES).getValue();
        assertEquals("sipx.example.org 1.1.1.1 yours.example.org", domainAliasesSettingValue);
    }
    
    public void testValidateDuplicateCodes() {
        SipxService presenceService = new SipxPresenceService();
        presenceService.setModelDir("sipxpresence");
        presenceService.setModelName("sipxpresence.xml");
        presenceService.setModelFilesContext(TestHelper.getModelFilesContext());
        
        SipxServiceManager serviceManager = EasyMock.createMock(SipxServiceManager.class);
        serviceManager.getServiceByBeanId(SipxPresenceService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(presenceService).anyTimes();
        EasyMock.replay(serviceManager);
        m_out.setSipxServiceManager(serviceManager);
        
        Setting registrarSettings = m_out.getSettings();
        Setting presenceSettings = presenceService.getSettings();
        
        presenceSettings.getSetting("presence-config/SIP_PRESENCE_SIGN_IN_CODE").setValue("*123");
        registrarSettings.getSetting("call-pick-up/SIP_REDIRECT.180-PICKUP.DIRECTED_CALL_PICKUP_CODE").setValue("*123");
        
        try {
            m_out.validate();
            fail("Expected validation exception due to duplicate codes.");
        } catch (UserException e) {
            // expected
        }
    }
}

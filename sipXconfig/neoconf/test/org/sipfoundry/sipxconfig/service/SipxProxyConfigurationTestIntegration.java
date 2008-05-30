/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxProxyConfigurationTestIntegration extends IntegrationTestCase {

    private SipxProxyService m_proxyService;

    public void testWrite() throws Exception {
        SipxProxyConfiguration out = new SipxProxyConfiguration();
        out.setVelocityEngine(TestHelper.getVelocityEngine());
        out.setTemplate("sipxproxy/sipXproxy-config.vm");
        
        Setting proxySettings = m_proxyService.getSettings().getSetting("proxy-configuration");
        proxySettings.getSetting("SIPX_PROXY_DEFAULT_EXPIRES").setValue("35");
        proxySettings.getSetting("SIPX_PROXY_DEFAULT_SERIAL_EXPIRES").setValue("170");
        proxySettings.getSetting("SIPX_PROXY_LOG_LEVEL").setValue("CRIT");
        
        m_proxyService.setIpAddress("192.168.1.1");
        m_proxyService.setHostname("sipx");
        m_proxyService.setFullHostname("sipx.example.org");
        m_proxyService.setDomainName("example.org");
        m_proxyService.setRealm("realm.example.org");
        m_proxyService.setSipPort("5060");
        m_proxyService.setSecureSipPort("5061");
        m_proxyService.setSipSrvOrHostport("sipsrv.example.org");
        m_proxyService.setCallResolverCallStateDb("CALL_RESOLVER_DB");
        
        out.generate(m_proxyService);
        
        StringWriter actualConfigWriter = new StringWriter();
        out.write(actualConfigWriter);
        
        Reader referenceConfigReader = new InputStreamReader(SipxProxyConfigurationTestIntegration.class
                .getResourceAsStream("expected-proxy-config"));
        String referenceConfig = IOUtils.toString(referenceConfigReader);
        
        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
        String actualConfig = IOUtils.toString(actualConfigReader);

        assertEquals(referenceConfig, actualConfig);
        
    }
    
    public void setSipxProxyService(SipxProxyService proxyService) {
        m_proxyService = proxyService;
    }
}

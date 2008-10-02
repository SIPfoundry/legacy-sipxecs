/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class SipxRegistrarConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxRegistrarConfiguration out = new SipxRegistrarConfiguration();
        out.setVelocityEngine(TestHelper.getVelocityEngine());
        out.setTemplate("sipxregistrar/registrar-config.vm");

        
        SipxRegistrarService registrarService = new SipxRegistrarService();
        registrarService.setModelDir("sipxregistrar");
        registrarService.setModelName("sipxregistrar.xml");
        registrarService.setModelFilesContext(TestHelper.getModelFilesContext());
        initCommonAttributes(registrarService);
        
        Domain domain = new Domain();
        domain.addAlias("another.example.org");
        IMocksControl domainManagerControl = EasyMock.createControl();
        DomainManager domainManager = domainManagerControl.createMock(DomainManager.class);
        domainManager.getDomain();
        EasyMock.expectLastCall().andReturn(domain).anyTimes();
        EasyMock.replay(domainManager);
        registrarService.setDomainManager(domainManager);
        
        setSettingValuesForGroup(registrarService, "logging", new String[] {
            "SIP_REGISTRAR_LOG_LEVEL"
        }, new String[] {
            "WARNING"
        });
        setSettingValuesForGroup(registrarService, "call-pick-up", new String[] {
            "SIP_REDIRECT.180-PICKUP.DIRECTED_CALL_PICKUP_CODE",
            "SIP_REDIRECT.180-PICKUP.CALL_RETRIEVE_CODE",
            "SIP_REDIRECT.180-PICKUP.CALL_PICKUP_WAIT"
        }, new String[] {
            "*42", "*43", "15.0"
        });
        setSettingValuesForGroup(registrarService, "isn", new String[] {
            "SIP_REDIRECT.190-ISN.BASE_DOMAIN", "SIP_REDIRECT.190-ISN.PREFIX"
        }, new String[] {
            "myisndomain.org", null
        });
        setSettingValuesForGroup(registrarService, "enum", new String[] {
            "SIP_REDIRECT.200-ENUM.BASE_DOMAIN", "SIP_REDIRECT.200-ENUM.DIAL_PREFIX",
            "SIP_REDIRECT.200-ENUM.ADD_PREFIX", "SIP_REDIRECT.200-ENUM.PREFIX_PLUS"
        }, new String[] {
            "myenumdomain.org", null, "*66", "Y"
        });
        
        registrarService.setMediaServerSipSrvOrHostport("media.example.org");
        registrarService.setOrbitServerSipSrvOrHostport("orbit.example.org");
        registrarService.setProxyServerSipHostport("proxy.example.org");
        registrarService.setRegistrarSipPort("5070");
        registrarService.setRegistrarEventSipPort("5075");

        out.generate(registrarService);

        assertCorrectFileGeneration(out, "expected-registrar-config");
    }

    private void setSettingValuesForGroup(SipxService registrarService, String group,
            String[] settingNames, String[] values) {
        for (int i = 0; i < settingNames.length; i++) {
            registrarService.getSettings().getSetting(group).getSetting(settingNames[i])
                    .setValue(values[i]);
        }
    }
}

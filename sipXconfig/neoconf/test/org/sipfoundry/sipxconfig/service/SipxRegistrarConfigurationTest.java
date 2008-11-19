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

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxRegistrarConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxRegistrarService registrarService = new SipxRegistrarService();
        registrarService.setModelDir("sipxregistrar");
        registrarService.setModelName("sipxregistrar.xml");
        registrarService.setModelFilesContext(TestHelper.getModelFilesContext());
        initCommonAttributes(registrarService);

        Domain domain = new Domain();
        domain.addAlias("another.example.org");
        domain.setName("example.org");
        DomainManager domainManager = createMock(DomainManager.class);
        expect(domainManager.getDomain()).andReturn(domain).anyTimes();
        expect(domainManager.getAuthorizationRealm()).andReturn("realm.example.org").anyTimes();
        registrarService.setDomainManager(domainManager);

        setSettingValuesForGroup(registrarService, "logging", new String[] {
            "SIP_REGISTRAR_LOG_LEVEL"
        }, new String[] {
            "WARNING"
        });
        setSettingValuesForGroup(registrarService, "call-pick-up", new String[] {
            "SIP_REDIRECT.100-PICKUP.DIRECTED_CALL_PICKUP_CODE",
            "SIP_REDIRECT.100-PICKUP.CALL_RETRIEVE_CODE",
            "SIP_REDIRECT.100-PICKUP.CALL_PICKUP_WAIT"
        }, new String[] {
            "*42", "*43", "15.0"
        });
        setSettingValuesForGroup(registrarService, "isn", new String[] {
            "SIP_REDIRECT.150-ISN.BASE_DOMAIN", "SIP_REDIRECT.150-ISN.PREFIX"
        }, new String[] {
            "myisndomain.org", null
        });
        setSettingValuesForGroup(registrarService, "enum", new String[] {
            "SIP_REDIRECT.160-ENUM.BASE_DOMAIN", "SIP_REDIRECT.160-ENUM.DIAL_PREFIX",
            "SIP_REDIRECT.160-ENUM.ADD_PREFIX", "SIP_REDIRECT.160-ENUM.PREFIX_PLUS"
        }, new String[] {
            "myenumdomain.org", null, "*66", "Y"
        });

        registrarService.setMediaServerSipSrvOrHostport("media.example.org");
        registrarService.setOrbitServerSipSrvOrHostport("orbit.example.org");
        registrarService.setProxyServerSipHostport("proxy.example.org");
        registrarService.setVoicemailHttpsPort("443");
        registrarService.setRegistrarSipPort("5070");
        registrarService.setRegistrarEventSipPort("5075");

        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxRegistrarService.BEAN_ID);
        expectLastCall().andReturn(registrarService).atLeastOnce();
        replay(domainManager, sipxServiceManager);

        SipxRegistrarConfiguration out = new SipxRegistrarConfiguration();
        out.setSipxServiceManager(sipxServiceManager);
        out.setTemplate("sipxregistrar/registrar-config.vm");

        assertCorrectFileGeneration(out, "expected-registrar-config");

        verify(domainManager, sipxServiceManager);
    }

    private void setSettingValuesForGroup(SipxRegistrarService registrarService, String group,
            String[] settingNames, String[] values) {
        for (int i = 0; i < settingNames.length; i++) {
            registrarService.getSettings().getSetting(group).getSetting(settingNames[i])
                    .setValue(values[i]);
        }
    }
}

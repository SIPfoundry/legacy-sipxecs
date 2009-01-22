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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class SipxRegistrarConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxRegistrarService registrarService = new SipxRegistrarService();
        registrarService.setBeanId(SipxRegistrarService.BEAN_ID);
        registrarService.setModelDir("sipxregistrar");
        registrarService.setModelName("sipxregistrar.xml");
        registrarService.setModelFilesContext(TestHelper.getModelFilesContext());
        initCommonAttributes(registrarService);

        SipxProxyService proxyService = new SipxProxyService();
        proxyService.setBeanId(SipxProxyService.BEAN_ID);
        proxyService.setSipPort("5060");

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

        registrarService.setProxyServerSipHostport("proxy.example.org");
        registrarService.setSipPort("5070");
        registrarService.setRegistrarEventSipPort("5075");

        SipxParkService parkService = new SipxParkService();
        parkService.setBeanId(SipxParkService.BEAN_ID);
        parkService.setSipPort("9909");

        SipxServiceManager sipxServiceManager = TestUtil.getMockSipxServiceManager(false, registrarService, proxyService, parkService);
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

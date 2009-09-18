/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.sipfoundry.sipxconfig.service.SipxService;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.config.AuthRules;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.admin.dialplan.config.FallbackRules;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ForwardingRules;
import org.sipfoundry.sipxconfig.admin.dialplan.config.MappingRules;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.SipxStatusService;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestUtil;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.ListableBeanFactory;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.createNiceMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.verify;

public class EagerDialPlanActivationManagerTest extends TestCase {
    private ConfigGenerator createConfigGenerator() {
        ConfigGenerator cg = new ConfigGenerator();

        DomainManager domainManager = TestUtil.getMockDomainManager();
        EasyMock.replay(domainManager);

        AuthRules authRules = new AuthRules();
        authRules.setDomainManager(domainManager);
        cg.setAuthRules(authRules);

        MappingRules mappingRules = new MappingRules();
        mappingRules.setDomainManager(domainManager);
        cg.setMappingRules(mappingRules);

        FallbackRules fallbackRules = new FallbackRules();
        fallbackRules.setDomainManager(domainManager);
        cg.setFallbackRules(fallbackRules);

        ForwardingRules forwardingRules = new ForwardingRules();
        cg.setForwardingRules(forwardingRules);

        forwardingRules.setVelocityEngine(TestHelper.getVelocityEngine());
        SbcManager sbcManager = createNiceMock(SbcManager.class);
        replay(sbcManager);

        forwardingRules.setSbcManager(sbcManager);

        SipxProxyService proxyService = new SipxProxyService();
        proxyService.setModelDir("sipxproxy");
        proxyService.setModelName("sipxproxy.xml");
        proxyService.setModelFilesContext(TestHelper.getModelFilesContext());
        proxyService.setBeanName(SipxProxyService.BEAN_ID);
        proxyService.setSipPort("9901");
        proxyService.setDomainManager(domainManager);

        SipxService statusService = new SipxStatusService();
        statusService.setBeanName(SipxStatusService.BEAN_ID);
        statusService.setModelFilesContext(TestHelper.getModelFilesContext());
        statusService.setSettings(TestHelper.loadSettings("sipxstatus/sipxstatus.xml"));
        Setting statusConfigSettings = statusService.getSettings().getSetting("status-config");
        statusConfigSettings.getSetting("SIP_STATUS_SIP_PORT").setValue("9905");

        SipxRegistrarService registrarService = new SipxRegistrarService();
        registrarService.setBeanName(SipxRegistrarService.BEAN_ID);
        registrarService.setRegistrarEventSipPort("9906");
        registrarService.setSipPort("9907");

        SipxServiceManager sipxServiceManager = TestUtil.getMockSipxServiceManager(true, proxyService,
                registrarService, statusService);
        forwardingRules.setSipxServiceManager(sipxServiceManager);

        return cg;
    }

    public void testActivateDialPlan() throws Exception {
        BeanFactory bf = createMock(ListableBeanFactory.class);
        for (int i = 0; i < 3; i++) {
            bf.getBean(ConfigGenerator.BEAN_NAME, ConfigGenerator.class);
            expectLastCall().andReturn(createConfigGenerator());
        }
        replay(bf);

        EagerDialPlanActivationManager manager = new MockDpam();
        manager.setDialPlanContext(new DialPlanContextImplTest.MockDialPlanContextImpl(new DialPlan()));
        manager.setBeanFactory(bf);

        final ConfigGenerator g1 = manager.generateDialPlan();
        final ConfigGenerator g2 = manager.generateDialPlan();
        final ConfigGenerator g3 = manager.generateDialPlan();
        assertNotNull(g1);
        assertNotNull(g2);
        assertNotSame(g1, g2);
        assertNotSame(g2, g3);

        verify(bf);
    }

    static class MockDpam extends EagerDialPlanActivationManager {

        public MockDpam() {
        }

        @Override
        public ServiceConfigurator getServiceConfigurator() {
            return null;
        }

        @Override
        public GatewayContext getGatewayContext() {
            return null;
        }

        @Override
        public ProfileManager getGatewayProfileManager() {
            return null;
        }
    }
}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Arrays;
import java.util.Collections;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.AutoAttendantsConfig;
import org.sipfoundry.sipxconfig.admin.dialplan.config.AuthRules;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.admin.dialplan.config.FallbackRules;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ForwardingRules;
import org.sipfoundry.sipxconfig.admin.dialplan.config.MappingRules;
import org.sipfoundry.sipxconfig.admin.dialplan.config.SpecialAutoAttendantMode;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.SipxStatusService;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestUtil;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.orm.hibernate3.HibernateTemplate;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.createNiceMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.verify;

/**
 * DialPlanContextImplTest
 */
public class DialPlanContextImplTest extends TestCase {

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

        cg.setAutoAttendantConfig(new AutoAttendantsConfig());

        ForwardingRules forwardingRules = new ForwardingRules();
        cg.setForwardingRules(forwardingRules);

        forwardingRules.setVelocityEngine(TestHelper.getVelocityEngine());
        SbcManager sbcManager = createNiceMock(SbcManager.class);
        replay(sbcManager);

        forwardingRules.setSbcManager(sbcManager);

        SipxProxyService proxyService = new SipxProxyService();
        proxyService.setBeanName(SipxProxyService.BEAN_ID);
        proxyService.setSipPort("9901");
        proxyService.setDomainManager(domainManager);

        SipxStatusService statusService = new SipxStatusService();
        statusService.setBeanName(SipxStatusService.BEAN_ID);
        statusService.setModelFilesContext(TestHelper.getModelFilesContext());
        statusService.setSettings(TestHelper.loadSettings("sipxstatus/sipxstatus.xml"));
        Setting statusConfigSettings = statusService.getSettings().getSetting("status-config");
        statusConfigSettings.getSetting("SIP_STATUS_SIP_PORT").setValue("9905");

        SipxRegistrarService registrarService = new SipxRegistrarService();
        registrarService.setBeanName(SipxRegistrarService.BEAN_ID);
        registrarService.setRegistrarEventSipPort("9906");
        registrarService.setSipPort("9907");

        SipxServiceManager sipxServiceManager =TestUtil.getMockSipxServiceManager(true, proxyService, registrarService, statusService);
        forwardingRules.setSipxServiceManager(sipxServiceManager);

        return cg;
    }

    public void testActivateDialPlan() throws Exception {
        BeanFactory bf = createMock(ListableBeanFactory.class);
        for(int i = 0; i < 3; i++) {
            bf.getBean(ConfigGenerator.BEAN_NAME, ConfigGenerator.class);
            expectLastCall().andReturn(createConfigGenerator());
        }
        replay(bf);

        DialPlanContextImpl manager = new MockDialPlanContextImpl(new DialPlan());
        manager.setBeanFactory(bf);

        final ConfigGenerator g1 = manager.getGenerator();
        final ConfigGenerator g2 = manager.generateDialPlan();
        final ConfigGenerator g3 = manager.getGenerator();
        assertNotNull(g1);
        assertNotNull(g2);
        assertNotSame(g1, g2);
        assertNotSame(g2, g3);

        verify(bf);
    }

    public void testMoveRules() throws Exception {
        DialPlan plan = createMock(DialPlan.class);
        plan.moveRules(Collections.singletonList(new Integer(5)), 3);
        replay(plan);

        MockDialPlanContextImpl manager = new MockDialPlanContextImpl(plan);
        manager.moveRules(Collections.singletonList(new Integer(5)), 3);
        verify(plan);
    }

    public void testLikelyVoiceMail() {
        DialPlan plan = new DialPlan();
        DialPlanContextImpl manager = new MockDialPlanContextImpl(plan);
        assertEquals("101", manager.getVoiceMail());

        DialingRule[] rules = new DialingRule[] {
            new CustomDialingRule()
        };
        plan.setRules(Arrays.asList(rules));
        assertEquals("101", manager.getVoiceMail());

        InternalRule irule = new InternalRule();
        irule.setVoiceMail("2000");
        rules = new DialingRule[] {
            new CustomDialingRule(), irule
        };
        plan.setRules(Arrays.asList(rules));
        assertEquals("2000", manager.getVoiceMail());
    }

    public void testNoLikelyEmergencyInfo() {
        DialPlan plan = new DialPlan();
        DialPlanContextImpl manager = new MockDialPlanContextImpl(plan);

        // no rules
        assertNull(manager.getLikelyEmergencyInfo());

        // no gateway
        EmergencyRule emergency = new EmergencyRule();
        emergency.setEnabled(true);
        DialingRule[] rules = new DialingRule[] {
            emergency
        };
        emergency.setEmergencyNumber("sos");
        plan.setRules(Arrays.asList(rules));
        assertNull(manager.getLikelyEmergencyInfo());


        // disabled rule
        emergency.setEnabled(false);
        assertNull(manager.getLikelyEmergencyInfo());

        // gateway has routing (e.g. SBC)
        emergency.setEnabled(true);
        Gateway gatewayWithSbc = new Gateway() {
            @Override
            public String getRoute() {
                return "sbc.example.org";
            }
        };
        rules[0].setGateways(Collections.singletonList(gatewayWithSbc));
        assertNull(manager.getLikelyEmergencyInfo());
    }

    public void testLikelyEmergencyInfo() {
        DialPlan plan = new DialPlan();
        DialPlanContextImpl manager = new MockDialPlanContextImpl(plan);

        EmergencyRule emergency = new EmergencyRule();
        emergency.setEnabled(true);
        DialingRule[] rules = new DialingRule[] {
            emergency
        };
        emergency.setEmergencyNumber("sos");
        plan.setRules(Arrays.asList(rules));
        Gateway gateway = new Gateway();
        gateway.setAddress("pstn.example.org");
        emergency.setGateways(Collections.singletonList(gateway));

        EmergencyInfo info = manager.getLikelyEmergencyInfo();
        assertEquals("pstn.example.org", info.getAddress());
        assertEquals("sos", info.getNumber());
        assertNull(info.getPort());

        gateway.setAddressPort(9050);
        assertEquals((Integer) 9050, manager.getLikelyEmergencyInfo().getPort());
    }

    private static class MockDialPlanContextImpl extends DialPlanContextImpl {
        private final DialPlan m_plan;

        MockDialPlanContextImpl(DialPlan plan) {
            m_plan = plan;
            setHibernateTemplate(createMock(HibernateTemplate.class));
        }

        @Override
        DialPlan getDialPlan() {
            return m_plan;
        }

        @Override
        public GatewayContext getGatewayContext() {
            return null;
        }

        @Override
        public ProfileManager getGatewayProfileManager() {
            return null;
        }

        @Override
        public SpecialAutoAttendantMode createSpecialAutoAttendantMode() {
            return null;
        }
    }
}

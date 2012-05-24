/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.gateway;


import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.InternationalRule;
import org.sipfoundry.sipxconfig.dialplan.DialPlanSetup;
import org.sipfoundry.sipxconfig.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class GatewayContextTestIntegration extends IntegrationTestCase {
    private GatewayContext m_gatewayContext;
    private ModelSource<GatewayModel> m_nakedGatewayModelSource;
    private DialPlanContext m_dialPlanContext;
    private GatewayModel m_genericModel;
    private GatewayModel m_genericSipTrunk;
    private SbcDeviceManager m_sbcDeviceManager;
    private BranchManager m_branchManager;
    private DialPlanSetup m_resetDialPlanTask;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        m_genericModel = m_nakedGatewayModelSource.getModel("genericGatewayStandard");
        m_genericSipTrunk = m_nakedGatewayModelSource.getModel("sipTrunkStandard");        
        clear();
        m_resetDialPlanTask.setupDefaultRegion();
    }

    public void testAddGateway() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        Gateway g1 = new Gateway(m_genericModel);
        Gateway g2 = new Gateway(m_genericModel);

        // add g1
        m_gatewayContext.saveGateway(g1);

        assertEquals(1, m_gatewayContext.getGateways().size());
        assertTrue(m_gatewayContext.getGateways().contains(g1));

        // add g2
        m_gatewayContext.saveGateway(g2);

        assertEquals(2, m_gatewayContext.getGateways().size());
        assertTrue(m_gatewayContext.getGateways().contains(g1));
        assertTrue(m_gatewayContext.getGateways().contains(g2));
    }

    public void testAddDuplicateGatewayDuplicate() throws Exception {
        Gateway g1 = new Gateway(m_genericModel);
        g1.setName("bongo");
        Gateway g2 = new SipTrunk(m_genericSipTrunk);
        g2.setName("bongo");

        // add g1
        m_gatewayContext.saveGateway(g1);

        // add g2
        try {
            m_gatewayContext.saveGateway(g2);
            fail("Duplicate gateway names should not be possible.");
        } catch (UserException e) {
            // ok
        }
    }

    public void testDeleteGateway() {
        Gateway g1 = new Gateway(m_genericModel);
        Gateway g2 = new Gateway(m_genericModel);
        Gateway g3 = new Gateway(m_genericModel);

        // add all
        m_gatewayContext.saveGateway(g1);
        m_gatewayContext.saveGateway(g2);
        m_gatewayContext.saveGateway(g3);

        Gateway[] toBeRemoved = {
            g1, g3
        };
        m_gatewayContext.deleteGateways(Arrays.asList(toBeRemoved));

        List gateways = m_gatewayContext.getGateways();

        assertEquals(1, gateways.size());
        assertFalse(gateways.contains(g1));
        assertTrue(gateways.contains(g2));
        assertFalse(gateways.contains(g3));
    }

    public void testUpdateGateway() throws Exception {
        Gateway g1 = new Gateway(m_genericModel);
        g1.setAddress("10.1.1.1");
        m_gatewayContext.saveGateway(g1);
        g1.setAddress("10.1.1.2");
        g1.setAddressPort(5050);
        g1.setPrefix("33");

        GatewayCallerAliasInfo info = new GatewayCallerAliasInfo();
        info.setDefaultCallerAlias("3211231234");
        info.setAnonymous(true);
        info.setKeepDigits(10);
        info.setIgnoreUserInfo(true);
        info.setTransformUserExtension(true);
        info.setAddPrefix("4321");
        info.setDisplayName("display name");
        info.setUrlParameters("param=value");
        g1.setCallerAliasInfo(info);

        m_gatewayContext.saveGateway(g1);
        assertEquals("10.1.1.2", g1.getAddress());
        assertEquals("10.1.1.2:5050", g1.getGatewayAddress());
        assertEquals(5050, g1.getAddressPort());
        assertEquals("33", g1.getPrefix());
        assertEquals("3211231234", g1.getCallerAliasInfo().getDefaultCallerAlias());
        assertEquals("4321", g1.getCallerAliasInfo().getAddPrefix());
        assertEquals(10, g1.getCallerAliasInfo().getKeepDigits());
        assertTrue(g1.getCallerAliasInfo().isAnonymous());
        assertTrue(g1.getCallerAliasInfo().isIgnoreUserInfo());
        assertTrue(g1.getCallerAliasInfo().isTransformUserExtension());
        assertEquals("display name", g1.getCallerAliasInfo().getDisplayName());
        assertEquals("param=value", g1.getCallerAliasInfo().getUrlParameters());
    }

    public void testSaveLoadUpdateGateway() throws Exception {
        Gateway g1 = new Gateway(m_genericModel);
        g1.setAddress("10.1.1.1");
        m_gatewayContext.saveGateway(g1);

        Gateway g2 = m_gatewayContext.getGateway(g1.getId());
        g2.setAddress("10.1.1.2");
        m_gatewayContext.saveGateway(g2);
        assertEquals("10.1.1.2", g2.getGatewayAddress());
        assertEquals("10.1.1.2", g2.getAddress());
    }

    public void testDeleteGatewayInUse() {
        Gateway g1 = new Gateway(m_genericModel);
        g1.setAddress("10.1.1.1");
        m_gatewayContext.saveGateway(g1);
        InternationalRule rule = new InternationalRule();
        rule.setName("testRule");
        rule.setInternationalPrefix("011");
        rule.addGateway(g1);

        m_dialPlanContext.storeRule(rule);
        // remove gateway
        m_gatewayContext.deleteGateways(Collections.singletonList(g1));

        Integer ruleId = rule.getId();

        rule = (InternationalRule) m_dialPlanContext.getRule(ruleId);
        assertTrue(rule.getGateways().isEmpty());
    }

    public void testGatewaysAndRule() throws Exception {
        InternationalRule rule = new InternationalRule();
        rule.setName("testRule");
        rule.setInternationalPrefix("011");

        sql("gateway/seed_gateway.sql");

        int gatewayId = 1001;
        Gateway gateway = m_gatewayContext.getGateway(gatewayId);

        rule.addGateway(gateway);

        m_dialPlanContext.storeRule(rule);
        commit();

        int ruleId = rule.getId();
        m_gatewayContext.removeGatewaysFromRule(ruleId, Collections.singleton(gatewayId));

        rule = (InternationalRule) m_dialPlanContext.getRule(ruleId);
        assertTrue(rule.getGateways().isEmpty());

        m_gatewayContext.addGatewaysToRule(ruleId, Collections.singleton(gatewayId));
        rule = (InternationalRule) m_dialPlanContext.getRule(ruleId);
        assertEquals(gateway, rule.getGateways().get(0));
    }

    public void testAllGateways() throws Exception {
        Collection<GatewayModel> models = m_nakedGatewayModelSource.getModels();
        for (GatewayModel model : models) {
            Gateway gateway = m_gatewayContext.newGateway(model);
            String beanId = model.getBeanId();            
            assertEquals(gateway.getClass(), getApplicationContext().getBean(beanId).getClass());
            if (beanId.equals("gwGeneric")) {
                assertNull(gateway.getSettings());
            } else {
                assertNotNull(gateway.getSettings());
            }
            m_gatewayContext.saveGateway(gateway);
        }
        // one gateway per row
        assertEquals(models.size(), countRowsInTable("gateway"));
    }

    public void testGetGatewaySettings() throws Exception {
        sql("gateway/seed_gateway.sql");
        Device gateway = m_gatewayContext.getGateway(1001);
        assertNotNull(gateway.getProfileGenerator());
    }

    public void testGatewayWithPorts() throws Exception {
        m_genericModel.setMaxPorts(10);
        Gateway g1 = new Gateway(m_genericModel);
        g1.setAddress("10.1.1.1");

        for (int i = 0; i < 3; i++) {
            g1.addPort(new FxoPort());
        }

        m_gatewayContext.saveGateway(g1);
        assertEquals(3, countRowsInTable("fxo_port"));
    }

    public void testRemovePortsFromGateway() throws Exception {
        sql("gateway/gateway_ports.sql");
        assertEquals(3, countRowsInTable("fxo_port"));

        Integer[] ids = new Integer[] {
            1000, 1005
        };
        m_gatewayContext.removePortsFromGateway(1001, Arrays.asList(ids));
        commit();
        assertEquals(1, countRowsInTable("fxo_port"));

        Gateway gateway = m_gatewayContext.getGateway(1001);
        assertEquals(1, gateway.getPorts().size());
    }

    public void testDeleteAssociateSbcDevice() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        sql("gateway/gateway_sbc_device.sql");

        SipTrunk sipTrunk = (SipTrunk) m_gatewayContext.getGateway(1002);
        assertNotNull(sipTrunk);
        assertNotNull(sipTrunk.getSbcDevice());
        assertEquals("10.1.2.2", sipTrunk.getSbcDevice().getAddress());

        m_sbcDeviceManager.deleteSbcDevice(sipTrunk.getSbcDevice().getId());
        sipTrunk = (SipTrunk) m_gatewayContext.getGateway(1002);
        assertNotNull(sipTrunk);
        assertNull(sipTrunk.getSbcDevice());
    }

    public void testDeleteAssociateSpecificBranch() throws Exception {
        sql("gateway/gateway_location.sql");

        Gateway g = m_gatewayContext.getGateway(1003);
        assertNotNull(g);
        assertNotNull(g.getBranch());
        assertEquals("branch1", g.getBranch().getName());

        m_branchManager.deleteBranches(Collections.singletonList(g.getBranch().getId()));
        commit();
        g = m_gatewayContext.getGateway(1003);
        assertNotNull(g);
        assertNull(g.getBranch());
    }

    public void setGatewayContext(GatewayContext gatewayContext) {
        m_gatewayContext = gatewayContext;
    }

    public void setNakedGatewayModelSource(ModelSource<GatewayModel> nakedGatewayModelSource) {
        m_nakedGatewayModelSource = nakedGatewayModelSource;
    }

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    public void setBranchManager(BranchManager branchManager) {
        m_branchManager = branchManager;
    }

    public void setResetDialPlanTask(DialPlanSetup resetDialPlanTask) {
        m_resetDialPlanTask = resetDialPlanTask;
    }
}

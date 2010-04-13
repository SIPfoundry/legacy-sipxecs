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

import org.dbunit.dataset.ITable;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.InternationalRule;
import org.sipfoundry.sipxconfig.admin.dialplan.ResetDialPlanTask;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.springframework.context.ApplicationContext;

public class GatewayContextTestDb extends SipxDatabaseTestCase {

    private GatewayContext m_context;

    private ModelSource<GatewayModel> m_modelSource;

    private DialPlanContext m_dialPlanContext;

    private ApplicationContext m_appContext;

    private GatewayModel m_genericModel;

    private GatewayModel m_genericSipTrunk;

    private SbcDeviceManager m_sbcDeviceManager;

    private BranchManager m_branchManager;

    private ResetDialPlanTask m_resetDialPlanTask;

    @Override
    protected void setUp() throws Exception {
        m_appContext = TestHelper.getApplicationContext();
        m_context = (GatewayContext) m_appContext.getBean(GatewayContext.CONTEXT_BEAN_NAME);
        m_dialPlanContext = (DialPlanContext) m_appContext.getBean(DialPlanContext.CONTEXT_BEAN_NAME);
        m_modelSource = (ModelSource<GatewayModel>) m_appContext.getBean("nakedGatewayModelSource");
        m_genericModel = m_modelSource.getModel("genericGatewayStandard");
        m_genericSipTrunk = m_modelSource.getModel("sipTrunkStandard");
        m_resetDialPlanTask = (ResetDialPlanTask) m_appContext.getBean("resetDialPlanTask");
        TestHelper.cleanInsert("ClearDb.xml");
        m_resetDialPlanTask.reset(true);
        m_sbcDeviceManager = (SbcDeviceManager) m_appContext.getBean(SbcDeviceManager.CONTEXT_BEAN_NAME);
        m_branchManager = (BranchManager) m_appContext.getBean("branchManager");
    }

    public void testAddGateway() {
        Gateway g1 = new Gateway(m_genericModel);
        Gateway g2 = new Gateway(m_genericModel);

        // add g1
        m_context.storeGateway(g1);

        assertEquals(1, m_context.getGateways().size());
        assertTrue(m_context.getGateways().contains(g1));

        // add g2
        m_context.storeGateway(g2);

        assertEquals(2, m_context.getGateways().size());
        assertTrue(m_context.getGateways().contains(g1));
        assertTrue(m_context.getGateways().contains(g2));
    }

    public void testAddDuplicateGatewayDuplicate() throws Exception {
        Gateway g1 = new Gateway(m_genericModel);
        g1.setName("bongo");
        Gateway g2 = new SipTrunk(m_genericSipTrunk);
        g2.setName("bongo");

        // add g1
        m_context.storeGateway(g1);

        // add g2
        try {
            m_context.storeGateway(g2);
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
        m_context.storeGateway(g1);
        m_context.storeGateway(g2);
        m_context.storeGateway(g3);

        Integer[] toBeRemoved = {
            g1.getId(), g3.getId()
        };
        m_context.deleteGateways(Arrays.asList(toBeRemoved));

        List gateways = m_context.getGateways();

        assertEquals(1, gateways.size());
        assertFalse(gateways.contains(g1));
        assertTrue(gateways.contains(g2));
        assertFalse(gateways.contains(g3));
    }

    public void testUpdateGateway() throws Exception {
        Gateway g1 = new Gateway(m_genericModel);
        g1.setAddress("10.1.1.1");
        m_context.storeGateway(g1);
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
        info.setEnableCallerId(true);
        info.setCallerId("user@domain.com");
        info.setDisplayName("display name");
        info.setUrlParameters("param=value");
        g1.setCallerAliasInfo(info);

        m_context.storeGateway(g1);
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
        assertTrue(g1.getCallerAliasInfo().isEnableCallerId());
        assertEquals("user@domain.com", g1.getCallerAliasInfo().getCallerId());
        assertEquals("display name", g1.getCallerAliasInfo().getDisplayName());
        assertEquals("param=value", g1.getCallerAliasInfo().getUrlParameters());
    }

    public void testSaveLoadUpdateGateway() throws Exception {
        Gateway g1 = new Gateway(m_genericModel);
        g1.setAddress("10.1.1.1");
        m_context.storeGateway(g1);

        Gateway g2 = m_context.getGateway(g1.getId());
        g2.setAddress("10.1.1.2");
        m_context.storeGateway(g2);
        assertEquals("10.1.1.2", g2.getGatewayAddress());
        assertEquals("10.1.1.2", g2.getAddress());
    }

    public void testDeleteGatewayInUse() {
        Gateway g1 = new Gateway(m_genericModel);
        g1.setAddress("10.1.1.1");
        m_context.storeGateway(g1);
        InternationalRule rule = new InternationalRule();
        rule.setName("testRule");
        rule.setInternationalPrefix("011");
        rule.addGateway(g1);

        m_dialPlanContext.storeRule(rule);
        // remove gateway
        m_context.deleteGateways(Collections.singletonList(g1.getId()));

        Integer ruleId = rule.getId();

        rule = (InternationalRule) m_dialPlanContext.getRule(ruleId);
        assertTrue(rule.getGateways().isEmpty());
    }

    public void testGatewaysAndRule() throws Exception {
        InternationalRule rule = new InternationalRule();
        rule.setName("testRule");
        rule.setInternationalPrefix("011");

        TestHelper.cleanInsertFlat("gateway/seed_gateway.db.xml");

        int gatewayId = 1001;
        Gateway gateway = m_context.getGateway(gatewayId);

        rule.addGateway(gateway);

        m_dialPlanContext.storeRule(rule);

        int ruleId = rule.getId();
        m_context.removeGatewaysFromRule(ruleId, Collections.singleton(gatewayId));

        rule = (InternationalRule) m_dialPlanContext.getRule(ruleId);
        assertTrue(rule.getGateways().isEmpty());

        m_context.addGatewaysToRule(ruleId, Collections.singleton(gatewayId));
        rule = (InternationalRule) m_dialPlanContext.getRule(ruleId);
        assertEquals(gateway, rule.getGateways().get(0));
    }

    public void testAllGateways() throws Exception {
        Collection<GatewayModel> models = m_modelSource.getModels();
        for (GatewayModel model : models) {
            Gateway gateway = m_context.newGateway(model);
            String beanId = model.getBeanId();
            assertEquals(gateway.getClass(), m_appContext.getBean(beanId).getClass());
            if (beanId.equals("gwGeneric")) {
                assertNull(gateway.getSettings());
            } else {
                assertNotNull(gateway.getSettings());
            }
            m_context.storeGateway(gateway);
        }
        ITable actual = TestHelper.getConnection().createDataSet().getTable("gateway");
        // one gateway per row
        assertEquals(models.size(), actual.getRowCount());
    }

    public void testGetGatewaySettings() throws Exception {
        TestHelper.cleanInsertFlat("gateway/seed_gateway.db.xml");
        Device gateway = m_context.getGateway(1001);
        assertNotNull(gateway.getProfileGenerator());
    }

    public void testGatewayWithPorts() throws Exception {
        m_genericModel.setMaxPorts(10);
        Gateway g1 = new Gateway(m_genericModel);
        g1.setAddress("10.1.1.1");

        for (int i = 0; i < 3; i++) {
            g1.addPort(new FxoPort());
        }

        m_context.storeGateway(g1);
        ITable actual = TestHelper.getConnection().createDataSet().getTable("fxo_port");
        assertEquals(3, actual.getRowCount());
    }

    public void testRemovePortsFromGateway() throws Exception {
        TestHelper.cleanInsertFlat("gateway/gateway_ports.db.xml");
        ITable ports = TestHelper.getConnection().createDataSet().getTable("fxo_port");
        assertEquals(3, ports.getRowCount());

        Integer[] ids = new Integer[] {
            1000, 1005
        };
        m_context.removePortsFromGateway(1001, Arrays.asList(ids));

        ports = TestHelper.getConnection().createDataSet().getTable("fxo_port");
        assertEquals(1, ports.getRowCount());

        Gateway gateway = m_context.getGateway(1001);
        assertEquals(1, gateway.getPorts().size());
    }

    public void testDeleteAssociateSbcDevice() throws Exception {
        TestHelper.insertFlat("gateway/gateway_sbc_device.db.xml");

        SipTrunk sipTrunk = (SipTrunk) m_context.getGateway(1002);
        assertNotNull(sipTrunk);
        assertNotNull(sipTrunk.getSbcDevice());
        assertEquals("10.1.2.2", sipTrunk.getSbcDevice().getAddress());

        m_sbcDeviceManager.deleteSbcDevice(sipTrunk.getSbcDevice().getId());
        sipTrunk = (SipTrunk) m_context.getGateway(1002);
        assertNotNull(sipTrunk);
        assertNull(sipTrunk.getSbcDevice());
    }

    public void testDeleteAssociateSpecificBranch() throws Exception {
        TestHelper.insertFlat("gateway/gateway_location.xml");

        Gateway g = m_context.getGateway(1003);
        assertNotNull(g);
        assertNotNull(g.getBranch());
        assertEquals("branch1", g.getBranch().getName());

        m_branchManager.deleteBranch(g.getBranch());

        g = m_context.getGateway(1003);
        assertNotNull(g);
        assertNull(g.getBranch());
    }
}

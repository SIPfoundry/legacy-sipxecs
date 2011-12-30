/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.gateway;

import junit.framework.TestCase;

import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.dialplan.CustomDialingRule;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;

/**
 * EditGatewayTest
 */
public class EditGatewayTest extends TestCase {
    private Creator m_pageMaker = new Creator();
    private EditGateway m_editGatewayPage;

    protected void setUp() throws Exception {
        m_editGatewayPage = (EditGateway) m_pageMaker.newInstance(EditGateway.class);
    }

    public void testAddNewGateway() {
        Gateway g = new Gateway();
        g.setUniqueId();

        IMocksControl contextControl = EasyMock.createControl();
        GatewayContext context = contextControl.createMock(GatewayContext.class);

        context.saveGateway(g);

        contextControl.replay();
        PropertyUtils.write(m_editGatewayPage, "gatewayContext", context);
        m_editGatewayPage.setGateway(g);
        m_editGatewayPage.setGatewayModel(new GatewayModel("xxx", "xxx"));
        m_editGatewayPage.saveValid();

        contextControl.verify();
    }

    public void testSaveAndAssign() {
        DialingRule rule = new CustomDialingRule();
        rule.setUniqueId();
        rule.setName("my rule name");

        Gateway g = new Gateway();
        g.setUniqueId();

        IMocksControl dialPlanContextControl = EasyMock.createStrictControl();
        DialPlanContext dialPlanContext = dialPlanContextControl.createMock(DialPlanContext.class);

        IMocksControl contextControl = EasyMock.createStrictControl();
        GatewayContext context = contextControl.createMock(GatewayContext.class);

        context.saveGateway(g);
        dialPlanContext.getRule(rule.getId());
        dialPlanContextControl.andReturn(rule);
        dialPlanContext.storeRule(rule);

        dialPlanContextControl.replay();
        contextControl.replay();

        PropertyUtils.write(m_editGatewayPage, "dialPlanContext", dialPlanContext);
        PropertyUtils.write(m_editGatewayPage, "gatewayContext", context);
        m_editGatewayPage.setGateway(g);
        m_editGatewayPage.setGatewayId(g.getId());
        m_editGatewayPage.setRuleId(rule.getId());
        m_editGatewayPage.pageBeginRender(null);
        m_editGatewayPage.setGatewayModel(new GatewayModel("xxx", "xxx"));
        m_editGatewayPage.saveValid();

        dialPlanContextControl.verify();
        contextControl.verify();

        assertEquals(1, rule.getGateways().size());
        assertTrue(rule.getGateways().contains(g));
    }

    public void testPageBeginRenderAdd() {
        Gateway gateway = new Gateway();
        gateway.setModel(new GatewayModel("xxx", "xxx"));

        IMocksControl contextControl = EasyMock.createStrictControl();
        GatewayContext context = contextControl.createMock(GatewayContext.class);
        context.newGateway(gateway.getModel());
        contextControl.andReturn(gateway);
        contextControl.replay();

        PropertyUtils.write(m_editGatewayPage, "gatewayContext", context);

        m_editGatewayPage.setGatewayModel(gateway.getModel());
        m_editGatewayPage.pageBeginRender(null);

        assertNotNull(m_editGatewayPage.getGateway());
        assertNull(m_editGatewayPage.getGatewayId());

        contextControl.verify();
    }

    public void testPageBeginRenderEdit() {
        Gateway gateway = new Gateway();
        gateway.setUniqueId();
        gateway.setName("kuku");
        Integer id = gateway.getId();

        IMocksControl contextControl = EasyMock.createStrictControl();
        GatewayContext context = contextControl.createMock(GatewayContext.class);
        context.getGateway(id);
        contextControl.andReturn(gateway);
        contextControl.replay();

        PropertyUtils.write(m_editGatewayPage, "gatewayContext", context);
        m_editGatewayPage.setGatewayId(id);
        m_editGatewayPage.pageBeginRender(null);

        contextControl.verify();

        assertEquals(id, m_editGatewayPage.getGatewayId());
        assertEquals("kuku", m_editGatewayPage.getGateway().getName());
    }
}

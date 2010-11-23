/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.openacd;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.NameInUseException;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchCondition;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.freeswitch.DefaultContextConfigurationTest;
import org.sipfoundry.sipxconfig.test.TestUtil;
import org.springframework.dao.support.DataAccessUtils;

public class OpenAcdContextTestIntegration extends IntegrationTestCase {
    private OpenAcdContextImpl m_openAcdContextImpl;
    private CoreContext m_coreContext;

    public void testOpenAcdExtensionCrud() throws Exception {
        // test save open acd extension
        assertEquals(0, m_openAcdContextImpl.getFreeswitchExtensions().size());
        OpenAcdExtension extension = DefaultContextConfigurationTest.createOpenAcdExtension("example");
        m_openAcdContextImpl.saveExtension(extension);
        assertEquals(1, m_openAcdContextImpl.getFreeswitchExtensions().size());

        // test save extension with same name
        try {
            OpenAcdExtension sameNameExtension = new OpenAcdExtension();
            sameNameExtension.setName("example");
            m_openAcdContextImpl.saveExtension(sameNameExtension);
            fail();
        } catch (NameInUseException ex) {
        }

        // test get extension by name
        OpenAcdExtension savedExtension = m_openAcdContextImpl.getExtensionByName("example");
        assertNotNull(extension);
        assertEquals("example", savedExtension.getName());
        // test modify extension without changing name
        try {
            m_openAcdContextImpl.saveExtension(savedExtension);
        } catch (NameInUseException ex) {
            fail();
        }

        // test get extension by id
        Integer id = savedExtension.getId();
        OpenAcdExtension extensionById = m_openAcdContextImpl.getExtensionById(id);
        assertNotNull(extensionById);
        assertEquals("example", extensionById.getName());

        // test saved conditions and actions
        Set<FreeswitchCondition> conditions = extensionById.getConditions();
        assertEquals(1, conditions.size());
        for (FreeswitchCondition condition : conditions) {
            assertEquals(8, condition.getActions().size());
            List<FreeswitchAction> actions = new LinkedList<FreeswitchAction>();
            actions.addAll(condition.getActions());
            assertEquals("answer", actions.get(0).getApplication());
            assertEquals("", actions.get(0).getData());
            assertEquals("set", actions.get(1).getApplication());
            assertEquals("domain_name=$${domain}", actions.get(1).getData());
            assertEquals("set", actions.get(2).getApplication());
            assertEquals("brand=1", actions.get(2).getData());
            assertEquals("set", actions.get(3).getApplication());
            assertEquals("queue=Sales", actions.get(3).getData());
            assertEquals("set", actions.get(4).getApplication());
            assertEquals("allow_voicemail=true", actions.get(4).getData());
            assertEquals("erlang_sendmsg", actions.get(5).getApplication());
            assertEquals("freeswitch_media_manager testme@192.168.1.1 inivr ${uuid}", actions.get(5).getData());
            assertEquals("playback", actions.get(6).getApplication());
            assertEquals("/usr/share/www/doc/stdprompts/welcome.wav", actions.get(6).getData());
            assertEquals("erlang", actions.get(7).getApplication());
            assertEquals("freeswitch_media_manager:! testme@192.168.1.1", actions.get(7).getData());
        }

        // test remove extension
        assertEquals(1, m_openAcdContextImpl.getFreeswitchExtensions().size());
        m_openAcdContextImpl.removeExtensions(Collections.singletonList(id));
        assertEquals(0, m_openAcdContextImpl.getFreeswitchExtensions().size());
    }

    public void testOpenAcdExtensionAliasProvider() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        SipxFreeswitchService service = new MockSipxFreeswitchService();
        service.setBeanId(SipxFreeswitchService.BEAN_ID);
        IMocksControl mc = EasyMock.createControl();
        LocationsManager locationsManager = mc.createMock(LocationsManager.class);
        locationsManager.getLocationsForService(service);
        Location location = new Location();
        location.setAddress("example.org");
        List<Location> locations = new ArrayList<Location>();
        locations.add(location);
        mc.andReturn(locations);
        mc.replay();
        service.setLocationsManager(locationsManager);

        SipxServiceManager sm = TestUtil.getMockSipxServiceManager(true, service);
        m_openAcdContextImpl.setSipxServiceManager(sm);

        OpenAcdExtension extension = DefaultContextConfigurationTest.createOpenAcdExtension("provider");
        m_openAcdContextImpl.saveExtension(extension);
        assertEquals(1, m_openAcdContextImpl.getBeanIdsOfObjectsWithAlias("provider").size());

        assertFalse(m_openAcdContextImpl.isAliasInUse("test"));
        assertTrue(m_openAcdContextImpl.isAliasInUse("provider"));

        Collection<AliasMapping> mappings = m_openAcdContextImpl.getAliasMappings();
        assertEquals(1, mappings.size());
        for (AliasMapping mapping : mappings) {
            assertEquals("openacd", mapping.getRelation());
            assertEquals("provider@example.org", mapping.getIdentity());
            assertEquals("sip:provider@example.org:50", mapping.getContact());
        }
    }

    public void testOpenAcdAgentGroupCrud() throws Exception {
        // 'Default' agent group
        assertEquals(1, m_openAcdContextImpl.getAgentGroups().size());

        // test get agent group by name
        OpenAcdAgentGroup defaultAgentGroup = m_openAcdContextImpl.getAgentGroupByName("Default");
        assertNotNull(defaultAgentGroup);
        assertEquals("Default", defaultAgentGroup.getName());

        // test save agent group without name
        OpenAcdAgentGroup group = new OpenAcdAgentGroup();
        try {
            m_openAcdContextImpl.saveAgentGroup(group);
            fail();
        } catch (UserException ex) {
        }

        // test save agent group without agents
        assertEquals(1, m_openAcdContextImpl.getAgentGroups().size());
        group.setName("Group");
        group.setDescription("Group description");
        m_openAcdContextImpl.saveAgentGroup(group);
        assertEquals(2, m_openAcdContextImpl.getAgentGroups().size());

        // test save agent group with agents
        loadDataSet("common/SampleUsersSeed.xml");
        User alpha = m_coreContext.loadUser(1001);
        OpenAcdAgent supervisor = new OpenAcdAgent();
        supervisor.setGroup(group);
        supervisor.setUser(alpha);
        supervisor.setPin("123456");
        supervisor.setSecurity(OpenAcdAgent.Security.SUPERVISOR.toString());

        User beta = m_coreContext.loadUser(1002);
        OpenAcdAgent agent = new OpenAcdAgent();
        agent.setGroup(group);
        agent.setUser(beta);
        agent.setPin("123457");
        agent.setSecurity(OpenAcdAgent.Security.AGENT.toString());

        assertEquals(0, group.getAgents().size());
        List<OpenAcdAgent> agents = new ArrayList<OpenAcdAgent>(2);
        agents.add(supervisor);
        agents.add(agent);
        m_openAcdContextImpl.addAgentsToGroup(group, agents);
        assertEquals(2, group.getAgents().size());

        // test add same agents to another group
        OpenAcdAgentGroup anotherGroup = new OpenAcdAgentGroup();
        anotherGroup.setName("anotherGroup");
        m_openAcdContextImpl.saveAgentGroup(anotherGroup);
        assertEquals(3, m_openAcdContextImpl.getAgentGroups().size());
        List<OpenAcdAgent> existingAgents = m_openAcdContextImpl.addAgentsToGroup(anotherGroup, agents);
        assertEquals(2, existingAgents.size());

        // test save agent group with same name
        try {
            OpenAcdAgentGroup sameAgentGroupName = new OpenAcdAgentGroup();
            sameAgentGroupName.setName("Group");
            m_openAcdContextImpl.saveAgentGroup(sameAgentGroupName);
            fail();
        } catch (UserException ex) {
        }

        // test get agent group by name
        OpenAcdAgentGroup savedAgentGroup = m_openAcdContextImpl.getAgentGroupByName("Group");
        assertNotNull(savedAgentGroup);
        assertEquals("Group", savedAgentGroup.getName());

        // test modify agent group without changing name
        try {
            m_openAcdContextImpl.saveAgentGroup(savedAgentGroup);
        } catch (NameInUseException ex) {
            fail();
        }

        // test get agent group by id
        Integer id = savedAgentGroup.getId();
        OpenAcdAgentGroup agentGroupById = m_openAcdContextImpl.getAgentGroupById(id);
        assertNotNull(agentGroupById);
        assertEquals("Group", agentGroupById.getName());

        // test remove agent groups but prevent 'Default' group deletion
        assertEquals(3, m_openAcdContextImpl.getAgentGroups().size());
        Collection<Integer> agentGroupIds = new ArrayList<Integer>();
        agentGroupIds.add(defaultAgentGroup.getId());
        agentGroupIds.add(group.getId());
        agentGroupIds.add(anotherGroup.getId());
        m_openAcdContextImpl.removeAgentGroups(agentGroupIds);
        assertEquals(1, m_openAcdContextImpl.getAgentGroups().size());
    }

    public void testOpenAcdAgentCrud() throws Exception {
        loadDataSet("common/SampleUsersSeed.xml");
        User charlie = m_coreContext.loadUser(1003);

        OpenAcdAgentGroup group = new OpenAcdAgentGroup();
        group.setName("Group");
        m_openAcdContextImpl.saveAgentGroup(group);

        // test save agent
        OpenAcdAgent agent = new OpenAcdAgent();
        assertEquals(0, m_openAcdContextImpl.getAgents().size());
        agent.setGroup(group);
        agent.setUser(charlie);
        agent.setPin("123456");
        m_openAcdContextImpl.saveAgent(group, agent);
        assertEquals(1, m_openAcdContextImpl.getAgents().size());
        assertEquals(1, group.getAgents().size());
        Set<OpenAcdAgent> agents = group.getAgents();
        OpenAcdAgent savedAgent = DataAccessUtils.singleResult(agents);
        assertEquals(charlie, savedAgent.getUser());
        assertEquals("123456", savedAgent.getPin());
        assertEquals("AGENT", savedAgent.getSecurity());

        // test get agent by id
        Integer id = savedAgent.getId();
        OpenAcdAgent agentGroupById = m_openAcdContextImpl.getAgentById(id);
        assertNotNull(agentGroupById);
        assertEquals(group, agentGroupById.getGroup());
        assertEquals(charlie, agentGroupById.getUser());
        assertEquals("AGENT", agentGroupById.getSecurity());

        // test add agents to group
        User delta = m_coreContext.loadUser(1004);
        User elephant = m_coreContext.loadUser(1005);
        OpenAcdAgentGroup newGroup = new OpenAcdAgentGroup();
        newGroup.setName("NewGroup");
        OpenAcdAgent agent1 = new OpenAcdAgent();
        agent1.setGroup(newGroup);
        agent1.setPin("1234");
        agent1.setUser(delta);
        OpenAcdAgent agent2 = new OpenAcdAgent();
        agent2.setGroup(newGroup);
        agent2.setPin("123433");
        agent2.setUser(elephant);
        newGroup.addAgent(agent1);
        newGroup.addAgent(agent2);
        m_openAcdContextImpl.saveAgentGroup(newGroup);

        OpenAcdAgentGroup grp = m_openAcdContextImpl.getAgentGroupByName("NewGroup");
        assertEquals(2, grp.getAgents().size());
        assertEquals(3, m_openAcdContextImpl.getAgents().size());

        // test remove agents from group
        grp.removeAgent(agent1);
        grp.removeAgent(agent2);
        m_openAcdContextImpl.saveAgentGroup(grp);
        assertEquals(1, m_openAcdContextImpl.getAgents().size());

        // remove groups
        Collection<Integer> agentGroupIds = new ArrayList<Integer>();
        agentGroupIds.add(group.getId());
        agentGroupIds.add(newGroup.getId());
        m_openAcdContextImpl.removeAgentGroups(agentGroupIds);
        assertEquals(1, m_openAcdContextImpl.getAgentGroups().size());
    }

    public void setOpenAcdContextImpl(OpenAcdContextImpl openAcdContext) {
        m_openAcdContextImpl = openAcdContext;
        OpenAcdProvisioningContext provisioning = EasyMock.createNiceMock(OpenAcdProvisioningContext.class);
        m_openAcdContextImpl.setProvisioningContext(provisioning);
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    private class MockSipxFreeswitchService extends SipxFreeswitchService {
        @Override
        public int getFreeswitchSipPort() {
            return 50;
        }
    }
}

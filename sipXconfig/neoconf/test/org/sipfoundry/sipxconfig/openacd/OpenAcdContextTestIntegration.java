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
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
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
    private LocationsManager m_locationsManager;

    public void testOpenAcdLineCrud() throws Exception {
        loadDataSetXml("admin/commserver/seedLocations.xml");
        Location location = m_locationsManager.getLocation(101);

        // test save open acd extension
        assertEquals(0, m_openAcdContextImpl.getFreeswitchExtensions().size());
        OpenAcdLine extension = DefaultContextConfigurationTest.createOpenAcdLine("example");
        extension.setLocation(location);
        m_openAcdContextImpl.saveExtension(extension);
        assertEquals(1, m_openAcdContextImpl.getFreeswitchExtensions().size());

        // test save extension with same name
        try {
            OpenAcdLine sameNameExtension = new OpenAcdLine();
            sameNameExtension.setName("example");
            sameNameExtension.setLocation(location);
            FreeswitchCondition condition = new FreeswitchCondition();
            condition.setField("destination_number");
            condition.setExpression("^301$");
            sameNameExtension.addCondition(condition);
            m_openAcdContextImpl.saveExtension(sameNameExtension);
            fail();
        } catch (NameInUseException ex) {
        }

        // test get extension by name
        OpenAcdLine savedExtension = (OpenAcdLine) m_openAcdContextImpl.getExtensionByName("example");
        assertNotNull(extension);
        assertEquals("example", savedExtension.getName());
        // test modify extension without changing name
        try {
            m_openAcdContextImpl.saveExtension(savedExtension);
            extension.setLocation(location);
        } catch (NameInUseException ex) {
            fail();
        }

        // test get extension by id
        Integer id = savedExtension.getId();
        OpenAcdLine extensionById = (OpenAcdLine) m_openAcdContextImpl.getExtensionById(id);
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

    public void testOpenAcdCommandCrud() throws Exception {
        loadDataSetXml("admin/commserver/seedLocations.xml");
        Location location = m_locationsManager.getLocation(101);

        // test save open acd extension
        assertEquals(0, m_openAcdContextImpl.getFreeswitchExtensions().size());
        OpenAcdCommand command = new OpenAcdCommand();
        command.setName("login");
        FreeswitchCondition fscondition = new FreeswitchCondition();
        fscondition.setField("destination_number");
        fscondition.setExpression("^*89$");
        fscondition.getActions().addAll((OpenAcdCommand.getDefaultActions(location)));
        command.addCondition(fscondition);
        command.setLocation(location);
        m_openAcdContextImpl.saveExtension(command);
        assertEquals(1, m_openAcdContextImpl.getFreeswitchExtensions().size());

        // test save extension with same name
        try {
            OpenAcdCommand sameNameExtension = new OpenAcdCommand();
            sameNameExtension.setName("login");
            sameNameExtension.setLocation(location);
            FreeswitchCondition condition1 = new FreeswitchCondition();
            condition1.setField("destination_number");
            condition1.setExpression("^*90$");
            sameNameExtension.addCondition(condition1);
            m_openAcdContextImpl.saveExtension(sameNameExtension);
            fail();
        } catch (NameInUseException ex) {
        }

        // test get extension by name
        OpenAcdCommand savedExtension = (OpenAcdCommand) m_openAcdContextImpl.getExtensionByName("login");
        assertNotNull(command);
        assertEquals("login", savedExtension.getName());
        // test modify extension without changing name
        try {
            m_openAcdContextImpl.saveExtension(savedExtension);
            command.setLocation(location);
        } catch (NameInUseException ex) {
            fail();
        }

        // test get extension by id
        Integer id = savedExtension.getId();
        OpenAcdCommand extensionById = (OpenAcdCommand) m_openAcdContextImpl.getExtensionById(id);
        assertNotNull(extensionById);
        assertEquals("login", extensionById.getName());

        // test saved conditions and actions
        Set<FreeswitchCondition> conditions = extensionById.getConditions();
        assertEquals(1, conditions.size());
        for (FreeswitchCondition condition : conditions) {
            assertEquals(4, condition.getActions().size());
            List<FreeswitchAction> actions = new LinkedList<FreeswitchAction>();
            actions.addAll(condition.getActions());
            assertEquals("erlang_sendmsg", actions.get(0).getApplication());
            assertEquals(
                    "agent_dialplan_listener  testme@localhost agent_login ${sip_from_user} pstn ${sip_from_uri}",
                    actions.get(0).getData());
            assertEquals("answer", actions.get(1).getApplication());
            assertNull(actions.get(1).getData());
            assertEquals("sleep", actions.get(2).getApplication());
            assertEquals("2000", actions.get(2).getData());
            assertEquals("hangup", actions.get(3).getApplication());
            assertEquals("NORMAL_CLEARING", actions.get(3).getData());
        }

        // test remove extension
        assertEquals(1, m_openAcdContextImpl.getFreeswitchExtensions().size());
        m_openAcdContextImpl.removeExtensions(Collections.singletonList(id));
        assertEquals(0, m_openAcdContextImpl.getFreeswitchExtensions().size());
    }

    public void testOpenAcdExtensionAliasProvider() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        loadDataSetXml("admin/commserver/seedLocationsAndServices6.xml");
        SipxFreeswitchService service = new MockSipxFreeswitchService();
        service.setBeanId(SipxFreeswitchService.BEAN_ID);
        service.setLocationsManager(m_locationsManager);

        SipxServiceManager sm = TestUtil.getMockSipxServiceManager(true, service);
        m_openAcdContextImpl.setSipxServiceManager(sm);

        OpenAcdLine extension = DefaultContextConfigurationTest.createOpenAcdLine("sales");
        Location l = m_locationsManager.getLocation(101);
        extension.setLocation(l);

        m_openAcdContextImpl.saveExtension(extension);

        assertEquals(1, m_openAcdContextImpl.getBeanIdsOfObjectsWithAlias("sales").size());
        assertFalse(m_openAcdContextImpl.isAliasInUse("test"));
        assertTrue(m_openAcdContextImpl.isAliasInUse("sales"));
        assertTrue(m_openAcdContextImpl.isAliasInUse("300"));

        Collection<AliasMapping> mappings = m_openAcdContextImpl.getAliasMappings();
        assertEquals(2, mappings.size());
        Iterator<AliasMapping> iter = mappings.iterator();
        AliasMapping mapping = iter.next();
        assertEquals("openacd", mapping.getRelation());
        assertEquals("sales@example.org", mapping.getIdentity());
        assertEquals("sip:300@10.1.1.2", mapping.getContact());
        mapping = iter.next();
        assertEquals("openacd", mapping.getRelation());
        assertEquals("300@example.org", mapping.getIdentity());
        assertEquals("sip:300@10.1.1.2:50", mapping.getContact());
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

    public void testOpenAcdSkillCrud() throws Exception {
        // test existing skills in 'Language' and 'Magic' skill groups
        assertEquals(8, m_openAcdContextImpl.getSkills().size());

        // test default skills existing in 'Magic' skill group
        assertEquals(6, m_openAcdContextImpl.getDefaultSkills().size());

        // test get skills ordered by group name
        Map<String, List<OpenAcdSkill>> groupedSkills = m_openAcdContextImpl.getGroupedSkills();
        assertTrue(groupedSkills.keySet().contains("Language"));
        assertTrue(groupedSkills.keySet().contains("Magic"));
        assertEquals(2, groupedSkills.get("Language").size());
        assertEquals(6, groupedSkills.get("Magic").size());
        // test get skill by name
        OpenAcdSkill englishSkill = m_openAcdContextImpl.getSkillByName("English");
        assertNotNull(englishSkill);
        assertEquals("English", englishSkill.getName());
        assertEquals("Language", englishSkill.getGroupName());
        assertFalse(englishSkill.isDefaultSkill());

        OpenAcdSkill allSkill = m_openAcdContextImpl.getSkillByName("All");
        assertNotNull(allSkill);
        assertEquals("All", allSkill.getName());
        assertEquals("Magic", allSkill.getGroupName());
        assertTrue(allSkill.isDefaultSkill());

        // test get skill by id
        Integer id = allSkill.getId();
        OpenAcdSkill skillById = m_openAcdContextImpl.getSkillById(id);
        assertNotNull(skillById);
        assertEquals("All", skillById.getName());
        assertEquals("Magic", skillById.getGroupName());

        // test get skill by atom
        String atom = allSkill.getAtom();
        OpenAcdSkill skillByAtom = m_openAcdContextImpl.getSkillByAtom(atom);
        assertNotNull(skillByAtom);
        assertEquals("_all", skillByAtom.getAtom());
        assertEquals("All", skillByAtom.getName());
        assertEquals("Magic", skillByAtom.getGroupName());

        // test save skill without name
        OpenAcdSkill newSkill = new OpenAcdSkill();
        try {
            m_openAcdContextImpl.saveSkill(newSkill);
            fail();
        } catch (UserException ex) {
        }
        // test save skill without atom
        newSkill.setName("TestSkill");
        try {
            m_openAcdContextImpl.saveSkill(newSkill);
            fail();
        } catch (UserException ex) {
        }
        // test save skill without group
        newSkill.setAtom("_atom");
        try {
            m_openAcdContextImpl.saveSkill(newSkill);
            fail();
        } catch (UserException ex) {
        }
        // test save skill
        newSkill.setGroupName("Group");
        assertEquals(8, m_openAcdContextImpl.getSkills().size());
        m_openAcdContextImpl.saveSkill(newSkill);
        assertEquals(9, m_openAcdContextImpl.getSkills().size());

        // test remove agent groups but prevent default skills deletion
        assertEquals(9, m_openAcdContextImpl.getSkills().size());
        Collection<Integer> skillIds = new ArrayList<Integer>();
        skillIds.add(allSkill.getId());
        skillIds.add(newSkill.getId());
        m_openAcdContextImpl.removeSkills(skillIds);
        assertEquals(8, m_openAcdContextImpl.getSkills().size());
    }

    public void testManageAgentGroupWithSkill() throws Exception {
        OpenAcdSkill englishSkill = m_openAcdContextImpl.getSkillByName("English");
        OpenAcdSkill germanSkill = m_openAcdContextImpl.getSkillByName("German");
        OpenAcdAgentGroup group = new OpenAcdAgentGroup();
        group.setName("Group");
        group.addSkill(englishSkill);
        m_openAcdContextImpl.saveAgentGroup(group);
        assertTrue(m_openAcdContextImpl.getAgentGroupByName("Group").getSkills().contains(englishSkill));

        // cannot delete assigned skill
        List<Integer> ids = new ArrayList<Integer>();
        ids.add(englishSkill.getId());
        ids.add(germanSkill.getId());
        assertEquals(8, m_openAcdContextImpl.getSkills().size());
        List<String> skills = m_openAcdContextImpl.removeSkills(ids);
        assertTrue(skills.contains("English"));
        assertEquals(7, m_openAcdContextImpl.getSkills().size());
    }

    public void testManageAgentWithSkill() throws Exception {
        loadDataSet("common/SampleUsersSeed.xml");
        User charlie = m_coreContext.loadUser(1003);
        OpenAcdSkill englishSkill = m_openAcdContextImpl.getSkillByName("English");
        OpenAcdSkill germanSkill = m_openAcdContextImpl.getSkillByName("German");
        OpenAcdAgentGroup group = new OpenAcdAgentGroup();
        group.setName("Group");
        m_openAcdContextImpl.saveAgentGroup(group);

        OpenAcdAgent agent = new OpenAcdAgent();
        agent.setGroup(group);
        agent.setUser(charlie);
        agent.setPin("123456");
        agent.addSkill(englishSkill);
        m_openAcdContextImpl.saveAgent(group, agent);
        assertTrue(m_openAcdContextImpl.getAgentByUser(charlie).getSkills().contains(englishSkill));

        // cannot delete assigned skill
        List<Integer> ids = new ArrayList<Integer>();
        ids.add(englishSkill.getId());
        ids.add(germanSkill.getId());
        assertEquals(8, m_openAcdContextImpl.getSkills().size());
        // cannot delete assigned skill
        List<String> skills = m_openAcdContextImpl.removeSkills(ids);
        assertTrue(skills.contains("English"));
        assertEquals(7, m_openAcdContextImpl.getSkills().size());
    }

    public void testGetSkillsAtoms() {
        OpenAcdSkill skill1 = new OpenAcdSkill();
        skill1.setName("Skill1");
        skill1.setAtom("_skill1");
        OpenAcdSkill skill2 = new OpenAcdSkill();
        skill2.setName("Skill2");
        skill2.setAtom("_skill2");
        OpenAcdAgentGroup group = new OpenAcdAgentGroup();
        group.addSkill(skill1);
        group.addSkill(skill2);
        OpenAcdAgent agent = new OpenAcdAgent();
        agent.addSkill(skill1);
        agent.addSkill(skill2);
        assertEquals("_skill1, _skill2", group.getSkillsAtoms());
        assertEquals("_skill1, _skill2", agent.getSkillsAtoms());
    }

    public void testGetGroupedSkills() {
        OpenAcdSkill newSkill = new OpenAcdSkill();
        newSkill.setName("NewSkill");
        newSkill.setAtom("_new");
        newSkill.setGroupName("NewGroup");
        m_openAcdContextImpl.saveSkill(newSkill);

        OpenAcdSkill anotherSkill = new OpenAcdSkill();
        anotherSkill.setName("AnotherSkill");
        anotherSkill.setAtom("_another");
        anotherSkill.setGroupName("NewGroup");
        m_openAcdContextImpl.saveSkill(anotherSkill);

        OpenAcdSkill thirdSkill = new OpenAcdSkill();
        thirdSkill.setName("ThirdSkill");
        thirdSkill.setAtom("_third");
        thirdSkill.setGroupName("ThirdGroup");
        m_openAcdContextImpl.saveSkill(thirdSkill);

        Map<String, List<OpenAcdSkill>> skills = m_openAcdContextImpl.getGroupedSkills();
        assertEquals(2, skills.get("NewGroup").size());
        assertTrue(skills.get("NewGroup").contains(m_openAcdContextImpl.getSkillByAtom("_new")));
        assertTrue(skills.get("NewGroup").contains(m_openAcdContextImpl.getSkillByAtom("_another")));
        assertEquals(1, skills.get("ThirdGroup").size());
        assertTrue(skills.get("ThirdGroup").contains(m_openAcdContextImpl.getSkillByAtom("_third")));
        assertEquals(2, skills.get("Language").size());
        assertEquals(6, skills.get("Magic").size());
    }

    public void testOpenAcdClient() {
        OpenAcdClient client = new OpenAcdClient();
        client.setName("client");
        client.setIdentity("10101");
        m_openAcdContextImpl.saveClient(client);
        assertEquals(1, m_openAcdContextImpl.getClients().size());

        OpenAcdClient anotherClient = new OpenAcdClient();
        anotherClient.setName("client");
        try {
            m_openAcdContextImpl.saveClient(anotherClient);
            fail();
        } catch (UserException ex) {
        }
        anotherClient.setName("anotherClient");
        anotherClient.setIdentity("10101");
        try {
            m_openAcdContextImpl.saveClient(anotherClient);
            fail();
        } catch (UserException ex) {
        }
        anotherClient.setIdentity("11111");
        m_openAcdContextImpl.saveClient(anotherClient);
        assertEquals(2, m_openAcdContextImpl.getClients().size());

        m_openAcdContextImpl.removeClients(Collections.singleton(anotherClient.getId()));
        assertEquals(1, m_openAcdContextImpl.getClients().size());

        assertEquals("client", m_openAcdContextImpl.getClientById(client.getId()).getName());
    }

    public void testOpenAcdQueueGroupCrud() throws Exception {
        // 'Default' queue group
        assertEquals(1, m_openAcdContextImpl.getQueueGroups().size());

        // test get queue group by name
        OpenAcdQueueGroup defaultQueueGroup = m_openAcdContextImpl.getQueueGroupByName("Default");
        assertNotNull(defaultQueueGroup);
        assertEquals("Default", defaultQueueGroup.getName());

        // test save queue group without name
        OpenAcdQueueGroup group = new OpenAcdQueueGroup();
        try {
            m_openAcdContextImpl.saveQueueGroup(group);
            fail();
        } catch (UserException ex) {
        }

        // test save queue group
        group.setName("QueueGroup");
        m_openAcdContextImpl.saveQueueGroup(group);
        assertEquals(2, m_openAcdContextImpl.getQueueGroups().size());

        // test get queue group by id
        OpenAcdQueueGroup existingQueueGroup = m_openAcdContextImpl.getQueueGroupById(group.getId());
        assertNotNull(existingQueueGroup);
        assertEquals("QueueGroup", existingQueueGroup.getName());

        // test save queue group with the same name
        OpenAcdQueueGroup anotherGroup = new OpenAcdQueueGroup();
        anotherGroup.setName("QueueGroup");
        try {
            m_openAcdContextImpl.saveQueueGroup(anotherGroup);
            fail();
        } catch (UserException ex) {
        }

        // test save queue group with skills
        assertEquals(2, m_openAcdContextImpl.getQueueGroups().size());
        Set<OpenAcdSkill> skills = new LinkedHashSet<OpenAcdSkill>();
        OpenAcdSkill englishSkill = m_openAcdContextImpl.getSkillByName("English");
        OpenAcdSkill germanSkill = m_openAcdContextImpl.getSkillByName("German");
        skills.add(englishSkill);
        skills.add(germanSkill);
        OpenAcdQueueGroup groupWithSkills = new OpenAcdQueueGroup();
        groupWithSkills.setName("QueueGroupWithSkills");
        groupWithSkills.setSkills(skills);
        m_openAcdContextImpl.saveQueueGroup(groupWithSkills);
        assertEquals(3, m_openAcdContextImpl.getQueueGroups().size());

        // test remove queue group but prevent 'Default' queue group deletion
        Collection<Integer> queueGroupIds = new ArrayList<Integer>();
        queueGroupIds.add(defaultQueueGroup.getId());
        queueGroupIds.add(group.getId());
        queueGroupIds.add(groupWithSkills.getId());
        m_openAcdContextImpl.removeQueueGroups(queueGroupIds);
        assertEquals(1, m_openAcdContextImpl.getQueueGroups().size());
        assertNotNull(m_openAcdContextImpl.getQueueGroupByName("Default"));
    }

    public void testOpenAcdQueueCrud() throws Exception {
        // get 'default_queue' queue
        assertEquals(1, m_openAcdContextImpl.getQueues().size());

        // test get queue by name
        OpenAcdQueue defaultQueue = m_openAcdContextImpl.getQueueByName("default_queue");
        assertNotNull(defaultQueue);
        assertEquals("default_queue", defaultQueue.getName());
        assertEquals("Default", defaultQueue.getGroup().getName());

        // test save queue without name
        OpenAcdQueue queue = new OpenAcdQueue();
        try {
            m_openAcdContextImpl.saveQueue(queue);
            fail();
        } catch (UserException ex) {
        }

        // test save queue
        OpenAcdQueueGroup defaultQueueGroup = m_openAcdContextImpl.getQueueGroupByName("Default");
        assertNotNull(defaultQueueGroup);
        queue.setName("Queue1");
        queue.setGroup(defaultQueueGroup);
        m_openAcdContextImpl.saveQueue(queue);
        assertEquals(2, m_openAcdContextImpl.getQueues().size());

        // test get queue by id
        OpenAcdQueue existingQueue = m_openAcdContextImpl.getQueueById(queue.getId());
        assertNotNull(existingQueue);
        assertEquals("Queue1", existingQueue.getName());
        assertEquals("Default", existingQueue.getGroup().getName());

        // test save queue group with the same name
        OpenAcdQueue anotherQueue = new OpenAcdQueue();
        anotherQueue.setName("Queue1");
        try {
            m_openAcdContextImpl.saveQueue(anotherQueue);
            fail();
        } catch (UserException ex) {
        }

        // test save queue with skills
        assertEquals(2, m_openAcdContextImpl.getQueues().size());
        Set<OpenAcdSkill> skills = new LinkedHashSet<OpenAcdSkill>();
        OpenAcdSkill englishSkill = m_openAcdContextImpl.getSkillByName("English");
        OpenAcdSkill germanSkill = m_openAcdContextImpl.getSkillByName("German");
        skills.add(englishSkill);
        skills.add(germanSkill);
        OpenAcdQueue queueWithSkills = new OpenAcdQueue();
        queueWithSkills.setName("QueueWithSkills");
        queueWithSkills.setGroup(defaultQueueGroup);
        queueWithSkills.setSkills(skills);
        m_openAcdContextImpl.saveQueue(queueWithSkills);
        assertEquals(3, m_openAcdContextImpl.getQueues().size());

        // test remove all queues
        Collection<Integer> queueIds = new ArrayList<Integer>();
        queueIds.add(defaultQueue.getId());
        queueIds.add(queue.getId());
        queueIds.add(queueWithSkills.getId());
        m_openAcdContextImpl.removeQueues(queueIds);
        assertEquals(0, m_openAcdContextImpl.getQueues().size());
    }

    public void setOpenAcdContextImpl(OpenAcdContextImpl openAcdContext) {
        m_openAcdContextImpl = openAcdContext;
        OpenAcdProvisioningContext provisioning = EasyMock.createNiceMock(OpenAcdProvisioningContext.class);
        m_openAcdContextImpl.setProvisioningContext(provisioning);
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    private class MockSipxFreeswitchService extends SipxFreeswitchService {
        @Override
        public int getFreeswitchSipPort() {
            return 50;
        }
    }
}

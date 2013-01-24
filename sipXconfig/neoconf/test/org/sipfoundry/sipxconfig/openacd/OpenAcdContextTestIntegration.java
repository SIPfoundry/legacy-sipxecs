/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.openacd;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.easymock.EasyMock;
import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.ExtensionInUseException;
import org.sipfoundry.sipxconfig.common.NameInUseException;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SameExtensionException;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchCondition;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.mongo.MongoTestIntegration;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContextImpl.DefaultAgentGroupDeleteException;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContextImpl.QueueGroupInUseException;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContextImpl.QueueInUseException;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContextImpl.SkillInUseException;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeAction.ACTION;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeCondition.CONDITION;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeStep.FREQUENCY;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.registrar.Registrar;
import org.springframework.dao.support.DataAccessUtils;

import com.mongodb.DBCollection;

/*
 * included here tests for the replication provider (OpenAcdReplicationProvider)
 */
public class OpenAcdContextTestIntegration extends MongoTestIntegration {
    private OpenAcdContext m_openAcdContext;
    private static String[][] LINE_ACTIONS = {
        {
            "answer", ""
        }, {
            "set", "domain_name=$${domain}"
        }, {
            "set", "brand=1"
        }, {
            "set", "queue=Sales"
        }, {
            "set", "allow_voicemail=true"
        }, {
            "erlang_sendmsg", "freeswitch_media_manager testme@192.168.1.1 inivr ${uuid}"
        }, {
            "playback", "/usr/share/www/doc/stdprompts/welcome.wav"
        }, {
            "erlang", "freeswitch_media_manager:! testme@192.168.1.1"
        },
    };

    private static String[][] AGENT_DIAL_SRING_ACTIONS = {
        {
            "erlang_sendmsg",
            "agent_dialplan_listener testme@localhost agent_login ${sip_from_user} pstn ${sip_from_uri}"
        }, {
            "answer", ""
        }, {
            "sleep", "2000"
        }, {
            "hangup", "NORMAL_CLEARING"
        },
    };

    private CoreContext m_coreContext;
    private FeatureManager m_featureManager;
    private OpenAcdReplicationProvider m_openAcdReplicationProvider;
    private LocationsManager m_locationsManager;

    private DBCollection getEntityCollection() {
        return getImdb().getCollection("entity");
    }

    @Override
    protected void onSetUpInTransaction() throws Exception {
        clear();
        loadDataSetXml("commserver/seedLocations.xml");
        loadDataSetXml("domain/DomainSeed.xml");
        sql("openacd/openacd.sql");
        getEntityCollection().drop();
        m_featureManager.enableLocationFeature(Registrar.FEATURE, m_locationsManager.getPrimaryLocation(), true);
        m_featureManager.enableLocationFeature(ProxyManager.FEATURE, m_locationsManager.getPrimaryLocation(), true);
        m_featureManager
                .enableLocationFeature(OpenAcdContext.FEATURE, m_locationsManager.getPrimaryLocation(), true);
    }

    public OpenAcdLine createOpenAcdLine(String extensionName) {
        OpenAcdLine extension = m_openAcdContext.newOpenAcdLine();
        extension.setName(extensionName);

        FreeswitchCondition condition = new FreeswitchCondition();
        condition.setField("destination_number");
        condition.setExpression("^300$");
        extension.addCondition(condition);

        for (int i = 0; i < LINE_ACTIONS.length; i++) {
            FreeswitchAction action = new FreeswitchAction();
            action.setApplication(LINE_ACTIONS[i][0]);
            action.setData(LINE_ACTIONS[i][1]);
            condition.addAction(action);
        }
        Address address = new Address(FreeswitchFeature.SIP_ADDRESS, "127.0.0.1", 1234);
        AddressManager mockAddressManager = EasyMock.createMock(AddressManager.class);
        mockAddressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        EasyMock.expectLastCall().andReturn(address).anyTimes();

        EasyMock.replay(mockAddressManager);

        extension.setAddressManager(mockAddressManager);
        return extension;
    }

    // test save open acd extension
    public void testOpenAcdLineCrud() throws Exception {
        assertEquals(0, m_openAcdContext.getLines().size());
        OpenAcdLine extension = createOpenAcdLine("example");
        extension.setAlias("alias");
        extension.setDid("1234567890");

        m_openAcdContext.saveExtension(extension);
        assertEquals(1, m_openAcdContext.getLines().size());
        m_openAcdContext.saveExtension(extension);
        // test save extension with same name
        try {
            OpenAcdLine sameNameExtension = new OpenAcdLine();
            sameNameExtension.setName("example");
            FreeswitchCondition condition = new FreeswitchCondition();
            condition.setField("destination_number");
            condition.setExpression("^301$");
            sameNameExtension.addCondition(condition);
            m_openAcdContext.saveExtension(sameNameExtension);
            fail();
        } catch (NameInUseException ex) {
        }

        OpenAcdLine line2 = new OpenAcdLine();
        line2.setName("example1");
        FreeswitchCondition condition2 = new FreeswitchCondition();
        condition2.setField("destination_number");
        condition2.setExpression("^301$");
        line2.addCondition(condition2);
        try {
            line2.setAlias("alias");// existing alias
            m_openAcdContext.saveExtension(line2);
            fail();
        } catch (ExtensionInUseException ex) {
        }
        try {
            line2.setAlias("300");// existing alias
            m_openAcdContext.saveExtension(line2);
            fail();
        } catch (ExtensionInUseException ex) {
        }
        try {
            line2.setAlias("");
            line2.setDid("alias");
            m_openAcdContext.saveExtension(line2);
            fail();
        } catch (ExtensionInUseException ex) {
        }
        try {
            line2.setDid("300");
            m_openAcdContext.saveExtension(line2);
            fail();
        } catch (ExtensionInUseException ex) {
        }
        try {
            line2.setAlias("alias");// existing alias
            m_openAcdContext.saveExtension(line2);
            fail();
        } catch (ExtensionInUseException ex) {
        }

        // test get extension by name
        OpenAcdLine savedExtension = (OpenAcdLine) m_openAcdContext.getExtensionByName("example");
        assertNotNull(extension);
        assertEquals("example", savedExtension.getName());
        // test modify extension without changing name
        try {
            m_openAcdContext.saveExtension(savedExtension);
        } catch (NameInUseException ex) {
            fail();
        }

        // test get extension by id
        Integer id = savedExtension.getId();
        OpenAcdLine extensionById = (OpenAcdLine) m_openAcdContext.getExtensionById(id);
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

        extension.setAlias("alias");
        extension.setDid("alias");
        try {
            m_openAcdContext.saveExtension(extension);
            fail();
        } catch (SameExtensionException e) {
        }

        extension.setAlias("300");
        try {
            m_openAcdContext.saveExtension(extension);
            fail();
        } catch (SameExtensionException e) {
        }

        extension.setDid("300");
        try {
            m_openAcdContext.saveExtension(extension);
            fail();
        } catch (SameExtensionException e) {
        }
        // test remove extension
        assertEquals(1, m_openAcdContext.getLines().size());
        m_openAcdContext.deleteExtension(extensionById);
        assertEquals(0, m_openAcdContext.getLines().size());
    }

    public static OpenAcdCommand createOpenAcdAgentDialString(String dialStringName) {
        OpenAcdCommand agentDialString = new OpenAcdCommand();
        agentDialString.setName(dialStringName);

        FreeswitchCondition condition = new FreeswitchCondition();
        condition.setField("destination_number");
        condition.setExpression("^*99$");
        agentDialString.addCondition(condition);

        for (int i = 0; i < AGENT_DIAL_SRING_ACTIONS.length; i++) {
            FreeswitchAction action = new FreeswitchAction();
            action.setApplication(AGENT_DIAL_SRING_ACTIONS[i][0]);
            action.setData(AGENT_DIAL_SRING_ACTIONS[i][1]);
            condition.addAction(action);
        }
        Address address = new Address(FreeswitchFeature.SIP_ADDRESS, "127.0.0.1", 1234);
        AddressManager mockAddressManager = EasyMock.createMock(AddressManager.class);
        mockAddressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        EasyMock.expectLastCall().andReturn(address).anyTimes();

        EasyMock.replay(mockAddressManager);

        agentDialString.setAddressManager(mockAddressManager);
        return agentDialString;
    }

    public void testOpenAcdCommandCrud() throws Exception {
        // test existing 'login', 'logout', 'go available' and 'release' default agent dial
        // strings
        assertEquals(4, m_openAcdContext.getCommands().size());

        // test save agent dial string
        OpenAcdCommand dialString = createOpenAcdAgentDialString("test");
        m_openAcdContext.saveExtension(dialString);
        assertEquals(5, m_openAcdContext.getCommands().size());

        // test save agent dial string with same name
        try {
            OpenAcdCommand sameName = new OpenAcdCommand();
            sameName.setName("test");
            FreeswitchCondition condition = new FreeswitchCondition();
            condition.setField("destination_number");
            condition.setExpression("^*100$");
            sameName.addCondition(condition);
            m_openAcdContext.saveExtension(sameName);
            fail();
        } catch (NameInUseException ex) {
        }

        // test get agent dial string by name
        OpenAcdCommand savedDialString = (OpenAcdCommand) m_openAcdContext.getExtensionByName("test");
        assertNotNull(savedDialString);
        assertEquals("test", savedDialString.getName());

        // test get agent dial string by id
        Integer id = savedDialString.getId();
        OpenAcdCommand dialStringById = (OpenAcdCommand) m_openAcdContext.getExtensionById(id);
        assertNotNull(dialStringById);
        assertEquals("test", dialStringById.getName());

        // test remove agent dial string
        assertEquals(5, m_openAcdContext.getFreeswitchExtensions().size());
        m_openAcdContext.deleteExtension(dialStringById);
        assertEquals(4, m_openAcdContext.getFreeswitchExtensions().size());
    }

    public void testOpenAcdExtensionAliasProvider() throws Exception {
        OpenAcdLine extension = createOpenAcdLine("sales");
        m_openAcdContext.saveExtension(extension);
        assertEquals(1, m_openAcdContext.getBeanIdsOfObjectsWithAlias("sales").size());
        assertFalse(m_openAcdContext.isAliasInUse("test"));
        assertTrue(m_openAcdContext.isAliasInUse("sales"));
        assertTrue(m_openAcdContext.isAliasInUse("300"));

    }

    public void testOpenAcdAgentGroupCrud() throws Exception {
        // 'Default' agent group
        assertEquals(1, m_openAcdContext.getAgentGroups().size());

        // test get agent group by name
        OpenAcdAgentGroup defaultAgentGroup = m_openAcdContext.getAgentGroupByName("Default");
        assertNotNull(defaultAgentGroup);
        assertEquals("Default", defaultAgentGroup.getName());

        // test save agent group without name
        OpenAcdAgentGroup group = new OpenAcdAgentGroup();
        try {
            m_openAcdContext.saveAgentGroup(group);
            fail();
        } catch (UserException ex) {
        }

        // test save agent group without agents
        assertEquals(1, m_openAcdContext.getAgentGroups().size());
        group.setName("Group");
        group.setDescription("Group description");
        m_openAcdContext.saveAgentGroup(group);
        assertEquals(2, m_openAcdContext.getAgentGroups().size());
        MongoTestCaseHelper.assertObjectWithFieldsValuesPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME
        }, new String[] {
            "openacdagentgroup", "Group"
        });

        // test save agent group with agents
        loadDataSet("common/SampleUsersSeed.xml");
        User alpha = m_coreContext.loadUser(1001);
        OpenAcdAgent supervisor = new OpenAcdAgent();
        supervisor.setGroup(group);
        supervisor.setUser(alpha);
        supervisor.setSecurity(OpenAcdAgent.Security.SUPERVISOR.toString());
        m_openAcdContext.saveAgent(supervisor);

        User beta = m_coreContext.loadUser(1002);
        OpenAcdAgent agent = new OpenAcdAgent();
        agent.setGroup(group);
        agent.setUser(beta);
        agent.setSecurity(OpenAcdAgent.Security.AGENT.toString());
        m_openAcdContext.saveAgent(agent);

        assertEquals(0, group.getAgents().size());
        group.addAgent(agent);
        group.addAgent(supervisor);
        m_openAcdContext.saveAgentGroup(group);

        assertEquals(2, group.getAgents().size());
        MongoTestCaseHelper.assertObjectWithFieldsValuesPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME, OpenAcdContext.AGENT_GROUP
        }, new String[] {
            "openacdagent", "alpha", "Group"
        });
        MongoTestCaseHelper.assertObjectWithFieldsValuesPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME, OpenAcdContext.AGENT_GROUP
        }, new String[] {
            "openacdagent", "beta", "Group"
        });
        // test save agent group with same name
        try {
            OpenAcdAgentGroup sameAgentGroupName = new OpenAcdAgentGroup();
            sameAgentGroupName.setName("Group");
            m_openAcdContext.saveAgentGroup(sameAgentGroupName);
            fail();
        } catch (UserException ex) {
        }

        // test get agent group by name
        OpenAcdAgentGroup savedAgentGroup = m_openAcdContext.getAgentGroupByName("Group");
        assertNotNull(savedAgentGroup);
        assertEquals("Group", savedAgentGroup.getName());

        // test modify agent group without changing name
        try {
            m_openAcdContext.saveAgentGroup(savedAgentGroup);
        } catch (NameInUseException ex) {
            fail();
        }

        // test get agent group by id
        Integer id = savedAgentGroup.getId();
        OpenAcdAgentGroup agentGroupById = m_openAcdContext.getAgentGroupById(id);
        assertNotNull(agentGroupById);
        assertEquals("Group", agentGroupById.getName());

        // test remove agent groups but prevent 'Default' group deletion
        OpenAcdAgentGroup anotherGroup = new OpenAcdAgentGroup();
        anotherGroup.setName("anotherGroup");
        m_openAcdContext.saveAgentGroup(anotherGroup);
        assertEquals(3, m_openAcdContext.getAgentGroups().size());

        try {
            m_openAcdContext.deleteAgentGroup(defaultAgentGroup);
            fail();
        } catch (DefaultAgentGroupDeleteException e) {
        }
        m_openAcdContext.deleteAgentGroup(group);
        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME
        }, new String[] {
            "openacdagentgroup", "Group"
        });
        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME, OpenAcdContext.AGENT_GROUP
        }, new String[] {
            "openacdagent", "alpha", "Group"
        });
        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME, OpenAcdContext.AGENT_GROUP
        }, new String[] {
            "openacdagent", "beta", "Group"
        });
        m_openAcdContext.deleteAgentGroup(anotherGroup);
        assertEquals(1, m_openAcdContext.getAgentGroups().size());
    }

    public void testOpenAcdAgentCrud() throws Exception {
        loadDataSet("common/SampleUsersSeed.xml");
        loadDataSetXml("commserver/seedLocations.xml");
        loadDataSetXml("domain/DomainSeed.xml");

        User charlie = m_coreContext.loadUser(1003);

        OpenAcdAgentGroup group = new OpenAcdAgentGroup();
        group.setName("Group");
        m_openAcdContext.saveAgentGroup(group);
        // test save agent
        OpenAcdAgent agent = new OpenAcdAgent();
        assertEquals(0, m_openAcdContext.getAgents().size());
        agent.setGroup(group);
        agent.setUser(charlie);
        m_openAcdContext.saveAgent(agent);
        MongoTestCaseHelper.assertObjectWithFieldsValuesPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME, OpenAcdContext.AGENT_GROUP
        }, new String[] {
            "openacdagent", "charlie", "Group"
        });
        group.addAgent(agent);
        m_openAcdContext.saveAgentGroup(group);

        List<Replicable> replicables = m_openAcdReplicationProvider.getReplicables();
        assertTrue(replicables.contains(agent));
        assertTrue(replicables.contains(group));

        assertEquals(1, m_openAcdContext.getAgents().size());
        assertEquals(1, group.getAgents().size());

        Set<OpenAcdAgent> agents = group.getAgents();
        OpenAcdAgent savedAgent = DataAccessUtils.singleResult(agents);
        assertEquals(charlie, savedAgent.getUser());
        assertEquals("AGENT", savedAgent.getSecurity());

        // test get agent by id
        Integer id = savedAgent.getId();
        OpenAcdAgent agentById = m_openAcdContext.getAgentById(id);
        assertNotNull(agentById);
        assertEquals(group, agentById.getGroup());
        assertEquals(charlie, agentById.getUser());
        assertEquals("AGENT", agentById.getSecurity());

        // test update agent
        agentById.setSecurity(OpenAcdAgent.Security.ADMIN.toString());
        m_openAcdContext.saveAgent(agentById);
        // FIXME: this test will always pass because agent will be retrieved from the session and
        // not from DB.
        assertEquals(OpenAcdAgent.Security.ADMIN.toString(), m_openAcdContext.getAgentById(id).getSecurity());

        // test add agents to group
        User delta = m_coreContext.loadUser(1004);
        User elephant = m_coreContext.loadUser(1005);

        OpenAcdAgentGroup newGroup = new OpenAcdAgentGroup();
        newGroup.setName("NewGroup");
        m_openAcdContext.saveAgentGroup(newGroup);

        OpenAcdAgentGroup grp = m_openAcdContext.getAgentGroupByName("NewGroup");

        OpenAcdAgent agent1 = new OpenAcdAgent();
        agent1.setGroup(grp);
        agent1.setUser(delta);
        m_openAcdContext.saveAgent(agent1);

        OpenAcdAgent agent2 = new OpenAcdAgent();
        agent2.setGroup(grp);
        agent2.setUser(elephant);
        m_openAcdContext.saveAgent(agent2);

        getHibernateTemplate().flush();

        grp.addAgent(agent1);
        grp.addAgent(agent2);
        m_openAcdContext.saveAgentGroup(newGroup);

        OpenAcdAgentGroup grp1 = m_openAcdContext.getAgentGroupByName("NewGroup");
        assertEquals(2, grp1.getAgents().size());
        assertEquals(3, m_openAcdContext.getAgents().size());

        // remove agents
        newGroup.getAgents().remove(agent2);
        m_openAcdContext.saveAgentGroup(newGroup);
        m_openAcdContext.deleteAgent(agent2);

        assertEquals(2, m_openAcdContext.getAgents().size());
        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME
        }, new String[] {
            "openacdagent", "elephant"
        });

        // remove user should remove also associated agent
        User newAgent = m_coreContext.newUser();
        newAgent.setUserName("test");
        m_coreContext.saveUser(newAgent);
        OpenAcdAgent agent3 = new OpenAcdAgent();
        agent3.setGroup(newGroup);
        agent3.setUser(newAgent);
        m_openAcdContext.saveAgent(agent3);
        assertEquals(3, m_openAcdContext.getAgents().size());

        MongoTestCaseHelper.assertObjectWithFieldsValuesPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME
        }, new String[] {
            "openacdagent", "test"
        });

        m_coreContext.deleteUser(newAgent);

        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME
        }, new String[] {
            "openacdagent", "test"
        });
        assertEquals(2, m_openAcdContext.getAgents().size());

        // remove groups
        Collection<Integer> agentGroupIds = new ArrayList<Integer>();
        agentGroupIds.add(group.getId());
        agentGroupIds.add(newGroup.getId());
        m_openAcdContext.deleteAgentGroup(group);
        m_openAcdContext.deleteAgentGroup(newGroup);
        assertEquals(1, m_openAcdContext.getAgentGroups().size());
    }

    public void testOpenAcdSkillGroupCrud() throws Exception {
        // test existing 'Language' and 'Magic' skill groups
        assertEquals(2, m_openAcdContext.getSkillGroups().size());

        // test get skill group by name
        OpenAcdSkillGroup magicSkillGroup = m_openAcdContext.getSkillGroupByName("Magic");
        assertNotNull(magicSkillGroup);
        assertEquals("Magic", magicSkillGroup.getName());
        OpenAcdSkillGroup languageSkillGroup = m_openAcdContext.getSkillGroupByName("Language");
        assertNotNull(languageSkillGroup);
        assertEquals("Language", languageSkillGroup.getName());

        // test save skill group without name
        OpenAcdSkillGroup group = new OpenAcdSkillGroup();
        try {
            m_openAcdContext.saveSkillGroup(group);
            fail();
        } catch (UserException ex) {
        }

        // test save skill group
        assertEquals(2, m_openAcdContext.getSkillGroups().size());
        group.setName("MySkillGroup");
        m_openAcdContext.saveSkillGroup(group);
        assertEquals(3, m_openAcdContext.getSkillGroups().size());

        // test save queue group with the same name
        OpenAcdSkillGroup anotherGroup = new OpenAcdSkillGroup();
        anotherGroup.setName("MySkillGroup");
        try {
            m_openAcdContext.saveSkillGroup(anotherGroup);
            fail();
        } catch (UserException ex) {
        }

        // test get skill group by id
        OpenAcdSkillGroup existingSkillGroup = m_openAcdContext.getSkillGroupById(group.getId());
        assertNotNull(existingSkillGroup);
        assertEquals("MySkillGroup", existingSkillGroup.getName());

        // test remove skill group but prevent 'Magic' skill group deletion
        assertEquals(3, m_openAcdContext.getSkillGroups().size());
        Collection<Integer> ids = new ArrayList<Integer>();
        ids.add(magicSkillGroup.getId());
        ids.add(languageSkillGroup.getId());
        ids.add(group.getId());
        m_openAcdContext.removeSkillGroups(ids);
    }

    public void testOpenAcdSkillCrud() throws Exception {
        // test existing skills in 'Language' and 'Magic' skill groups
        assertEquals(8, m_openAcdContext.getSkills().size());

        // test default skills existing in 'Magic' skill group
        assertEquals(6, m_openAcdContext.getDefaultSkills().size());

        // test default skills for 'default_queue' queue
        OpenAcdQueue defaultQueue = m_openAcdContext.getQueueByName("default_queue");
        assertEquals(2, defaultQueue.getSkills().size());
        List<String> skillNames = new ArrayList<String>();
        for (OpenAcdSkill skill : defaultQueue.getSkills()) {
            skillNames.add(skill.getName());
        }
        assertTrue(skillNames.contains("English"));
        assertTrue(skillNames.contains("Node"));

        // test get skills ordered by group name
        Map<String, List<OpenAcdSkill>> groupedSkills = m_openAcdContext.getGroupedSkills();
        assertTrue(groupedSkills.keySet().contains("Language"));
        assertTrue(groupedSkills.keySet().contains("Magic"));
        assertEquals(2, groupedSkills.get("Language").size());
        assertEquals(6, groupedSkills.get("Magic").size());
        // test get skill by name
        OpenAcdSkill englishSkill = m_openAcdContext.getSkillByName("English");
        assertNotNull(englishSkill);
        assertEquals("English", englishSkill.getName());
        assertEquals("Language", englishSkill.getGroupName());
        assertFalse(englishSkill.isDefaultSkill());

        OpenAcdSkill allSkill = m_openAcdContext.getSkillByName("All");
        assertNotNull(allSkill);
        assertEquals("All", allSkill.getName());
        assertEquals("Magic", allSkill.getGroupName());
        assertTrue(allSkill.isDefaultSkill());

        // test get skill by id
        Integer id = allSkill.getId();
        OpenAcdSkill skillById = m_openAcdContext.getSkillById(id);
        assertNotNull(skillById);
        assertEquals("All", skillById.getName());
        assertEquals("Magic", skillById.getGroupName());

        // test get skill by atom
        String atom = allSkill.getAtom();
        OpenAcdSkill skillByAtom = m_openAcdContext.getSkillByAtom(atom);
        assertNotNull(skillByAtom);
        assertEquals("_all", skillByAtom.getAtom());
        assertEquals("All", skillByAtom.getName());
        assertEquals("Magic", skillByAtom.getGroupName());

        // test save skill without name
        OpenAcdSkill newSkill = new OpenAcdSkill();
        try {
            m_openAcdContext.saveSkill(newSkill);
            fail();
        } catch (UserException ex) {
        }
        // test save skill without atom
        newSkill.setName("TestSkill");
        try {
            m_openAcdContext.saveSkill(newSkill);
            fail();
        } catch (UserException ex) {
        }
        // test save skill without group
        newSkill.setAtom("_atom");
        try {
            m_openAcdContext.saveSkill(newSkill);
            fail();
        } catch (UserException ex) {
        }
        // test save a magic skill
        OpenAcdSkillGroup magicSkillGroup = m_openAcdContext
                .getSkillGroupByName(OpenAcdContext.MAGIC_SKILL_GROUP_NAME);
        newSkill.setGroup(magicSkillGroup);
        try {
            m_openAcdContext.saveSkill(newSkill);
            fail();
        } catch (UserException ex) {
        }

        // test save skill
        OpenAcdSkillGroup skillGroup = new OpenAcdSkillGroup();
        skillGroup.setName("Group");
        m_openAcdContext.saveSkillGroup(skillGroup);
        newSkill.setGroup(skillGroup);

        List<Replicable> replicables = m_openAcdReplicationProvider.getReplicables();
        for (OpenAcdSkill skill : m_openAcdContext.getSkills()) {
            assertTrue(replicables.contains(skill));
        }

        assertEquals(8, m_openAcdContext.getSkills().size());
        m_openAcdContext.saveSkill(newSkill);
        assertEquals(9, m_openAcdContext.getSkills().size());
        MongoTestCaseHelper.assertObjectWithFieldsValuesPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME, OpenAcdContext.GROUP_NAME
        }, new String[] {
            "openacdskill", "TestSkill", "Group"
        });

        // test remove skills but prevent 'magic' skills deletion
        assertEquals(9, m_openAcdContext.getSkills().size());
        try {
            m_openAcdContext.deleteSkill(allSkill);
            fail();
        } catch (SkillInUseException e) {
        }
        m_openAcdContext.deleteSkill(newSkill);
        assertEquals(8, m_openAcdContext.getSkills().size());
        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME, OpenAcdContext.AGENT_GROUP
        }, new String[] {
            "openacdskill", "TestSkill", "Group"
        });

        OpenAcdSkill newSkill1 = new OpenAcdSkill();
        newSkill1.setName("TestSkill1");
        newSkill1.setAtom("_atom");
        newSkill1.setGroup(skillGroup);
        m_openAcdContext.saveSkill(newSkill1);
        MongoTestCaseHelper.assertObjectWithFieldsValuesPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME, OpenAcdContext.GROUP_NAME
        }, new String[] {
            "openacdskill", "TestSkill1", "Group"
        });
        m_openAcdContext.removeSkillGroups(Collections.singletonList(skillGroup.getId()));
        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME, OpenAcdContext.AGENT_GROUP
        }, new String[] {
            "openacdskill", "TestSkill1", "Group"
        });

    }

    public void testManageAgentGroupWithSkill() throws Exception {
        OpenAcdSkill englishSkill = m_openAcdContext.getSkillByName("English");
        OpenAcdSkill germanSkill = m_openAcdContext.getSkillByName("German");
        OpenAcdAgentGroup group = new OpenAcdAgentGroup();
        group.setName("Group");
        group.addSkill(englishSkill);
        m_openAcdContext.saveAgentGroup(group);
        assertTrue(m_openAcdContext.getAgentGroupByName("Group").getSkills().contains(englishSkill));

        // cannot delete assigned skill
        assertEquals(8, m_openAcdContext.getSkills().size());
        try {
            m_openAcdContext.deleteSkill(englishSkill);
            fail();
        } catch (SkillInUseException e) {
        }
        m_openAcdContext.deleteSkill(germanSkill);
        assertEquals(7, m_openAcdContext.getSkills().size());
    }

    public void testManageAgentWithSkill() throws Exception {
        loadDataSet("common/SampleUsersSeed.xml");
        User charlie = m_coreContext.loadUser(1003);
        OpenAcdSkill englishSkill = m_openAcdContext.getSkillByName("English");
        OpenAcdSkill germanSkill = m_openAcdContext.getSkillByName("German");
        OpenAcdAgentGroup group = new OpenAcdAgentGroup();
        group.setName("Group");
        m_openAcdContext.saveAgentGroup(group);

        OpenAcdAgent agent = new OpenAcdAgent();
        agent.setGroup(group);
        agent.setUser(charlie);
        agent.addSkill(englishSkill);
        m_openAcdContext.saveAgent(agent);
        assertTrue(m_openAcdContext.getAgentByUser(charlie).getSkills().contains(englishSkill));

        // cannot delete assigned skill
        assertEquals(8, m_openAcdContext.getSkills().size());
        // cannot delete assigned skill
        try {
            m_openAcdContext.deleteSkill(englishSkill);
            fail();
        } catch (SkillInUseException e) {
        }
        m_openAcdContext.deleteSkill(germanSkill);
        assertEquals(7, m_openAcdContext.getSkills().size());
    }

    public void testManageQueueGroupWithSkill() throws Exception {
        OpenAcdSkill englishSkill = m_openAcdContext.getSkillByName("English");
        OpenAcdSkill germanSkill = m_openAcdContext.getSkillByName("German");
        OpenAcdQueueGroup queueGroup = new OpenAcdQueueGroup();
        queueGroup.setName("QGroup");
        queueGroup.addSkill(englishSkill);
        m_openAcdContext.saveQueueGroup(queueGroup);
        assertTrue(m_openAcdContext.getQueueGroupByName("QGroup").getSkills().contains(englishSkill));

        // cannot delete assigned skill
        assertEquals(8, m_openAcdContext.getSkills().size());
        try {
            m_openAcdContext.deleteSkill(englishSkill);
            fail();
        } catch (SkillInUseException e) {
        }
        m_openAcdContext.deleteSkill(germanSkill);
        assertEquals(7, m_openAcdContext.getSkills().size());
    }

    public void testManageQueueWithSkill() throws Exception {
        OpenAcdSkill englishSkill = m_openAcdContext.getSkillByName("English");
        OpenAcdSkill germanSkill = m_openAcdContext.getSkillByName("German");
        OpenAcdQueueGroup defaultQueueGroup = m_openAcdContext.getQueueGroupByName("Default");
        assertNotNull(defaultQueueGroup);
        OpenAcdQueue queue = new OpenAcdQueue();
        queue.setName("Queue");
        queue.setGroup(defaultQueueGroup);
        queue.addSkill(englishSkill);
        m_openAcdContext.saveQueue(queue);
        assertTrue(m_openAcdContext.getQueueByName("Queue").getSkills().contains(englishSkill));

        // cannot delete assigned skill
        assertEquals(8, m_openAcdContext.getSkills().size());
        try {
            m_openAcdContext.deleteSkill(englishSkill);
            fail();
        } catch (SkillInUseException e) {
        }
        m_openAcdContext.deleteSkill(germanSkill);
        assertEquals(7, m_openAcdContext.getSkills().size());
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

    public void testGetGroupedSkills() throws Exception {
        OpenAcdSkillGroup skillGroup = new OpenAcdSkillGroup();
        skillGroup.setName("NewGroup");
        m_openAcdContext.saveSkillGroup(skillGroup);

        OpenAcdSkill newSkill = new OpenAcdSkill();
        newSkill.setName("NewSkill");
        newSkill.setAtom("_new");
        newSkill.setGroup(skillGroup);
        m_openAcdContext.saveSkill(newSkill);

        OpenAcdSkill anotherSkill = new OpenAcdSkill();
        anotherSkill.setName("AnotherSkill");
        anotherSkill.setAtom("_another");
        anotherSkill.setGroup(skillGroup);
        m_openAcdContext.saveSkill(anotherSkill);

        OpenAcdSkillGroup anotherSkillGroup = new OpenAcdSkillGroup();
        anotherSkillGroup.setName("AnotherSkillGroup");
        m_openAcdContext.saveSkillGroup(anotherSkillGroup);

        OpenAcdSkill thirdSkill = new OpenAcdSkill();
        thirdSkill.setName("ThirdSkill");
        thirdSkill.setAtom("_third");
        thirdSkill.setGroup(anotherSkillGroup);
        m_openAcdContext.saveSkill(thirdSkill);

        Map<String, List<OpenAcdSkill>> skills = m_openAcdContext.getGroupedSkills();
        assertEquals(2, skills.get("NewGroup").size());
        assertTrue(skills.get("NewGroup").contains(m_openAcdContext.getSkillByAtom("_new")));
        assertTrue(skills.get("NewGroup").contains(m_openAcdContext.getSkillByAtom("_another")));
        assertEquals(1, skills.get("AnotherSkillGroup").size());
        assertTrue(skills.get("AnotherSkillGroup").contains(m_openAcdContext.getSkillByAtom("_third")));
        assertEquals(2, skills.get("Language").size());
        assertEquals(6, skills.get("Magic").size());
    }

    public void testOpenAcdClient() throws Exception {
        // test save client
        assertEquals(0, m_openAcdContext.getClients().size());
        OpenAcdClient client = new OpenAcdClient();
        client.setName("client");
        client.setIdentity("10101");
        m_openAcdContext.saveClient(client);
        assertEquals(1, m_openAcdContext.getClients().size());

        List<Replicable> replicables = m_openAcdReplicationProvider.getReplicables();
        assertTrue(replicables.contains(client));

        // test save client with the same name
        OpenAcdClient anotherClient = new OpenAcdClient();
        anotherClient.setName("client");
        try {
            m_openAcdContext.saveClient(anotherClient);
            fail();
        } catch (UserException ex) {
        }

        // test save client with the same identity
        anotherClient.setName("anotherClient");
        anotherClient.setIdentity("10101");
        try {
            m_openAcdContext.saveClient(anotherClient);
            fail();
        } catch (UserException ex) {
        }

        anotherClient.setIdentity("11111");
        m_openAcdContext.saveClient(anotherClient);
        assertEquals(2, m_openAcdContext.getClients().size());
        MongoTestCaseHelper.assertObjectWithFieldsValuesPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME
        }, new String[] {
            "openacdclient", "client"
        });

        // test remove clients
        m_openAcdContext.deleteClient(client);
        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME
        }, new String[] {
            "openacdclient", "client"
        });
        m_openAcdContext.deleteClient(anotherClient);
        assertEquals(0, m_openAcdContext.getClients().size());
    }

    public void testOpenAcdQueueGroupCrud() throws Exception {
        // 'Default' queue group
        assertEquals(1, m_openAcdContext.getQueueGroups().size());

        // test get queue group by name
        OpenAcdQueueGroup defaultQueueGroup = m_openAcdContext.getQueueGroupByName("Default");
        assertNotNull(defaultQueueGroup);
        assertEquals("Default", defaultQueueGroup.getName());

        // test save queue group without name
        OpenAcdQueueGroup group = new OpenAcdQueueGroup();
        try {
            m_openAcdContext.saveQueueGroup(group);
            fail();
        } catch (UserException ex) {
        }

        // test save queue group
        group.setName("QueueGroup");
        m_openAcdContext.saveQueueGroup(group);
        assertEquals(2, m_openAcdContext.getQueueGroups().size());
        MongoTestCaseHelper.assertObjectWithFieldsValuesPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME
        }, new String[] {
            "openacdqueuegroup", "QueueGroup"
        });

        // test get queue group by id
        OpenAcdQueueGroup existingQueueGroup = m_openAcdContext.getQueueGroupById(group.getId());
        assertNotNull(existingQueueGroup);
        assertEquals("QueueGroup", existingQueueGroup.getName());

        // test save queue group with the same name
        OpenAcdQueueGroup anotherGroup = new OpenAcdQueueGroup();
        anotherGroup.setName("QueueGroup");
        try {
            m_openAcdContext.saveQueueGroup(anotherGroup);
            fail();
        } catch (UserException ex) {
        }

        // test save queue group with skills
        assertEquals(2, m_openAcdContext.getQueueGroups().size());
        Set<OpenAcdSkill> skills = new LinkedHashSet<OpenAcdSkill>();
        OpenAcdSkill englishSkill = m_openAcdContext.getSkillByName("English");
        OpenAcdSkill germanSkill = m_openAcdContext.getSkillByName("German");
        skills.add(englishSkill);
        skills.add(germanSkill);
        OpenAcdQueueGroup groupWithSkills = new OpenAcdQueueGroup();
        groupWithSkills.setName("QueueGroupWithSkills");
        groupWithSkills.setSkills(skills);
        m_openAcdContext.saveQueueGroup(groupWithSkills);
        assertEquals(3, m_openAcdContext.getQueueGroups().size());

        List<Replicable> replicables = m_openAcdReplicationProvider.getReplicables();
        assertTrue(replicables.contains(groupWithSkills));
        assertTrue(replicables.contains(defaultQueueGroup));
        // test remove queue group but prevent 'Default' queue group deletion
        Collection<Integer> queueGroupIds = new ArrayList<Integer>();
        queueGroupIds.add(defaultQueueGroup.getId());
        queueGroupIds.add(group.getId());
        queueGroupIds.add(groupWithSkills.getId());
        try {
            m_openAcdContext.deleteQueueGroup(defaultQueueGroup);
            fail();
        } catch (QueueGroupInUseException e) {
        }
        m_openAcdContext.deleteQueueGroup(group);
        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME
        }, new String[] {
            "openacdqueuegroup", "QueueGroup"
        });
        m_openAcdContext.deleteQueueGroup(groupWithSkills);
        assertEquals(1, m_openAcdContext.getQueueGroups().size());
        assertNotNull(m_openAcdContext.getQueueGroupByName("Default"));
    }

    public void testOpenAcdQueueCrud() throws Exception {
        // get 'default_queue' queue
        assertEquals(1, m_openAcdContext.getQueues().size());

        // test get queue by name
        OpenAcdQueue defaultQueue = m_openAcdContext.getQueueByName("default_queue");
        assertNotNull(defaultQueue);
        assertEquals("default_queue", defaultQueue.getName());
        assertEquals("Default", defaultQueue.getGroup().getName());

        // test save queue without name
        OpenAcdQueue queue = new OpenAcdQueue();
        try {
            m_openAcdContext.saveQueue(queue);
            fail();
        } catch (UserException ex) {
        }

        // test save queue
        OpenAcdQueueGroup defaultQueueGroup = m_openAcdContext.getQueueGroupByName("Default");
        assertNotNull(defaultQueueGroup);
        queue.setName("Queue1");
        queue.setGroup(defaultQueueGroup);
        m_openAcdContext.saveQueue(queue);
        assertEquals(2, m_openAcdContext.getQueues().size());
        MongoTestCaseHelper.assertObjectWithFieldsValuesPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME
        }, new String[] {
            "openacdqueue", "Queue1"
        });

        // test get queue by id
        OpenAcdQueue existingQueue = m_openAcdContext.getQueueById(queue.getId());
        assertNotNull(existingQueue);
        assertEquals("Queue1", existingQueue.getName());
        assertEquals("Default", existingQueue.getGroup().getName());

        // test save queue group with the same name
        OpenAcdQueue anotherQueue = new OpenAcdQueue();
        anotherQueue.setName("Queue1");
        try {
            m_openAcdContext.saveQueue(anotherQueue);
            fail();
        } catch (UserException ex) {
        }

        // test save queue with skills
        assertEquals(2, m_openAcdContext.getQueues().size());
        Set<OpenAcdSkill> skills = new LinkedHashSet<OpenAcdSkill>();
        OpenAcdSkill englishSkill = m_openAcdContext.getSkillByName("English");
        OpenAcdSkill germanSkill = m_openAcdContext.getSkillByName("German");
        skills.add(englishSkill);
        skills.add(germanSkill);
        OpenAcdQueue queueWithSkills = new OpenAcdQueue();
        queueWithSkills.setName("QueueWithSkills");
        queueWithSkills.setGroup(defaultQueueGroup);
        queueWithSkills.setSkills(skills);
        m_openAcdContext.saveQueue(queueWithSkills);
        assertEquals(3, m_openAcdContext.getQueues().size());

        List<Replicable> replicables = m_openAcdReplicationProvider.getReplicables();
        assertTrue(replicables.contains(queueWithSkills));
        assertTrue(replicables.contains(defaultQueue));

        // test save queue with recipe steps
        OpenAcdRecipeAction recipeAction1 = new OpenAcdRecipeAction();
        recipeAction1.setAction(ACTION.SET_PRIORITY.toString());
        recipeAction1.setActionValue("1");

        OpenAcdRecipeCondition recipeCondition1 = new OpenAcdRecipeCondition();
        recipeCondition1.setCondition(CONDITION.TICK_INTERVAL.toString());
        recipeCondition1.setRelation("is");
        recipeCondition1.setValueCondition("6");

        OpenAcdRecipeStep recipeStep1 = new OpenAcdRecipeStep();
        recipeStep1.setName("RecipeStep1");
        recipeStep1.setFrequency(FREQUENCY.RUN_ONCE.toString());
        recipeStep1.setAction(recipeAction1);
        recipeStep1.addCondition(recipeCondition1);

        OpenAcdRecipeAction recipeAction2 = new OpenAcdRecipeAction();
        recipeAction2.setAction(ACTION.PRIORITIZE.toString());

        OpenAcdRecipeCondition recipeCondition2 = new OpenAcdRecipeCondition();
        recipeCondition2.setCondition(CONDITION.CALLS_IN_QUEUE.toString());
        recipeCondition2.setRelation("less");
        recipeCondition2.setValueCondition("10");

        OpenAcdRecipeStep recipeStep2 = new OpenAcdRecipeStep();
        recipeStep2.setName("RecipeStep2");
        recipeStep2.setFrequency(FREQUENCY.RUN_MANY.toString());
        recipeStep2.setAction(recipeAction2);
        recipeStep2.addCondition(recipeCondition1);
        recipeStep2.addCondition(recipeCondition2);

        OpenAcdQueue queueWithRecipeStep = new OpenAcdQueue();
        queueWithRecipeStep.setName("QueueRecipe");
        queueWithRecipeStep.setGroup(defaultQueueGroup);
        queueWithRecipeStep.addStep(recipeStep1);
        queueWithRecipeStep.addStep(recipeStep2);
        m_openAcdContext.saveQueue(queueWithRecipeStep);

        assertEquals(4, m_openAcdContext.getQueues().size());
        OpenAcdQueue queueRecipe = m_openAcdContext.getQueueByName("QueueRecipe");
        assertNotNull(queueRecipe);
        assertEquals(2, queueRecipe.getSteps().size());

        // test remove step from queue
        queueRecipe.removeStep(recipeStep1);
        m_openAcdContext.saveQueue(queueWithRecipeStep);
        assertEquals(1, queueRecipe.getSteps().size());

        // test remove queue with recipe steps
        m_openAcdContext.deleteQueue(queueWithRecipeStep);
        assertEquals(3, m_openAcdContext.getQueues().size());

        // test remove all queues but prevent 'default_queue' deletion
        Collection<Integer> queueIds = new ArrayList<Integer>();
        queueIds.add(defaultQueue.getId());
        queueIds.add(queue.getId());
        queueIds.add(queueWithSkills.getId());
        try {
            m_openAcdContext.deleteQueue(defaultQueue);
            fail();
        } catch (QueueInUseException e) {
        }
        m_openAcdContext.deleteQueue(queue);
        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME
        }, new String[] {
            "openacdqueue", "Queue1"
        });
        m_openAcdContext.deleteQueue(queueWithSkills);
        assertEquals(1, m_openAcdContext.getQueues().size());

        OpenAcdQueueGroup group = new OpenAcdQueueGroup();
        group.setName("QueueGroup");
        group.addQueue(anotherQueue);
        anotherQueue.setName("q2");
        anotherQueue.setGroup(group);
        m_openAcdContext.saveQueueGroup(group);
        m_openAcdContext.saveQueue(anotherQueue);
        MongoTestCaseHelper.assertObjectWithFieldsValuesPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME
        }, new String[] {
            "openacdqueue", "q2"
        });
        m_openAcdContext.deleteQueueGroup(group);
        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, MongoConstants.NAME
        }, new String[] {
            "openacdqueue", "q2"
        });
    }

    public void testGroupedSkills() throws Exception {
        OpenAcdSkill brand = m_openAcdContext.getSkillByAtom("_brand");
        OpenAcdSkill agent = m_openAcdContext.getSkillByAtom("_agent");
        OpenAcdSkill profile = m_openAcdContext.getSkillByAtom("_profile");
        OpenAcdSkill node = m_openAcdContext.getSkillByAtom("_node");
        OpenAcdSkill queue = m_openAcdContext.getSkillByAtom("_queue");
        OpenAcdSkill all = m_openAcdContext.getSkillByAtom("_all");
        List<OpenAcdSkill> agentSkills = m_openAcdContext.getAgentGroupedSkills().get("Magic");
        assertFalse(agentSkills.contains(brand));
        assertTrue(agentSkills.contains(agent));
        assertTrue(agentSkills.contains(profile));
        assertTrue(agentSkills.contains(node));
        assertFalse(agentSkills.contains(queue));
        assertTrue(agentSkills.contains(all));
        List<OpenAcdSkill> queueSkills = m_openAcdContext.getQueueGroupedSkills().get("Magic");
        assertTrue(queueSkills.contains(brand));
        assertFalse(queueSkills.contains(agent));
        assertFalse(queueSkills.contains(profile));
        assertTrue(queueSkills.contains(node));
        assertTrue(queueSkills.contains(queue));
        assertTrue(queueSkills.contains(all));
    }

    public void testOpenAcdReleaseCodes() throws Exception {
        // test save release code
        assertEquals(0, m_openAcdContext.getReleaseCodes().size());
        OpenAcdReleaseCode code = new OpenAcdReleaseCode();
        code.setLabel("negative label");
        code.setBias(-1);
        code.setDescription("negative description");
        m_openAcdContext.saveReleaseCode(code);
        assertEquals(1, m_openAcdContext.getReleaseCodes().size());

        // test save release code with the same label
        OpenAcdReleaseCode anotherCode = new OpenAcdReleaseCode();
        anotherCode.setLabel("negative label");
        try {
            m_openAcdContext.saveReleaseCode(anotherCode);
            fail();
        } catch (UserException ex) {
        }

        anotherCode.setLabel("positive label");
        anotherCode.setBias(1);
        m_openAcdContext.saveReleaseCode(anotherCode);
        assertEquals(2, m_openAcdContext.getReleaseCodes().size());
        MongoTestCaseHelper.assertObjectWithFieldsValuesPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, OpenAcdContext.LABEL
        }, new String[] {
            "openacdreleasecode", "positive label"
        });
        List<Replicable> replicables = m_openAcdReplicationProvider.getReplicables();
        assertTrue(replicables.contains(anotherCode));

        // test remove release codes
        Collection<Integer> codeIds = new ArrayList<Integer>();
        codeIds.add(code.getId());
        codeIds.add(anotherCode.getId());
        m_openAcdContext.removeReleaseCodes(codeIds);
        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, OpenAcdContext.LABEL
        }, new String[] {
            "openacdreleasecode", "negative label"
        });
        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            OpenAcdContext.TYPE, OpenAcdContext.LABEL
        }, new String[] {
            "openacdreleasecode", "positive label"
        });
        assertEquals(0, m_openAcdContext.getReleaseCodes().size());

    }

    public void testGetReplicables() {
        List<Replicable> replicables = m_openAcdReplicationProvider.getReplicables();
        assertEquals(15, replicables.size());
    }

    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setOpenAcdReplicationProvider(OpenAcdReplicationProvider openAcdReplicationProvider) {
        m_openAcdReplicationProvider = openAcdReplicationProvider;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}

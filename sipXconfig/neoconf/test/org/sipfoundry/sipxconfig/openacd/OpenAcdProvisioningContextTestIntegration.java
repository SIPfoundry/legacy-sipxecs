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

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.sipfoundry.commons.mongo.MongoDbTemplate;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.MongoTestCaseHelper;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;

public class OpenAcdProvisioningContextTestIntegration extends IntegrationTestCase {
    private OpenAcdContextImpl m_openAcdContextImpl;
    private OpenAcdSkillGroupMigrationContext m_migrationContext;
    private OpenAcdProvisioningContext m_openAcdProvisioningContext;

    @Override
    protected void onSetUpInTransaction() throws Exception {
        m_migrationContext.migrateSkillGroup();
    }

    public void testOpenAcdCommands() {
        MockOpenAcdProvisioningContext provContext = new MockOpenAcdProvisioningContext();
        m_openAcdContextImpl.setProvisioningContext(provContext);

        // test openacd client creation
        OpenAcdClient client = new OpenAcdClient();
        client.setIdentity("001");
        client.setName("clientName");
        m_openAcdContextImpl.saveClient(client);
        // test client update
        client.setName("newClientName");
        m_openAcdContextImpl.saveClient(client);

        // test openacd skill creation
        OpenAcdSkillGroup skillGroup = new OpenAcdSkillGroup();
        skillGroup.setName("Programming");
        m_openAcdContextImpl.saveSkillGroup(skillGroup);

        OpenAcdSkill skill = new OpenAcdSkill();
        skill.setName("Java");
        skill.setAtom("_java");
        skill.setGroup(skillGroup);
        skill.setDescription("Java Skill");
        m_openAcdContextImpl.saveSkill(skill);
        // test skill update
        skill.setName("Python");
        m_openAcdContextImpl.saveSkill(skill);

        OpenAcdSkill skill1 = new OpenAcdSkill();
        skill1.setName("C");
        skill1.setAtom("_c");
        skill1.setGroup(skillGroup);
        skill1.setDescription("C Skill");
        m_openAcdContextImpl.saveSkill(skill1);

        // test agent group creation
        OpenAcdAgentGroup group = new OpenAcdAgentGroup();
        group.setName("Group");
        group.setClients(Collections.singleton(client));
        group.addSkill(skill);
        group.addSkill(skill1);
        group.addQueue(m_openAcdContextImpl.getQueueByName("default_queue"));
        m_openAcdContextImpl.saveAgentGroup(group);

        // test queue creation
        OpenAcdQueue queue = new OpenAcdQueue();
        queue.setName("QueueName");
        queue.setAgentGroups(Collections.singleton(group));
        queue.addSkill(skill);
        queue.addSkill(skill1);
        queue.setGroup(m_openAcdContextImpl.getQueueGroupByName("Default"));
        m_openAcdContextImpl.saveQueue(queue);

        // test create queue group
        OpenAcdQueueGroup qGroup = new OpenAcdQueueGroup();
        qGroup.setName("QGroup");
        qGroup.addSkill(skill);
        qGroup.addSkill(skill1);
        qGroup.addAgentGroup(group);
        m_openAcdContextImpl.saveQueueGroup(qGroup);

        List<BasicDBObject> commands = provContext.getCommands();
        BasicDBObject addClientCommand = commands.get(0);
        assertEquals("ADD", addClientCommand.get("command"));
        assertEquals(1, addClientCommand.get("count"));
        List<BasicDBObject> objects = (List<BasicDBObject>) addClientCommand.get("objects");
        assertEquals(1, objects.size());
        assertEquals("client", objects.get(0).get("type"));
        assertEquals("clientName", objects.get(0).get("name"));
        assertEquals("001", objects.get(0).get("identity"));

        BasicDBObject updateClientCommand = commands.get(1);
        assertEquals("UPDATE", updateClientCommand.get("command"));
        assertEquals(1, updateClientCommand.get("count"));
        objects = (List<BasicDBObject>) updateClientCommand.get("objects");
        assertEquals("newClientName", objects.get(0).get("name"));

        BasicDBObject addSkillCommand = commands.get(2);
        assertEquals("ADD", addSkillCommand.get("command"));
        assertEquals(1, addSkillCommand.get("count"));
        objects = (List<BasicDBObject>) addSkillCommand.get("objects");
        assertEquals(1, objects.size());
        assertEquals("skill", objects.get(0).get("type"));
        assertEquals("Java", objects.get(0).get("name"));
        assertEquals("_java", objects.get(0).get("atom"));
        assertEquals("Programming", objects.get(0).get("groupName"));

        BasicDBObject updateSkillCommand = commands.get(3);
        assertEquals("UPDATE", updateSkillCommand.get("command"));
        assertEquals(1, updateSkillCommand.get("count"));
        objects = (List<BasicDBObject>) updateSkillCommand.get("objects");
        assertEquals(1, objects.size());
        assertEquals("skill", objects.get(0).get("type"));
        assertEquals("Python", objects.get(0).get("name"));

        BasicDBObject addAnotherSkillCommand = commands.get(4);
        assertEquals("ADD", addSkillCommand.get("command"));
        assertEquals(1, addAnotherSkillCommand.get("count"));
        objects = (List<BasicDBObject>) addAnotherSkillCommand.get("objects");
        assertEquals(1, objects.size());
        assertEquals("skill", objects.get(0).get("type"));

        BasicDBObject addAgentGroupCommand = commands.get(5);
        assertEquals("ADD", addAgentGroupCommand.get("command"));
        assertEquals(1, addAgentGroupCommand.get("count"));
        objects = (List<BasicDBObject>) addAgentGroupCommand.get("objects");
        assertEquals(1, objects.size());
        assertEquals("profile", objects.get(0).get("type"));
        assertEquals("Group", objects.get(0).get("name"));
        assertEquals("_java, _c", objects.get(0).get("skillsAtoms"));
        assertEquals("default_queue", objects.get(0).get("queuesName"));
        assertEquals("newClientName", objects.get(0).get("clientsName"));

        BasicDBObject addQueueCommand = commands.get(6);
        assertEquals("ADD", addQueueCommand.get("command"));
        assertEquals(1, addQueueCommand.get("count"));
        objects = (List<BasicDBObject>) addQueueCommand.get("objects");
        assertEquals(1, objects.size());
        assertEquals("queue", objects.get(0).get("type"));
        assertEquals("QueueName", objects.get(0).get("name"));
        assertEquals("Default", objects.get(0).get("queueGroup"));
        assertEquals("_java, _c", objects.get(0).get("skillsAtoms"));
        assertEquals("Group", objects.get(0).get("profiles"));
        assertEquals("1", objects.get(0).get("weight"));

        BasicDBObject addQueueGroupCommand = commands.get(7);
        assertEquals("ADD", addQueueGroupCommand.get("command"));
        assertEquals(1, addQueueGroupCommand.get("count"));
        objects = (List<BasicDBObject>) addQueueGroupCommand.get("objects");
        assertEquals(1, objects.size());
        assertEquals("queueGroup", objects.get(0).get("type"));
        assertEquals("QGroup", objects.get(0).get("name"));
        assertEquals("_java, _c", objects.get(0).get("skillsAtoms"));
        assertEquals("Group", objects.get(0).get("profiles"));
    }

    public void testOpenAcdConfigureCommands() {
        MockOpenAcdProvisioningContext provContext = new MockOpenAcdProvisioningContext();
        FreeswitchMediaCommand command = new FreeswitchMediaCommand(true, "test@testme",
                "{ignore_early_media=true}sofia/mydomain.com/$1");
        provContext.configure(Collections.singletonList(command));
        OpenAcdAgentConfigCommand agentCommand = new OpenAcdAgentConfigCommand(true);
        provContext.configure(Collections.singletonList(agentCommand));
        BasicDBObject addQueueGroupCommand = provContext.getCommands().get(0);
        assertEquals("CONFIGURE", addQueueGroupCommand.get("command"));
        assertEquals(1, addQueueGroupCommand.get("count"));
        List<BasicDBObject> objects = (List<BasicDBObject>) addQueueGroupCommand.get("objects");
        assertEquals(1, objects.size());
        assertEquals("freeswitch_media_manager", objects.get(0).get("type"));
        assertEquals("true", objects.get(0).get("enabled"));
        assertEquals("test@testme", objects.get(0).get("node"));
        assertEquals("{ignore_early_media=true}sofia/mydomain.com/$1", objects.get(0).get("dialString"));
        BasicDBObject enableListenerCommand = provContext.getCommands().get(1);
        assertEquals("CONFIGURE", enableListenerCommand.get("command"));
        assertEquals(1, enableListenerCommand.get("count"));
        objects = (List<BasicDBObject>) enableListenerCommand.get("objects");
        assertEquals("agent_configuration", objects.get(0).get("type"));
        assertEquals("true", objects.get(0).get("listenerEnabled"));

        provContext = new MockOpenAcdProvisioningContext();
        OpenAcdLogConfigCommand logConfigCommand = new OpenAcdLogConfigCommand("warning", "/var/etc");
        provContext.configure(Collections.singletonList(logConfigCommand));
        BasicDBObject configCommand = provContext.getCommands().get(0);
        assertEquals("CONFIGURE", configCommand.get("command"));
        assertEquals(1, configCommand.get("count"));
        objects = (List<BasicDBObject>) configCommand.get("objects");
        assertEquals(1, objects.size());
        assertEquals("log_configuration", objects.get(0).get("type"));
        assertEquals("warning", objects.get(0).get("logLevel"));
        assertEquals("/var/etc/openacd/", objects.get(0).get("logDir"));
    }

    public void testResync() throws Exception {
        Map<String, OpenAcdConfigObjectProvider> beanMap = getApplicationContext().getBeanFactory().getBeansOfType(
                OpenAcdConfigObjectProvider.class);
        assertTrue(beanMap.size() == 2);
        assertTrue(beanMap.containsKey("sipxOpenAcdService"));
        assertTrue(beanMap.containsKey("openAcdContextImpl"));
        loadDataSetXml("ClearDb.xml");
        loadDataSetXml("domain/DomainSeed.xml");

        // test openacd client creation
        OpenAcdClient client = new OpenAcdClient();
        client.setIdentity("001");
        client.setName("clientName");
        m_openAcdContextImpl.saveClient(client);

        // test openacd skill creation
        OpenAcdSkillGroup skillGroup = new OpenAcdSkillGroup();
        skillGroup.setName("Programming");
        m_openAcdContextImpl.saveSkillGroup(skillGroup);

        OpenAcdSkill skill = new OpenAcdSkill();
        skill.setName("Java");
        skill.setAtom("_java");
        skill.setGroup(skillGroup);
        skill.setDescription("Java Skill");
        m_openAcdContextImpl.saveSkill(skill);

        // test agent group creation
        OpenAcdAgentGroup group = new OpenAcdAgentGroup();
        group.setName("Group");
        group.setClients(Collections.singleton(client));
        group.addSkill(skill);
        group.addQueue(m_openAcdContextImpl.getQueueByName("default_queue"));
        m_openAcdContextImpl.saveAgentGroup(group);

        OpenAcdQueue queue = new OpenAcdQueue();
        queue.setName("QueueName");
        queue.setAgentGroups(Collections.singleton(group));
        queue.addSkill(skill);
        queue.setGroup(m_openAcdContextImpl.getQueueGroupByName("Default"));
        m_openAcdContextImpl.saveQueue(queue);

        MongoDbTemplate openacd = m_openAcdProvisioningContext.getDb();
        openacd.drop();
        DBCollection collection = openacd.getDb().getCollection("commands");

        m_openAcdProvisioningContext.resync();
        //TODO: improve this test by adding all config objects, and
        //searching in mongo everything, including defaults
        MongoTestCaseHelper.assertCollectionCount(collection, 19);
    }

    public void setOpenAcdContextImpl(OpenAcdContextImpl openAcdContext) {
        m_openAcdContextImpl = openAcdContext;
    }

    public void setOpenAcdSkillGroupMigrationContext(OpenAcdSkillGroupMigrationContext migrationContext) {
        m_migrationContext = migrationContext;
    }

    private class MockOpenAcdProvisioningContext extends OpenAcdProvisioningContextImpl {
        private List<BasicDBObject> m_commands = new LinkedList<BasicDBObject>();

        @Override
        protected void storeCommand(BasicDBObject command) {
            m_commands.add(command);
        }

        public List<BasicDBObject> getCommands() {
            return m_commands;
        }
    }

    public void setOpenAcdProvisioningContext(OpenAcdProvisioningContext openAcdProvisioningContext) {
        m_openAcdProvisioningContext = openAcdProvisioningContext;
    }

}

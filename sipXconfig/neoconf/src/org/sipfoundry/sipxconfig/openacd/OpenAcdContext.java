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

import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchExtensionProvider;

public interface OpenAcdContext extends FreeswitchExtensionProvider, AliasProvider, AliasOwner {

    void saveExtension(OpenAcdExtension extension);

    void removeExtensions(Collection<Integer> extensionIds);

    OpenAcdExtension getExtensionById(Integer extensionId);

    OpenAcdExtension getExtensionByName(String extensionName);

    List<OpenAcdExtension> getFreeswitchExtensions();

    String[] getOpenAcdApplicationNames();

    Set<OpenAcdLine> getLines(Location l);

    Set<OpenAcdCommand> getCommands(Location l);

    List<OpenAcdAgentGroup> getAgentGroups();

    OpenAcdAgentGroup getAgentGroupById(Integer agentGroupId);

    OpenAcdAgentGroup getAgentGroupByName(String agentGroupName);

    void saveAgentGroup(OpenAcdAgentGroup agentGroup);

    boolean removeAgentGroups(Collection<Integer> agentGroupIds);

    List<OpenAcdAgent> addAgentsToGroup(OpenAcdAgentGroup agentGroup, Collection<OpenAcdAgent> agents);

    List<OpenAcdAgent> getAgents();

    OpenAcdAgent getAgentById(Integer agentId);

    OpenAcdAgent getAgentByUserId(Integer userId);

    void saveAgent(OpenAcdAgentGroup agentGroup, OpenAcdAgent agent);

    void deleteAgents(Integer groupId, Collection<Integer> agentIds);

    void saveAgent(OpenAcdAgent agent);

    OpenAcdAgent getAgentByUser(User user);

    boolean isOpenAcdAgent(User user);

    List<OpenAcdSkill> getSkills();

    List<OpenAcdSkill> getDefaultSkills();

    OpenAcdSkill getSkillById(Integer skillId);

    OpenAcdSkill getSkillByName(String skillName);

    OpenAcdSkill getSkillByAtom(String atom);

    void saveSkill(OpenAcdSkill skill);

    List<String> removeSkills(Collection<Integer> skillIds);

    Map<String, List<OpenAcdSkill>> getGroupedSkills();

    void saveClient(OpenAcdClient client);

    void removeClients(Collection<Integer> clientsId);

    List<OpenAcdClient> getClients();

    OpenAcdClient getClientById(Integer clientId);

    List<OpenAcdQueueGroup> getQueueGroups();

    OpenAcdQueueGroup getQueueGroupById(Integer queueGroupId);

    OpenAcdQueueGroup getQueueGroupByName(String queueGroupName);

    void saveQueueGroup(OpenAcdQueueGroup queueGroup);

    boolean removeQueueGroups(Collection<Integer> queueGroupIds);

    List<OpenAcdQueue> getQueues();

    OpenAcdQueue getQueueById(Integer queueId);

    OpenAcdQueue getQueueByName(String queueName);

    void saveQueue(OpenAcdQueue queue);

    void removeQueues(Collection<Integer> queueIds);
}

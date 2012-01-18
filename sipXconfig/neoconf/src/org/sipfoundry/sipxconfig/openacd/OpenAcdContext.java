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

import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.ReplicableProvider;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchExtensionProvider;
import org.springframework.context.ApplicationListener;

public interface OpenAcdContext extends FreeswitchExtensionProvider, AliasOwner, ReplicableProvider,
        ApplicationListener {
    public static final LocationFeature FEATURE = new LocationFeature("openacd");

    OpenAcdSettings getSettings();

    void saveSettings(OpenAcdSettings settings);

    void saveExtension(OpenAcdExtension extension);

    void deleteExtension(OpenAcdExtension ext);

    OpenAcdExtension getExtensionById(Integer extensionId);

    OpenAcdExtension getExtensionByName(String extensionName);

    List<OpenAcdExtension> getFreeswitchExtensions();

    String[] getOpenAcdApplicationNames();

    Set<OpenAcdLine> getLines();

    Set<OpenAcdCommand> getCommands();

    List<OpenAcdAgentGroup> getAgentGroups();

    OpenAcdAgentGroup getAgentGroupById(Integer agentGroupId);

    OpenAcdAgentGroup getAgentGroupByName(String agentGroupName);

    void saveAgentGroup(OpenAcdAgentGroup agentGroup);

    boolean removeAgentGroups(Collection<Integer> agentGroupIds);

    void addAgentsToGroup(OpenAcdAgentGroup agentGroup, Collection<OpenAcdAgent> agents);

    List<OpenAcdAgent> getAgents();

    OpenAcdAgent getAgentById(Integer agentId);

    OpenAcdAgent getAgentByUserId(Integer userId);

    void saveAgent(OpenAcdAgentGroup agentGroup, OpenAcdAgent agent);

    void deleteAgents(Collection<Integer> agentIds);

    void saveAgent(OpenAcdAgent agent);

    OpenAcdAgent getAgentByUser(User user);

    boolean isOpenAcdAgent(User user);

    List<OpenAcdSkillGroup> getSkillGroups();

    OpenAcdSkillGroup getSkillGroupById(Integer skillGroupId);

    OpenAcdSkillGroup getSkillGroupByName(String skillGroupName);

    void saveSkillGroup(OpenAcdSkillGroup skillGroup);

    List<String> removeSkillGroups(Collection<Integer> skillGroupIds);

    List<OpenAcdSkill> getSkills();

    List<OpenAcdSkill> getDefaultSkills();

    OpenAcdSkill getSkillById(Integer skillId);

    OpenAcdSkill getSkillByName(String skillName);

    OpenAcdSkill getSkillByAtom(String atom);

    void saveSkill(OpenAcdSkill skill);

    List<String> removeSkills(Collection<Integer> skillIds);

    Map<String, List<OpenAcdSkill>> getGroupedSkills();

    Map<String, List<OpenAcdSkill>> getAgentGroupedSkills();

    Map<String, List<OpenAcdSkill>> getQueueGroupedSkills();

    void saveClient(OpenAcdClient client);

    List<String> removeClients(Collection<Integer> clientsId);

    List<OpenAcdClient> getClients();

    OpenAcdClient getClientById(Integer clientId);

    OpenAcdClient getClientByIdentity(String identity);

    List<OpenAcdQueueGroup> getQueueGroups();

    OpenAcdQueueGroup getQueueGroupById(Integer queueGroupId);

    OpenAcdQueueGroup getQueueGroupByName(String queueGroupName);

    void saveQueueGroup(OpenAcdQueueGroup queueGroup);

    List<String> removeQueueGroups(Collection<Integer> queueGroupIds);

    List<OpenAcdQueue> getQueues();

    OpenAcdQueue getQueueById(Integer queueId);

    OpenAcdQueue getQueueByName(String queueName);

    void saveQueue(OpenAcdQueue queue);

    OpenAcdRecipeStep getRecipeStepById(Integer recipeStepId);

    List<String> removeQueues(Collection<Integer> queueIds);
    public OpenAcdLine newOpenAcdLine();
    public OpenAcdCommand newOpenAcdCommand();
    public void replicateConfig();
    public void resync();
}

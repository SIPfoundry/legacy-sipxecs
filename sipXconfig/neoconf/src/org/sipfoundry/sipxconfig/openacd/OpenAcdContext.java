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

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchExtensionProvider;

public interface OpenAcdContext extends FreeswitchExtensionProvider, AliasOwner {
    public static final LocationFeature FEATURE = new LocationFeature("openacd");
    public static final AddressType OPENACD_WEB = new AddressType("openacdWebUI");
    public static final AddressType OPENACD_SECURE_WEB = new AddressType("openacdSecureWebUI");

    public static final String MAGIC_SKILL_GROUP_NAME = "Magic";
    public static final String GROUP_NAME_DEFAULT = "Default";
    public static final String OPENACD_LOG = "/full.log";
    //Mongo constants
    static final String QUEUE_GROUP = "qgrp";
    static final String SKILLS = "skl";
    static final String PROFILES = "prfl";
    static final String WEIGHT = "wht";
    static final String OLD_NAME = "oldnm";
    static final String ACTION = "actn";
    static final String ACTION_VALUE = "actionValue";
    static final String CONDITION = "cndt";
    static final String FREQUENCY = "frq";
    static final String STEP_NAME = "stpnm";
    static final String RECIPES = "rcps";

    static final String PIN = "pin";
    static final String AGENT_GROUP = "aggrp";
    static final String SKILLS_ATOM = "sklatm";
    static final String QUEUES = "qs";
    static final String CLIENTS = "clns";
    static final String FIRST_NAME = "fnm";
    static final String LAST_NAME = "lnm";
    static final String SECURITY = "scrty";

    static final String VALUE = "vlu";
    static final String NODE = "nd";
    static final String DIAL_STRING = "dlst";
    static final String LISTENER_ENABLED = "lstenbl";
    static final String LOG_LEVEL = "lglvl";
    static final String LOG_FILE = "lgfile";
    static final String ATOM = "atom";
    static final String GROUP_NAME = "grpnm";
    static final String DESCRIPTION = "dscr";
    static final String UUID = "uuid";
    static final String TYPE = "type";
    static final String LABEL = "lbl";
    static final String BIAS = "bias";
    static final String SSL_ENABLED = "sslenbl";
    static final String SSL = "ssl";
    static final String SSL_PORT = "sslprt";

    static final String CLIENT_NAME = "clnm";
    static final String CLIENT_ID = "clid";
    static final String DID = "did";
    static final String Q_NAME = "qnm";
    static final String LINE_NAME = "linm";


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

    void deleteAgentGroup(OpenAcdAgentGroup group);

    List<OpenAcdAgent> getAgents();

    OpenAcdAgent getAgentById(Integer agentId);

    OpenAcdAgent getAgentByUserId(Integer userId);

    void deleteAgent(OpenAcdAgent agent);

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

    void deleteSkill(OpenAcdSkill skill);

    Map<String, List<OpenAcdSkill>> getGroupedSkills();

    Map<String, List<OpenAcdSkill>> getAgentGroupedSkills();

    Map<String, List<OpenAcdSkill>> getQueueGroupedSkills();

    void saveClient(OpenAcdClient client);

    void deleteClient(OpenAcdClient client);

    List<OpenAcdClient> getClients();

    OpenAcdClient getClientById(Integer clientId);

    OpenAcdClient getClientByIdentity(String identity);

    List<OpenAcdQueueGroup> getQueueGroups();

    OpenAcdQueueGroup getQueueGroupById(Integer queueGroupId);

    OpenAcdQueueGroup getQueueGroupByName(String queueGroupName);

    void saveQueueGroup(OpenAcdQueueGroup queueGroup);

    void deleteQueueGroup(OpenAcdQueueGroup queueGroup);

    List<OpenAcdQueue> getQueues();

    OpenAcdQueue getQueueById(Integer queueId);

    OpenAcdQueue getQueueByName(String queueName);

    void saveQueue(OpenAcdQueue queue);

    OpenAcdRecipeStep getRecipeStepById(Integer recipeStepId);

    void deleteQueue(OpenAcdQueue queue);
    public OpenAcdLine newOpenAcdLine();
    public OpenAcdCommand newOpenAcdCommand();
    public void resync();

    boolean containsUsedSkills(OpenAcdSkillGroup skillGroup);

    OpenAcdClient getClientByName(String string);

    List<OpenAcdReleaseCode> getReleaseCodes();

    OpenAcdReleaseCode getReleaseCodeById(Integer id);

    void saveReleaseCode(OpenAcdReleaseCode code);

    void removeReleaseCodes(Collection<Integer> codesId);
}

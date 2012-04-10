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
package org.sipfoundry.sipxconfig.site.openacd;

import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgent;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgentGroup;
import org.sipfoundry.sipxconfig.openacd.OpenAcdClient;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueue;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueueGroup;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeAction;
import org.sipfoundry.sipxconfig.openacd.OpenAcdSkill;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class OpenAcdSkillsList extends BaseComponent {

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Parameter
    public abstract OpenAcdAgentGroup getAgentGroup();

    @Parameter
    public abstract OpenAcdAgent getAgent();

    @Parameter
    public abstract OpenAcdQueueGroup getQueueGroup();

    @Parameter
    public abstract OpenAcdQueue getQueue();

    @Parameter
    public abstract OpenAcdRecipeAction getRecipeAction();

    @Parameter
    public abstract Set<OpenAcdSkill> getDefaultRecipeSkills();

    public abstract OpenAcdSkill getCurrentSkill();

    // map to hold skill and boolean selected, true if skill associated with group
    public abstract Map<OpenAcdSkill, Boolean> getSkillSelections();

    public abstract void setSkillSelections(Map<OpenAcdSkill, Boolean> selections);

    public abstract Map<String, List<OpenAcdSkill>> getGroupedSkills();

    public abstract void setGroupedSkills(Map<String, List<OpenAcdSkill>> skills);

    public abstract String getGroupName();

    public abstract void setGroupName(String groupName);

    public abstract int getSkillsSize();

    public abstract void setSkillsSize(int size);

    // map to hold queue and boolean selected, true if queue associated
    public abstract Map<OpenAcdQueue, Boolean> getQueueSelections();

    public abstract void setQueueSelections(Map<OpenAcdQueue, Boolean> selections);

    public abstract OpenAcdQueue getCurrentQueue();

    // map to hold client and boolean selected, true if queue associated
    public abstract Map<OpenAcdClient, Boolean> getClientSelections();

    public abstract void setClientSelections(Map<OpenAcdClient, Boolean> selections);

    public abstract OpenAcdClient getCurrentClient();

    // map to hold profiles and boolean selected, true if queue associated
    public abstract Map<OpenAcdAgentGroup, Boolean> getProfileSelections();

    public abstract void setProfileSelections(Map<OpenAcdAgentGroup, Boolean> selections);

    public abstract OpenAcdAgentGroup getCurrentProfile();

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);

        setSkillsSize(0);
        Set<OpenAcdSkill> assignedSkills = new HashSet<OpenAcdSkill>();
        if (getAgentGroup() != null) {
            assignedSkills = getAgentGroup().getSkills();
            initQueueSelections(getAgentGroup().getQueues());
            initClientSelections(getAgentGroup().getClients());
            setGroupedSkills(getOpenAcdContext().getAgentGroupedSkills());
        } else if (getAgent() != null) {
            assignedSkills = getAgent().getSkills();
            initQueueSelections(getAgent().getQueues());
            initClientSelections(getAgent().getClients());
            setGroupedSkills(getOpenAcdContext().getAgentGroupedSkills());
        } else if (getQueueGroup() != null) {
            assignedSkills = getQueueGroup().getSkills();
            initProfileSelections(getQueueGroup().getAgentGroups());
            setGroupedSkills(getOpenAcdContext().getQueueGroupedSkills());
        } else if (getQueue() != null) {
            assignedSkills = getQueue().getSkills();
            initProfileSelections(getQueue().getAgentGroups());
            setGroupedSkills(getOpenAcdContext().getQueueGroupedSkills());
        } else if (getRecipeAction() != null) {
            assignedSkills = getRecipeAction().getSkills();
            if (getRecipeAction().isNew() && assignedSkills.isEmpty()) {
                assignedSkills = getDefaultRecipeSkills();
            }
            setGroupedSkills(getOpenAcdContext().getQueueGroupedSkills());
        }

        initSkillsSelections(assignedSkills);

    }

    public boolean getShowLabel() {
        if (getRecipeAction() != null) {
            return false;
        }
        return true;
    }

    private void initQueueSelections(Set<OpenAcdQueue> selectedQueues) {
        if (getQueueSelections() == null) {
            setQueueSelections(new HashMap<OpenAcdQueue, Boolean>());
        }
        List<OpenAcdQueue> queues = getOpenAcdContext().getQueues();
        for (OpenAcdQueue queue : queues) {
            if (selectedQueues.contains(queue)) {
                getQueueSelections().put(queue, true);
            } else {
                getQueueSelections().put(queue, false);
            }
        }
        setSkillsSize(getSkillsSize() + 1 + queues.size());
    }

    private void initClientSelections(Set<OpenAcdClient> selectedClients) {
        if (getClientSelections() == null) {
            setClientSelections(new HashMap<OpenAcdClient, Boolean>());
        }
        List<OpenAcdClient> clients = getOpenAcdContext().getClients();
        for (OpenAcdClient client : clients) {
            if (selectedClients.contains(client)) {
                getClientSelections().put(client, true);
            } else {
                getClientSelections().put(client, false);
            }
        }
        if (clients.size() > 0) {
            setSkillsSize(getSkillsSize() + 1 + clients.size());
        }
    }

    private void initSkillsSelections(Set<OpenAcdSkill> selectedSkills) {
        if (getSkillSelections() == null) {
            setSkillSelections(new HashMap<OpenAcdSkill, Boolean>());
        }
        List<OpenAcdSkill> skills = getOpenAcdContext().getSkills();
        for (OpenAcdSkill skill : skills) {
            if (selectedSkills.contains(skill)) {
                getSkillSelections().put(skill, true);
            } else {
                getSkillSelections().put(skill, false);
            }
        }
        setSkillsSize(getSkillsSize() + getGroupedSkills().keySet().size() + skills.size());
    }

    private void initProfileSelections(Set<OpenAcdAgentGroup> selectedProfiles) {
        if (getProfileSelections() == null) {
            setProfileSelections(new HashMap<OpenAcdAgentGroup, Boolean>());
        }
        List<OpenAcdAgentGroup> profiles = getOpenAcdContext().getAgentGroups();
        for (OpenAcdAgentGroup profile : profiles) {
            if (selectedProfiles.contains(profile)) {
                getProfileSelections().put(profile, true);
            } else {
                getProfileSelections().put(profile, false);
            }
        }
        setSkillsSize(getSkillsSize() + 1 + profiles.size());
    }

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this)) {
            afterRewind(cycle);
        }
    }

    private void afterRewind(IRequestCycle cycle) {
        Set<OpenAcdSkill> skills = new LinkedHashSet<OpenAcdSkill>();
        for (OpenAcdSkill skill : getSkillSelections().keySet()) {
            if (getSkillSelections().get(skill)) {
                skills.add(skill);
            }
        }
        if (getAgentGroup() != null) {
            getAgentGroup().setSkills(skills);
            getAgentGroup().setQueues(getSelectedQueues());
            getAgentGroup().setClients(getSelectedClients());
        } else if (getAgent() != null) {
            getAgent().setSkills(skills);
            getAgent().setQueues(getSelectedQueues());
            getAgent().setClients(getSelectedClients());
        } else if (getQueueGroup() != null) {
            getQueueGroup().setSkills(skills);
            getQueueGroup().setAgentGroups(getSelectedProfiles());
        } else if (getQueue() != null) {
            getQueue().setSkills(skills);
            getQueue().setAgentGroups(getSelectedProfiles());
        } else if (getRecipeAction() != null) {
            getRecipeAction().setSkills(skills);
        }
    }

    private Set<OpenAcdQueue> getSelectedQueues() {
        Set<OpenAcdQueue> queues = new LinkedHashSet<OpenAcdQueue>();
        for (OpenAcdQueue queue : getQueueSelections().keySet()) {
            if (getQueueSelections().get(queue)) {
                queues.add(queue);
            }
        }
        return queues;
    }

    private Set<OpenAcdClient> getSelectedClients() {
        Set<OpenAcdClient> clients = new LinkedHashSet<OpenAcdClient>();
        for (OpenAcdClient client : getClientSelections().keySet()) {
            if (getClientSelections().get(client)) {
                clients.add(client);
            }
        }
        return clients;
    }

    private Set<OpenAcdAgentGroup> getSelectedProfiles() {
        Set<OpenAcdAgentGroup> profiles = new LinkedHashSet<OpenAcdAgentGroup>();
        for (OpenAcdAgentGroup profile : getProfileSelections().keySet()) {
            if (getProfileSelections().get(profile)) {
                profiles.add(profile);
            }
        }
        return profiles;
    }
}

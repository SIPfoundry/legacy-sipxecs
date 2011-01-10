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
package org.sipfoundry.sipxconfig.site.openacd;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptGroup;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgent;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgentGroup;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdSkill;
import org.sipfoundry.sipxconfig.site.setting.BulkGroupAction;
import org.sipfoundry.sipxconfig.site.user.SelectUsers;
import org.sipfoundry.sipxconfig.site.user.SelectUsersCallback;

public abstract class EditOpenAcdAgentGroupPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "openacd/EditOpenAcdAgentGroupPage";

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

    @Persist
    public abstract Integer getGroupId();

    public abstract void setGroupId(Integer groupId);

    public abstract OpenAcdAgentGroup getAgentGroup();

    public abstract void setAgentGroup(OpenAcdAgentGroup agentGroup);

    public abstract Collection getAddedAgents();

    public abstract void setAddedAgents(Collection addedAgents);

    public abstract void setCurrentRow(OpenAcdAgent agent);

    public abstract Collection getSelectedRows();

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

    public void addAgentGroup(String returnPage) {
        setGroupId(null);
        setAgentGroup(null);
        setReturnPage(returnPage);
    }

    public void editAgentGroup(Integer groupId, String returnPage) {
        setGroupId(groupId);
        setReturnPage(returnPage);
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        if (getSkillSelections() == null) {
            setSkillSelections(new HashMap<OpenAcdSkill, Boolean>());
        }

        setGroupedSkills(getOpenAcdContext().getGroupedSkills());

        // initialize skills map, mark all skills as unselected
        List<OpenAcdSkill> skills = getOpenAcdContext().getSkills();
        for (OpenAcdSkill skill : skills) {
            getSkillSelections().put(skill, false);
        }

        // set the number of options to show in skills select component
        setSkillsSize(getGroupedSkills().keySet().size() + skills.size());

        if (getGroupId() == null) {
            setAgentGroup(new OpenAcdAgentGroup());
        } else {
            OpenAcdAgentGroup agentGroup = getOpenAcdContext().getAgentGroupById(getGroupId());
            if (agentGroup.getSkills().size() != 0) {
                for (OpenAcdSkill skill : agentGroup.getSkills()) {
                    // mark skills associated with this group as selected
                    getSkillSelections().put(skill, true);
                }
            }
            setAgentGroup(agentGroup);
        }

        OpenAcdAgentGroup group = getAgentGroup();
        if (getAddedAgents() != null && getAddedAgents().size() > 0) {
            List<OpenAcdAgent> existingAgents = new ArrayList<OpenAcdAgent>();
            try {
                existingAgents = getOpenAcdContext().addAgentsToGroup(group,
                        getAgentsBySelectedIds(getAddedAgents()));
            } catch (UserException uex) {
                IValidationDelegate validator = TapestryUtils.getValidator(getPage());
                validator.record(new ValidatorException(getMessages().getMessage(uex.getMessage())));
            }
            if (!existingAgents.isEmpty()) {
                List<String> existingAgentNames = new ArrayList<String>(existingAgents.size());
                for (OpenAcdAgent agent : existingAgents) {
                    existingAgentNames.add(agent.getUser().getUserName());
                }
                String msg = getMessages().format("duplicate.agents.error",
                        StringUtils.join(existingAgentNames, ", "));
                getValidator().record(new ValidatorException(msg));
            }
            // force a reload
            setAgentGroup(getOpenAcdContext().getAgentGroupById(group.getId()));
        }
    }

    private List<OpenAcdAgent> getAgentsBySelectedIds(Collection selectedIds) {
        List<OpenAcdAgent> agents = new ArrayList<OpenAcdAgent>();
        for (Object obj : selectedIds) {
            OpenAcdAgent agent = new OpenAcdAgent();
            agent.setUser(getCoreContext().loadUser((Integer) obj));
            agents.add(agent);
        }
        return agents;
    }

    public IPage addAgent(IRequestCycle cycle) {
        SelectUsers addAgents = (SelectUsers) cycle.getPage(SelectUsers.PAGE);
        SelectUsersCallback callback = new SelectUsersCallback(this.getPage());
        callback.setIdsPropertyName("addedAgents");
        addAgents.setCallback(callback);
        addAgents.setTitle(getMessages().getMessage("title.selectAgents"));
        addAgents.setPrompt(getMessages().getMessage("prompt.selectAgents"));
        return addAgents;
    }

    public IPage editAgent(IRequestCycle cycle, Integer agentId) {
        EditOpenAcdAgentPage page = (EditOpenAcdAgentPage) cycle.getPage(EditOpenAcdAgentPage.PAGE);
        page.editAgent(agentId, getPage().getPageName());
        return page;
    }

    public void delete() {
        Collection ids = getSelections().getAllSelected();
        if (ids.isEmpty()) {
            return;
        }
        OpenAcdAgentGroup group = getAgentGroup();
        getOpenAcdContext().deleteAgents(group.getId(), ids);
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        OpenAcdAgentGroup group = getAgentGroup();
        Set<OpenAcdSkill> skills = new LinkedHashSet<OpenAcdSkill>();
        // walk through all skills and add only those marked as selected
        for (OpenAcdSkill skill : getSkillSelections().keySet()) {
            if (getSkillSelections().get(skill)) {
                skills.add(skill);
            }
        }
        group.setSkills(skills);
        getOpenAcdContext().saveAgentGroup(group);
        setGroupId(getAgentGroup().getId());
    }

    public IPropertySelectionModel getActionModel() {
        Collection<OptionAdapter> actions = new ArrayList<OptionAdapter>();
        Collection<OpenAcdAgentGroup> groups = getOpenAcdContext().getAgentGroups();

        if (!groups.isEmpty()) {
            actions.add(new OptGroup(getMessages().getMessage("label.moveToAgentGroup")));

            for (OpenAcdAgentGroup group : groups) {
                actions.add(new AddToAgentGroupAction(group));
            }
        }

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);

        return model;
    }

    class AddToAgentGroupAction extends BulkGroupAction {

        private final OpenAcdAgentGroup m_group;

        AddToAgentGroupAction(OpenAcdAgentGroup group) {
            super(null);
            m_group = group;
        }

        @Override
        public String getLabel(Object option, int index) {
            return m_group.getName();
        }

        @Override
        public Object getValue(Object option, int index) {
            return m_group.getId();
        }

        @Override
        public String squeezeOption(Object option, int index) {
            return m_group.getId().toString();
        }

        public void actionTriggered(IComponent component, IRequestCycle cycle) {
            Collection<Integer> ids = getSelections().getAllSelected();
            if (ids.isEmpty()) {
                return;
            }
            for (Integer id : ids) {
                OpenAcdAgent agent = getOpenAcdContext().getAgentById(id);
                agent.setGroup(m_group);
                getOpenAcdContext().saveAgent(agent);
            }
        }
    }
}

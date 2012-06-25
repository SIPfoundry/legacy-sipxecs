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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgent;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgentGroup;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.site.user.SelectUsers;
import org.sipfoundry.sipxconfig.site.user.SelectUsersCallback;
import org.sipfoundry.sipxconfig.site.user.UserTable;

public abstract class OpenAcdAddAgentsToGroupPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "openacd/OpenAcdAddAgentsToGroupPage";

    public static final String DELIM = ", ";

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

    @Persist
    public abstract OpenAcdAgentGroup getAgentGroup();

    public abstract void setAgentGroup(OpenAcdAgentGroup agentGroup);

    @Persist
    public abstract String getSecurity();

    public abstract void setSecurity(String security);

    @Persist
    public abstract OpenAcdAgent getDummyAgent();

    public abstract void setDummyAgent(OpenAcdAgent agent);

    @Persist
    public abstract List<User> getUsers();

    public abstract void setUsers(List<User> users);

    public abstract Collection<Integer> getAddedUsers();

    public abstract void setAddedUsers(Collection<Integer> addedUsers);

    public abstract List<String> getInheritedSkills();

    public abstract void setInheritedSkills(List<String> skills);

    public abstract String getSkill();

    public abstract void setSkill(String skill);

    public abstract void setCurrentRow(OpenAcdAgent agent);

    public abstract Collection getSelectedRows();

    public void addAgentsToGroup(String returnPage) {
        setAgentGroup(null);
        setDummyAgent(null);
        setUsers(null);
        setReturnPage(returnPage);
    }

    public IPage addSkills(IRequestCycle cycle) {
        OpenAcdServerPage page = (OpenAcdServerPage) cycle.getPage(OpenAcdServerPage.PAGE);
        page.setTab("skills");
        return page;
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        if (getDummyAgent() == null) {
            setDummyAgent(new OpenAcdAgent());
        }

        if (getUsers() == null) {
            setUsers(new ArrayList<User>());
        }

        if (getAddedUsers() != null && getAddedUsers().size() > 0) {
            List<User> users = getUsersBySelectedIds(getAddedUsers());
            List<User> existingUsers = new ArrayList<User>();
            for (User user : users) {
                if (!getOpenAcdContext().isOpenAcdAgent(user)) {
                    getUsers().add(user);
                } else {
                    existingUsers.add(user);
                }
            }

            if (!existingUsers.isEmpty()) {
                List<String> existingUserNames = new ArrayList<String>(existingUsers.size());
                for (User user : existingUsers) {
                    existingUserNames.add(user.getUserName());
                }
                String msg = getMessages().format("duplicate.agents.error",
                        StringUtils.join(existingUserNames, DELIM));
                getValidator().record(new ValidatorException(msg));
            }
        }

        OpenAcdAgentGroup group = getAgentGroup();
        if (group != null) {
            setInheritedSkills(group.getAllSkillNames());
        }
    }

    public void formSubmit() {
        // Process form submission
        setAgentGroup(getAgentGroup());
    }

    public IPage addUsers(IRequestCycle cycle) {
        SelectUsers selectUsersPage = (SelectUsers) cycle.getPage(SelectUsers.PAGE);
        SelectUsersCallback callback = new SelectUsersCallback(this.getPage());
        callback.setIdsPropertyName("addedUsers");
        selectUsersPage.setCallback(callback);
        selectUsersPage.setTitle(getMessages().getMessage("title.selectAgents"));
        selectUsersPage.setPrompt(getMessages().getMessage("prompt.selectAgents"));
        return selectUsersPage;
    }

    private List<User> getUsersBySelectedIds(Collection<Integer> selectedIds) {
        List<User> users = new ArrayList<User>();
        for (Integer id : selectedIds) {
            users.add(getCoreContext().loadUser(id));
        }
        return users;
    }

    public void delete() {
        UserTable usersTable = (UserTable) getComponent("extensionsTable");
        Collection selectedIdsToDelete = usersTable.getSelections().getAllSelected();
        getUsers().removeAll(getUsersBySelectedIds(selectedIdsToDelete));
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        List<User> users = getUsers();
        if (users != null && users.size() > 0) {
            for (User user : users) {
                OpenAcdAgent agent = new OpenAcdAgent();
                agent.setGroup(getAgentGroup());
                agent.setUser(user);
                agent.setSecurity(getSecurity());
                agent.setSkills(getDummyAgent().getSkills());
                agent.setQueues(getDummyAgent().getQueues());
                agent.setClients(getDummyAgent().getClients());
                getOpenAcdContext().saveAgent(agent);
            }
        } else {
            throw new UserException(getMessages().getMessage("error.requiredUser"));
        }
    }

    public IPropertySelectionModel getAgentSecurityModel() {
        return new StringPropertySelectionModel(new String[] {
            OpenAcdAgent.Security.AGENT.toString(), OpenAcdAgent.Security.SUPERVISOR.toString(),
            OpenAcdAgent.Security.ADMIN.toString()
        });
    }
}

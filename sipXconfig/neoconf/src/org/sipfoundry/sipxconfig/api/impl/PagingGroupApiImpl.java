/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.api.impl;

import java.util.HashSet;
import java.util.List;

import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import org.mozilla.javascript.edu.emory.mathcs.backport.java.util.Collections;
import org.sipfoundry.sipxconfig.api.PagingGroupApi;
import org.sipfoundry.sipxconfig.api.model.PageGroupBean;
import org.sipfoundry.sipxconfig.api.model.PageGroupBean.UserBean;
import org.sipfoundry.sipxconfig.api.model.PageGroupList;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.paging.PagingContext;
import org.sipfoundry.sipxconfig.paging.PagingFeatureContext;
import org.sipfoundry.sipxconfig.paging.PagingGroup;
import org.sipfoundry.sipxconfig.paging.PagingSettings;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;

public class PagingGroupApiImpl extends BaseServiceApiImpl implements PagingGroupApi {
    private static final String PAGING_GROUP_NOT_FOUND = "Paging group not found.";
    private PagingContext m_context;
    private PagingFeatureContext m_featureContext;
    private CoreContext m_coreContext;

    @Override
    public Response getPageGroups() {
        List<PagingGroup> groups = m_context.getPagingGroups();
        if (groups != null) {
            return Response.ok().entity(PageGroupList.convertPageList(groups)).build();
        }
        return Response.status(Status.NOT_FOUND).entity("No paging group found.").build();
    }

    @Override
    public Response newPageGroup(PageGroupBean bean) {
        Response response = checkPrompt(bean);
        if (response != null) {
            return response;
        }
        PagingGroup group = new PagingGroup();
        PageGroupBean.populateGroup(bean, group);
        Response populateUsersFailure = populateUsers(bean, group);
        if (populateUsersFailure != null) {
            return populateUsersFailure;
        }
        m_context.savePagingGroup(group);
        return Response.ok().build();
    }

    @Override
    public Response getPageGroup(Integer groupId) {
        PagingGroup group = m_context.getPagingGroupById(groupId);
        if (group != null) {
            return Response.ok().entity(PageGroupBean.convertGroup(group)).build();
        }
        return Response.status(Status.NOT_FOUND).entity(PAGING_GROUP_NOT_FOUND).build();
    }

    @Override
    public Response deletePageGroup(Integer groupId) {
        PagingGroup group = m_context.getPagingGroupById(groupId);
        if (group != null) {
            m_featureContext.deletePagingGroupsById(Collections.singletonList(groupId));
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).entity(PAGING_GROUP_NOT_FOUND).build();
    }

    @Override
    public Response updatePageGroup(Integer groupId, PageGroupBean bean) {
        Response response = checkPrompt(bean);
        if (response != null) {
            return response;
        }
        PagingGroup group = m_context.getPagingGroupById(groupId);
        if (group != null) {
            PageGroupBean.populateGroup(bean, group);
            Response populateUsersFailure = populateUsers(bean, group);
            if (populateUsersFailure != null) {
                return populateUsersFailure;
            }
            m_context.savePagingGroup(group);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).entity(PAGING_GROUP_NOT_FOUND).build();
    }

    private Response populateUsers(PageGroupBean bean, PagingGroup group) {
        group.setUsers(new HashSet<User>());
        for (UserBean userBean : bean.getUsers()) {
            User user = null;
            if (userBean.getId() != null) {
                user = m_coreContext.getUser(userBean.getId());
            } else if (userBean.getUserName() != null) {
                user = m_coreContext.loadUserByUserNameOrAlias(userBean.getUserName());
            }
            if (user == null) {
                return Response.status(Status.NOT_FOUND).entity(PAGING_GROUP_NOT_FOUND).build();
            }
            group.getUsers().add(user);
        }
        return null;
    }

    private Response checkPrompt(PageGroupBean bean) {
        if (bean != null) {
            return checkPrompt(bean.getSound());
        }
        return null;
    }

    public void setPagingContext(PagingContext context) {
        m_context = context;
    }

    public void setPagingFeatureContext(PagingFeatureContext context) {
        m_featureContext = context;
    }

    public void setCoreContext(CoreContext context) {
        m_coreContext = context;
    }

    @Override
    protected PersistableSettings getSettings() {
        return m_context.getSettings();
    }

    @Override
    protected void saveSettings(PersistableSettings settings) {
        m_context.saveSettings((PagingSettings) settings);
    }

}

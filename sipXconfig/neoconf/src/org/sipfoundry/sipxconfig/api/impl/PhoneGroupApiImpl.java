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

import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import org.sipfoundry.sipxconfig.api.PhoneGroupApi;
import org.sipfoundry.sipxconfig.api.model.GroupBean;
import org.sipfoundry.sipxconfig.api.model.GroupList;
import org.sipfoundry.sipxconfig.api.model.ModelBean.ModelList;
import org.sipfoundry.sipxconfig.api.model.SettingsList;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.beans.factory.annotation.Required;

public class PhoneGroupApiImpl implements PhoneGroupApi {
    private PhoneContext m_phoneContext;
    private SettingDao m_settingDao;
    private ModelSource<PhoneModel> m_phoneModelSource;

    @Override
    public Response getPhoneGroups() {
        return buildPhoneGroupList(m_phoneContext.getGroups(),
            m_settingDao.getGroupMemberCountIndexedByGroupId(Phone.class));
    }

    @Override
    public Response newPhoneGroup(GroupBean groupBean) {
        Group group = new Group();
        GroupBean.convertToPhoneGroup(groupBean, group);
        m_phoneContext.saveGroup(group);
        return Response.ok().entity(group.getId()).build();
    }

    @Override
    public Response getPhoneGroup(String phoneGroupId) {
        Group group = getPhoneGroupByIdOrName(phoneGroupId);
        if (group != null) {
            GroupBean groupBean = new GroupBean();
            GroupBean.convertGroup(group, groupBean);
            return Response.ok().entity(groupBean).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response deletePhoneGroup(String id) {
        Group group = getPhoneGroupByIdOrName(id);
        if (group != null) {
            if (m_phoneContext.deleteGroups(Collections.singletonList(group.getId()))) {
                //ERROR to be returned: msg.error.removeAdminGroup = administrators group can't be deleted
                return Response.status(Status.FORBIDDEN).build();
            }
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    private Group getPhoneGroupByIdOrName(String id) {
        Group group = null;
        try {
            int groupId = Integer.parseInt(id);
            group = m_settingDao.loadGroup(groupId);
        } catch (NumberFormatException e) {
            group = m_phoneContext.getGroupByName(id, false);
        }
        return group;
    }

    @Override
    public Response updatePhoneGroup(String groupId, GroupBean groupBean) {
        Group group = getPhoneGroupByIdOrName(groupId);
        if (group != null) {
            GroupBean.convertToPhoneGroup(groupBean, group);
            m_phoneContext.saveGroup(group);
            return Response.ok().entity(group.getId()).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response movePhoneGroupUp(String groupId) {
        Group group = getPhoneGroupByIdOrName(groupId);
        if (group != null) {
            m_settingDao.moveGroups(m_phoneContext.getGroups(), Collections.singletonList(group.getId()), -1);
            return Response.ok().entity(group.getId()).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response movePhoneGroupDown(String groupId) {
        Group group = getPhoneGroupByIdOrName(groupId);
        if (group != null) {
            m_settingDao.moveGroups(m_phoneContext.getGroups(), Collections.singletonList(group.getId()), 1);
            return Response.ok().entity(group.getId()).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getPhoneGroupModels(String groupId) {
        Collection<PhoneModel> models = m_phoneModelSource.getModels();
        if (models != null) {
            return Response.ok().entity(ModelList.convertModelList(models)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getPhoneGroupModelSettings(String groupId, String modelName, HttpServletRequest request) {
        PhoneModel phoneModel = m_phoneModelSource.getModel(modelName);

        if (phoneModel != null) {
            Setting settings = m_phoneContext.newPhone(phoneModel).getSettings();
            return Response.ok().entity(SettingsList.convertSettingsList(settings, request.getLocale())).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getPhoneGroupSetting(String groupId, String modelName, String path, HttpServletRequest request) {
        Group group = getPhoneGroupByIdOrName(groupId);
        if (group != null) {
            PhoneModel phoneModel = m_phoneModelSource.getModel(modelName);
            Phone phone = m_phoneContext.newPhone(phoneModel);
            phone.setSettingValue(path, group.getSettingValue(path));
            return ResponseUtils.buildSettingResponse(phone, path, request.getLocale());
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response setPhoneGroupSetting(String groupId, String modelName, String path, String value) {
        Group group = getPhoneGroupByIdOrName(groupId);
        if (group != null) {
            group.setSettingValue(path, value);
            m_settingDao.saveGroup(group);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response deletePhoneGroupSetting(String groupId, String modelName, String path) {
        Group group = getPhoneGroupByIdOrName(groupId);
        if (group != null) {
            group.getDatabaseValues().remove(path);
            m_settingDao.saveGroup(group);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    private Response buildPhoneGroupList(List<Group> phoneGroups, Map count) {
        if (phoneGroups != null) {
            return Response.ok().entity(GroupList.convertGroupList(phoneGroups, count)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Required
    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    @Required
    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    @Required
    public void setPhoneModelSource(ModelSource<PhoneModel> phoneModelSource) {
        m_phoneModelSource = phoneModelSource;
    }
}

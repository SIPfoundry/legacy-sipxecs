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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.api.PhoneApi;
import org.sipfoundry.sipxconfig.api.model.PhoneBean;
import org.sipfoundry.sipxconfig.api.model.PhoneBean.GroupBean;
import org.sipfoundry.sipxconfig.api.model.PhoneBean.GroupList;
import org.sipfoundry.sipxconfig.api.model.PhoneBean.ModelBean;
import org.sipfoundry.sipxconfig.api.model.PhoneBean.ModelList;
import org.sipfoundry.sipxconfig.api.model.PhoneList;
import org.sipfoundry.sipxconfig.api.model.SettingsList;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public class PhoneApiImpl implements PhoneApi {
    private PhoneContext m_phoneContext;
    private ModelSource<PhoneModel> m_modelSource;
    private SettingDao m_settingDao;

    @Override
    public Response getPhones(Integer startId, Integer pageSize) {
        if (startId != null && pageSize != null) {
            return buildPhoneList(m_phoneContext.loadPhonesByPage(startId, pageSize));
        }
        return buildPhoneList(m_phoneContext.loadPhones());
    }

    private Response buildPhoneList(List<Phone> phones) {
        if (phones != null) {
            return Response.ok().entity(PhoneList.convertPhoneList(phones)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getPhoneModels() {
        Collection<PhoneModel> models = m_modelSource.getModels();
        if (models != null) {
            return Response.ok().entity(ModelList.convertModelList(models)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getPhone(String id) {
        Phone phone = getPhoneByIdOrMac(id);
        if (phone != null) {
            return Response.ok().entity(PhoneBean.convertPhone(phone)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response newPhone(PhoneBean phoneBean) {
        PhoneModel model = getPhoneModel(phoneBean);
        if (model == null) {
            return Response.status(Status.NOT_FOUND).entity("No such phone model defined").build();
        }
        Phone phone = m_phoneContext.newPhone(model);
        convertToPhone(phoneBean, phone);
        m_phoneContext.storePhone(phone);
        return Response.ok().entity(phone.getId()).build();
    }

    @Override
    public Response updatePhone(String phoneId, PhoneBean phoneBean) {
        Phone phone = getPhoneByIdOrMac(phoneId);
        if (phone != null) {
            convertToPhone(phoneBean, phone);
            m_phoneContext.storePhone(phone);
            return Response.ok().entity(phone.getId()).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    private PhoneModel getPhoneModel(PhoneBean phoneBean) {
        ModelBean modelBean = phoneBean.getModel();
        String modelId = null;
        PhoneModel model = null;
        if (modelBean != null) {
            modelId = modelBean.getModelId();
            model = m_modelSource.getModel(modelId);
        }
        return model;
    }

    private void convertToPhone(PhoneBean phoneBean, Phone phone) {
        phone.setSerialNumber(phoneBean.getSerialNo());
        phone.setDescription(phoneBean.getDescription());
        phone.setDeviceVersion(DeviceVersion.getDeviceVersion(phoneBean.getDeviceVersion()));
        String groupNames = phone.getGroupsNames();
        for (GroupBean groupBean : phoneBean.getGroups()) {
            String groupName = groupBean.getName();
            if (!StringUtils.contains(groupNames, groupName)) {
                // add group only if it doesn't already exist
                Group g = m_settingDao.getGroupCreateIfNotFound(PHONE_RES, groupBean.getName());
                phone.addGroup(g);
            }
        }
    }

    @Override
    public Response deletePhone(String id) {
        Phone phone = getPhoneByIdOrMac(id);
        if (phone != null) {
            m_phoneContext.deletePhone(phone);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getPhoneGroups(String phoneId) {
        Phone phone = getPhoneByIdOrMac(phoneId);
        if (phone != null) {
            return Response.ok().entity(GroupList.convertGroupList(phone.getGroupsAsList())).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response removePhoneGroups(String phoneId) {
        Phone phone = getPhoneByIdOrMac(phoneId);
        if (phone != null) {
            phone.setGroupsAsList(new ArrayList<Group>());
            m_phoneContext.storePhone(phone);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response addPhoneInGroup(String phoneId, String groupName) {
        Phone phone = getPhoneByIdOrMac(phoneId);
        if (phone != null) {
            Group g = m_settingDao.getGroupCreateIfNotFound(PHONE_RES, groupName);
            phone.addGroup(g);
            m_phoneContext.storePhone(phone);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response removePhoneFromGroup(String phoneId, String groupName) {
        Phone phone = getPhoneByIdOrMac(phoneId);
        if (phone != null) {
            Group group = m_settingDao.getGroupByName("phone", groupName);
            if (group != null) {
                phone.removeGroup(group);
                m_phoneContext.storePhone(phone);
                return Response.ok().build();
            }
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getPhoneSettings(String phoneId, HttpServletRequest request) {
        Phone phone = getPhoneByIdOrMac(phoneId);
        if (phone != null) {
            Setting settings = phone.getSettings();
            return Response.ok().entity(SettingsList.convertSettingsList(settings, request.getLocale())).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getPhoneSetting(String phoneId, String path, HttpServletRequest request) {
        return ResponseUtils.buildSettingResponse(getPhoneByIdOrMac(phoneId), path, request.getLocale());
    }

    @Override
    public Response setPhoneSetting(String phoneId, String path, String value) {
        Phone phone = getPhoneByIdOrMac(phoneId);
        if (phone != null) {
            phone.setSettingValue(path, value);
            m_phoneContext.storePhone(phone);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response deletePhoneSetting(String phoneId, String path) {
        Phone phone = getPhoneByIdOrMac(phoneId);
        if (phone != null) {
            Setting setting = phone.getSettings().getSetting(path);
            setting.setValue(setting.getDefaultValue());
            m_phoneContext.storePhone(phone);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    private Phone getPhoneByIdOrMac(String id) {
        Phone phone = null;
        try {
            int phoneId = Integer.parseInt(id);
            phone = m_phoneContext.loadPhone(phoneId);
        } catch (NumberFormatException e) {
            // no id then it must be MAC
            phone = m_phoneContext.getPhoneBySerialNumber(id);
        }
        return phone;
    }

    public void setPhoneContext(PhoneContext context) {
        m_phoneContext = context;
    }

    public void setPhoneModelSource(ModelSource<PhoneModel> modelSource) {
        m_modelSource = modelSource;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

}

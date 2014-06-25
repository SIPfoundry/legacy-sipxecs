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
package org.sipfoundry.sipxconfig.api.model;

import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlType;

import org.codehaus.jackson.annotate.JsonPropertyOrder;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.Group;

@XmlRootElement(name = "Phone")
@XmlType(propOrder = {
        "id", "serialNo", "deviceVersion", "description", "model", "lines", "groups"
        })
@JsonPropertyOrder({
        "id", "serialNo", "deviceVersion", "description", "model", "lines", "groups"
    })
public class PhoneBean {
    private int m_id;
    private String m_serialNo;
    private String m_description;
    private String m_version;
    private ModelBean m_model;
    private List<LineBean> m_lines;
    private List<GroupBean> m_groups;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public String getSerialNo() {
        return m_serialNo;
    }

    public void setSerialNo(String serial) {
        m_serialNo = serial;
    }

    public String getDeviceVersion() {
        return m_version;
    }

    public void setDeviceVersion(String version) {
        m_version = version;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    @XmlElement(name = "Model")
    public ModelBean getModel() {
        return m_model;
    }

    public void setModel(ModelBean model) {
        m_model = model;
    }

    public void setLines(List<LineBean> lines) {
        m_lines = lines;
    }

    @XmlElementWrapper(name = "Lines")
    @XmlElement(name = "Line")
    public List<LineBean> getLines() {
        return m_lines;
    }

    public void setGroups(List<GroupBean> groups) {
        m_groups = groups;
    }

    @XmlElementWrapper(name = "Groups")
    @XmlElement(name = "Group")
    public List<GroupBean> getGroups() {
        return m_groups;
    }

    public static PhoneBean convertPhone(Phone phone) {
        PhoneModel phoneModel = phone.getModel();
        PhoneBean bean = new PhoneBean();
        bean.setId(phone.getId());
        bean.setSerialNo(phone.getSerialNumber());
        if (phone.getDeviceVersion() != null) {
            bean.setDeviceVersion(phone.getDeviceVersion().getName());
        }
        bean.setDescription(phone.getDescription());
        bean.setModel(ModelBean.convertModel(phoneModel));
        bean.setLines(LineBean.buildLineList(phone.getLines()));
        bean.setGroups(GroupBean.buildGroupList(phone.getGroupsAsList()));
        return bean;
    }

    public static Phone convertToPhone() {
        return null;
    }

    @XmlType(propOrder = {
            "modelId", "label", "vendor", "versions"
            })
    @JsonPropertyOrder({
            "modelId", "label", "vendor", "versions"
            })
    public static class ModelBean {
        private String m_modelId;
        private String m_label;
        private String m_vendor;
        private List<String> m_versions;

        public void setModelId(String model) {
            m_modelId = model;
        }

        public String getModelId() {
            return m_modelId;
        }

        public void setLabel(String label) {
            m_label = label;
        }

        public String getLabel() {
            return m_label;
        }

        public void setVendor(String vendor) {
            m_vendor = vendor;
        }

        public String getVendor() {
            return m_vendor;
        }

        public void setVersions(List<String> versions) {
            m_versions = versions;
        }

        @XmlElementWrapper(name = "Versions")
        @XmlElement(name = "Version")
        public List<String> getVersions() {
            return m_versions;
        }

        public static ModelBean convertModel(PhoneModel model) {
            ModelBean modelBean = new ModelBean();
            modelBean.setLabel(model.getLabel());
            modelBean.setModelId(model.getModelId());
            modelBean.setVendor(model.getVendor());
            List<String> versions = new ArrayList<String>();
            for (DeviceVersion deviceVersion : model.getVersions()) {
                versions.add(deviceVersion.getName());
            }
            if (versions.size() > 0) {
                modelBean.setVersions(versions);
            }
            return modelBean;
        }

        public static List<ModelBean> buildModelList(Collection<PhoneModel> phoneModels) {
            List<ModelBean> models = new LinkedList<ModelBean>();
            for (PhoneModel model : phoneModels) {
                models.add(convertModel(model));
            }
            if (models.size() > 0) {
                return models;
            }
            return null;
        }
    }

    @XmlRootElement(name = "Line")
    @XmlType(propOrder = {
            "id", "uri", "user", "userId", "extension", "displayName", "password", "registrationServer",
            "registrationServerPort", "voicemail"
            })
    @JsonPropertyOrder({
            "id", "uri", "user", "userId", "extension", "displayName", "password", "registrationServer",
            "registrationServerPort", "voicemail"
            })
    public static class LineBean {
        private int m_id;
        private String m_user;
        private String m_userId;
        private String m_extension;
        private String m_password;
        private String m_uri;
        private String m_displayName;
        private String m_registrationServer;
        private String m_registrationServerPort;
        private String m_voicemail;

        public int getId() {
            return m_id;
        }

        public void setId(int id) {
            m_id = id;
        }

        public void setUser(String user) {
            m_user = user;
        }

        public String getUser() {
            return m_user;
        }

        public void setUserId(String userId) {
            m_userId = userId;
        }

        public String getUserId() {
            return m_userId;
        }

        public void setUri(String uri) {
            m_uri = uri;
        }

        public String getUri() {
            return m_uri;
        }

        public void setExtension(String ext) {
            m_extension = ext;
        }

        public String getExtension() {
            return m_extension;
        }

        public void setPassword(String pwd) {
            m_password = pwd;
        }

        public String getPassword() {
            return m_password;
        }

        public void setDisplayName(String name) {
            m_displayName = name;
        }

        public String getDisplayName() {
            return m_displayName;
        }

        public void setRegistrationServerPort(String server) {
            m_registrationServerPort = server;
        }

        public String getRegistrationServerPort() {
            return m_registrationServerPort;
        }

        public void setRegistrationServer(String server) {
            m_registrationServer = server;
        }

        public String getRegistrationServer() {
            return m_registrationServer;
        }

        public void setVoicemail(String voicemail) {
            m_voicemail = voicemail;
        }

        public String getVoicemail() {
            return m_voicemail;
        }

        public static LineBean convertLine(Line line) {
            LineBean lineBean = new LineBean();
            lineBean.setId(line.getId());
            lineBean.setUri(line.getUri());
            lineBean.setUser(line.getUserName());
            LineInfo info = line.getLineInfo();
            if (info != null) {
                lineBean.setUserId(info.getUserId());
                lineBean.setDisplayName(info.getDisplayName());
                lineBean.setExtension(info.getExtension());
                lineBean.setPassword(info.getPassword());
                lineBean.setRegistrationServer(info.getRegistrationServer());
                lineBean.setRegistrationServerPort(info.getRegistrationServerPort());
                lineBean.setVoicemail(info.getVoiceMail());
            }
            return lineBean;
        }

        public static List<LineBean> buildLineList(List<Line> phoneLines) {
            List<LineBean> lines = new LinkedList<LineBean>();
            for (Line line : phoneLines) {
                lines.add(convertLine(line));
            }
            if (lines.size() > 0) {
                return lines;
            }
            return null;
        }
    }

    @XmlType(propOrder = {
            "id", "name", "weight"
            })
    @JsonPropertyOrder({
            "id", "name", "weight"
            })
    public static class GroupBean {
        private int m_id;
        private String m_name;
        private Integer m_weight;

        public int getId() {
            return m_id;
        }

        public void setId(int id) {
            m_id = id;
        }

        public void setName(String name) {
            m_name = name;
        }

        public String getName() {
            return m_name;
        }

        public void setWeight(int weight) {
            m_weight = weight;
        }

        public int getWeight() {
            return m_weight;
        }

        public static List<GroupBean> buildGroupList(List<Group> phoneGroups) {
            List<GroupBean> groups = new LinkedList<GroupBean>();
            for (Group group : phoneGroups) {
                GroupBean groupBean = new GroupBean();
                groupBean.setId(group.getId());
                groupBean.setName(group.getName());
                groupBean.setWeight(group.getWeight());
                groups.add(groupBean);
            }
            if (groups.size() > 0) {
                return groups;
            }
            return null;
        }
    }

    @XmlRootElement(name = "Groups")
    public static class GroupList {

        private List<GroupBean> m_groups;

        public void setGroups(List<GroupBean> groups) {
            m_groups = groups;
        }

        @XmlElement(name = "Group")
        public List<GroupBean> getGroups() {
            if (m_groups == null) {
                m_groups = new ArrayList<GroupBean>();
            }
            return m_groups;
        }

        public static GroupList convertGroupList(List<Group> groups) {
            GroupList list = new GroupList();
            list.setGroups(GroupBean.buildGroupList(groups));
            return list;
        }
    }

    @XmlRootElement(name = "Lines")
    public static class LineList {

        private List<LineBean> m_lines;

        public void setLines(List<LineBean> lines) {
            m_lines = lines;
        }

        @XmlElement(name = "Line")
        public List<LineBean> getLines() {
            if (m_lines == null) {
                m_lines = new ArrayList<LineBean>();
            }
            return m_lines;
        }

        public static LineList convertLineList(List<Line> lines) {
            LineList list = new LineList();
            list.setLines(LineBean.buildLineList(lines));
            return list;
        }
    }

    @XmlRootElement(name = "Models")
    public static class ModelList {

        private List<ModelBean> m_models;

        public void setModels(List<ModelBean> models) {
            m_models = models;
        }

        @XmlElement(name = "Model")
        public List<ModelBean> getModels() {
            if (m_models == null) {
                m_models = new ArrayList<ModelBean>();
            }
            return m_models;
        }

        public static ModelList convertModelList(Collection<PhoneModel> models) {
            ModelList list = new ModelList();
            list.setModels(ModelBean.buildModelList(models));
            return list;
        }
    }
}

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
import org.sipfoundry.sipxconfig.device.DeviceDescriptor;
import org.sipfoundry.sipxconfig.device.DeviceVersion;

@XmlType(propOrder = {
        "modelId", "label", "vendor", "versions"
        })
@JsonPropertyOrder({
        "modelId", "label", "vendor", "versions"
        })
public class ModelBean {
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

    public static ModelBean convertModel(DeviceDescriptor model) {
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

    public static List<ModelBean> buildModelList(Collection<? extends DeviceDescriptor> deviceModels) {
        List<ModelBean> models = new LinkedList<ModelBean>();
        for (DeviceDescriptor model : deviceModels) {
            models.add(convertModel(model));
        }
        if (models.size() > 0) {
            return models;
        }
        return null;
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

        public static ModelList convertModelList(Collection<? extends DeviceDescriptor> models) {
            ModelList list = new ModelList();
            list.setModels(ModelBean.buildModelList(models));
            return list;
        }
    }
}

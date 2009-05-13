/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc;

import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.gateway.SipTrunk;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.beans.factory.annotation.Required;

public class SbcDevice extends Device implements NamedObject {
    private String m_address;

    private int m_port;

    private String m_name;

    private String m_description;

    private ModelSource<SbcDescriptor> m_modelSource;

    private SbcDescriptor m_model;

    public SbcDevice() {
    }

    public void setModel(SbcDescriptor model) {
        m_model = model;
        setModelId(model.getModelId());
        setBeanId(model.getBeanId());
    }

    @Override
    public SbcDescriptor getModel() {
        if (m_model != null) {
            return m_model;
        }
        if (getModelId() == null) {
            throw new IllegalStateException("Model ID not set");
        }
        if (m_modelSource == null) {
            throw new IllegalStateException("ModelSource not set");
        }
        m_model = m_modelSource.getModel(getModelId());
        return m_model;
    }

    @Required
    public void setModelSource(ModelSource<SbcDescriptor> modelSource) {
        m_modelSource = modelSource;
    }

    public void setAddress(String address) {
        m_address = address;
    }

    public String getAddress() {
        return m_address;
    }

    public void setPort(int port) {
        m_port = port;
    }

    public int getPort() {
        return m_port;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getName() {
        return m_name;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getDescription() {
        return m_description;
    }

    /**
     * No settings for default SBC
     */
    @Override
    protected Setting loadSettings() {
        return null;
    }

    public String getRoute() {
        StringBuilder route = new StringBuilder(m_address);
        if (m_port > 0 && m_port != SipTrunk.DEFAULT_PORT) {
            route.append(':');
            route.append(getPort());
        }
        return route.toString();
    }
}

/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.device.Model;


public class SipxServiceBundle implements Model {
    private final String m_name;
    private boolean m_autoEnable;
    private String m_modelId;

    public SipxServiceBundle(String name) {
        m_name = name;
    }

    public String getName() {
        return m_name;
    }

    @Override
    public int hashCode() {
        return m_name.hashCode();
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null || getClass() != obj.getClass()) {
            return false;
        }
        SipxServiceBundle bundle = (SipxServiceBundle) obj;
        return m_name.equals(bundle.m_name);
    }

    public void setAutoEnable(boolean autoEnable) {
        m_autoEnable = autoEnable;
    }

    public boolean isAutoEnable() {
        return m_autoEnable;
    }

    public void setModelId(String modelId) {
        m_modelId = modelId;
    }

    public String getModelId() {
        return m_modelId;
    }
}

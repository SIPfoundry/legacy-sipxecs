/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.gateway;

import org.sipfoundry.sipxconfig.device.DeviceDescriptor;

public class GatewayModel extends DeviceDescriptor {
    private int m_maxPorts;

    /**
     * Array of setting names that comprise the port label
     */
    private String[] m_portLabelSettings;

    private String m_portLabelFormat;

    public GatewayModel() {
    }

    public GatewayModel(String beanId) {
        super(beanId);
    }

    public GatewayModel(String beanId, String modelId) {
        super(beanId, modelId);
    }

    public void setMaxPorts(int maxPorts) {
        m_maxPorts = maxPorts;
    }

    public int getMaxPorts() {
        return m_maxPorts;
    }

    public void setPortLabelSettings(String... portLabelSettings) {
        m_portLabelSettings = portLabelSettings;
    }

    public String[] getPortLabelSettings() {
        return m_portLabelSettings;
    }

    public void setPortLabelFormat(String portLabelFormat) {
        m_portLabelFormat = portLabelFormat;
    }

    public String getPortLabelFormat() {
        return m_portLabelFormat;
    }
}

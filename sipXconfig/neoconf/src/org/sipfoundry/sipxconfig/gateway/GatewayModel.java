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
    private boolean m_dialPlanAware = true;
    private boolean m_callerIdAware = true;

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

    public boolean isDialPlanAware() {
        return m_dialPlanAware;
    }

    public void setDialPlanAware(boolean dialPlanAware) {
        m_dialPlanAware = dialPlanAware;
    }

    public boolean isCallerIdAware() {
        return m_callerIdAware;
    }

    public void setCallerIdAware(boolean callerIdAware) {
        m_callerIdAware = callerIdAware;
    }
}

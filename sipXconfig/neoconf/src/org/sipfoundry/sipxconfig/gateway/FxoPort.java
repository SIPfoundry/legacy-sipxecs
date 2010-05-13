/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.gateway;

import java.util.Iterator;

import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Object representing FXO port or a digital trunk (T1/E1) in an FXO/PSTN gateway
 */
public class FxoPort extends BeanWithSettings {
    private Gateway m_gateway;

    private boolean m_initialized;

    public Gateway getGateway() {
        return m_gateway;
    }

    public void setGateway(Gateway gateway) {
        m_gateway = gateway;
    }

    @Override
    protected Setting loadSettings() {
        Gateway gateway = getGateway();
        Setting settings = gateway.loadPortSettings();
        if (settings != null) {
            // kludge - not obvious place to initialize, but latest place
            initialize();
        }

        return settings;
    }

    @Override
    public synchronized void initialize() {
        if (m_initialized) {
            return;
        }
        Gateway gateway = getGateway();
        if (m_gateway == null) {
            return;
        }
        gateway.initializePort(this);
        m_initialized = true;
    }

    private int getPortNumber(int portId) {
        int portNumber = 1;
        Iterator iports = getGateway().getPorts().iterator();
        for (; iports.hasNext(); portNumber++) {
            if (((FxoPort) iports.next()).getId() == portId) {
                return portNumber;
            }
        }

        return 0;
    }

    public String getNumber() {
        return Integer.toString(getPortNumber(getId()));
    }

    public String getLabel() {
        String[] settingNames = getGateway().getModel().getPortLabelSettings();
        String format = getGateway().getModel().getPortLabelFormat();
        if (settingNames == null) {
            return "Port";
        }
        Object[] values = new Object[settingNames.length];
        for (int i = 0; i < settingNames.length; i++) {
            Setting setting = getSettings().getSetting(settingNames[i]);
            Object value = setting.getValue();
            values[i] = setting.getType().getLabel(value);
        }
        return String.format(format, values);
    }
}

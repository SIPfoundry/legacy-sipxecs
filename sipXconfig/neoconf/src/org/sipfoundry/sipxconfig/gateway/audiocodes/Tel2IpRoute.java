/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Object representing Tel2Ip route on an AudioCodes PSTN gateway
 */
public class Tel2IpRoute extends BeanWithSettings {
    private static final String PREFIX_ANY = "*";
    private static final String STRIP_NONE = "0";

    private Gateway m_gateway;
    private boolean m_initialized;
    private String m_routeId;
    private String m_description;
    private String m_settingProxyAddress;
    private String m_settingProxyKeepalive;
    private String m_settingDestManipulation;

    public Gateway getGateway() {
        return m_gateway;
    }

    public void setGateway(Gateway gateway) {
        m_gateway = gateway;
    }

    public String getRouteId() {
        return m_routeId;
    }

    public void setRouteId(String routeId) {
        m_routeId = routeId;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getProxyAddress() {
        return m_gateway.getSettingValue(m_settingProxyAddress);
    }

    public void setSettingProxyAddress(String settingProxyAddress) {
        m_settingProxyAddress = settingProxyAddress;
    }

    public String getProxyKeepalive() {
        return m_gateway.getSettingValue(m_settingProxyKeepalive);
    }

    public void setSettingProxyKeepalive(String settingProxyKeepalive) {
        m_settingProxyKeepalive = settingProxyKeepalive;
    }

    private String getDestManipulationItem(int itemIndex) {
        String destManipulation = m_gateway.getSettingValue(m_settingDestManipulation);
        if (destManipulation != null) {
            String[] items = destManipulation.split(",");
            if (items.length > itemIndex) {
                return items[itemIndex].trim();
            }
        }
        return null;
    }

    public String getDestPrefix() {
        String item = getDestManipulationItem(0);
        if (item == null) {
            item = PREFIX_ANY;
        }
        return item;
    }

    public String getSrcPrefix() {
        String item = getDestManipulationItem(1);
        if (item == null) {
            item = PREFIX_ANY;
        }
        return item;
    }

    public String getDestLeftStrip() {
        String item = getDestManipulationItem(2);
        if (item == null) {
            item = STRIP_NONE;
        }
        return item;
    }

    public String getDestRightStrip() {
        String item = getDestManipulationItem(3);
        if (item == null) {
            item = STRIP_NONE;
        }
        return item;
    }

    public void setSettingDestManipulation(String settingDestManipulation) {
        m_settingDestManipulation = settingDestManipulation;
    }


    @Override
    protected Setting loadSettings() {
        return null;
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
        if (gateway instanceof AudioCodesGateway) {
            ((AudioCodesGateway) gateway).initializeTel2IpRoute(this);
            m_initialized = true;
        }
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

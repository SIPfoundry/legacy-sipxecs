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
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Object representing Tel2Ip route on an AudioCodes PSTN gateway
 */
public class Tel2IpRoute extends RouteTable {
    private boolean m_initialized;
    private String m_description;
    private String m_settingProxyAddress;
    private String m_settingProxyKeepalive;
    private String m_settingProxyKeeptime;

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getProxyAddress() {
        return getGateway().getSettingValue(m_settingProxyAddress);
    }

    public void setSettingProxyAddress(String settingProxyAddress) {
        m_settingProxyAddress = settingProxyAddress;
    }

    public String getProxyKeepalive() {
        return getGateway().getSettingValue(m_settingProxyKeepalive);
    }

    public void setSettingProxyKeepalive(String settingProxyKeepalive) {
        m_settingProxyKeepalive = settingProxyKeepalive;
    }

    public String getProxyKeeptime() {
        return getGateway().getSettingValue(m_settingProxyKeeptime);
    }

    public void setSettingProxyKeeptime(String settingProxyKeeptime) {
        m_settingProxyKeeptime = settingProxyKeeptime;
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
        if (gateway != null) {
            if (gateway instanceof AudioCodesGateway) {
                ((AudioCodesGateway) gateway).initializeTel2IpRoute(this);
                m_initialized = true;
            }
        }
    }

}

/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.setting.SettingValue;
import org.sipfoundry.sipxconfig.setting.SettingValueHandler;
import org.sipfoundry.sipxconfig.setting.SettingValueImpl;

public class AudioCodesGatewayDefaults implements SettingValueHandler {
    private AudioCodesGateway m_gateway;
    private DeviceDefaults m_defaults;

    AudioCodesGatewayDefaults(AudioCodesGateway gateway, DeviceDefaults defaults) {
        m_gateway = gateway;
        m_defaults = defaults;
    }

    @SettingEntry(path = "SIP/SIPGATEWAYNAME")
    public String getGatewayName() {
        return m_gateway.getDefaults().getDomainName();
    }

    @SettingEntry(path = "SIP/SIPDESTINATIONPORT")
    public String getDestinationPort() {
        return m_gateway.getDefaults().getProxyServerSipPort();
    }
    
    @SettingEntry(path = "Network/NTPServerIP")
    public String getNtpServer() {
        return m_gateway.getDefaults().getNtpServer();
    }

    public SettingValue getSettingValue(Setting setting) {
        SettingValue value = null;
        String path = setting.getPath();
        AudioCodesModel model = (AudioCodesModel) m_gateway.getModel();
        if (path.equals(model.getProxyNameSetting())) {
            value = new SettingValueImpl(m_defaults.getDomainName());
        } else if (path.equals(model.getProxyIpSetting())) {
            value = new SettingValueImpl(m_defaults.getProxyServerAddr());
        }

        return value;
    }
}

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
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class AudioCodesGatewayDefaults {
    private AudioCodesGateway m_gateway;
    private DeviceDefaults m_defaults;

    AudioCodesGatewayDefaults(AudioCodesGateway gateway, DeviceDefaults defaults) {
        m_gateway = gateway;
        m_defaults = defaults;
    }

    @SettingEntry(path = "SIP_Proxy_Registration/SIPGatewayName")
    public String getGatewayName() {
        return m_gateway.getName();
    }

    @SettingEntry(path = "SIP_Proxy_Registration/SIPDestinationPort")
    public String getDestinationPort() {
        return m_defaults.getProxyServerSipPort();
    }

    @SettingEntry(path = "SIP_Proxy_Registration/ProxyIP")
    public String getDomainName() {
        return m_defaults.getDomainName();
    }

    @SettingEntry(path = "Network/NTPServerIP")
    public String getNtpServer() {
        return m_gateway.getDefaults().getNtpServer();
    }
}

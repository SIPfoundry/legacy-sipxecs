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

/**
 * FIXME: This is currently shared by fxs and fxo gateways. Ideally both gateway classes would be
 * derived from the same base.
 */
public class AudioCodesGatewayDefaults {
    @SuppressWarnings("unused")
    private AudioCodesGateway m_gateway;

    @SuppressWarnings("unused")
    private AudioCodesFxsGateway m_fxsGateway;

    private DeviceDefaults m_defaults;

    AudioCodesGatewayDefaults(AudioCodesGateway gateway, DeviceDefaults defaults) {
        m_gateway = gateway;
        m_defaults = defaults;
    }

    AudioCodesGatewayDefaults(AudioCodesFxsGateway fxsGateway, DeviceDefaults defaults) {
        m_fxsGateway = fxsGateway;
        m_defaults = defaults;
    }

    @SettingEntry(path = "SIP_Proxy_Registration/SIPDestinationPort")
    public String getDestinationPort() {
        return m_defaults.getProxyServerSipPort();
    }

    @SettingEntry(paths = { "SIP_Proxy_Registration/ProxyIP", 
            "SIP_Proxy_Registration/RegistrarIP", "SIP_Proxy_Registration/SIPGatewayName" })
    public String getDomainName() {
        return m_defaults.getDomainName();
    }

    @SettingEntry(path = "Network/NTPServerIP")
    public String getNtpServer() {
        return m_defaults.getNtpServer();
    }
}

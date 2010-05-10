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

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.service.UnmanagedService;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

/**
 * FIXME: This is currently shared by fxs and fxo gateways. Ideally both gateway classes would be
 * derived from the same base.
 */
public class AudioCodesGatewayDefaults {
    private static final String REL_6_0_OR_LATER = "6.0orLater";
    private static final String OPTION_ZERO = "0";
    private static final String OPTION_ONE = "1";
    private static final String AUTH_PER_ENDPOINT = OPTION_ZERO;
    private static final String AUTH_PER_GATEWAY = OPTION_ONE;
    private static final String CSMODE_DESTPHONE = OPTION_ZERO;
    private static final String CSMODE_CYCLICASCEND = OPTION_ONE;

    private AudioCodesGateway m_fxoGateway;

    private AudioCodesFxsGateway m_fxsGateway;

    private DeviceDefaults m_defaults;

    AudioCodesGatewayDefaults(AudioCodesGateway fxoGateway, DeviceDefaults defaults) {
        m_fxoGateway = fxoGateway;
        m_defaults = defaults;
    }

    AudioCodesGatewayDefaults(AudioCodesFxsGateway fxsGateway, DeviceDefaults defaults) {
        m_fxsGateway = fxsGateway;
        m_defaults = defaults;
    }

    /**
     * We need to return "Cyclic ascendant(1)" for FXO gateway and "By phone number(0)" for FXO
     * gateways.
     */
    @SettingEntry(path = "SIP_general/ChannelSelectMode")
    public String getChannelSelecMode() {
        if (m_fxoGateway != null) {
            //
            return CSMODE_CYCLICASCEND;
        }
        return CSMODE_DESTPHONE;
    }

    @SettingEntry(path = "SIP_Proxy_Registration/SIPDestinationPort")
    public String getDestinationPort() {
        return m_defaults.getProxyServerSipPort();
    }

    @SettingEntry(paths = { "SIP_Proxy_Registration/ProxyIP",
            "SIP_Proxy_Registration/RegistrarIP",
            "SIP_Proxy_Registration/SIPGatewayName" })
    public String getDomainName() {
        return m_defaults.getDomainName();
    }

    @SettingEntry(path = "SIP_Proxy_Registration/AuthenticationMode")
    public String getAuthenticationMode() {
        if (m_fxoGateway != null) {
            return AUTH_PER_GATEWAY;
        }
        return AUTH_PER_ENDPOINT;
    }

    @SettingEntry(path = "Network/NTPServerIP")
    public String getNtpServer() {
        String ntpIpAddress;
        try {
            ntpIpAddress = InetAddress.getByName(m_defaults.getNtpServer()).getHostAddress();
        } catch (UnknownHostException e) {
            ntpIpAddress = "0.0.0.0";
        }
        return ntpIpAddress;
    }

    @SettingEntry(path = "Network/NTPServerUTCOffset")
    public String getNTPServerUTCOffset() {
        // Get the offset in seconds where GMT=0.
        return Integer.toString(m_defaults.getTimeZone().getOffset() * 60);
    }

    @SettingEntry(path = "Network/DNSPriServerIP")
    public String getDNSPriServerIP() {
        return m_defaults.getServer(0, UnmanagedService.DNS);
    }

    @SettingEntry(path = "Network/DNSSecServerIP")
    public String getDNSSecServerIP() {
        return m_defaults.getServer(1, UnmanagedService.DNS);
    }

    @SettingEntry(path = "Network/EnableSyslog")
    public boolean getEnableSyslog() {
        return null != m_defaults.getServer(0, UnmanagedService.SYSLOG);
    }

    @SettingEntry(paths = { "Network/SyslogServerIP", "advanced_general/CDR/CDRSyslogServerIP" })
    public String getSyslogServerIP() {
        return m_defaults.getServer(0, UnmanagedService.SYSLOG);
    }

    @SettingEntry(path = "advanced_general/MaxActiveCalls")
    public int getMaxActiveCalls() {
        DeviceVersion myVersion;
        int numPorts;
        if (m_fxoGateway != null) {
            myVersion = m_fxoGateway.getDeviceVersion();
            numPorts = m_fxoGateway.getMaxCalls();
        } else {
            myVersion = m_fxsGateway.getDeviceVersion();
            numPorts = m_fxsGateway.getLines().size();
        }
        // Release 6.00 firmware allows two calls per port.
        int multiplier = 1;
        if (myVersion.isSupported(REL_6_0_OR_LATER)) {
            multiplier = 2;
        }
        return (multiplier * numPorts);
    }

    /**
     * Only allow calls from SIP proxy by default.
     */
    @SettingEntry(path = "advanced_general/AllowedIPs")
    public String getAllowedIPs() {
        return m_defaults.getProxyServerAddr();
    }

    @SettingEntry(path = "advanced_general/SAS/SASDefaultGatewayIP")
    public String getSasGatewayIP() {
        return m_fxoGateway.getAddress();
    }

    @SettingEntry(path = "tel2ip-call-routing/tel-to-ip-normal/ProxyAddress")
    public String getNormalProxy() {
        return m_defaults.getDomainName();
    }

    @SettingEntry(path = "tel2ip-call-routing/tel-to-ip-failover/ProxyAddress")
    public String getFailoverProxy() {
        return (m_fxoGateway.getAddress() + ":5080");
    }
}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.nt1535;

import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

/**
 * Nortel 1535 phone.
 */
public class NT1535Phone extends Phone {
    private static final String SYSTEM_CONFIG_FILE = "sysconf_2890d_sip.cfg";

    public NT1535Phone() {
    }

    @Override
    public void initializeLine(Line line) {
        DeviceDefaults phoneDefaults = getPhoneContext().getPhoneDefaults();
        NT1535LineDefaults defaults = new NT1535LineDefaults(phoneDefaults, line);
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initialize() {
        DeviceDefaults phoneDefaults = getPhoneContext().getPhoneDefaults();
        NT1535PhoneDefaults defaults = new NT1535PhoneDefaults(phoneDefaults);
        addDefaultBeanSettingHandler(defaults);
    }

    public String getDeviceFileName() {
        return getSerialNumber().toUpperCase() + ".cfg";
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = NT1535LineDefaults.getLineInfo(line);
        return lineInfo;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
        NT1535LineDefaults.setLineInfo(line, lineInfo);
    }

    @Override
    public void restart() {
        sendCheckSyncToMac();
    }

    static class NT1535ConfigProfile extends Profile {
        private static final String VERSION_HW_VERSION = "VERSION/hw_version";

        private static final String VERSION_SW_VERSION = "VERSION/sw_version";

        public NT1535ConfigProfile(NT1535Phone phone, String name) {
            super(formatProfileName(phone, name));
        }

        private static String formatProfileName(NT1535Phone phone, String name) {
            String hwVersion = phone.getSettingValue(VERSION_HW_VERSION);
            String swVersion = phone.getSettingValue(VERSION_SW_VERSION);
            return String.format("%s/%sS/%s", hwVersion, swVersion, name);
        }
    }

    static class DeviceConfigProfile extends NT1535ConfigProfile {
        public DeviceConfigProfile(NT1535Phone phone, String name) {
            super(phone, name);
        }

        @Override
        protected ProfileContext createContext(Device device) {
            return new ProfileContext(device, "nt1535/mac.cfg.vm");
        }
    }

    static class SystemConfigProfile extends NT1535ConfigProfile {
        public SystemConfigProfile(NT1535Phone phone, String name) {
            super(phone, name);
        }

        @Override
        protected ProfileContext createContext(Device device) {
            return new ProfileContext(device, "nt1535/sysconf_2890d_sip.cfg.vm");
        }
    }

    @Override
    public Profile[] getProfileTypes() {
        Profile[] profileTypes;
        profileTypes = new Profile[] {
            new DeviceConfigProfile(this, getDeviceFileName()),
            new SystemConfigProfile(this, SYSTEM_CONFIG_FILE)
        };

        return profileTypes;
    }

    public static class NT1535LineDefaults {
        private static final String VOIP_AUTHNAME = "VOIP/authname";
        private static final String VOIP_DISPLAYNAME = "VOIP/display_name";
        private static final String VOIP_NAME = "VOIP/name";
        private static final String VOIP_PASSWORD = "VOIP/password";
        private static final String VOIP_PROXY_ADDRESS = "VOIP/proxy_address";
        private static final String VOIP_PROXY_PORT = "VOIP/proxy_port";

        private final Line m_line;
        private final DeviceDefaults m_defaults;

        NT1535LineDefaults(DeviceDefaults defaults, Line line) {
            m_line = line;
            m_defaults = defaults;
        }

        @SettingEntry(path = VOIP_DISPLAYNAME)
        public String getDisplayName() {
            User user = m_line.getUser();
            if (user != null) {
                return user.getDisplayName();
            }
            return null;
        }

        @SettingEntry(path = VOIP_NAME)
        public String getUserName() {
            return m_line.getUserName();
        }

        @SettingEntry(path = VOIP_AUTHNAME)
        public String getAuthorizationUserName() {
            return m_line.getAuthenticationUserName();
        }

        @SettingEntry(path = VOIP_PASSWORD)
        public String getSipPassword() {
            User user = m_line.getUser();
            if (user != null) {
                return user.getSipPassword();
            }
            return null;
        }

        public static LineInfo getLineInfo(Line line) {
            LineInfo lineInfo = new LineInfo();
            lineInfo.setUserId(line.getSettingValue(VOIP_AUTHNAME));
            lineInfo.setDisplayName(line.getSettingValue(VOIP_DISPLAYNAME));
            lineInfo.setRegistrationServer(line.getSettingValue(VOIP_PROXY_ADDRESS));
            lineInfo.setRegistrationServerPort(line.getSettingValue(VOIP_PROXY_PORT));
            return lineInfo;
        }

        public static void setLineInfo(Line line, LineInfo lineInfo) {
            line.setSettingValue(VOIP_NAME, lineInfo.getUserId());
            line.setSettingValue(VOIP_AUTHNAME, lineInfo.getUserId());
            line.setSettingValue(VOIP_DISPLAYNAME, lineInfo.getDisplayName());
            line.setSettingValue(VOIP_PROXY_ADDRESS, lineInfo.getRegistrationServer());
            line.setSettingValue(VOIP_PROXY_PORT, lineInfo.getRegistrationServerPort());
        }

        @SettingEntry(path = VOIP_PROXY_ADDRESS)
        public String getServerIp() {
            return m_defaults.getDomainName();
        }

        @SettingEntry(path = VOIP_PROXY_PORT)
        public String getProxyPort() {
            return m_defaults.getProxyServerSipPort();
        }
    }

    public static class NT1535PhoneDefaults {

        private static final String VOIP_SIP_SERVICE_DOMAIN = "VOIP/sip_service_domain";
        private static final String VOIP_OUTBOUND_PROXY_SERVER = "VOIP/outbound_proxy_server";
        private static final String VOIP_OUTBOUND_PROXY_PORT = "VOIP/outbound_proxy_port";
        private static final String VOIP_MOH_URL = "VOIP/moh_url";
        private static final String LAN_TFTP_SERVER_ADDRESS = "LAN/tftp_server_address";
        private static final String NETTIME_SNTP_SERVER_ADDRESS = "NETTIME/sntp_server_address";

        private final DeviceDefaults m_defaults;

        NT1535PhoneDefaults(DeviceDefaults defaults) {
            m_defaults = defaults;
        }

        @SettingEntry(path = VOIP_SIP_SERVICE_DOMAIN)
        public String getSipDomain1() {
            return m_defaults.getDomainName();
        }

        @SettingEntry(path = VOIP_OUTBOUND_PROXY_SERVER)
        public String getOutboundProxy() {
            return m_defaults.getDomainName();
        }

        @SettingEntry(path = VOIP_OUTBOUND_PROXY_PORT)
        public String getProxyPort() {
            return m_defaults.getProxyServerSipPort();
        }

        @SettingEntry(paths = LAN_TFTP_SERVER_ADDRESS)
        public String getTftpServer() {
            return m_defaults.getTftpServer();
        }

        @SettingEntry(path = NETTIME_SNTP_SERVER_ADDRESS)
        public String getNtpServer() {
            return m_defaults.getNtpServer();
        }

        @SettingEntry(path = VOIP_MOH_URL)
        public String getMohUrl() {
            String mohUri = m_defaults.getMusicOnHoldUri();
            return SipUri.stripSipPrefix(mohUri);
        }
    }
}

/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.phone.aastra;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class AastraLineDefaults {

    private static final String SIP_USER_NAME = "sip_id/user_name";
    private static final String SIP_AUTH_NAME = "sip_id/auth_name";
    private static final String SIP_AUTH_PASSWD = "sip_id/password";
    private static final String SIP_SCREEN_NAME = "sip_id/screen_name";
    private static final String SIP_SCREEN_NAME2 = "sip_id/screen_name2";
    private static final String SIP_DISPLAY_NAME = "sip_id/display_name";
    private static final String PROXY_IP = "server/proxyIp";
    private static final String PROXY_PORT = "server/proxy_port";
    private static final String REGISTRAR_IP = "server/registrar_ip";
    private static final String REGISTRAR_PORT = "server/registrar_port";

    private DeviceDefaults m_defaults;
    private Line m_line;

    AastraLineDefaults(DeviceDefaults defaults, Line line) {
        m_defaults = defaults;
        m_line = line;
    }

    @SettingEntry(path = SIP_AUTH_NAME)
    public String getAuthorizationId() {
        return getAddress();
    }

    @SettingEntry(path = SIP_USER_NAME)
    public String getAddress() {
        User u = m_line.getUser();
        if (u != null) {
            return u.getUserName();
        }
        return null;
    }

    @SettingEntry(path = SIP_AUTH_PASSWD)
    public String getAuthorizationPassword() {
        User u = m_line.getUser();
        if (u != null) {
            return u.getSipPassword();
        }
        return null;
    }

    @SettingEntry(path = SIP_DISPLAY_NAME)
    public String getDisplayName() {
        User u = m_line.getUser();
        if (u != null) {
            return u.getDisplayName();
        }
        return null;
    }

    @SettingEntry(path = SIP_SCREEN_NAME)
    public String getScreenName() {
        User u = m_line.getUser();
        if (u != null) {
            return u.getUserName() + "@" + m_defaults.getDomainName();
            // return u.getFirstName();
        }
        return null;
    }

    @SettingEntry(path = SIP_SCREEN_NAME2)
    public String getScreenName2() {
        User u = m_line.getUser();
        if (u != null) {
            return u.getDisplayName();
            // return u.getLastName();
        }
        return null;
    }

    @SettingEntry(paths = { REGISTRAR_IP, PROXY_IP })
    public String getRegistrationServer() {
        User u = m_line.getUser();
        if (u != null) {
            return m_defaults.getDomainName();
        }
        return null;
    }

    @SettingEntry(paths = { REGISTRAR_PORT, PROXY_PORT })
    public String getProxyPort() {
        User u = m_line.getUser();
        if (u != null) {
            return m_defaults.getProxyServerSipPort();
        }
        return null;
    }
}

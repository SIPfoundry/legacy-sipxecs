/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.lg_nortel;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class LgNortelLineDefaults {

    private static final String VOIP_PROXY_PORT = "VOIP/proxy_port";
    private static final String VOIP_PROXY_ADDRESS = "VOIP/proxy_address";
    private static final String VOIP_AUTHNAME = "VOIP/authname";
    private static final String VOIP_PASSWORD = "VOIP/password";
    private static final String VOIP_NAME = "VOIP/name";
    private static final String VOIP_DISPLAYNAME = "VOIP/displayname";
    private static final String VOIP_TYPE = "VOIP/type";

    private final Line m_line;
    private final DeviceDefaults m_defaults;

    public LgNortelLineDefaults(DeviceDefaults defaults, Line line) {
        m_line = line;
        m_defaults = defaults;
    }

    @SettingEntry(path = VOIP_PROXY_ADDRESS)
    public String getProxyAddress() {

        return m_defaults.getDomainName();
    }

    @SettingEntry(path = VOIP_PROXY_PORT)
    public String getProxyPort() {
        return m_defaults.getProxyServerSipPort();
    }

    @SettingEntry(path = VOIP_NAME)
    public String getUserName() {
        return m_line.getUserName();
    }

    @SettingEntry(path = VOIP_AUTHNAME)
    public String getAuthUserName() {
        if (m_line.getSettingValue(VOIP_TYPE) == "dss") {
            return null;
        }

        return m_line.getAuthenticationUserName();
    }

    @SettingEntry(path = VOIP_DISPLAYNAME)
    public String getDisplayname() {
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }

        return user.getDisplayName();
    }

    @SettingEntry(path = VOIP_PASSWORD)
    public String getPassword() {
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }

        return user.getSipPassword();
    }

    @SettingEntry(path = "VOIP/extension")
    public String getExtension() {
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }

        return user.getExtension(false);
    }

    public static LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = new LineInfo();
        lineInfo.setUserId(line.getSettingValue(VOIP_NAME));
        lineInfo.setDisplayName(line.getSettingValue(VOIP_DISPLAYNAME));
        lineInfo.setPassword(line.getSettingValue(VOIP_PASSWORD));
        lineInfo.setRegistrationServer(line.getSettingValue(VOIP_PROXY_ADDRESS));
        lineInfo.setRegistrationServerPort(line.getSettingValue(VOIP_PROXY_PORT));
        return lineInfo;
    }

    public static void setLineInfo(Line line, LineInfo lineInfo) {
        line.setSettingValue(VOIP_DISPLAYNAME, lineInfo.getDisplayName());
        line.setSettingValue(VOIP_NAME, lineInfo.getUserId());
        line.setSettingValue(VOIP_PASSWORD, lineInfo.getPassword());

        line.setSettingValue(VOIP_AUTHNAME, lineInfo.getUserId());

        line.setSettingValue(VOIP_PROXY_ADDRESS, lineInfo.getRegistrationServer());
        line.setSettingValue(VOIP_PROXY_PORT, lineInfo.getRegistrationServerPort());
    }
}

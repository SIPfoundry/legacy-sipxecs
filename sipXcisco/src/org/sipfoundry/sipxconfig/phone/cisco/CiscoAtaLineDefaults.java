/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.cisco;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class CiscoAtaLineDefaults {
    private static final String PASSWORD_PATH = "port/PWD";
    private static final String LOGIN_ID_PATH = "port/LoginID";
    private static final String USER_ID_PATH = "port/UID";
    private static final String REGISTRATION_PORT_PATH = "port/_ProxyPort.79";
    private static final String REGISTRATION_PORT_ATA_PATH = "port/_ProxyPort.18x";
    private static final String DISPLAY_NAME_PATH = "port/DisplayName";
    private static final String REGISTRATION_PATH = "port/_Proxy.79";
    private static final String REGISTRATION_ATA_PATH = "port/_Proxy.18x";

    private Line m_line;

    public CiscoAtaLineDefaults(Line line) {
        m_line = line;
    }

    @SettingEntry(paths = { USER_ID_PATH, LOGIN_ID_PATH })
    public String getUserName() {
        String userId = null;
        User u = m_line.getUser();
        if (u != null) {
            userId = u.getUserName();
        }
        return userId;
    }

    @SettingEntry(path = PASSWORD_PATH)
    public String getPassword() {
        String password = null;
        User u = m_line.getUser();
        if (u != null) {
            password = u.getSipPassword();
        }
        return password;
    }

    @SettingEntry(path = DISPLAY_NAME_PATH)
    public String getDisplayName() {
        String displayName = null;
        User u = m_line.getUser();
        if (u != null) {
            displayName = u.getDisplayName();
        }
        return displayName;
    }

    @SettingEntry(paths = { REGISTRATION_PATH, REGISTRATION_ATA_PATH })
    public String getRegistrationServer() {
        return m_line.getPhoneContext().getPhoneDefaults().getDomainName();
    }

    static LineInfo getLineInfo(CiscoModel model, Line line) {
        LineInfo lineInfo = new LineInfo();
        lineInfo.setDisplayName(line.getSettingValue(DISPLAY_NAME_PATH));
        lineInfo.setUserId(line.getSettingValue(USER_ID_PATH));
        if (model.isAta()) {
            lineInfo.setRegistrationServer(line.getSettingValue(REGISTRATION_ATA_PATH));
            lineInfo.setRegistrationServerPort(line.getSettingValue(REGISTRATION_PORT_ATA_PATH));
        } else {
            lineInfo.setRegistrationServer(line.getSettingValue(REGISTRATION_PATH));
            lineInfo.setRegistrationServerPort(line.getSettingValue(REGISTRATION_PORT_PATH));
        }
        return lineInfo;
    }

    static void setLineInfo(CiscoModel model, Line line, LineInfo lineInfo) {
        line.setSettingValue(DISPLAY_NAME_PATH, lineInfo.getDisplayName());
        line.setSettingValue(USER_ID_PATH, lineInfo.getUserId());
        if (model.isAta()) {
            line.setSettingValue(REGISTRATION_ATA_PATH, lineInfo.getRegistrationServer());
            line
                    .setSettingValue(REGISTRATION_PORT_ATA_PATH, lineInfo
                            .getRegistrationServerPort());
        } else {
            line.setSettingValue(REGISTRATION_PATH, lineInfo.getRegistrationServer());
            line.setSettingValue(REGISTRATION_PORT_PATH, lineInfo.getRegistrationServerPort());
        }
    }

    public static class StubAtaLine {
        @SettingEntry(paths = { DISPLAY_NAME_PATH, USER_ID_PATH, LOGIN_ID_PATH, USER_ID_PATH,
                PASSWORD_PATH })
        public String getZero() {
            return "0";
        }
    }
}

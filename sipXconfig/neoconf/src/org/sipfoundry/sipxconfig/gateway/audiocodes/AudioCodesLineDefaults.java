/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class AudioCodesLineDefaults {
    private static final String SIP_USER_NAME = "SIP/Authentication/Username";
    private static final String SIP_PASSWORD = "SIP/Authentication/Password";

    private Line m_line;

    public AudioCodesLineDefaults(Line line) {
        m_line = line;
    }

    @SettingEntry(path = SIP_USER_NAME)
    public String getUserName() {
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }

        return user.getUserName();
    }

    @SettingEntry(path = SIP_PASSWORD)
    public String getPassword() {
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }

        return user.getSipPassword();
    }

    static LineInfo getLineInfo(Line line, DeviceDefaults defaults) {
        LineInfo li = new LineInfo();
        li.setUserId(line.getSettingValue(SIP_USER_NAME));
        li.setPassword(line.getSettingValue(SIP_PASSWORD));
        li.setRegistrationServer(defaults.getDomainName());
        li.setRegistrationServerPort(defaults.getProxyServerSipPort());
        return li;
    }

    static void setLineInfo(Line line, LineInfo lineInfo) {
        line.setSettingValue(SIP_USER_NAME, lineInfo.getUserId());
        line.setSettingValue(SIP_PASSWORD, lineInfo.getPassword());
    }
}

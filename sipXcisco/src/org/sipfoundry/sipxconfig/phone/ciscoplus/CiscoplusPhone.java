/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.ciscoplus;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

/**
 * Ciscoplus942 phone.
 */
public class CiscoplusPhone extends Ciscoplus {

    private static final String USER_ID_SETTING = "sipLines/name";
    private static final String AUTH_NAME_SETTING = "sipLines/authName";
    private static final String DISPLAY_NAME_SETTING = "sipLines/displayName";
    private static final String PASSWORD_SETTING = "sipLines/authPassword";
    private static final String PROXY_SETTING = "sipLines/proxy";
    private static final String PROXY_PORT_SETTING = "sipLines/port";

    public CiscoplusPhone() {
    }

    @Override
    public void initializeLine(Line line) {
        line.addDefaultBeanSettingHandler(new CiscoplusLineDefaults(line));
    }

    @Override
    public String getProfileFilename() {
        return "SEP" + getSerialNumber().toUpperCase() + ".cnf.xml";
    }

    @Override
    public int getMaxLineCount() {
        return getModel().getMaxLineCount();
    }

    public Collection<Setting> getProfileLines() {
        int lineCount = getModel().getMaxLineCount();
        List<Setting> linesSettings = new ArrayList<Setting>(getMaxLineCount());

        Collection<Line> lines = getLines();
        int i = 0;
        Iterator<Line> ilines = lines.iterator();
        for (; ilines.hasNext() && (i < lineCount); i++) {
            linesSettings.add(ilines.next().getSettings());
        }

        for (; i < lineCount; i++) {
            Line line = createLine();
            line.setPhone(this);
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }

    public static class CiscoplusLineDefaults {
        private final Line m_line;

        CiscoplusLineDefaults(Line line) {
            m_line = line;
        }

        @SettingEntry(paths = { USER_ID_SETTING, AUTH_NAME_SETTING })
        public String getUserName() {
            String userName = null;
            User user = m_line.getUser();
            if (user != null) {
                userName = user.getUserName();
            }
            return userName;
        }

        @SettingEntry(path = DISPLAY_NAME_SETTING)
        public String getDisplayName() {
            String displayName = null;
            User user = m_line.getUser();
            if (user != null) {
                displayName = user.getDisplayName();
            }
            return displayName;
        }

        @SettingEntry(path = PASSWORD_SETTING)
        public String getPassword() {
            String password = null;
            User user = m_line.getUser();
            if (user != null) {
                password = user.getSipPassword();
            }
            return password;
        }

        @SettingEntry(path = PROXY_SETTING)
        public String getRegistrationServer() {
            DeviceDefaults defaults = m_line.getPhoneContext().getPhoneDefaults();
            return defaults.getDomainName();
        }

        @SettingEntry(path = PROXY_PORT_SETTING)
        public String getRegistrationServerPort() {
            DeviceDefaults defaults = m_line.getPhoneContext().getPhoneDefaults();
            return defaults.getProxyServerSipPort();
        }
    }

    /**
     * Each subclass must decide how as much of this generic line information translates into its
     * own setting model.
     */
    @Override
    protected void setLineInfo(Line line, LineInfo externalLine) {
        line.setSettingValue(DISPLAY_NAME_SETTING, externalLine.getDisplayName());
        line.setSettingValue(USER_ID_SETTING, externalLine.getUserId());
        line.setSettingValue(PASSWORD_SETTING, externalLine.getPassword());
        line.setSettingValue(PROXY_SETTING, externalLine.getRegistrationServer());
        line.setSettingValue(PROXY_PORT_SETTING, externalLine.getRegistrationServerPort());
    }

    /**
     * Each subclass must decide how as much of this generic line information can be contructed
     * from its own setting model.
     */
    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo info = new LineInfo();
        info.setDisplayName(line.getSettingValue(DISPLAY_NAME_SETTING));
        info.setUserId(line.getSettingValue(USER_ID_SETTING));
        info.setPassword(line.getSettingValue(PASSWORD_SETTING));
        info.setRegistrationServer(line.getSettingValue(PROXY_SETTING));
        info.setRegistrationServerPort(line.getSettingValue(PROXY_PORT_SETTING));
        return info;
    }
}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.Ciscospa;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

import static org.sipfoundry.sipxconfig.common.SipUri.stripSipPrefix;

/**
 * Ciscospa phone.
 */
public class CiscospaPhone extends Ciscospa {

    private static final String USER_ID_SETTING = "Ext/User_ID";
    private static final String DISPLAY_NAME_SETTING = "Ext/Display_Name";
    private static final String PASSWORD_SETTING = "Ext/Password";
    private static final String REGISTRATION_SERVER_SETTING = "Proxy_and_Registration/Proxy";

    public CiscospaPhone() {
    }

    @Override
    protected void initialize() {
        DeviceDefaults defaults = getPhoneContext().getPhoneDefaults();
        addDefaultBeanSettingHandler(new CiscospaPhoneDefaults(defaults));
    }

    @Override
    public void initializeLine(Line line) {
        DeviceDefaults defaults = getPhoneContext().getPhoneDefaults();
        line.addDefaultBeanSettingHandler(new CiscospaLineDefaults(defaults, line));
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

    public static class CiscospaPhoneDefaults {
        private final DeviceDefaults m_defaults;

        CiscospaPhoneDefaults(DeviceDefaults defaults) {
            m_defaults = defaults;
        }

        @SettingEntry(path = "Phone/Voice_Mail_Number")
        public String getVoicemailNumber() {
            return m_defaults.getVoiceMail();
        }
    }

    public static class CiscospaLineDefaults {
        private final Line m_line;
        private final DeviceDefaults m_defaults;

        CiscospaLineDefaults(DeviceDefaults defaults, Line line) {
            m_defaults = defaults;
            m_line = line;
        }

        @SettingEntry(path = USER_ID_SETTING)
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

        @SettingEntry(path = REGISTRATION_SERVER_SETTING)
        public String getRegistrationServer() {
            DeviceDefaults defaults = m_line.getPhoneContext().getPhoneDefaults();
            return defaults.getDomainName();
        }

        @SettingEntry(path = "Call_Feature_Settings/MOH_Server")
        public String getMohUrl() {
            String mohUri;
            User u = m_line.getUser();
            if (u != null) {
                mohUri = u.getMusicOnHoldUri();
            } else {
                mohUri = m_defaults.getMusicOnHoldUri();
            }
            return  SipUri.stripSipPrefix(mohUri);
        }

        @SettingEntry(path = "Call_Feature_Settings/Voice_Mail_Server")
        public String getVoicemailServer() {
            User user = m_line.getUser();
            if (user != null) {
                return stripSipPrefix(user.getAddrSpec(m_defaults.getDomainName()));
            }
            return null;
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
        line.setSettingValue(REGISTRATION_SERVER_SETTING, externalLine.getRegistrationServer());
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
        info.setRegistrationServer(line.getSettingValue(REGISTRATION_SERVER_SETTING));
        return info;
    }
}

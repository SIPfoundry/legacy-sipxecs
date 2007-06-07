/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.linksys;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

/**
 * Linksys942 phone.
 */
public class LinksysPhone extends Linksys {

    private static final String USER_ID_SETTING = "Extension_1/User_ID_1_";
    private static final String DISPLAY_NAME_SETTING = "Extension_1/Display_Name_1_";
    private static final String PASSWORD_SETTING = "Extension_1/Password_1_";
    private static final String REGISTRATION_SERVER_SETTING = "Extension_1/Outbound_Proxy_1_";
    private static final String REGISTRATION_SERVER_PORT_SETTING = "Extension_1/SIP_Port_1_";

    public LinksysPhone() {
    }

    @Override
    public void initializeLine(Line line) {
        line.addDefaultBeanSettingHandler(new LinksysLineDefaults(line));
    }

    @Override
    public String getProfileFilename() {
        return "spa" + getSerialNumber() + ".cfg";
    }

    public static class LinksysLineDefaults {
        private Line m_line;

        LinksysLineDefaults(Line line) {
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
    }

    /**
     * Each subclass must decide how as much of this generic line information translates into its
     * own setting model.
     */
    @Override
    protected void setLineInfo(Line line, LineInfo info) {
        line.setSettingValue(USER_ID_SETTING, info.getUserId());
        line.setSettingValue(DISPLAY_NAME_SETTING, info.getDisplayName());
        line.setSettingValue(PASSWORD_SETTING, info.getPassword());
        line.setSettingValue(REGISTRATION_SERVER_SETTING, info.getRegistrationServer());
        line.setSettingValue(REGISTRATION_SERVER_PORT_SETTING, info.getRegistrationServerPort());
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
        info.setRegistrationServerPort(line.getSettingValue(REGISTRATION_SERVER_PORT_SETTING));
        return info;
    }
}

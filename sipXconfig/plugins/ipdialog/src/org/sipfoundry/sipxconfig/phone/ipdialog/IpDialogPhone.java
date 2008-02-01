/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.ipdialog;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class IpDialogPhone extends Phone {

    public static final PhoneModel MODEL = new PhoneModel("ipDialog", "ipDialog SipTone V");
    public static final String BEAN_ID = "ipDialogPhoneStandard";
    public static final String USER_ID_SETTING = "sipAccounts/authname";
    public static final String PASSWORD_SETTING = "sipAccounts/authPassword";
    public static final String REGISTRATION_SERVER_SETTING = "sipAccounts/registrarAddress";
    public static final String PROXY_SERVER_SETTING = "sipAccounts/proxyAddress";

    public IpDialogPhone() {
    }

    @Override
    public void initializeLine(Line line) {
        line.addDefaultBeanSettingHandler(new IpDialogLineDefaults(line));
    }

    public String getProfileFilename() {
        return "SipTone/config/" + "ipdSIP" + getSerialNumber().toUpperCase() + ".xml";

    }

    public static class IpDialogLineDefaults {
        private Line m_line;

        IpDialogLineDefaults(Line line) {
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

        @SettingEntry(path = PROXY_SERVER_SETTING)
        public String getVoiceMail() {
            DeviceDefaults defaults = m_line.getPhoneContext().getPhoneDefaults();
            return defaults.getDomainName();
        }

    }

    @Override
    protected void setLineInfo(Line line, LineInfo info) {
        line.setSettingValue(USER_ID_SETTING, info.getUserId());
        line.setSettingValue(PASSWORD_SETTING, info.getPassword());
        line.setSettingValue(REGISTRATION_SERVER_SETTING, info.getRegistrationServer());
        line.setSettingValue(PROXY_SERVER_SETTING, info.getVoiceMail());
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo info = new LineInfo();
        info.setUserId(line.getSettingValue(USER_ID_SETTING));
        info.setPassword(line.getSettingValue(PASSWORD_SETTING));
        info.setRegistrationServer(line.getSettingValue(REGISTRATION_SERVER_SETTING));
        info.setVoiceMail(line.getSettingValue(PROXY_SERVER_SETTING));
        return info;
    }

    public void restart() {
        sendCheckSyncToFirstLine();
    }
}

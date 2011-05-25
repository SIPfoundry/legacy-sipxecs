/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.gtek;

import java.util.Collection;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

/**
 * Helix phone.
 */
public class GtekPhone extends Phone {
    private static final String USER_ID_SETTING = "SIP_ACCOUNT/REGIST_ID";
    private static final String DISPLAY_NAME_SETTING = "SIP_ACCOUNT/DISP_NAME";
    private static final String PASSWORD_SETTING = "SIP_ACCOUNT/REGIST_PASS";
    private static final String REGISTRATION_SERVER_SETTING = "SIP_ACCOUNT/REGIST_SERVER";
    private static final String MOH = "SIP_ACCOUNT/MOH";

    @Override
    public void initializeLine(Line line) {
        GtekPhoneDefaults defaults = new GtekPhoneDefaults(getPhoneContext().getPhoneDefaults());
        addDefaultBeanSettingHandler(defaults);
        line.addDefaultBeanSettingHandler(new HelixLineDefaults(line));
    }

    @Override
    public String getProfileFilename() {
        return getSerialNumber() + ".cfg";
    }

    public static class HelixLineDefaults {
        private final Line m_line;

        HelixLineDefaults(Line line) {
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

        @SettingEntry(path = MOH)
        public String getMusicOnHoldUri() {
            User u = m_line.getUser();
            if (u != null) {
                String mohUri = u.getMusicOnHoldUri();
                return  SipUri.stripSipPrefix(mohUri);
            }

            DeviceDefaults defaults = m_line.getPhoneContext().getPhoneDefaults();
            return defaults.getMusicOnHoldUri();
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

    @Override
    protected ProfileContext createContext() {
        SpeedDial speedDial = getPhoneContext().getSpeedDial(this);
        return new GtekProfileContext(this, speedDial, getModel().getProfileTemplate());
    }

    static class GtekProfileContext extends ProfileContext {
        private final SpeedDial m_speeddial;

        GtekProfileContext(GtekPhone phone, SpeedDial speeddial, String profileTemplate) {
            super(phone, profileTemplate);
            m_speeddial = speeddial;
        }

        @Override
        public Map<String, Object> getContext() {
            Map<String, Object> context = super.getContext();
            Phone phone = (Phone) getDevice();

            if (m_speeddial != null) {
                context.put("speeddial", m_speeddial);
            }

            int speeddialOffset = 0;
            Collection lines = phone.getLines();
            if (lines != null) {
                speeddialOffset = lines.size();
            }
            context.put("speeddial_offset", speeddialOffset);

            return context;
        }
    }
}

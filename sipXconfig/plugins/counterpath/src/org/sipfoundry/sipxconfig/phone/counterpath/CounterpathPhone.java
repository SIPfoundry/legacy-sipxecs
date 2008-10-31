/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.counterpath;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class CounterpathPhone extends Phone {
    private static final String REG_USERNAME = "registration/username";
    private static final String REG_AUTH_USERNAME = "registration/authorization_username";
    private static final String REG_DISPLAY_NAME = "registration/display_name";
    private static final String REG_PASSWORD = "registration/password";
    private static final String REG_DOMAIN = "registration/domain";
    private static final String SUBSCRIPTION_AOR = "network/sip_signaling/proxies:proxy0:workgroup_subscription_aor";

    public CounterpathPhone() {
    }

    @Override
    public String getProfileFilename() {
        return getSerialNumber() + ".ini";
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new CounterpathPhoneDefaults(this));

    }

    @Override
    protected ProfileContext createContext() {
        return new CounterpathProfileContext(this, getModel().getProfileTemplate());
    }

    @Override
    public void initializeLine(Line line) {
        line.addDefaultBeanSettingHandler(new CounterpathLineDefaults(line));
    }

    @Override
    protected void setLineInfo(Line line, LineInfo info) {
        line.setSettingValue(REG_USERNAME, info.getUserId());
        line.setSettingValue(REG_AUTH_USERNAME, info.getUserId());
        line.setSettingValue(REG_DISPLAY_NAME, info.getDisplayName());
        line.setSettingValue(REG_PASSWORD, info.getPassword());
        line.setSettingValue(REG_DOMAIN, info.getRegistrationServer());
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo info = new LineInfo();
        info.setDisplayName(line.getSettingValue(REG_DISPLAY_NAME));
        info.setUserId(line.getSettingValue(REG_USERNAME));
        info.setPassword(line.getSettingValue(REG_PASSWORD));
        info.setRegistrationServer(line.getSettingValue(REG_DOMAIN));
        return info;
    }

    public class CounterpathPhoneDefaults {
        private Phone m_phone;

        public CounterpathPhoneDefaults(Phone phone) {
            m_phone = phone;
        }

        @SettingEntry(path = SUBSCRIPTION_AOR)
        public String getWorkgroupSubscriptionAor() {
            SpeedDial speedDial = getPhoneContext().getSpeedDial(m_phone);
            if (speedDial != null) {
                return "sip:" + speedDial.getResourceListId(true);
            } else {
                return null;
            }
        }
    }

    public static class CounterpathLineDefaults {
        private Line m_line;

        public CounterpathLineDefaults(Line line) {
            m_line = line;
        }

        @SettingEntry(paths = { REG_USERNAME, REG_AUTH_USERNAME })
        public String getUserName() {
            String userName = null;
            User user = m_line.getUser();
            if (user != null) {
                userName = user.getUserName();
            }
            return userName;
        }

        @SettingEntry(path = REG_DISPLAY_NAME)
        public String getDisplayName() {
            String displayName = null;
            User user = m_line.getUser();
            if (user != null) {
                displayName = user.getDisplayName();
            }
            return displayName;
        }

        @SettingEntry(path = REG_PASSWORD)
        public String getPassword() {
            String password = null;
            User user = m_line.getUser();
            if (user != null) {
                password = user.getSipPassword();
            }
            return password;
        }

        @SettingEntry(path = REG_DOMAIN)
        public String getDomain() {
            DeviceDefaults defaults = m_line.getPhoneContext().getPhoneDefaults();
            return defaults.getDomainName();
        }
    }
}

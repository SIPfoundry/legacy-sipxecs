/*
 *
 *
 * Copyright (C) 2010 Karel Elektronik, A.S. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.phone.karel_ip11x;

import org.sipfoundry.sipxconfig.common.User;

import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileFilter;

import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class KarelIP11xPhone extends Phone {
    public static final String MIME_TYPE_PLAIN = "text/plain";
    private static final String USER_ID_SETTING = "account/UserName";
    private static final String AUTH_USER_ID_SETTING = "account/AuthName";
    private static final String DISPLAY_NAME_SETTING = "account/DisplayName";
    private static final String PASSWORD_SETTING = "account/password";
    private static final String REGISTRATION_SERVER_SETTING = "account/SIPServerHost";

    @Override
    public void initializeLine(Line line) {
        KarelIP11xPhoneDefaults defaults = new KarelIP11xPhoneDefaults(getPhoneContext().getPhoneDefaults());
        addDefaultBeanSettingHandler(defaults);
        line.addDefaultBeanSettingHandler(new KarelIP11xLineDefaults(line));
    }

    @Override
    public void initialize() {
        KarelIP11xPhoneDefaults defaults = new KarelIP11xPhoneDefaults(getPhoneContext().getPhoneDefaults());
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public String getProfileFilename() {
        return getSerialNumber() + ".cfg";
    }

    public static class KarelIP11xLineDefaults {
        private final Line m_line;

        KarelIP11xLineDefaults(Line line) {
            m_line = line;
        }

        @SettingEntry(path = USER_ID_SETTING)
        public String getUserName() {
            return m_line.getUserName();
        }

        @SettingEntry(path = AUTH_USER_ID_SETTING)
        public String getAuthUserName() {
            return m_line.getAuthenticationUserName();
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
    }

    /**
     * Each subclass must decide how as much of this generic line information can be constructed
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

    public String getPhoneFilename() {
        return getProfileFilename();
    }

//  DialNow support
    public String getDialNowFilename() {
        return getSerialNumber() + "_dialnow.xml";
    }

    @Override
    public Profile[] getProfileTypes() {
        Profile[] profileTypes = new Profile[] {
            new PhoneProfile(getPhoneFilename()), new DialNowProfile(getDialNowFilename())
        };
        return profileTypes;
    }

    static class PhoneProfile extends Profile {
        public PhoneProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            KarelIP11xPhone phone = (KarelIP11xPhone) device;
            return new PhoneConfiguration(phone);
        }
    }

    static class DialNowProfile extends Profile {
        public DialNowProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            KarelIP11xPhone phone = (KarelIP11xPhone) device;
            return new DialNowConfiguration(phone);
        }
    }
}

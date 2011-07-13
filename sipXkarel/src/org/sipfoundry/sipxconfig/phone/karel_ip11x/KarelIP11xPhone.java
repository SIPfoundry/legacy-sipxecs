/*
 *
 *
 * Copyright (C) 2010 Karel Electronics Corp. All rights reserved.
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

import java.text.MessageFormat;
import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileFilter;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class KarelIP11xPhone extends Phone {
    public static final String MIME_TYPE_PLAIN = "text/plain";
    private static final String USER_ID_SETTING = "account/UserName";
    private static final String AUTH_USER_ID_SETTING = "account/AuthName";
    private static final String DISPLAY_NAME_SETTING = "account/DisplayName";
    private static final String LABEL_SETTING = "account/Label";
    private static final String PASSWORD_SETTING = "account/password";
    private static final String REGISTRATION_SERVER_SETTING = "account/SIPServerHost";
    private static final String MOH_SERVER_SETTING = "account/MusicServerUri";
    private static final String SPACE_CHAR = " ";

    private String m_phonebookFilename = "{0}/contactData1.xml";
    private String m_dialNowFilename = "{0}/dialnow.xml";

    private LocalizationContext m_localizationContext;

    public void setLocalizationContext(LocalizationContext localizationContext) {
        m_localizationContext = localizationContext;
    }

    @Override
    public void initializeLine(Line line) {
	String currentLanguage = null;
	if (m_localizationContext != null) {
            currentLanguage = m_localizationContext.getCurrentLanguage();
        }
        KarelIP11xPhoneDefaults defaults = new KarelIP11xPhoneDefaults(getPhoneContext().getPhoneDefaults(),
                currentLanguage);
        addDefaultBeanSettingHandler(defaults);
        line.addDefaultBeanSettingHandler(new KarelIP11xLineDefaults(line));
    }

    @Override
    public void initialize() {
	String currentLanguage = null;
	if (m_localizationContext != null) {
            currentLanguage = m_localizationContext.getCurrentLanguage();
        }
        KarelIP11xPhoneDefaults defaults = new KarelIP11xPhoneDefaults(getPhoneContext().getPhoneDefaults(),
                currentLanguage);
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public String getProfileFilename() {
        return getSerialNumber() + ".cfg";
    }

    @Override
    public void restart() {
        sendCheckSyncToFirstLine();
    }

    public static class KarelIP11xLineDefaults {
        private final Line m_line;

        KarelIP11xLineDefaults(Line line) {
            m_line = line;
        }

        @SettingEntry(path = LABEL_SETTING)
        public String getLabelString() {
            String labelName = null;
            User user = m_line.getUser();
            if (user != null) {
                labelName = m_line.getUserName();
                if (user.getFirstName() != null) {
                    if (user.getFirstName().length() > 5) {
                        labelName = labelName + SPACE_CHAR + user.getFirstName().substring(0, 5);
                    } else {
                        labelName = labelName + SPACE_CHAR + user.getFirstName();
                    }
                }
            }
            return labelName;
        }

        @SettingEntry(path = USER_ID_SETTING)
        public String getUserName() {
            return m_line.getUserName();
        }

        @SettingEntry(path = AUTH_USER_ID_SETTING)
        public String getAuthUserName() {
            return m_line.getUserName();
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

        @SettingEntry(path = MOH_SERVER_SETTING)
        public String getMusicOnHoldUri() {
            DeviceDefaults defaults = m_line.getPhoneContext().getPhoneDefaults();
            String mohUri;
            User u = m_line.getUser();
            if (u != null) {
                mohUri = u.getMusicOnHoldUri();
            } else {
                mohUri = defaults.getMusicOnHoldUri();
            }
            return mohUri + ":5060";
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

    @Override
    public Profile[] getProfileTypes() {
        Profile[] profileTypes = new Profile[] {
            new PhoneProfile(getPhoneFilename()), new DialNowProfile(getDialNowFilename()),
            new PhonebookProfile(getPhonebookFilename())
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

    public ProfileContext getPhonebook() {
        Collection<PhonebookEntry> entries = getPhoneContext().getPhonebookEntries(this);
        return new KarelIP11xPhonebook(entries);
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

    static class PhonebookProfile extends Profile {
        public PhonebookProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            KarelIP11xPhone phone = (KarelIP11xPhone) device;
            return phone.getPhonebook();
        }
    }

    public void setPhonebookFilename(String phonebookFilename) {
        m_phonebookFilename = phonebookFilename;
    }

    public String getPhonebookFilename() {
        return MessageFormat.format(m_phonebookFilename, getSerialNumber());
    }

    public void setDialNowFilename(String dialnowFilename) {
        m_dialNowFilename = dialnowFilename;
    }

    public String getDialNowFilename() {
        return MessageFormat.format(m_dialNowFilename, getSerialNumber());
    }
}

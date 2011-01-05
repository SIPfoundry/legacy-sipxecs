/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.unidata;

import java.text.MessageFormat;
import java.util.Collection;

import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileFilter;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class UnidataPhone extends Phone {
    private static final String USER_ID_SETTING = "USER_ACCOUNT/User_ID";
    private static final String DISPLAY_NAME_SETTING = "USER_ACCOUNT/Displayname";
    private static final String PASSWORD_SETTING = "USER_ACCOUNT/User_Password";
    private static final String REGISTRATION_SERVER_SETTING = "SERVER_SETTINGS/1st_Proxy";

    private String m_phonebookFilename = "{0}-phonebook.csv";

    public UnidataPhone() {
    }

    @Override
    public void initializeLine(Line line) {
        UnidataLineDefaults defaults = new UnidataLineDefaults(line);
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initialize() {
        UnidataPhoneDefaults defaults = new UnidataPhoneDefaults(getPhoneContext().getPhoneDefaults());
        addDefaultBeanSettingHandler(defaults);
        UnidataPhonebookDefaults phonebookDefaults = new UnidataPhonebookDefaults();
        addDefaultBeanSettingHandler(phonebookDefaults);
    }

    @Override
    public Profile[] getProfileTypes() {
        Profile[] profileTypes;
        PhonebookManager phonebookManager = getPhonebookManager();
        if (phonebookManager.getPhonebookManagementEnabled()) {
            profileTypes = new Profile[] {
                new Profile(this), new PhonebookProfile(getPhonebookFilename())
            };
        } else {
            profileTypes = new Profile[] {
                new Profile(this)
            };
        }

        return profileTypes;
    }

    public ProfileContext getPhonebook() {
        Collection<PhonebookEntry> entries = getPhoneContext().getPhonebookEntries(this);
        return new UnidataPhonebook(entries);
    }

    static class PhonebookProfile extends Profile {
        public PhonebookProfile(String name) {
            super(name, "text/csv");
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            UnidataPhone phone = (UnidataPhone) device;
            return phone.getPhonebook();
        }
    }

    @Override
    public String getProfileFilename() {
        String serialNumber = getSerialNumber();
        return "e1_" + serialNumber + ".ini";
    }

    @Override
    public void restart() {
        sendCheckSyncToFirstLine();
    }

    public void setPhonebookFilename(String phonebookFilename) {
        m_phonebookFilename = phonebookFilename;
    }

    public String getPhonebookFilename() {
        return MessageFormat.format(m_phonebookFilename, getSerialNumber());
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo info = new LineInfo();
        info.setDisplayName(line.getSettingValue(DISPLAY_NAME_SETTING));
        info.setUserId(line.getSettingValue(USER_ID_SETTING));
        info.setPassword(line.getSettingValue(PASSWORD_SETTING));
        info.setRegistrationServer(getSettingValue(REGISTRATION_SERVER_SETTING));

        return info;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
        line.setSettingValue(DISPLAY_NAME_SETTING, lineInfo.getDisplayName());
        line.setSettingValue(USER_ID_SETTING, lineInfo.getUserId());
        line.setSettingValue(PASSWORD_SETTING, lineInfo.getPassword());
        line.setSettingValue(REGISTRATION_SERVER_SETTING, lineInfo.getRegistrationServer());
    }

    public class UnidataPhonebookDefaults {
        @SettingEntry(path = "PROVISION/Phonebook_Name")
        public String getPhonebookName() {
            return getPhonebookFilename();
        }
    }
}
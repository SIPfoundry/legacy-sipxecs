/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.lg_nortel;

import java.text.MessageFormat;
import java.util.Collection;
import java.util.Collections;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileFilter;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class LgNortelPhone extends Phone {
    private String m_phonebookFilename = "{0}-phonebook.csv";

    public LgNortelPhone() {
    }

    @Override
    public void initializeLine(Line line) {
        DeviceDefaults phoneDefaults = getPhoneContext().getPhoneDefaults();
        LgNortelLineDefaults defaults = new LgNortelLineDefaults(phoneDefaults, line);
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initialize() {
        DeviceDefaults phoneDefaults = getPhoneContext().getPhoneDefaults();
        int lines = getLines().size();
        LgNortelPhoneDefaults defaults = new LgNortelPhoneDefaults(phoneDefaults, lines);
        addDefaultBeanSettingHandler(defaults);
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
            profileTypes = new Profile[] {new Profile(this)};
        }

        return profileTypes;
    }

    public ProfileContext getPhonebook() {
        Collection<PhonebookEntry> entries = getPhoneContext().getPhonebookEntries(this);
        return new LgNortelPhonebook(entries);
    }

    @Override
    public void removeProfiles(ProfileLocation location) {
        super.removeProfiles(location);
        location.removeProfile(getPhonebookFilename());
    }

    @Override
    protected ProfileContext createContext() {
        SpeedDial speedDial = getPhoneContext().getSpeedDial(this);
        return new LgNortelProfileContext(this, speedDial, getModel().getProfileTemplate());
    }

    static class LgNortelProfileContext extends ProfileContext {
        private SpeedDial m_speeddial;

        LgNortelProfileContext(LgNortelPhone phone, SpeedDial speeddial, String profileTemplate) {
            super(phone, profileTemplate);
            m_speeddial = speeddial;
        }

        public Map<String, Object> getContext() {
            Map<String, Object> context = super.getContext();
            Collection buttons = Collections.EMPTY_LIST;
            if (m_speeddial != null) {
                buttons = m_speeddial.getButtons();
            }
            context.put("speeddials", buttons);

            int speeddialOffset = 0;
            Collection lines = ((Phone) getDevice()).getLines();
            if (lines != null) {
                speeddialOffset = lines.size();
            }
            context.put("speeddial_offset", speeddialOffset);

            return context;
        }
    }

    static class PhonebookProfile extends Profile {
        public PhonebookProfile(String name) {
            super(name, "text/csv");
        }

        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        protected ProfileContext createContext(Device device) {
            LgNortelPhone phone = (LgNortelPhone) device;
            return phone.getPhonebook();
        }
    }

    @Override
    public String getProfileFilename() {
        return StringUtils.defaultString(getSerialNumber()).toUpperCase();
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = LgNortelLineDefaults.getLineInfo(line);
        return lineInfo;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
        LgNortelLineDefaults.setLineInfo(line, lineInfo);
    }

    @Override
    public void restart() {
        sendCheckSyncToFirstLine();
    }

    public void setPhonebookFilename(String phonebookFilename) {
        m_phonebookFilename = phonebookFilename;
    }

    public String getPhonebookFilename() {
        return MessageFormat.format(m_phonebookFilename, getProfileFilename());
    }
}

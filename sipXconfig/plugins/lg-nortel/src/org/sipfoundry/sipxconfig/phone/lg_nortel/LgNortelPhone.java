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
import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;

import org.apache.commons.lang.StringUtils;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
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
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class LgNortelPhone extends Phone {
    private static final String VOIP_EXTENSION = "VOIP/extension";
    private static final String VOIP_DISPLAYNAME = "VOIP/displayname";
    private static final String VOIP_NAME = "VOIP/name";
    private static final String VOIP_TYPE = "VOIP/type";
    private CoreContext m_coreContext;

    private String m_phonebookFilename = "{0}-phonebook.csv";

    public LgNortelPhone() {
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
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
            profileTypes = new Profile[] {
                new Profile(this)
            };
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
        return new LgNortelProfileContext(this, speedDial, m_coreContext, getModel().getProfileTemplate());
    }

    static class LgNortelProfileContext extends ProfileContext {
        private final SpeedDial m_speeddial;
        private CoreContext m_coreContext;

        LgNortelProfileContext(LgNortelPhone phone, SpeedDial speeddial, CoreContext coreContext,
                String profileTemplate) {
            super(phone, profileTemplate);
            m_speeddial = speeddial;
            m_coreContext = coreContext;
        }

        public void setCoreContext(CoreContext coreContext) {
            m_coreContext = coreContext;
        }

        @Override
        public Map<String, Object> getContext() {
            Map<String, Object> context = super.getContext();
            Phone phone = (Phone) getDevice();

            if (m_speeddial != null) {
                boolean hasBlf = false;
                Collection<Button> speeddials = new ArrayList<Button>();
                Collection<Button> buttons = m_speeddial.getButtons();
                for (Button button : buttons) {
                    if (button.isBlf()) {
                        hasBlf = true;
                        Line line = new Line();
                        line.setPhone(phone);
                        phone.initializeLine(line);
                        String number = button.getNumber();
                        User user = m_coreContext.loadUserByAlias(number);
                        if (user != null) {
                            // if number matches any known user make sure
                            // to use username and not an alias
                            number = user.getUserName();
                        }
                        line.setSettingValue(VOIP_DISPLAYNAME, button.getLabel());
                        line.setSettingValue(VOIP_NAME, number);
                        line.setSettingValue(VOIP_EXTENSION, number);
                        line.setSettingValue(VOIP_TYPE, "dss");
                        phone.addLine(line);
                    } else {
                        speeddials.add(button);
                    }
                }
                context.put("has_blf", hasBlf);
                context.put("speeddials", speeddials);
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
        sendCheckSyncToMac();
    }

    public void setPhonebookFilename(String phonebookFilename) {
        m_phonebookFilename = phonebookFilename;
    }

    public String getPhonebookFilename() {
        return MessageFormat.format(m_phonebookFilename, getProfileFilename());
    }
}

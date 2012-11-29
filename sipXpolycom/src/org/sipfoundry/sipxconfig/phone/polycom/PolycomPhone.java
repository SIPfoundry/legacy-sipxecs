/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import static java.lang.String.format;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.SAXReader;
import org.dom4j.io.XMLWriter;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileFilter;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.setting.DelegatingSettingModel.InsertValueFilter;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlEscapeValueFilter;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

/**
 * Support for Polycom 300, 400, and 500 series phones and model 3000 conference phone
 */
public class PolycomPhone extends Phone {
    static final String COMMA = ",";
    static final String EQUALS = "=";
    static final String MIME_TYPE_PLAIN = "text/plain";
    static final String BEAN_ID = "polycom";
    static final String CALL = "call";
    static final String EMERGENCY = "dialplan/digitmap/routing.1/emergency.1.value";
    static final String REGISTRATION_PATH = "reg/server/1/address";
    static final String REGISTRATION_PORT_PATH = "reg/server/1/port";
    static final String CONTACT_MODE = "contact";
    static final String DISPLAY_NAME_PATH = "reg/displayName";
    static final String REGISTRATION_LABEL = "reg/label";
    static final String TYPE_PATH = "reg/type";
    static final String THIRD_PARTY_NAME_PATH = "reg/thirdPartyName";
    static final String PASSWORD_PATH = "reg/auth.password";
    static final String USER_ID_PATH = "reg/address";
    static final String AUTHORIZATION_ID_PATH = "reg/auth.userId";
    static final String CALL_BACK_PATH = "msg.mwi/callBack";
    static final String CALL_BACK_MODE_PATH = "msg.mwi/callBackMode";
    static final String SUBSCRIBE_PATH = "msg.mwi/subscribe";
    static final String TEMPLATE_DIR = "polycom/mac-address.d";
    static final String MB_PROXY = "mb/proxy";
    static final String MB_IDLE_DISPLAY_HOME_PAGE = "mb/idleDisplay/home";
    static final String MB_IDLE_DISPLAY_REFRESH = "mb/idleDisplay/refresh";
    static final String MB_MAIN_HOME_PAGE = "mb/main/home";
    static final String MB_MAIN_HOME_IDLE = "mb/main/idleTimeout";
    static final String MB_MAIN_HOME_STATUSBAR = "mb/main/statusbar";
    static final String MB_LIMITS_NODES = "mb/limits/nodes";
    static final String MB_LIMITS_CACHE = "mb/limits/cache";

    public PolycomPhone() {
        setDeviceVersion(PolycomModel.VER_2_0);
    }

    public String getDefaultVersionId() {
        DeviceVersion version = getDeviceVersion();
        return version != null ? version.getVersionId() : null;
    }

    /**
     * Default firmware version for polycom phones. Default is 1.6 right now
     *
     * @param defaultVersionId 1.6 or 2.0
     */
    public void setDefaultVersionId(String defaultVersionId) {
        setDeviceVersion(DeviceVersion.getDeviceVersion(PolycomPhone.BEAN_ID + defaultVersionId));
    }

    @Override
    public void initialize() {
        SpeedDial speedDial = getPhoneContext().getSpeedDial(this);
        PolycomPhoneDefaults phoneDefaults = new PolycomPhoneDefaults(getPhoneContext().getPhoneDefaults(),
                speedDial);
        addDefaultBeanSettingHandler(phoneDefaults);

        PolycomIntercomDefaults intercomDefaults = new PolycomIntercomDefaults(this);
        addDefaultBeanSettingHandler(intercomDefaults);
    }

    @Override
    public void initializeLine(Line line) {
        PolycomLineDefaults lineDefaults = new PolycomLineDefaults(getPhoneContext().getPhoneDefaults(), line);
        line.addDefaultBeanSettingHandler(lineDefaults);
    }

    @Override
    protected ProfileFilter getProfileFilter() {
        return new FormatFilter();
    }

    @Override
    public Profile[] getProfileTypes() {
        Profile[] profileTypes = new Profile[] {
            new ApplicationProfile(getAppFilename()), new SipProfile(getSipFilename()),
            new PhoneProfile(getPhoneFilename()), new DeviceProfile(getDeviceFilename())
        };

        if (getPhonebookManager().getPhonebookManagementEnabled()) {
            profileTypes = (Profile[]) ArrayUtils.add(profileTypes, new DirectoryProfile(getDirectoryFilename()));
        }

        return profileTypes;
    }

    @Override
    public String getProfileFilename() {
        return getSerialNumber();
    }

    @Override
    protected void beforeProfileGeneration() {
        InsertValueFilter insert = new InsertValueFilter(XmlEscapeValueFilter.FILTER);
        Setting settings = getSettings();
        settings.acceptVisitor(insert);
        for (Line line : getLines()) {
            Setting lineSettings = line.getSettings();
            lineSettings.acceptVisitor(insert);
        }
    }

    @Override
    public void removeProfiles(ProfileLocation location) {
        Profile[] profiles = getProfileTypes();
        for (Profile profile : profiles) {
            location.removeProfile(profile.getName());
        }
    }

    /**
     * Polycom 430 1.6.5 would not read files w/being formatted first. Unclear why.
     */
    static class FormatFilter implements ProfileFilter {

        public void copy(InputStream in, OutputStream out) throws IOException {
            SAXReader xmlReader = new SAXReader();
            Document doc;
            try {
                doc = xmlReader.read(in);
            } catch (DocumentException e1) {
                throw new RuntimeException(e1);
            }
            OutputFormat pretty = OutputFormat.createPrettyPrint();
            XMLWriter xml = new XMLWriter(out, pretty);
            xml.write(doc);
        }
    }

    public String getAdditionalPhoneSettings() {
        List<String> settings = new ArrayList<String>();
        addSetting(settings, MB_PROXY, MB_IDLE_DISPLAY_HOME_PAGE, MB_IDLE_DISPLAY_REFRESH, MB_MAIN_HOME_PAGE,
                MB_MAIN_HOME_IDLE, MB_MAIN_HOME_STATUSBAR, MB_LIMITS_NODES, MB_LIMITS_CACHE);

        return StringUtils.join(settings, COMMA);
    }

    public void setAdditionalPhoneSettings(String additionalSettings) {
        List<String> settings = Arrays.asList(StringUtils.split(additionalSettings, COMMA));
        settings = Arrays.asList(StringUtils.split(additionalSettings, COMMA));
        for (String setting : settings) {
            setSettingValue(StringUtils.substringBefore(setting, EQUALS),
                    StringUtils.substringAfter(setting, EQUALS));
        }
    }

    public List<String> getLinePaths() {
        List<String> paths = new ArrayList<String>();
        paths.add(REGISTRATION_LABEL);

        return paths;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo externalLine) {
        line.setSettingValue(DISPLAY_NAME_PATH, externalLine.getDisplayName());
        line.setSettingValue(USER_ID_PATH, externalLine.getUserId());
        line.setSettingValue(PASSWORD_PATH, externalLine.getPassword());

        String voiceMail = externalLine.getVoiceMail();
        if (voiceMail != null) {
            line.setSettingValue(CALL_BACK_PATH, voiceMail);
            line.setSettingValue(CALL_BACK_MODE_PATH, CONTACT_MODE);
            line.setSettingValue(SUBSCRIBE_PATH, externalLine.getUserId());
        }

        // Both userId and authId are required, see XCF-914
        line.setSettingValue(AUTHORIZATION_ID_PATH, externalLine.getUserId());

        line.setSettingValue(REGISTRATION_PATH, externalLine.getRegistrationServer());
        line.setSettingValue(REGISTRATION_PORT_PATH, externalLine.getRegistrationServerPort());
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = new LineInfo();
        lineInfo.setUserId(line.getSettingValue(USER_ID_PATH));
        lineInfo.setDisplayName(line.getSettingValue(DISPLAY_NAME_PATH));
        lineInfo.setPassword(line.getSettingValue(PASSWORD_PATH));
        lineInfo.setRegistrationServer(line.getSettingValue(REGISTRATION_PATH));
        lineInfo.setRegistrationServerPort(line.getSettingValue(REGISTRATION_PORT_PATH));
        return lineInfo;
    }

    @Override
    public void restart() {
        sendCheckSyncToMac();
    }

    public String getAppFilename() {
        return format("%s.cfg", getProfileFilename());
    }

    public String getSipFilename() {
        return format("%s-sipx-sip.cfg", getProfileFilename());
    }

    public String getPhoneFilename() {
        return format("%s-sipx-phone.cfg", getProfileFilename());
    }

    public String getDirectoryFilename() {
        return format("%s-directory.xml", getProfileFilename());
    }

    public String getDeviceFilename() {
        return format("%s-sipx-device.cfg", getProfileFilename());
    }

    private void addSetting(List<String> settingsList, String... paths) {
        Setting settings = getSettings();
        if (settings != null) {
            for (String path : paths) {
                Setting setting = settings.getSetting(path);
                String settingValue = (null == setting ? null : (setting.getValue() == null ? null : setting
                        .getValue()));
                if (!StringUtils.isEmpty(settingValue)) {
                    settingsList.add(path + EQUALS + settingValue);
                }
            }
        }
    }

    static class ApplicationProfile extends Profile {
        public ApplicationProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            PolycomPhone phone = (PolycomPhone) device;
            return new ApplicationConfiguration(phone);
        }
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
            PolycomPhone phone = (PolycomPhone) device;
            return new PhoneConfiguration(phone);
        }
    }

    static class SipProfile extends Profile {
        public SipProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            PolycomPhone phone = (PolycomPhone) device;
            return new SipConfiguration(phone);
        }
    }

    static class DeviceProfile extends Profile {
        public DeviceProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            PolycomPhone phone = (PolycomPhone) device;
            return new DeviceConfiguration(phone);
        }
    }

    static class DirectoryProfile extends Profile {
        public DirectoryProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            Phone phone = (Phone) device;
            PhoneContext phoneContext = phone.getPhoneContext();
            Collection<PhonebookEntry> entries = phoneContext.getPhonebookEntries(phone);
            SpeedDial speedDial = phoneContext.getSpeedDial(phone);
            return new DirectoryConfiguration(entries, speedDial);
        }
    }
}

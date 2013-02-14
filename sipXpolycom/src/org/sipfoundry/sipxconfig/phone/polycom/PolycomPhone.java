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

import static java.lang.String.format;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.SAXReader;
import org.dom4j.io.XMLWriter;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.cert.CertificateManager;
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
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.setting.DelegatingSettingModel.InsertValueFilter;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlEscapeValueFilter;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.springframework.beans.BeansException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;

/**
 * Support for Polycom 300, 400, and 500 series phones and model 3000 conference phone
 */
public class PolycomPhone extends Phone implements BeanFactoryAware {
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
    static final String TEMPLATE_DIR40 = "polycom/mac-address.d.40";
    static final String TEMPLATE_DIR41 = "polycom/mac-address.d.41";
    static final String TEMPLATE_DIR32 = "polycom/mac-address.d.32";
    static final String MB_PROXY = "mb/proxy";
    static final String MB_IDLE_DISPLAY_HOME_PAGE = "mb/idleDisplay/home";
    static final String MB_IDLE_DISPLAY_REFRESH = "mb/idleDisplay/refresh";
    static final String MB_MAIN_HOME_PAGE = "mb/main/home";
    static final String MB_MAIN_HOME_IDLE = "mb/main/idleTimeout";
    static final String MB_MAIN_HOME_STATUSBAR = "mb/main/statusbar";
    static final String MB_LIMITS_NODES = "mb/limits/nodes";
    static final String MB_LIMITS_CACHE = "mb/limits/cache";
    static final String POLY_300 = "polycom300";
    static final String POLY_301 = "polycom301";
    static final String POLY_500 = "polycom500";
    static final String POLY_501 = "polycom501";
    static final String POLY_600 = "polycom600";
    static final String POLY_601 = "polycom601";
    static final String POLY_550 = "polycom550";
    static final String POLY_560 = "polycom560";
    static final String POLY_320 = "polycom320";
    static final String POLY_330 = "polycom330";
    static final String POLY_650 = "polycom650";
    static final String POLY_670 = "polycom670";
    static final String PHONE_XML = "phone.xml";
    static final String LINE_XML = "line.xml";
    static final String[] UNSUPPORTED_MODELS = new String[] {
        POLY_300, POLY_500
    };

    private AddressManager m_addressManager;
    private BeanFactory m_beanFactory;
    private CertificateManager m_certificateManager;

    public void setCertificateManager(CertificateManager certificateManager) {
        m_certificateManager = certificateManager;
    }

    public String getDefaultVersionId() {
        DeviceVersion version = getDeviceVersion();
        return version != null ? version.getVersionId() : null;
    }

    public String getTemplateDir() {
        if (getDeviceVersion() == PolycomModel.VER_4_0_X) {
            return TEMPLATE_DIR40;
        } else if (getDeviceVersion() == PolycomModel.VER_3_2_X) {
            return TEMPLATE_DIR32;
        } else if (getDeviceVersion() == PolycomModel.VER_4_1_X) {
            return TEMPLATE_DIR41;
        }
        return TEMPLATE_DIR;
    }

    /*
     * make use of getApplicationFilename?
     */
    public String getAppFile() {
        if (getDeviceVersion() == PolycomModel.VER_4_0_X) {
            return "/mac-address-40.cfg.vm";
        } else if (getDeviceVersion() == PolycomModel.VER_3_2_X) {
            return "/mac-address-32.cfg.vm";
        } else if (getDeviceVersion() == PolycomModel.VER_3_1_X) {
            return "/mac-address-31.cfg.vm";
        } else if (getDeviceVersion() == PolycomModel.VER_4_1_X) {
            return "/mac-address-41.cfg.vm";
        }
        return "/mac-address.cfg.vm";
    }

    /**
     * Default firmware version for polycom phones. Default is 1.6 right now
     *
     * @param defaultVersionId 1.6 or 2.0
     */
    @Override
    public void setDeviceVersion(DeviceVersion version) {
        super.setDeviceVersion(version);
        DeviceVersion myVersion = getDeviceVersion();
        if (myVersion == PolycomModel.VER_4_0_X) {
            getModel().setSettingsFile("phone-40.xml");
            getModel().setLineSettingsFile("line-40.xml");
            getModel().setStaticProfileFilenames(new String[] {});
        } else if (myVersion == PolycomModel.VER_4_1_X) {
            getModel().setSettingsFile("phone-41.xml");
            getModel().setLineSettingsFile("line-41.xml");
            getModel().setStaticProfileFilenames(new String[] {});
        } else if (myVersion == PolycomModel.VER_3_1_X) {
            getModel().setSettingsFile(PHONE_XML);
            getModel().setLineSettingsFile(LINE_XML);
            getModel().setStaticProfileFilenames(new String[] {
                "polycom_phone1_3.1.X.cfg", "polycom_sip_3.1.X.cfg"
            });
        } else if (myVersion == PolycomModel.VER_3_2_X) {
            // we need to explicitly define these here otherwise changing versions will not work
            getModel().setSettingsFile("phone-32.xml");
            getModel().setLineSettingsFile("line-32.xml");
            getModel().setStaticProfileFilenames(new String[] {
                "polycom_phone1.cfg", "polycom_sip.cfg"
            });
        } else {
            // we need to explicitly define these here otherwise changing versions will not work
            getModel().setSettingsFile(PHONE_XML);
            getModel().setLineSettingsFile(LINE_XML);
            getModel().setStaticProfileFilenames(new String[] {
                "polycom_phone1_2.1.X.cfg", "polycom_sip_2.1.X.cfg"
            });
        }
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
        Profile[] profileTypes;
        if (getDeviceVersion() == PolycomModel.VER_4_0_X || getDeviceVersion() == PolycomModel.VER_4_1_X) {
            profileTypes = new Profile[] {
                new ApplicationProfile(getAppFilename()), new ApplicationsProfile(getAppsFilename()),
                new FeaturesProfile(getFeaturesFilename()), new RegAdvancedProfile(getRegAdvancedFilename()),
                new RegionProfile(getRegionFilename()), new SipBasicProfile(getSipBasicFilename()),
                new SipInteropProfile(getSipInteropFilename()), new SiteProfile(getSiteFilename()),
                new VideoProfile(getVideoFilename())
            };
        } else {
            profileTypes = new Profile[] {
                new ApplicationProfile(getAppFilename()), new SipProfile(getSipFilename()),
                new PhoneProfile(getPhoneFilename()), new DeviceProfile(getDeviceFilename())
            };
        }

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

    public String getAppsFilename() {
        return format("%s-sipx-applications.cfg", getProfileFilename());
    }

    public String getSipFilename() {
        return format("%s-sipx-sip.cfg", getProfileFilename());
    }

    public String getSipInteropFilename() {
        return format("%s-sipx-sip-interop.cfg", getProfileFilename());
    }

    public String getSipBasicFilename() {
        return format("%s-sipx-sip-basic.cfg", getProfileFilename());
    }

    public String getFeaturesFilename() {
        return format("%s-sipx-features.cfg", getProfileFilename());
    }

    public String getRegAdvancedFilename() {
        return format("%s-sipx-reg-advanced.cfg", getProfileFilename());
    }

    public String getRegBasicFilename() {
        return format("%s-sipx-reg-basic.cfg", getProfileFilename());
    }

    public String getRegionFilename() {
        return format("%s-sipx-region.cfg", getProfileFilename());
    }

    public String getSiteFilename() {
        return format("%s-sipx-site.cfg", getProfileFilename());
    }

    public String getVideoFilename() {
        return format("%s-sipx-video.cfg", getProfileFilename());
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

    private class ApplicationProfile extends Profile {
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
            String addrFormat = "http://%s:%d/";
            List<Address> addresses = m_addressManager.getAddresses(new AddressType("provisionService", addrFormat));
            if (!addresses.isEmpty()) {
                return new ApplicationConfiguration(phone, String.format(addrFormat, addresses.get(0).getAddress(),
                        addresses.get(0).getPort()));
            }
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

    static class SipInteropProfile extends Profile {
        public SipInteropProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            PolycomPhone phone = (PolycomPhone) device;
            return new SipInteropConfiguration(phone);
        }
    }

    static class SipBasicProfile extends Profile {
        public SipBasicProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            PolycomPhone phone = (PolycomPhone) device;
            return new SipBasicConfiguration(phone);
        }
    }

    static class ApplicationsProfile extends Profile {
        public ApplicationsProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            PolycomPhone phone = (PolycomPhone) device;
            return new ApplicationsConfiguration(phone);
        }
    }

    static class RegAdvancedProfile extends Profile {
        public RegAdvancedProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            PolycomPhone phone = (PolycomPhone) device;
            return new RegAdvancedConfiguration(phone);
        }
    }

    class SiteProfile extends Profile {
        public SiteProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            PolycomPhone phone = (PolycomPhone) device;
            return new SiteConfiguration(phone, m_certificateManager);
        }
    }

    static class VideoProfile extends Profile {
        public VideoProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            PolycomPhone phone = (PolycomPhone) device;
            return new VideoConfiguration(phone);
        }
    }

    static class RegionProfile extends Profile {
        public RegionProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            PolycomPhone phone = (PolycomPhone) device;
            return new RegionConfiguration(phone);
        }
    }

    static class FeaturesProfile extends Profile {
        public FeaturesProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            PolycomPhone phone = (PolycomPhone) device;
            return new FeaturesConfiguration(phone);
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

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    @Override
    public Collection< ? extends PhoneModel> getModelIdsForSelection(String beanId) {
        PolycomModel p300 = m_beanFactory.getBean(POLY_300, PolycomModel.class);
        PolycomModel p301 = m_beanFactory.getBean(POLY_301, PolycomModel.class);
        PolycomModel p500 = m_beanFactory.getBean(POLY_500, PolycomModel.class);
        PolycomModel p501 = m_beanFactory.getBean(POLY_501, PolycomModel.class);
        PolycomModel p600 = m_beanFactory.getBean(POLY_600, PolycomModel.class);
        PolycomModel p601 = m_beanFactory.getBean(POLY_601, PolycomModel.class);
        PolycomModel p550 = m_beanFactory.getBean(POLY_550, PolycomModel.class);
        PolycomModel p560 = m_beanFactory.getBean(POLY_560, PolycomModel.class);
        PolycomModel p330 = m_beanFactory.getBean(POLY_330, PolycomModel.class);
        PolycomModel p320 = m_beanFactory.getBean(POLY_320, PolycomModel.class);
        PolycomModel p650 = m_beanFactory.getBean(POLY_650, PolycomModel.class);
        PolycomModel p670 = m_beanFactory.getBean(POLY_670, PolycomModel.class);

        if (isModel(beanId, POLY_300, POLY_301)) {
            return Arrays.asList(new PolycomModel[] {
                p300, p301
            });
        }
        if (isModel(beanId, POLY_500, POLY_501)) {
            return Arrays.asList(new PolycomModel[] {
                p500, p501
            });
        }
        if (isModel(beanId, POLY_600, POLY_601)) {
            return Arrays.asList(new PolycomModel[] {
                p600, p601
            });
        }
        if (isModel(beanId, POLY_550, POLY_560)) {
            return Arrays.asList(new PolycomModel[] {
                p550, p560
            });
        }
        if (isModel(beanId, POLY_320, POLY_330)) {
            return Arrays.asList(new PolycomModel[] {
                p320, p330
            });
        }
        if (isModel(beanId, POLY_650, POLY_670)) {
            return Arrays.asList(new PolycomModel[] {
                p650, p670
            });
        }
        return null;
    }

    private boolean isModel(String modelId, String... models) {
        for (String model : models) {
            if (StringUtils.equals(modelId, model)) {
                return true;
            }
        }
        return false;
    }
    @Override
    public void setBeanFactory(BeanFactory beanFactory) throws BeansException {
        m_beanFactory = beanFactory;
    }
}

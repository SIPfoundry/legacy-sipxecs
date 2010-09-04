/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.nortel;

import java.text.MessageFormat;
import java.util.Collection;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileFilter;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;


public class NortelPhone extends Phone {

    private static final String NORTEL_FORCE_CONFIG = "nortel/11xxeSIP.cfg";
    private static final String REL_3_2_OR_LATER = "3.2orLater";
    private static final String TYPE = "text/plain";
    private String m_phonebookFilename = "{0}-phonebook.ab";
    private String m_featureKeyListFilename = "{0}-fkl.fk";
    private CoreContext m_coreContext;

    public NortelPhone() {

    }
    /**
     * sets the coreContext , whicn provides the service layer access
     * to the user objects
     * @param coreContext
     */

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Override
    public void initializeLine(Line line) {
        DeviceDefaults phoneDefaults = getPhoneContext().getPhoneDefaults();
        NortelPhoneDefaults defaults = new NortelPhoneDefaults(phoneDefaults, line);
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initialize() {
        DeviceDefaults phoneDefaults = getPhoneContext().getPhoneDefaults();
        Line line = new Line();
        NortelPhoneDefaults defaults = new NortelPhoneDefaults(phoneDefaults, line);
        addDefaultBeanSettingHandler(defaults);
    }
    /**
     * (add profiles to be generated during the profile generation processs)
     * @see org.sipfoundry.sipxconfig.device.Device#getProfileTypes()
     */
    @Override
    public Profile[] getProfileTypes() {
        Profile[] profileTypes;
        PhonebookManager phonebookManager = getPhonebookManager();
        if (phonebookManager.getPhonebookManagementEnabled()) {
            /* adds phone profile (or MAC file), phonebook profile and featurekeyList profile in the profiles array. */
            profileTypes = new Profile[] {
                new Profile(this),
                new PhonebookProfile(getPhonebookFilename()),
                new FeatureKeyListProfile(getFeatureKeyListFilename())
            };
        } else {
            profileTypes = new Profile[] {
                new Profile(this)
            };
        }

        return profileTypes;
    }

    @Override
    public void setDeviceVersion(DeviceVersion version) {
        super.setDeviceVersion(version);
        DeviceVersion myVersion = getDeviceVersion();

        if (myVersion == NortelPhoneModel.FIRM_3_2) {
            myVersion.addSupportedFeature(REL_3_2_OR_LATER);
        }
    }

    /**
     * Gets the phonebook entries from the phoneContext
     * and creates a ProfileContext by creating a new nortelPhonebook
     * @return ProfileContext for Phonebook by creating a new object for NortelPhonebook
     */

    public ProfileContext getPhonebook() {
        Collection<PhonebookEntry> entries = getPhoneContext().getPhonebookEntries(this);
        return new NortelPhonebook(entries, m_coreContext);
    }
    /**
     * @return ProfileContext for FeatureKeyList
     */
    public ProfileContext getFeatureKeyList() {
        SpeedDial speedDial = getPhoneContext().getSpeedDial(this);
        return new NortelFeatureKeyList(speedDial);
    }

    @Override
    public void removeProfiles(ProfileLocation location) {
        super.removeProfiles(location);
        location.removeProfile(getPhonebookFilename());
    }
    /**
     * Creates a Phonebook Profile
     * NortelPhone intends to generate a Phonbook profile thats why
     * a PhonebookProfile should extend Profile class which
     * encapsulates the details of creating a profile/configuration file.
     */
    static class PhonebookProfile extends Profile {
        public PhonebookProfile(String name) {
            super(name, TYPE);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }
        /**
         * @param Device which is a nortel phone
         * @return ProfileContext for phonebook by calling getPhonebook
         * which actually creates a ProfileContext
         */
        @Override
        protected ProfileContext createContext(Device device) {
            NortelPhone phone = (NortelPhone) device;
            return phone.getPhonebook();
        }
    }

    public void setPhonebookFilename(String phonebookFilename) {
        m_phonebookFilename = phonebookFilename;
    }

    public String getPhonebookFilename() {
        return MessageFormat.format(m_phonebookFilename, getProfileFilename());
    }
    /**
     * Creates a FeatureKeyList Profile
     * NortelPhone intends to generate a FeatureKeyList profile thats why
     * a FeatureKeyListProfile should extend Profile class which
     * encapsulates the details of creating a profile/configuration file.
     */
    static class FeatureKeyListProfile extends Profile {
        public FeatureKeyListProfile(String name) {
            super(name, TYPE);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }
        /**
         * @param Device which is a nortel phone
         * @return ProfileContext for FeatureKeyList by calling getFeatureKeyList
         * which actually creates a ProfileContext
         */
        @Override
        protected ProfileContext createContext(Device device) {
            NortelPhone phone = (NortelPhone) device;
            return phone.getFeatureKeyList();
        }
    }

    public void setFeatureKeyListFileName(String featureKeyListFilename) {
        m_featureKeyListFilename = featureKeyListFilename;
    }

    public String getFeatureKeyListFilename() {
        return MessageFormat.format(m_featureKeyListFilename, getProfileFilename());
    }

    @Override
    public String getProfileFilename() {
        return "SIP" + getSerialNumber().toUpperCase() + ".cfg";
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = NortelPhoneDefaults.getLineInfo(line);
        return lineInfo;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {

        NortelPhoneDefaults.setLineInfo(line, lineInfo);
    }

    @Override
    public void restart() {
        sendCheckSyncToFirstLine();
    }

    @Override
    protected void copyFiles(ProfileLocation location) {
        super.copyFiles(location);
        getProfileGenerator().copy(location, NORTEL_FORCE_CONFIG, "1120eSIP.cfg");
        getProfileGenerator().copy(location, NORTEL_FORCE_CONFIG, "1140eSIP.cfg");
    }

    public static class NortelPhoneDefaults {

        private static final String NETWORK_DNS_DOMAIN = "NETWORK/DNS_DOMAIN";
        private static final String VOIP_SIP_DOMAIN1 = "VOIP/SIP_DOMAIN1";
        private static final String VOIP_SERVER_IP1_1 = "VOIP/SERVER_IP1_1";
        private static final String VOIP_SERVER_PORT1_1 = "VOIP/SERVER_PORT1_1";
        private static final String VOIP_SERVER_IP1_2 = "VOIP/SERVER_IP1_2";
        private static final String VOIP_DEF_USER1 = "VOIP/DEF_USER1";
        private static final String VM_VMAIL = "VM/VMAIL";
        private static final String AUTOLOGIN_ID_KEY = "AUTOLOGIN/AUTOLOGIN_ID_KEY";
        private static final String AUTOLOGIN_ID_KEY_PASSWD = "AUTOLOGIN/AUTOLOGIN_PASSWD_KEY";

        private final Line m_line;
        private final DeviceDefaults m_defaults;

        NortelPhoneDefaults(DeviceDefaults defaults, Line line) {
            m_line = line;
            m_defaults = defaults;
        }

        @SettingEntry(paths = { NETWORK_DNS_DOMAIN, VOIP_SIP_DOMAIN1 })
        public String getSipDomain1() {
            return m_defaults.getDomainName();
        }

        @SettingEntry(paths = { VOIP_SERVER_IP1_1, VOIP_SERVER_IP1_2 })
        public String getServerIp() {
            return m_defaults.getProxyServerAddr();
        }

        @SettingEntry(path = VOIP_SERVER_PORT1_1)
        public String getProxyPort() {
            return m_defaults.getProxyServerSipPort();
        }

        @SettingEntry(path = VOIP_DEF_USER1)
        public String getUserName() {
            String userName = null;
            User user = m_line.getUser();
            if (user != null) {
                userName = user.getUserName();
            }
            return userName;
        }

        @SettingEntry(path = AUTOLOGIN_ID_KEY)
        public String getUserNameAutologin() {
            String userName = null;
            User user = m_line.getUser();
            if (user != null) {
                userName = user.getUserName();
            }
            return userName;
        }

        @SettingEntry(path = AUTOLOGIN_ID_KEY_PASSWD)
        public String getPasswordAutologin() {
            String passwd = null;
            User user = m_line.getUser();
            if (user != null) {
                passwd = user.getSipPassword();
            }
            return passwd;
        }

        @SettingEntry(path = VM_VMAIL)
        public String getVoicemailExtension() {
            return m_defaults.getVoiceMail();
        }

        public static LineInfo getLineInfo(Line line) {
            LineInfo lineInfo = new LineInfo();
            lineInfo.setUserId(line.getSettingValue(VOIP_DEF_USER1));
            lineInfo.setRegistrationServer(line.getSettingValue(VOIP_SERVER_IP1_1));
            lineInfo.setRegistrationServerPort(line.getSettingValue(VOIP_SERVER_PORT1_1));
            return lineInfo;
        }

        public static void setLineInfo(Line line, LineInfo lineInfo) {
            line.setSettingValue(VOIP_DEF_USER1, lineInfo.getUserId());
            line.setSettingValue(VOIP_SERVER_IP1_1, lineInfo.getRegistrationServer());
            line.setSettingValue(VOIP_SERVER_PORT1_1, lineInfo.getRegistrationServerPort());
        }
    }
}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phone.counterpath;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import static java.lang.String.format;

import org.apache.commons.codec.digest.DigestUtils;
import org.apache.commons.lang.ArrayUtils;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileFilter;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class CounterpathPhone extends Phone {

    public static final String MIME_TYPE_PLAIN = "text/plain";

    private static final String REG_USERNAME = "registration/username";
    private static final String REG_AUTH_USERNAME = "registration/authorization_username";
    private static final String REG_DISPLAY_NAME = "registration/display_name";
    private static final String REG_PASSWORD = "registration/password";
    private static final String REG_DOMAIN = "registration/domain";
    private static final String SUBSCRIPTION_AOR =
        "network/sip_signaling/proxies:proxy0:workgroup_subscription_aor";
    private static final String RESOURCE_LISTS_PATH =
        "system/resources/system:contact_list_storage:resource_lists_path";
    private static final String RESOURCE_LISTS_FILENAME =
        "system/resources/system:contact_list_storage:contacts_server_filename";
    private static final String RESOURCE_LISTS_USER_NAME =
        "system/resources/system:contact_list_storage:resource_lists_user_name";
    private static final String RESOURCE_LISTS_PASSWORD =
        "system/resources/system:contact_list_storage:resource_lists_password";
    private static final String VOICEMAIL_URL = "voicemail/voicemail_url";
    private static final String WEBDAV_REALM = ":WebDAV:";
    private static final String AUTH_FILE_NAME = "webdav.users.passwd";
    private static final String WEBDAV_DIR_NAME = "/webdav/";
    private static final String DIRECTORY_FILE_FORMAT = "%s-directory.xml";
    private static final String NEW_LINE = "\n";
    private String m_syswwwdir;

    // private static final String WEBDAV_URL = "resources/resource_lists_path";
    public CounterpathPhone() {
    }

    public String getSyswwwdir() {
        return m_syswwwdir;
    }

    public void setSyswwwdir(String syswwwdir) {
        m_syswwwdir = syswwwdir;
    }

    @Override
    public Profile[] getProfileTypes() {
        Profile[] profileTypes = new Profile[] {
            new PhoneProfile(getPhoneFilename())
        };

        if (getPhonebookManager().getPhonebookManagementEnabled()) {
            profileTypes = (Profile[]) ArrayUtils.add(profileTypes, new DirectoryProfile(getDirectoryFilename()));
        }

        return profileTypes;
    }

    @Override
    public String getProfileFilename() {
        return getSerialNumber() + ".ini";
    }

    @Override
    protected void beforeProfileGeneration() {
        updateWebDAVUserAuthFile();
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new CounterpathPhoneDefaults(this));

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
        private final Phone m_phone;

        public CounterpathPhoneDefaults(Phone phone) {
            m_phone = phone;
        }

        @SettingEntry(path = SUBSCRIPTION_AOR)
        public String getWorkgroupSubscriptionAor() {
            SpeedDial speedDial = getPhoneContext().getSpeedDial(m_phone);
            if (speedDial == null) {
                return null;
            }
            String domain = getPhoneContext().getPhoneDefaults().getDomainName();
            return SipUri.format(speedDial.getResourceListId(true), domain, false);
        }

        @SettingEntry(path = RESOURCE_LISTS_PATH)
        public String geResourceListsPath() {
            return "http://" + getPhoneContext().getPhoneDefaults().getDomainName() + "/webdav";
        }

        @SettingEntry(path = RESOURCE_LISTS_FILENAME)
        public String geResourceListsFilename() {
            return format(DIRECTORY_FILE_FORMAT, getSerialNumber());
        }

        @SettingEntry(path = RESOURCE_LISTS_USER_NAME)
        public String geResourceListsUserName() {
            return m_phone.getLine(0).getUser().getUserName();
        }

        @SettingEntry(path = RESOURCE_LISTS_PASSWORD)
        public String geResourceListsPassword() {
            return m_phone.getLine(0).getUser().getSipPassword();
        }

    }

    public static class CounterpathLineDefaults {
        private final Line m_line;
        private final User m_user;
        private final ImAccount m_imAccount;

        public CounterpathLineDefaults(Line line) {
            m_line = line;
            m_user = m_line.getUser();
            if (m_user != null) {
                m_imAccount = new ImAccount(m_user);
            } else {
                m_imAccount = null;
            }
        }

        @SettingEntry(path = REG_USERNAME)
        public String getUserName() {
            return m_line.getUserName();
        }

        @SettingEntry(path = REG_AUTH_USERNAME)
        public String getAuthenticationUserName() {
            return m_line.getAuthenticationUserName();
        }


        @SettingEntry(path = "xmpp-config/enabled")
        public boolean isEnabled() {
            if (m_imAccount == null) {
                return false;
            }
            return m_imAccount.isEnabled();
        }

        @SettingEntry(path = "xmpp-config/username")
        public String getImId() {
            if (m_imAccount == null) {
                return null;
            }
            return m_imAccount.getImId();
        }

        @SettingEntry(path = REG_DISPLAY_NAME)
        public String getDisplayName() {
            if (m_user == null) {
                return null;
            }
            return m_user.getDisplayName();
        }

        @SettingEntry(path = "xmpp-config/account_name")
        public String getImDisplayName() {
            if (m_imAccount == null) {
                return null;
            }
            return m_imAccount.getImDisplayName();
        }

        @SettingEntry(path = REG_PASSWORD)
        public String getPassword() {
            if (m_user == null) {
                return null;
            }
            return m_user.getSipPassword();
        }

        @SettingEntry(path = "xmpp-config/password")
        public String getImPassword() {
            if (m_imAccount == null) {
                return null;
            }
            return m_imAccount.getImPassword();
        }

        @SettingEntry(paths = { REG_DOMAIN, "xmpp-config/domain" })
        public String getDomain() {
            DeviceDefaults defaults = m_line.getPhoneContext().getPhoneDefaults();
            return defaults.getDomainName();
        }

        @SettingEntry(path = VOICEMAIL_URL)
        public String getVoicemailURL() {
            return m_line.getPhoneContext().getPhoneDefaults().getVoiceMail();
        }
    }

    public String getPhoneFilename() {
        return getProfileFilename();
    }

    public String getDirectoryFilename() {
        return format(DIRECTORY_FILE_FORMAT, getSerialNumber());
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
            return new DirectoryConfiguration(entries, phoneContext.getPhoneDefaults().getDomainName());
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
            CounterpathPhone phone = (CounterpathPhone) device;
            return new CounterpathProfileContext(phone, phone.getModel().getProfileTemplate());
        }
    }

    private void updateWebDAVUserAuthFile() {
        try {
            User user = this.getLine(0).getUser();
            String passwordFileNamePath = getSyswwwdir() + WEBDAV_DIR_NAME + AUTH_FILE_NAME;
            File passwordFile = new File(passwordFileNamePath);
            String md5text = DigestUtils.md5Hex(user.getUserName() + WEBDAV_REALM + user.getSipPassword());

            if (passwordFile.exists()) {
                Boolean updated = false;

                Map<String, String> authDB = new HashMap<String, String>();
                BufferedReader br = new BufferedReader(new InputStreamReader(new FileInputStream(passwordFile)));

                String strLine;
                String[] tokens = new String[3];

                while ((strLine = br.readLine()) != null) {
                    tokens = strLine.split(":");
                    if ((tokens[0].equals(user.getUserName())) && (!tokens[2].equals(md5text))) {
                        tokens[2] = md5text;
                        updated = true;
                    }
                    authDB.put(tokens[0], tokens[2]);
                }

                // Close the input stream
                br.close();

                if (updated) {

                    passwordFile.delete();

                    BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(
                            passwordFileNamePath)));
                    for (Map.Entry<String, String> entry : authDB.entrySet()) {
                        bw.write(entry.getKey() + WEBDAV_REALM + entry.getValue() + NEW_LINE);
                    }
                    bw.close();
                }
            } else {
                BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(passwordFile)));
                bw.write(user.getUserName() + WEBDAV_REALM + md5text + NEW_LINE);
                bw.close();
            }

        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }

}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.ipdialog;

import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class IpDialogPhone extends Phone {

    public static final PhoneModel MODEL = new PhoneModel("ipDialog", "ipDialog SipTone V");
    public static final String BEAN_ID = "ipDialogPhoneStandard";
    public static final String USER_ID_SETTING = "registrationAndProxy/authname";
    public static final String PASSWORD_SETTING = "registrationAndProxy/authPassword";
    public static final String DISPLAY_NAME = "registrationAndProxy/displayname";
    public static final String REGISTRATION_SERVER_SETTING = "registrationAndProxy/registrarAddress";
    public static final String PROXY_SERVER_SETTING = "registrationAndProxy/proxyAddress";
    public static final String MWI_SUBSCRIBE_SETTING = "registrationAndProxy/mwiReqUri";
    public static final String MOH_SETTING = "registrationAndProxy/mohServer";
    public static final String VOICEMAIL_ACCESS_NUMBER_SETTING = "registrationAndProxy/voiceMailServerAddress";
    public static final String PRESENCE_SERVER_SETTING = "registrationAndProxy/presenceServer";
    public static final String RLS_SETTING = "presence/sipRlsUri";

    private static final int PHONEBOOK_MAX = 10;
    private static final int SPEEDDIAL_MAX = 16;

    public IpDialogPhone() {
    }

    @Override
    public void initialize() {
        SpeedDial speedDial = getPhoneContext().getSpeedDial(this);
        IpDialogPhoneDefaults defaults = new IpDialogPhoneDefaults(getPhoneContext().getPhoneDefaults(), speedDial);
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initializeLine(Line line) {
        line.addDefaultBeanSettingHandler(new IpDialogLineDefaults(line));
    }

    @Override
    protected ProfileContext createContext() {
        SpeedDial speedDial = getPhoneContext().getSpeedDial(this);
        Collection<PhonebookEntry> phoneBook = getPhoneContext().getPhonebookEntries(this);
        return new IpDialogContext(this, speedDial, phoneBook, getModel().getProfileTemplate());
    }

    @Override
    protected void setLineInfo(Line line, LineInfo info) {
        line.setSettingValue(USER_ID_SETTING, info.getUserId());
        line.setSettingValue(PASSWORD_SETTING, info.getPassword());
        line.setSettingValue(REGISTRATION_SERVER_SETTING, info.getRegistrationServer());
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo info = new LineInfo();
        info.setUserId(line.getSettingValue(USER_ID_SETTING));
        info.setPassword(line.getSettingValue(PASSWORD_SETTING));
        info.setRegistrationServer(line.getSettingValue(REGISTRATION_SERVER_SETTING));
        return info;
    }

    @Override
    public String getProfileFilename() {
        return "SipTone/config/" + "ipdSIP" + getSerialNumber().toUpperCase() + ".xml";

    }

    public static class IpDialogPhoneDefaults {

        private final DeviceDefaults m_defaults;
        private final SpeedDial m_speedDial;

        IpDialogPhoneDefaults(DeviceDefaults defaults, SpeedDial speedDial) {
            m_defaults = defaults;
            m_speedDial = speedDial;
        }

        @SettingEntry(path = RLS_SETTING)
        public String getRLSUri() {

            if (m_speedDial != null && m_speedDial.isBlf()) {
                return SipUri.format(m_speedDial.getResourceListId(true), m_defaults.getDomainName(), false);
            }
            return null;
        }

    }

    public static class IpDialogLineDefaults {
        private final Line m_line;

        IpDialogLineDefaults(Line line) {
            m_line = line;
        }

        @SettingEntry(path = USER_ID_SETTING)
        public String getUserName() {
            String userName = null;
            User user = m_line.getUser();
            if (user != null) {
                userName = user.getUserName();
            }
            return userName;
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

        @SettingEntry(path = DISPLAY_NAME)
        public String getDisplayName() {
            String displayName = null;
            User user = m_line.getUser();
            if (user != null) {
                displayName = user.getDisplayName();
                if (displayName != null) {
                    if (displayName.length() > 22) {
                        displayName = displayName.substring(0, 22);
                    }
                }
            }

            return displayName;
        }

        @SettingEntry(path = REGISTRATION_SERVER_SETTING)
        public String getRegistrationServer() {
            DeviceDefaults defaults = m_line.getPhoneContext().getPhoneDefaults();
            return defaults.getDomainName();
        }

        @SettingEntry(path = PROXY_SERVER_SETTING)
        public String getProxyAddress() {
            DeviceDefaults defaults = m_line.getPhoneContext().getPhoneDefaults();
            return defaults.getDomainName();
        }

        @SettingEntry(path = MWI_SUBSCRIBE_SETTING)
        public String getMwiSubscribe() {
            DeviceDefaults defaults = m_line.getPhoneContext().getPhoneDefaults();
            String uri = null;
            User u = m_line.getUser();
            if (u != null) {
                uri = u.getUserName() + '@' + defaults.getDomainName();
            }
            return uri;
        }

        @SettingEntry(path = VOICEMAIL_ACCESS_NUMBER_SETTING)
        public String getVoiceMailNumber() {
            DeviceDefaults defaults = m_line.getPhoneContext().getPhoneDefaults();
            String uri = null;
            User u = m_line.getUser();
            if (u != null) {
                uri = defaults.getVoiceMail() + '@' + defaults.getDomainName();
            }
            return uri;
        }

        @SettingEntry(path = MOH_SETTING)
        public String getUserMoh() {
            User u = m_line.getUser();
            if (u != null) {
                return u.getMusicOnHoldUri();
            }

            DeviceDefaults defaults = m_line.getPhoneContext().getPhoneDefaults();
            return defaults.getMusicOnHoldUri();
        }

        @SettingEntry(path = PRESENCE_SERVER_SETTING)
        public String getPresenceServer() {
            DeviceDefaults defaults = m_line.getPhoneContext().getPhoneDefaults();
            return defaults.getDomainName();
        }
    }

    @Override
    public void restart() {
        sendCheckSyncToFirstLine();
    }

    static class IpDialogContext extends ProfileContext {
        private final SpeedDial m_speedDial;
        private final Collection<PhonebookEntry> m_phoneBook;

        public IpDialogContext(IpDialogPhone device, SpeedDial speedDial, Collection<PhonebookEntry> phoneBook,
                String profileTemplate) {
            super(device, profileTemplate);
            m_speedDial = speedDial;
            m_phoneBook = trim(phoneBook);
        }

        @Override
        public Map<String, Object> getContext() {
            Map<String, Object> context = super.getContext();
            context.put("speedDialNumber", getNumbers());
            context.put("speedDialNames", getNames());
            context.put("speedDialPresence", getPresence());
            context.put("phoneBook", m_phoneBook);
            return context;
        }

        Collection<PhonebookEntry> trim(Collection<PhonebookEntry> phoneBook) {
            if (phoneBook == null) {
                return Collections.emptyList();
            }
            if (phoneBook.size() <= PHONEBOOK_MAX) {
                return phoneBook;
            }
            Predicate keepOnlyN = new Predicate() {
                private int m_i;

                public boolean evaluate(Object arg0) {
                    return m_i++ < PHONEBOOK_MAX;
                }
            };
            CollectionUtils.filter(phoneBook, keepOnlyN);
            return phoneBook;
        }

        String[] getNumbers() {
            if (m_speedDial == null) {
                return ArrayUtils.EMPTY_STRING_ARRAY;
            }
            List<Button> buttons = m_speedDial.getButtons();
            int size = Math.min(SPEEDDIAL_MAX, buttons.size());
            String[] numbers = new String[size];
            for (int i = 0; i < numbers.length; i++) {
                numbers[i] = buttons.get(i).getNumber();
            }
            return numbers;
        }

        String[] getNames() {
            if (m_speedDial == null) {
                return ArrayUtils.EMPTY_STRING_ARRAY;
            }
            List<Button> buttons = m_speedDial.getButtons();
            int size = Math.min(SPEEDDIAL_MAX, buttons.size());
            String[] numbers = new String[size];
            for (int i = 0; i < numbers.length; i++) {
                String memName = buttons.get(i).getLabel();
                numbers[i] = StringUtils.substring(memName, 0, 15);
            }
            return numbers;
        }

        String[] getPresence() {
            if (m_speedDial == null) {
                return ArrayUtils.EMPTY_STRING_ARRAY;
            }
            List<Button> buttons = m_speedDial.getButtons();
            int size = Math.min(SPEEDDIAL_MAX, buttons.size());
            String[] numbers = new String[size];
            for (int i = 0; i < numbers.length; i++) {
                if (buttons.get(i).isBlf()) {
                    numbers[i] = "1";
                } else {
                    numbers[i] = "0";
                }
            }
            return numbers;
        }

    }
}

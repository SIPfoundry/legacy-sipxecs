/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.nortel12x0;

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

public class Nortel12x0Phone extends Phone {

    public static final PhoneModel MODEL = new PhoneModel("nortel12x0", "Nortel IP Phone 1230");
    public static final String BEAN_ID = "nortel12x0PhoneStandard";
    public static final String USER_ID_SETTING = "registrationAndProxy/authname";
    public static final String PASSWORD_SETTING = "registrationAndProxy/authPassword";
    public static final String DISPLAY_NAME = "registrationAndProxy/displayname";
    public static final String REGISTRATION_SERVER_SETTING = "registrationAndProxy/registrarAddress";
    public static final String PROXY_SERVER_SETTING = "sipSettings/sipProxy/proxyAddress";
    public static final String MWI_SUBSCRIBE_SETTING = "registrationAndProxy/mwiReqUri";
    public static final String MOH_SETTING = "phoneSettings/mohServer";
    public static final String VOICEMAIL_ACCESS_NUMBER_SETTING = "registrationAndProxy/voiceMailServerAddress";
    public static final String PRESENCE_SERVER_SETTING = "phoneSettings/presenceServer";
    public static final String RLS_SETTING = "presence/sipRlsUri";
    public static final String TIME_SERVER_NAME = "timesettings/ntpIp";
    public static final String ALTERNATE_TIME_SERVER_NAME = "timesettings/ntpIp2";
    public static final String CALL_PICKUP_PREFIX = "callPickup/callPickupPrefix";
    public static final String PAGING_PREFIX = "phoneSettings/groupPagingPrefix";

    private static final int PHONEBOOK_MAX = 200;
    private static final int SPEEDDIAL_MAX = 200;
    private static final int CONFIGURATION_PARAMS_PER_SPEEDDIAL_ENTRY = 3;

    public Nortel12x0Phone() {
    }

    @Override
    public void initialize() {
        SpeedDial speedDial = getPhoneContext().getSpeedDial(this);
        Nortel12x0PhoneDefaults defaults = new Nortel12x0PhoneDefaults(getPhoneContext().getPhoneDefaults(), speedDial);
        addDefaultBeanSettingHandler(defaults);

        Nortel12x0IntercomDefaults intercomDefaults = new Nortel12x0IntercomDefaults(this);
        addDefaultBeanSettingHandler(intercomDefaults);

    }

    @Override
    public void initializeLine(Line line) {
        line.addDefaultBeanSettingHandler(new Nortel12x0LineDefaults(line));
    }

    @Override
    protected ProfileContext createContext() {
        SpeedDial speedDial = getPhoneContext().getSpeedDial(this);
        Collection<PhonebookEntry> phoneBook = getPhoneContext().getPhonebookEntries(this);
        return new Nortel12x0Context(this, speedDial, phoneBook, getModel().getProfileTemplate());
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

    public String getProfileFilename() {
        return "Nortel/config/" + "SIP" + getSerialNumber().toUpperCase() + ".xml";

    }

    public static class Nortel12x0PhoneDefaults {

        private DeviceDefaults m_defaults;
        private SpeedDial m_speedDial;

        Nortel12x0PhoneDefaults(DeviceDefaults defaults, SpeedDial speedDial) {
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

        @SettingEntry(path = PROXY_SERVER_SETTING)
        public String getProxyAddress() {
            return m_defaults.getDomainName();
        }

        @SettingEntry(path = MOH_SETTING)
        public String getUserMoh() {
            return m_defaults.getMusicOnHoldUri(m_defaults.getDomainName());
        }

        @SettingEntry(path = PRESENCE_SERVER_SETTING)
        public String getPresenceServer() {
            return m_defaults.getDomainName();
        }

        @SettingEntry(path = TIME_SERVER_NAME)
        public String getNtpServer() {
            return m_defaults.getNtpServer();
        }

        @SettingEntry(path = ALTERNATE_TIME_SERVER_NAME)
        public String getAlternateNtpServer() {
            return m_defaults.getAlternateNtpServer();
        }

        @SettingEntry(path = CALL_PICKUP_PREFIX)
        public String getDirectedCallPickupString() {
            return m_defaults.getDirectedCallPickupCode();
        }


    }

    public static class Nortel12x0LineDefaults {
        private Line m_line;

        Nortel12x0LineDefaults(Line line) {
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

    }

    public void restart() {
        sendCheckSyncToFirstLine();
    }

    static class Nortel12x0Context extends ProfileContext {
        private SpeedDial m_speedDial;
        private Collection<PhonebookEntry> m_phoneBook;

        public Nortel12x0Context(Nortel12x0Phone device, SpeedDial speedDial, Collection<PhonebookEntry> phoneBook,
                String profileTemplate) {
            super(device, profileTemplate);
            m_speedDial = speedDial;
            m_phoneBook = trim(phoneBook);
        }

        public Map<String, Object> getContext() {
            Map<String, Object> context = super.getContext();
            context.put("speedDialInfo", getSpeedDialEntries());
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

        String[] getSpeedDialEntries() {
            if (m_speedDial == null) {
                return ArrayUtils.EMPTY_STRING_ARRAY;
            }
            List<Button> buttons = m_speedDial.getButtons();
            String spName = null;
            String spNumber = null;
            String spPresence = null;
            int size = Math.min(SPEEDDIAL_MAX, buttons.size());
            String[] numbers = new String[size * CONFIGURATION_PARAMS_PER_SPEEDDIAL_ENTRY];

            for (int i = 0; i < numbers.length / CONFIGURATION_PARAMS_PER_SPEEDDIAL_ENTRY; i++) {
                for (int j = 0; j < CONFIGURATION_PARAMS_PER_SPEEDDIAL_ENTRY; j++) {
                    spName = buttons.get(i).getLabel();
                    if (spName == null) { 
                        spName = buttons.get(i).getNumber();
                    }
                    spName = StringUtils.substring(spName, 0, 15);
                    spNumber = buttons.get(i).getNumber();
                    if (buttons.get(i).isBlf()) {
                        spPresence = "1";
                    } else {
                        spPresence = "0";
                    }

                    if (j == 0) {
                        numbers[i * CONFIGURATION_PARAMS_PER_SPEEDDIAL_ENTRY + j] = spName;
                    } else if (j == 1) {
                        numbers[i * CONFIGURATION_PARAMS_PER_SPEEDDIAL_ENTRY + j] = spNumber;
                    } else {
                        numbers[i * CONFIGURATION_PARAMS_PER_SPEEDDIAL_ENTRY + j] = spPresence;
                    }
                }
            }
            return numbers;
        }
    }
}

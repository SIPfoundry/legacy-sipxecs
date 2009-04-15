/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.snom;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.service.UnmanagedService;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class SnomPhone extends Phone {
    public static final String BEAN_ID = "snom";

    private static final int SPEEDDIAL_MAX = 33;
    private static final int PHONEBOOK_MAX = 100;

    private static final String USER_HOST = "line/user_host";
    // suspiciously, no registration server port?

    private static final String MAILBOX = "line/user_mailbox";
    private static final String OUTBOUND_PROXY = "sip/user_outbound";
    private static final String USER_NAME = "line/user_name";
    private static final String PASSWORD = "line/user_pass";
    private static final String AUTH_NAME = "line/user_pname";
    private static final String DISPLAY_NAME = "line/user_realname";

    private static final String TIMEZONE_SETTING = "network/utc_offset";
    private static final String CONFIG_URL = "update/setting_server";
    private static final String DST_SETTING = "network/dst";
    private static final String UDP_TRANSPORT_TAG = ";transport=udp";

    public SnomPhone() {
    }

    @Override
    public void initialize() {
        SnomDefaults defaults = new SnomDefaults(getPhoneContext().getPhoneDefaults(), this);
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initializeLine(Line line) {
        SnomLineDefaults defaults = new SnomLineDefaults(getPhoneContext().getPhoneDefaults(), line);
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    protected ProfileContext createContext() {
        SpeedDial speedDial = getPhoneContext().getSpeedDial(this);
        Collection<PhonebookEntry> phoneBook = getPhoneContext().getPhonebookEntries(this);
        return new SnomContext(this, speedDial, phoneBook, getModel().getProfileTemplate());
    }

    @Override
    protected void setLineInfo(Line line, LineInfo externalLine) {
        line.setSettingValue(DISPLAY_NAME, externalLine.getDisplayName());
        line.setSettingValue(USER_NAME, externalLine.getUserId());
        line.setSettingValue(PASSWORD, externalLine.getPassword());
        line.setSettingValue(USER_HOST, externalLine.getRegistrationServer());
        line.setSettingValue(MAILBOX, externalLine.getVoiceMail());
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = new LineInfo();
        lineInfo.setUserId(line.getSettingValue(USER_NAME));
        lineInfo.setDisplayName(line.getSettingValue(DISPLAY_NAME));
        lineInfo.setPassword(line.getSettingValue(PASSWORD));
        lineInfo.setRegistrationServer(line.getSettingValue(USER_HOST));
        lineInfo.setVoiceMail(line.getSettingValue(MAILBOX));
        return lineInfo;
    }

    @Override
    public String getProfileFilename() {
        String serialNumber = getSerialNumber();
        if (StringUtils.isEmpty(serialNumber)) {
            return "snom.htm";
        }
        StringBuilder buffer = new StringBuilder(serialNumber.toUpperCase());
        buffer.append(".htm");
        return buffer.toString();
    }

    public int getMaxLineCount() {
        return getModel().getMaxLineCount();
    }

    public Collection<Setting> getProfileLines() {
        int lineCount = getModel().getMaxLineCount();
        List<Setting> linesSettings = new ArrayList<Setting>(getMaxLineCount());

        Collection<Line> lines = getLines();
        int i = 0;
        Iterator<Line> ilines = lines.iterator();
        for (; ilines.hasNext() && (i < lineCount); i++) {
            linesSettings.add(ilines.next().getSettings());
        }

        for (; i < lineCount; i++) {
            Line line = createLine();
            line.setPhone(this);
            line.setPosition(i);
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }

    public static class SnomDefaults {
        private final DeviceDefaults m_defaults;
        private final SnomPhone m_phone;

        SnomDefaults(DeviceDefaults defaults, SnomPhone phone) {
            m_defaults = defaults;
            m_phone = phone;
        }

        @SettingEntry(path = CONFIG_URL)
        public String getConfigUrl() {
            String configUrl = m_defaults.getProfileRootUrl() + '/' + m_phone.getProfileFilename();
            return configUrl;
        }

        @SettingEntry(path = TIMEZONE_SETTING)
        public String getTimeZoneOffset() {
            int tzsec = m_defaults.getTimeZone().getOffset();

            if (tzsec <= 0) {
                return String.valueOf(tzsec);
            }

            return '+' + String.valueOf(tzsec);
        }

        @SettingEntry(path = "Basic_Network_Settings/ntp_server")
        public String getNtpServer() {
            return m_defaults.getNtpServer();
        }

        @SettingEntry(path = "Advanced_Network_Settings/syslog_server")
        public String getSyslogServer() {
            return m_defaults.getServer(0, UnmanagedService.SYSLOG);
        }

        @SettingEntry(path = DST_SETTING)
        public String getDstSetting() {
            DeviceTimeZone zone = m_defaults.getTimeZone();
            if (zone.getDstSavings() == 0) {
                return null;
            }

            int stopWeek = adjustWeek(zone.getStopWeek());
            int startWeek = adjustWeek(zone.getStartWeek());
            int startDayOfWeek = adjustDayOfWeek(zone.getStartDayOfWeek());
            int stopDayOfWeek = adjustDayOfWeek(zone.getStopDayOfWeek());
            return String.format("%d %02d.%02d.%02d %02d:00:00 %02d.%02d.%02d %02d:00:00", zone
                    .getDstSavingsInSeconds(), zone.getStartMonth(), startWeek, startDayOfWeek, zone
                    .getStartTimeInHours(), zone.getStopMonth(), stopWeek, stopDayOfWeek, zone.getStopTimeInHours());
        }

        private int adjustWeek(int week) {
            if (week == DeviceTimeZone.DST_LASTWEEK) {
                return 5;
            }
            return Math.min(week, 5);
        }

        private int adjustDayOfWeek(int dayOfWeek) {
            return (dayOfWeek + 5) % 7 + 1;
        }
    }

    public static class SnomLineDefaults {
        private final Line m_line;
        private final DeviceDefaults m_defaults;

        SnomLineDefaults(DeviceDefaults defaults, Line line) {
            m_line = line;
            m_defaults = defaults;
        }

        @SettingEntry(path = USER_HOST)
        public String getUserHost() {
            String registrationUri = StringUtils.EMPTY;
            User u = m_line.getUser();
            if (u != null) {
                String domainName = m_defaults.getDomainName();
                registrationUri = domainName + UDP_TRANSPORT_TAG;
            }
            return registrationUri;
        }

        @SettingEntry(path = OUTBOUND_PROXY)
        public String getOutboundProxy() {
            String outboundProxy = null;
            User user = m_line.getUser();
            if (user != null) {
                // XPB-398 This forces TCP, this is defined in conjunction with "transport=udp"
                // This is benign w/o SRV, but required w/SRV
                // XCF-1680 - IFF this still needs to be set, this should domain name to allow
                // for HA systems. Remember, domain name is really FQHN for non SRV systems
                outboundProxy = m_defaults.getDomainName();
            }

            return outboundProxy;
        }

        @SettingEntry(path = MAILBOX)
        public String getMailbox() {
            // XCF-722 Setting this to the mailbox (e.g. 101) would fix issue
            // where mailbox button on phone calls voicemail server, but would
            // break MWI subscription because SUBSCRIBE message fails
            // authentication unless this value is user's username
            return getUserName();
        }

        @SettingEntry(paths = { USER_NAME, AUTH_NAME })
        public String getUserName() {
            String username = null;
            User user = m_line.getUser();
            if (user != null) {
                username = user.getUserName();
            }

            return username;
        }

        @SettingEntry(path = PASSWORD)
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
            }

            return displayName;
        }

        @SettingEntry(path = "sip/user_moh")
        public String getUserMoh() {
            return m_defaults.getMusicOnHoldUri(m_defaults.getDomainName());
        }
    }

    @Override
    public void restart() {
        sendCheckSyncToFirstLine();
    }

    static class SnomContext extends ProfileContext {
        private final SpeedDial m_speedDial;
        private final Collection<PhonebookEntry> m_phoneBook;

        public SnomContext(SnomPhone device, SpeedDial speedDial, Collection<PhonebookEntry> phoneBook,
                String profileTemplate) {
            super(device, profileTemplate);
            m_speedDial = speedDial;
            m_phoneBook = trim(phoneBook);
        }

        @Override
        public Map<String, Object> getContext() {
            Map<String, Object> context = super.getContext();
            context.put("speedDial", getNumbers());
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

        /**
         * Create SNOM speed dial information
         *
         * @return and array of at most 33 (0-32) phone numbers
         */
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

    }

}

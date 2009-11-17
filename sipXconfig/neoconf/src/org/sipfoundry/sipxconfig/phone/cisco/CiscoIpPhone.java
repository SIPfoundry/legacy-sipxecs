/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.cisco;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

/**
 * Support for Cisco 7940/7960
 */
public class CiscoIpPhone extends CiscoPhone {
    private static final String ZEROMIN = ":00";
    private static final String SHORTNAME_PATH = "line/shortname";
    private static final String AUTH_NAME_PATH = "line/authname";
    private static final String USER_NAME_PATH = "line/name";
    private static final String PASSWORD_PATH = "line/password";
    private static final String DISPLAY_NAME_PATH = "line/displayname";
    private static final String REGISTRATION_PATH = "proxy/address";
    private static final String REGISTRATION_PORT_PATH = "proxy/port";
    private static final String MESSAGES_URI_PATH = "phone/messages_uri";

    public CiscoIpPhone() {
    }

    @Override
    public void initialize() {
        CiscoIpDefaults defaults = new CiscoIpDefaults(getPhoneContext().getPhoneDefaults());
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public String getProfileFilename() {
        String phoneFilename = getSerialNumber();
        return "SIP" + phoneFilename.toUpperCase() + ".cnf";
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
        line.setSettingValue(DISPLAY_NAME_PATH, lineInfo.getDisplayName());
        line.setSettingValue(USER_NAME_PATH, lineInfo.getUserId());
        line.setSettingValue(PASSWORD_PATH, lineInfo.getPassword());
        line.setSettingValue(REGISTRATION_PATH, lineInfo.getRegistrationServer());
        line.setSettingValue(REGISTRATION_PORT_PATH, lineInfo.getRegistrationServerPort());
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = new LineInfo();
        lineInfo.setDisplayName(line.getSettingValue(DISPLAY_NAME_PATH));
        lineInfo.setUserId(line.getSettingValue(USER_NAME_PATH));
        lineInfo.setPassword(line.getSettingValue(PASSWORD_PATH));
        lineInfo.setRegistrationServer(line.getSettingValue(REGISTRATION_PATH));
        lineInfo.setRegistrationServerPort(line.getSettingValue(REGISTRATION_PORT_PATH));
        return lineInfo;
    }

    public static class CiscoIpDefaults {
        private final DeviceDefaults m_defaults;

        CiscoIpDefaults(DeviceDefaults defaults) {
            m_defaults = defaults;
        }

        @SettingEntry(path = MESSAGES_URI_PATH)
        public String getMessagesUri() {
            return m_defaults.getVoiceMail();
        }

        private DeviceTimeZone getZone() {
            return m_defaults.getTimeZone();
        }

        @SettingEntry(path = "datetime/dst_auto_adjust")
        public boolean isDstAutoAdjust() {
            return getZone().getDstSavings() != 0;
        }

        @SettingEntry(path = "datetime/dst_offset")
        public long getDstOffset() {
            return isDstAutoAdjust() ? getZone().getDstSavings() / 60 : 0;
        }

        @SettingEntry(path = "datetime/dst_start_day_of_week")
        public int getDstStartDayOfWeek() {
            return isDstAutoAdjust() ? getZone().getStartDayOfWeek() : 0;
        }

        @SettingEntry(path = "datetime/dst_start_month")
        public int getDstStartMonth() {
            return isDstAutoAdjust() ? getZone().getStartMonth() : 0;
        }

        @SettingEntry(path = "datetime/dst_start_time")
        public String getDstStartTime() {
            return isDstAutoAdjust() ? time(getZone().getStartTime()) : null;
        }

        @SettingEntry(path = "datetime/dst_start_week_of_month")
        public int getDstStartWeekOfMonth() {
            return isDstAutoAdjust() ? (getZone().getStartWeek() == DeviceTimeZone.DST_LASTWEEK ? 8 : getZone()
                    .getStartWeek()) : 0;
        }

        @SettingEntry(path = "datetime/dst_stop_day_of_week")
        public int getDstStopDayOfWeek() {
            return isDstAutoAdjust() ? getZone().getStopDayOfWeek() : 0;
        }

        @SettingEntry(path = "datetime/dst_stop_month")
        public int getDstStopMonth() {
            return isDstAutoAdjust() ? getZone().getStopMonth() : 0;
        }

        @SettingEntry(path = "datetime/dst_stop_time")
        public String getStopTime() {
            return isDstAutoAdjust() ? time(getZone().getStopTime()) : null;
        }

        @SettingEntry(path = "datetime/dst_stop_week_of_month")
        public int getStopWeekOfMonth() {
            int stopWeek = getZone().getStopWeek();
            if (stopWeek == DeviceTimeZone.DST_LASTWEEK) {
                stopWeek = 8;
            }
            return isDstAutoAdjust() ? stopWeek : 0;
        }

        @SettingEntry(path = "network/sntp_server")
        public String getNtpServer() {
            return m_defaults.getNtpServer();
        }

        @SettingEntry(path = "sip/proxy_emergency")
        public String getEmergencyHost() {
            return m_defaults.getEmergencyAddress();
        }

        @SettingEntry(path = "sip/proxy_emergency_port")
        public Integer getEmergencyPort() {
            return m_defaults.getEmergencyPort();
        }

        private String time(int time) {
            return String.valueOf(time / 3600) + ZEROMIN;
        }

    }

    @Override
    public void initializeLine(Line line) {
        line.addDefaultBeanSettingHandler(new CiscoIpLineDefaults(line));
    }

    public class CiscoIpLineDefaults {
        private final Line m_line;

        CiscoIpLineDefaults(Line line) {
            m_line = line;
        }

        @SettingEntry(path = PASSWORD_PATH)
        public String getPassword() {
            String password = null;
            User u = m_line.getUser();
            if (u != null) {
                password = u.getSipPassword();
            }
            return password;
        }

        @SettingEntry(paths = { SHORTNAME_PATH, AUTH_NAME_PATH, USER_NAME_PATH })
        public String getUserName() {
            String name = null;
            User u = m_line.getUser();
            if (u != null) {
                name = u.getUserName();
            }
            return name;
        }

        @SettingEntry(path = DISPLAY_NAME_PATH)
        public String getDisplayName() {
            String displayName = null;
            User u = m_line.getUser();
            if (u != null) {
                displayName = u.getDisplayName();
            }
            return displayName;
        }

        @SettingEntry(path = REGISTRATION_PATH)
        public String getProxyAddress() {
            return m_line.getPhoneContext().getPhoneDefaults().getDomainName();
        }
    }

    public Collection getProfileLines() {
        ArrayList linesSettings = new ArrayList(getMaxLineCount());

        Collection lines = getLines();
        int i = 0;
        Iterator ilines = lines.iterator();
        for (; ilines.hasNext() && (i < getMaxLineCount()); i++) {
            linesSettings.add(((Line) ilines.next()).getSettings());
        }

        // copy in blank lines of all unused lines
        for (; i < getMaxLineCount(); i++) {
            Line line = createLine();
            linesSettings.add(line.getSettings());
            line.addDefaultBeanSettingHandler(new CiscpIpStubbedLineDefaults());
        }

        return linesSettings;
    }

    public static class CiscpIpStubbedLineDefaults {
        @SettingEntry(paths = { REGISTRATION_PATH, REGISTRATION_PORT_PATH })
        public String getEmpty() {
            return StringUtils.EMPTY;
        }
    }
}

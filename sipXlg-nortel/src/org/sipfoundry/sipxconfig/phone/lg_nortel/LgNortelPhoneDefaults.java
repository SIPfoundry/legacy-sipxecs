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

import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

/**
 * Default parameters for LgNortelPhone
 *
 * The following parameters: outbound_proxy_server, outbound_proxy_port, SIP_service_domain are
 * intentionally *not* set here. Phones use per-line parameters to find proxy and set SIP domain.
 */
public class LgNortelPhoneDefaults {

    private final DeviceDefaults m_defaults;
    private final int m_lines;

    public LgNortelPhoneDefaults(DeviceDefaults defaults, int lines) {
        m_defaults = defaults;
        m_lines = lines;
    }

    private DeviceTimeZone getZone() {
        return m_defaults.getTimeZone();
    }

    @SettingEntry(path = "VOIP/max_line_num")
    public int getMaxLineNum() {
        return m_lines;
    }

    @SettingEntry(path = "VOIP/moh_url")
    public String getMohUrl() {
        String mohUri = m_defaults.getMusicOnHoldUri();
        return SipUri.stripSipPrefix(mohUri);
    }

    @SettingEntry(path = "NETTIME/sntp_server_address")
    public String getNtpServer() {
        return m_defaults.getNtpServer();
    }

    @SettingEntry(path = "NETTIME/timezone")
    public int getTimezone() {
        return getTimezoneFromRawOffsetSeconds(getZone().getOffsetInSeconds());
    }

    //@SettingEntry(path = "VOIP/outbound_proxy_server")
    public String getOutboundProxyServer() {
        return m_defaults.getDomainName();
    }

    private static int getTimezoneFromRawOffsetSeconds(int offset) {
        switch (offset) {
        case -43200:  // GMT-12:00 Int.Date Line, West
            return 60;

        case -39600:  // GMT-11:00 Midway/Samoa
            return 61;

        case -36000:  // GMT-10:00 Hawaii
            return 1;

        case -32400:  // GMT-09:00 Alaska
            return 62;

        case -28800:  // GMT-08:00 Pacific Standard
            return 3;

        case -25200:  // GMT-07:00 Mountain Standard
            return 4;

        case -21600:  // GMT-06:00 Central Standard
            return 5;

        case -18000:  // GMT-05:00 Eastern Standard
            return 9;

        case -14400:  // GMT-04:00 Atlantic Standard
            return 12;

        case -12600:  // GMT-03:30 Newfoundland
            return 15;

        case -10800:  // GMT-03:00 Brasilia, Brazil
            return 16;

        case -7200:   // GMT-02:00 Nuuk, Greenland
            return 63;

        case -3600:   // GMT-01:00 Azores, Portugal
            return 19;

        case 0:       // GMT 00:00 London, England
            return 23;

        case 3600:    // GMT+01:00 Central European
            return 28;

        case 7200:    // GMT+02:00 Istanbul, Turkey
            return 38;

        case 10800:   // GMT+03:00 Moscow, Russia
            return 41;

        case 12600:   // GMT+03:30 Tehran, Iran
            return 44;

        case 14400:   // GMT+04:00 Abu Dhabi, UAE
            return 45;

        case 16200:   // GMT+04:30 Kabul, Afghanistan
            return 47;

        case 18000:   // GMT+05:00 Islamabad, Pakistan
            return 64;

        case 19800:   // GMT+05:30 New Delhi, India
            return 48;

        case 21600:   // GMT+06:00 Dhaka, Bangladesh
            return 65;

        case 23400:   // GMT+06:30 Yangon, Myanmar
            return 66;

        case 25200:   // GMT+07:00 Jakarta, Indonesia
            return 50;

        case 28800:   // GMT+08:00 Bejing, China
            return 51;

        case 32400:   // GMT+09:00 Seoul, Korea
            return 52;

        case 34200:   // GMT+09:30 Darwin, Australia
            return 54;

        case 36000:   // GMT+10:00 Guam Standard
            return 55;

        case 39600:   // GMT+11:00 Solomon Islands
            return 67;

        case 43200:   // GMT+12:00 Auckland, Wellington
            return 57;

        case 46800:   // GMT+13:00 Nuku'Alofa
            return 58;

        default: // GMT by default
            return 23;
        }
    }

    @SettingEntry(path = "NETTIME/dst_auto_adjust")
    public boolean getUseDst() {
        return getZone().getUseDaylight();
    }

    public int getMonthIndexLgFromJava(int indexJava) {
        // LG Month mapping starts at 1, but Java starts at 0.
        return indexJava + 1;
    }

    private Integer nullUnlessDst(int i) {
        return (getUseDst() ? i : null);
    }

    @SettingEntry(path = "NETTIME/dst_start_month")
    public Integer getStartMonth() {
        return nullUnlessDst(getMonthIndexLgFromJava(getZone().getStartMonth()));
    }

    @SettingEntry(path = "NETTIME/dst_start_day_of_week")
    public Integer getStartDayOfWeek() {
        return nullUnlessDst(getZone().getStartDayOfWeek());
    }

    @SettingEntry(path = "NETTIME/dst_start_week_of_month")
    public Integer getStartWeekOfMonth() {
        int week = getZone().getStartWeek();
        return nullUnlessDst(adjustWeekOfMonth(week));
    }

    @SettingEntry(path = "NETTIME/dst_start_time")
    public Integer getStartTime() {
        return nullUnlessDst(getZone().getStartTimeInHours());
    }

    @SettingEntry(path = "NETTIME/dst_stop_month")
    public Integer getStopMonth() {
        return nullUnlessDst(getMonthIndexLgFromJava(getZone().getStopMonth()));
    }

    @SettingEntry(path = "NETTIME/dst_stop_day_of_week")
    public Integer getStopDayOfWeek() {
        return nullUnlessDst(getZone().getStopDayOfWeek());
    }

    @SettingEntry(path = "NETTIME/dst_stop_week_of_month")
    public Integer getStopWeekOfMonth() {
        int week = getZone().getStopWeek();
        return nullUnlessDst(adjustWeekOfMonth(week));
    }

    @SettingEntry(path = "NETTIME/dst_stop_time")
    public Integer getStopTime() {
        return nullUnlessDst(getZone().getStopTimeInHours());
    }

    @SettingEntry(path = "VOIP/message_url")
    public String getVoicemailExtension() {
        return m_defaults.getVoiceMail();
    }

    @SettingEntry(path = "DIAL/emergency_number")
    public String getEmergencyNumber() {
        return m_defaults.getEmergencyNumber();
    }

    @SettingEntry(path = "DIAL/emergency_address")
    public String getEmergencyAddress() {
        return m_defaults.getEmergencyAddress();
    }

    /**
     * Adjusts week of month value for LG phones.
     *
     * In case of LG phones '7' means last month of the week.
     */
    private int adjustWeekOfMonth(int week) {
        return week != DeviceTimeZone.DST_LASTWEEK ? week : 7;
    }
}

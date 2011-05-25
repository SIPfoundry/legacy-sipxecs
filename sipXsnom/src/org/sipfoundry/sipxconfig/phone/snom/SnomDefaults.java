/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.phone.snom;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.service.UnmanagedService;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

import static org.sipfoundry.sipxconfig.phone.snom.SnomConstants.CONFIG_URL;
import static org.sipfoundry.sipxconfig.phone.snom.SnomConstants.DST_SETTING;
import static org.sipfoundry.sipxconfig.phone.snom.SnomConstants.NTP;
import static org.sipfoundry.sipxconfig.phone.snom.SnomConstants.SYSLOG_SERVER;
import static org.sipfoundry.sipxconfig.phone.snom.SnomConstants.TIMEZONE_SETTING;

public class SnomDefaults {
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

    @SettingEntry(path = NTP)
    public String getNtpServer() {
        return m_defaults.getNtpServer();
    }

    @SettingEntry(path = SYSLOG_SERVER)
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
        return String.format("%d %02d.%02d.%02d %02d:00:00 %02d.%02d.%02d %02d:00:00",
                zone.getDstSavingsInSeconds(), zone.getStartMonth(), startWeek, startDayOfWeek, zone
                        .getStartTimeInHours(), zone.getStopMonth(), stopWeek, stopDayOfWeek, zone
                        .getStopTimeInHours());
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

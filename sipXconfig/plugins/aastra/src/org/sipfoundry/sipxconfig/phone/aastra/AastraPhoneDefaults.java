/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.phone.aastra;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

import static org.apache.commons.lang.StringUtils.EMPTY;

public class AastraPhoneDefaults {
    private static final String DEFAULTTIME = "US-Eastern";
    private DeviceDefaults m_defaults;

    AastraPhoneDefaults(DeviceDefaults defaults) {
        m_defaults = defaults;
    }

    private DeviceTimeZone getZone() {
        return m_defaults.getTimeZone();
    }

    @SettingEntry(path = "network/time_server/time_server1")
    public String getNtpServer() {
        return m_defaults.getNtpServer();
    }

    @SettingEntry(path = "network/time_server/time_server2")
    public String getAlternateNtpServer() {
        return m_defaults.getAlternateNtpServer();
    }

    @SettingEntry(path = "td/dstconf")
    public int getDSTSavings() {
        int time = 0;
        // FIXME: not sure why it's 30 or 60
        return time;
    }

    @SettingEntry(path = "td/time_zone_name")
    public String getTimeZoneName() {
        return getTimezoneFromRawOffsetSeconds(getZone().getOffset());
    }

    @SettingEntry(path = "security/emergency_dialplan")
    public String getEmergencyNumber() {
        return m_defaults.getEmergencyNumber();
    }

    private static String getTimezoneFromRawOffsetSeconds(int offset) {

        switch (offset) {
        case -43200: // GMT-12:00 Int.Date Line, West
            return EMPTY;

        case -39600: // GMT-11:00 Midway/Samoa
            return EMPTY;

        case -36000: // GMT-10:00 Hawaii
            return "US-Hawaii";

        case -32400: // GMT-09:00 Alaska
            return "US-Alaska";

        case -28800: // GMT-08:00 Pacific Standard
            return "US-Pacific";

        case -25200: // GMT-07:00 Mountain Standard
            return "US-Mountain";

        case -21600: // GMT-06:00 Central Standard
            return "US-Central";

        case -18000: // GMT-05:00 Eastern Standard
            return DEFAULTTIME;

        case -14400: // GMT-04:00 Atlantic Standard
            return "CA-Atlantic";

        case -12600: // GMT-03:30 Newfoundland
            return "CA-Newfoundland";

        case -10800: // GMT-03:00 Brasilia, Brazil
            return "BR-Sao Paulo";

        case -7200: // GMT-02:00 Nuuk, Greenland
            return EMPTY;

        case -3600: // GMT-01:00 Azores, Portugal
            return EMPTY;

        case 0: // GMT 00:00 London, England
            return "GB-London";

        case 3600: // GMT+01:00 Central European
            return "BE-Brussels";

        case 7200: // GMT+02:00 Istanbul, Turkey
            return "GR-Athens";

        case 10800: // GMT+03:00 Moscow, Russia
            return "RU-Moscow";

        case 12600: // GMT+03:30 Tehran, Iran
            return EMPTY;

        case 14400: // GMT+04:00 Abu Dhabi, UAE
            return EMPTY;

        case 16200: // GMT+04:30 Kabul, Afghanistan
            return EMPTY;

        case 18000: // GMT+05:00 Islamabad, Pakistan
            return EMPTY;

        case 19800: // GMT+05:30 New Delhi, India
            return "SG-Singapore";

        case 21600: // GMT+06:00 Dhaka, Bangladesh
            return EMPTY;

        case 23400: // GMT+06:30 Yangon, Myanmar
            return EMPTY;

        case 25200: // GMT+07:00 Jakarta, Indonesia
            return EMPTY;

        case 28800: // GMT+08:00 Bejing, China
            return "CN-China";

        case 32400: // GMT+09:00 Seoul, Korea
            return "JP-Tokyo";

        case 34200: // GMT+09:30 Darwin, Australia
            return "AU-Darwin";

        case 36000: // GMT+10:00 Guam Standard
            return EMPTY;

        case 39600: // GMT+11:00 Solomon Islands
            return EMPTY;

        case 43200: // GMT+12:00 Auckland, Wellington
            return "NZ-Auckland";

        case 46800: // GMT+13:00 Nuku'Alofa
            return EMPTY;

        default: // GMT by default
            return DEFAULTTIME;
        }
    }

}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phone.gtek_aq;

import org.apache.commons.lang.math.RandomUtils;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class GtekAquiferPhoneDefaults {
    private final DeviceDefaults m_defaults;

    GtekAquiferPhoneDefaults(DeviceDefaults defaults) {
        m_defaults = defaults;
    }

    @SettingEntry(path = "AutoConfigVersion/AutoConfigVersion")
    public int getAutoConfigVersion() {
        return RandomUtils.nextInt(256);
    }

    @SettingEntry(path = "MoHSettings/NMOH")
    public String getMohUrl() {
        String mohUri = m_defaults.getMusicOnHoldUri();
        return SipUri.stripSipPrefix(mohUri);
    }

    @SettingEntry(path = "SNTP_Settings/SNTP1stServ")
    public String getNtpServer() {
        return m_defaults.getNtpServer();
    }

    @SettingEntry(path = "SNTP_Settings/SNTP2ndServ")
    public String getAlternateNtpServer() {
        return m_defaults.getAlternateNtpServer();
    }

    @SettingEntry(path = "SNTP_Settings/SNTPTimeZone")
    public String getTimeZone() {
        return getTimezoneFromRawOffsetSeconds(getZone().getOffsetInSeconds());
    }

    private DeviceTimeZone getZone() {
        return m_defaults.getTimeZone();
    }

    private static String getTimezoneFromRawOffsetSeconds(int offset) {

        switch (offset) {
        case -43200:
            return "-12:00";

        case -39600:
            return "-11:00";

        case -36000:
            return "-10:00";

        case -32400:
            return "-09:00";

        case -28800:
            return "-08:00";

        case -25200:
            return "-07:00";

        case -21600:
            return "-06:00";

        case -18000:
            return "-05:00";

        case -14400:
            return "-04:00";

        case -12600:
            return "-03:30";

        case -10800:
            return "-03:00";

        case -7200:
            return "-02:00";

        case -3600:
            return "-01:00";

        case 3600:
            return "+01:00";

        case 7200:
            return "+02:00";

        case 10800:
            return "+03:00";

        case 12600:
            return "+03:30";

        case 14400:
            return "+04:00";

        case 16200:
            return "+04:30";

        case 18000:
            return "+05:00";

        case 19800:
            return "+05:30";

        case 21600:
            return "+06:00";

        case 25200:
            return "+07:00";

        case 28800:
            return "+08:00";

        case 32400:
            return "+09:00";

        case 34200:
            return "+09:30";

        case 36000:
            return "+10:00";

        case 39600:
            return "+11:00";

        case 43200:
            return "+12:00";

        case 0:
        default: // GMT by default
            return "+0:00";
        }
    }
}

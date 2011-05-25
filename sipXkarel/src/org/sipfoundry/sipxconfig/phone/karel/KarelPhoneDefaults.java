/*
 *
 *
 * Copyright (C) 2010 Karel Elektronik, A.S. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.phone.karel;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class KarelPhoneDefaults {
    private final DeviceDefaults m_defaults;

    KarelPhoneDefaults(DeviceDefaults defaults) {
        m_defaults = defaults;
    }

    @SettingEntry(path = "TIME_SETTINGS/sntpServer")
    public String getNtpServer() {
        return m_defaults.getNtpServer();
    }

    @SettingEntry(path = "TIME_SETTINGS/dst")
    public boolean isDstEnabled() {
        return getZone().getUseDaylight();
    }

    @SettingEntry(path = "TIME_SETTINGS/timeZone")
    public int getTimeZone() {
        return getTimezoneFromRawOffsetSeconds(getZone().getOffsetInSeconds());
    }

    private DeviceTimeZone getZone() {
        return m_defaults.getTimeZone();
    }

    private static int getTimezoneFromRawOffsetSeconds(int offset) {

        switch (offset) {
        case -43200:
            return 0;

        case -39600:
            return 1;

        case -36000:
            return 2;

        case -32400:
            return 3;

        case -28800:
            return 4;

        case -25200:
            return 5;

        case -21600:
            return 6;

        case -18000:
            return 7;

        case -14400:
            return 8;

        case -10800:
            return 9;

        case -7200:
            return 10;

        case -3600:
            return 11;

        case 3600:
            return 13;

        case 7200:
            return 14;

        case 10800:
            return 15;

        case 14400:
            return 16;

        case 18000:
            return 17;

        case 21600:
            return 18;

        case 25200:
            return 19;

        case 28800:
            return 20;

        case 32400:
            return 21;

        case 36000:
            return 22;

        case 39600:
            return 23;

        case 43200:
            return 24;

        case 0:
        default: // GMT by default
            return 12;
        }
    }
}

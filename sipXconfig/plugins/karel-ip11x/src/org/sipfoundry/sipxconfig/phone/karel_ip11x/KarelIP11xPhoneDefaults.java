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
package org.sipfoundry.sipxconfig.phone.karel_ip11x;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class KarelIP11xPhoneDefaults {
    private final DeviceDefaults m_defaults;

    KarelIP11xPhoneDefaults(DeviceDefaults defaults) {
        m_defaults = defaults;
    }

    @SettingEntry(path = "Time/TimeServer1")
    public String getNtpServer() {
        return m_defaults.getNtpServer();
    }
/*
    @SettingEntry(path = "Time/SummerTime")
    public boolean isDstEnabled() {
        return getZone().getUseDaylight();
    }
*/
    @SettingEntry(path = "Time/TimeZone")
    public int getTimeZone() {
        return getTimezoneFromRawOffsetSeconds(getZone().getOffsetInSeconds());
    }

    private DeviceTimeZone getZone() {
        return m_defaults.getTimeZone();
    }

    private static int getTimezoneFromRawOffsetSeconds(int offset) {

        switch (offset) {
        case -43200:
            return -12;

        case -39600:
            return -11;

        case -36000:
            return -10;

        case -32400:
            return -9;

        case -28800:
            return -8;

        case -25200:
            return -7;

        case -21600:
            return -6;

        case -18000:
            return -5;

        case -14400:
            return -4;

        case -10800:
            return -3;

        case -7200:
            return -2;

        case -3600:
            return -1;

        case 3600:
            return 1;

        case 7200:
            return 2;

        case 10800:
            return 3;

        case 14400:
            return 4;

        case 18000:
            return 5;

        case 21600:
            return 6;

        case 25200:
            return 7;

        case 28800:
            return 8;

        case 32400:
            return 9;

        case 36000:
            return 10;

        case 39600:
            return 11;

        case 43200:
            return 12;

        case 0:
        default: // GMT by default
            return 0;
        }
    }
}

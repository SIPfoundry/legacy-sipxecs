/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.phone.snom;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

import static org.sipfoundry.sipxconfig.phone.snom.SnomM3Constants.NTP;

public class SnomM3Defaults {
    private final DeviceDefaults m_defaults;
    private final SnomM3Phone m_phone;

    SnomM3Defaults(DeviceDefaults defaults, SnomM3Phone phone) {
        m_defaults = defaults;
        m_phone = phone;
    }

    @SettingEntry(path = NTP)
    public String getNtpServer() {
        return m_defaults.getNtpServer();
    }
}

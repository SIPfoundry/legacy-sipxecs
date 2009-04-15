/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.clearone;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class ClearonePhoneDefaults {

    private final DeviceDefaults m_defaults;
    private final String m_dialPlanName;

    public ClearonePhoneDefaults(DeviceDefaults defaults, String dialplanName) {
        m_defaults = defaults;
        m_dialPlanName = dialplanName;
    }

    @SettingEntry(path = "time/SNTP_server_1")
    public String getNtpServer() {
        return m_defaults.getNtpServer();
    }

    @SettingEntry(path = "time/SNTP_server_2")
    public String getAlternateNtpServer() {
        return m_defaults.getAlternateNtpServer();
    }

    @SettingEntry(path = "time/timezone")
    public int getTimezone() {
        int tzhrs = m_defaults.getTimeZone().getOffset() / 60;
        // start counting from date change line
        return tzhrs + 12;
    }

    @SettingEntry(path = "time/adjust_dst")
    public boolean getAdjustDst() {
        return m_defaults.getTimeZone().getUseDaylight();
    }

    @SettingEntry(path = "basic/dialplan")
    public String getDialPlanName() {
        return m_dialPlanName;
    }
}

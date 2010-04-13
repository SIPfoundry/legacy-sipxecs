/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.mitel;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class MitelPhoneDefaults {

    private final DeviceDefaults m_defaults;

    public MitelPhoneDefaults(DeviceDefaults defaults) {
        m_defaults = defaults;
    }

    @SettingEntry(path = "time/timezone")
    public int getTimezone() {
        int tzhrs = m_defaults.getTimeZone().getOffset() / 3600;
        // start counting from date change line
        return tzhrs + 12;
    }

    @SettingEntry(path = "time/adjust_dst")
    public boolean getAdjustDst() {
        return m_defaults.getTimeZone().getUseDaylight();
    }
}

/*
 *
 *
 * Copyright (C) 2004-2009 iscoord ltd.
 * Beustweg 12, 8032 Zurich, Switzerland
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.phone.isphone;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

import static org.sipfoundry.sipxconfig.phone.isphone.IsphoneConstants.CONFIG_URL;

public class IsphoneDefaults {
    private final DeviceDefaults m_defaults;
    private final IsphonePhone m_phone;

    IsphoneDefaults(DeviceDefaults defaults, IsphonePhone phone) {
        m_defaults = defaults;
        m_phone = phone;
    }

    @SettingEntry(path = CONFIG_URL)
    public String getConfigUrl() {
        String configUrl = m_defaults.getProfileRootUrl() + '/' + m_phone.getProfileFilename();
        return configUrl;
    }

}

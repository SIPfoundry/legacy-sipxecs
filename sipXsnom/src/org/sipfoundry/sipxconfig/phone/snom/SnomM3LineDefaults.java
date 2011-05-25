/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.phone.snom;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

import static org.sipfoundry.sipxconfig.phone.snom.SnomM3Constants.ALIAS;
import static org.sipfoundry.sipxconfig.phone.snom.SnomM3Constants.AUTH_NAME;
import static org.sipfoundry.sipxconfig.phone.snom.SnomM3Constants.DISPLAY_NAME;
import static org.sipfoundry.sipxconfig.phone.snom.SnomM3Constants.DOMAIN;
import static org.sipfoundry.sipxconfig.phone.snom.SnomM3Constants.MAILBOX;
import static org.sipfoundry.sipxconfig.phone.snom.SnomM3Constants.MAILBOX_NUMBER;
import static org.sipfoundry.sipxconfig.phone.snom.SnomM3Constants.PASSWORD;
import static org.sipfoundry.sipxconfig.phone.snom.SnomM3Constants.USER_NAME;

public class SnomM3LineDefaults {
    private final Line m_line;
    private final DeviceDefaults m_defaults;

    SnomM3LineDefaults(DeviceDefaults defaults, Line line) {
        m_line = line;
        m_defaults = defaults;
    }

    @SettingEntry(path = DOMAIN)
    public String getUserHost() {
        String registrationUri = StringUtils.EMPTY;
        User u = m_line.getUser();
        if (u != null) {
            String domainName = m_defaults.getDomainName();
            registrationUri = domainName;
        }
        return registrationUri;
    }

    @SettingEntry(paths = { USER_NAME, AUTH_NAME, ALIAS, MAILBOX })
    public String getUserName() {
        String username = null;
        User user = m_line.getUser();
        if (user != null) {
            username = user.getUserName();
        }

        return username;
    }

    @SettingEntry(path = PASSWORD)
    public String getPassword() {
        String password = null;
        User user = m_line.getUser();
        if (user != null) {
            password = user.getSipPassword();
        }

        return password;
    }

    @SettingEntry(path = DISPLAY_NAME)
    public String getDisplayName() {
        String displayName = null;
        User user = m_line.getUser();
        if (user != null) {
            displayName = user.getDisplayName();
        }

        return displayName;
    }

    @SettingEntry(path = MAILBOX_NUMBER)
    public String getMailBoxNumber() {
        String mailBoxNumber = null;
        User user = m_line.getUser();
        if (user != null) {
            mailBoxNumber = m_defaults.getVoiceMail();
        }

        return mailBoxNumber;
    }
}

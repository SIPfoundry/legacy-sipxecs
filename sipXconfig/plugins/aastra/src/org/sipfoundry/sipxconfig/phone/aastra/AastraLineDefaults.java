/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.phone.aastra;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class AastraLineDefaults {

    private DeviceDefaults m_defaults;
    private Line m_line;

    AastraLineDefaults(DeviceDefaults defaults, Line line) {
        m_defaults = defaults;
        m_line = line;
    }

    @SettingEntry(path = AastraPhone.AUTHORIZATION_ID_PATH)
    public String getAuthorizationId() {
        return getAddress();
    }

    @SettingEntry(path = AastraPhone.USER_ID_PATH)
    public String getAddress() {
        User u = m_line.getUser();
        if (u != null) {
            return u.getUserName();
        }
        return null;
    }

    @SettingEntry(path = AastraPhone.PASSWORD_PATH)
    public String getAuthorizationPassword() {
        User u = m_line.getUser();
        if (u != null) {
            return u.getSipPassword();
        }
        return null;
    }

    @SettingEntry(path = AastraPhone.DISPLAY_NAME_PATH)
    public String getDisplayName() {
        User u = m_line.getUser();
        if (u != null) {
            return u.getDisplayName();
        }
        return null;
    }

    @SettingEntry(path = AastraPhone.REGISTRATION_PATH)
    public String getRegistrationServer() {
        User u = m_line.getUser();
        if (u != null) {
            return m_line.getPhoneContext().getPhoneDefaults().getDomainName();
        }
        return null;
    }

}

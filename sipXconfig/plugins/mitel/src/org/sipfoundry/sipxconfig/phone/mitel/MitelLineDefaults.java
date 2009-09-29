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

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class MitelLineDefaults {
    private Line m_line;
    private DeviceDefaults m_defaults;

    public MitelLineDefaults(Line line, DeviceDefaults defaults) {
        m_line = line;
        m_defaults = defaults;
    }

    @SettingEntry(paths = { "sip/ID", "sip/AuthName" })
    public String getUserName() {
        return m_line.getDisplayLabel();
    }

    @SettingEntry(path = "sip/DispName")
    public String getDisplayName() {
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }
        return user.getDisplayName();
    }

    @SettingEntry(path = "sip/Realm")
    public String getRalm() {
        return m_defaults.getAuthorizationRealm();
    }


    @SettingEntry(path = "sip/Pwd")
    public String getSipPassword() {
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }
        return user.getSipPassword();
    }

    @SettingEntry(paths = { "sip/RegSvr", "sip/ProxySvr" })
    public String getSipProxyServer() {
        return m_defaults.getDomainName();

    }

    @SettingEntry(paths = { "sip/RegPort", "sip/ProxyPort" })
    public String getSipProxyPort() {
        return m_defaults.getProxyServerSipPort();
    }
}

/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.unidata;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class UnidataPhoneDefaults {

    private final DeviceDefaults m_defaults;

    public UnidataPhoneDefaults(DeviceDefaults defaults) {
        m_defaults = defaults;
    }

    @SettingEntry(path = "SERVER_SETTINGS/1st_Proxy")
    public String getDomainName() {
        return m_defaults.getFullyQualifiedDomainName();
    }

    @SettingEntry(path = "SERVER_SETTINGS/Domain_Realm")
    public String getAuthorizationRealm() {
        return m_defaults.getAuthorizationRealm();
    }

    @SettingEntry(path = "TIME/NTP_Server1")
    public String getNtpServer() {
        return m_defaults.getNtpServer();
    }

    @SettingEntry(path = "TIME/NTP_Server2")
    public String getAlternateNtpServer() {
        return m_defaults.getAlternateNtpServer();
    }

    @SettingEntry(path = "TIME/Enable_Daylight_Saving_Time")
    public boolean getUseDst() {
        return m_defaults.getTimeZone().getUseDaylight();
    }

    @SettingEntry(path = "SERVER_SETTINGS/SYSLOG/Server_IP")
    public String getSyslogAddress() {
        return m_defaults.getFullyQualifiedDomainName();
    }
    @SettingEntry(path = "BASIC_CALL/MWI/Subscribe_Server")
    public String getMWIAddress() {
        return m_defaults.getFullyQualifiedDomainName();
    }

    @SettingEntry(path = "SMS/Message_Server")
    public String getMessageAddress() {
        return m_defaults.getFullyQualifiedDomainName();
    }
}
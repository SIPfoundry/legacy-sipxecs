/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.hitachi;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class HitachiPhoneDefaults {

    private final DeviceDefaults m_defaults;

    public HitachiPhoneDefaults(DeviceDefaults defaults) {
        m_defaults = defaults;
    }

    @SettingEntry(paths = {
            "SERVER_SETTINGS/1st_Proxy", "SERVER_SETTINGS/1st_Registrar"
            })
    public String getDomainName() {
        return m_defaults.getFullyQualifiedDomainName();
    }

    @SettingEntry(path = "SERVER_SETTINGS/Domain_Realm")
    public String getAuthorizationRealm() {
        return m_defaults.getAuthorizationRealm();
    }

    @SettingEntry(path = "SNTP/NTP_Server1")
    public String getNtpServer() {
        return m_defaults.getNtpServer();
    }

    @SettingEntry(path = "SNTP/NTP_Server2")
    public String getAlternateNtpServer() {
        return m_defaults.getAlternateNtpServer();
    }

    @SettingEntry(path = "SNTP/Time_Zone")
    public String getTimeZoneOffset() {
        int tzhrs = m_defaults.getTimeZone().getOffset() / 60;
        int tzmin = m_defaults.getTimeZone().getOffset() % 60;

        String time = String.format("%02d:%02d", tzhrs, tzmin);
        if (tzhrs <= 0) {
            return time;
        }
        return '+' + time;
    }

    @SettingEntry(path = "SNTP/DST_Start_Month")
    public String getDstStartMonth() {
        int stmth = m_defaults.getTimeZone().getStartMonth();
        return String.valueOf(stmth);
    }

    @SettingEntry(path = "SNTP/DST_Start_Hour")
    public String getDstStartHour() {
        int sthrs = m_defaults.getTimeZone().getStartTime() / 3600;
        return String.valueOf(sthrs);
    }

    @SettingEntry(path = "SNTP/DST_Start_Min")
    public String getDstStartMin() {
        int stmin = (m_defaults.getTimeZone().getStartTime() % 3600) / 60;
        return String.valueOf(stmin);
    }

    @SettingEntry(path = "SNTP/DST_End_Month")
    public String getDstEndMonth() {
        int stpmonth = m_defaults.getTimeZone().getStopMonth();
        return String.valueOf(stpmonth);
    }

    @SettingEntry(path = "SNTP/DST_End_Hour")
    public String getDstEndHour() {
        int stphrs = m_defaults.getTimeZone().getStopTime() / 3600;
        return String.valueOf(stphrs);
    }

    @SettingEntry(path = "SNTP/DST_End_Min")
    public String getDstEndMin() {
        int stpmin = (m_defaults.getTimeZone().getStopTime() % 3600) / 60;
        return String.valueOf(stpmin);
    }

    @SettingEntry(path = "SYSTEM/TFTP_Server_IP")
    public String getTftpServerIp() {
        return m_defaults.getTftpServer();
    }
}

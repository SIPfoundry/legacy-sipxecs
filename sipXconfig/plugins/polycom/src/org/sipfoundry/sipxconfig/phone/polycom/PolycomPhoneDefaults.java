/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.service.UnmanagedService;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class PolycomPhoneDefaults {
    private final DeviceDefaults m_defaults;
    private final SpeedDial m_speedDial;

    PolycomPhoneDefaults(DeviceDefaults defaults, SpeedDial speedDial) {
        m_defaults = defaults;
        m_speedDial = speedDial;
    }

    private DeviceTimeZone getZone() {
        return m_defaults.getTimeZone();
    }

    @SettingEntry(path = "tcpIpApp.sntp/gmtOffset")
    public long getTimeZoneGmtOffset() {
        return getZone().getOffsetInSeconds();
    }

    @SettingEntry(path = "tcpIpApp.sntp/address")
    public String getNtpServer() {
        return m_defaults.getNtpServer();
    }


    // The attributes that control Daylight Savings Time (DST) are explained here:
    //    http://sipx-wiki.calivia.com/index.php/
    //       Polycom_SoundPointIP_Configuration_File_Notes#sip_-_tcpIpApp.sntp.daylightSavings

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.enable")
    public boolean isDstEnabled() {
        return getZone().getUseDaylight();
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.time")
    public int getDstStartTime() {
        return getZone().getStartTime() / DeviceTimeZone.MINUTES_PER_HOUR;
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.time")
    public int getDstStopTime() {
        return getZone().getStopTime() / DeviceTimeZone.MINUTES_PER_HOUR;
    }

    public int getNthOccurencePolycomFromWeekIndex(int weekIndex) {
        // Assume fixedDayEnable=0, since DeviceTimeZone can't express anything else.
        return (Math.max(0, weekIndex - 1) * 7) + 1; // Results in: 1, 8, 15, or 22
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.date")
    public int getDstStartDate() {
        return getNthOccurencePolycomFromWeekIndex(getZone().getStartWeek());
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.date")
    public int getDstStopDate() {
        return getNthOccurencePolycomFromWeekIndex(getZone().getStopWeek());
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.dayOfWeek.lastInMonth")
    public boolean isDstStartDayOfWeekLastInMonth() {
        return getZone().getStartWeek() == DeviceTimeZone.DST_LASTWEEK;
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.dayOfWeek.lastInMonth")
    public boolean isDstStopDayOfWeekLastInMonth() {
        return getZone().getStopWeek() == DeviceTimeZone.DST_LASTWEEK;
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.dayOfWeek")
    public int getDstStartDayOfWeek() {
        // Polycom Day of Week mapping is the same as Java.
        return getZone().getStartDayOfWeek();
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.dayOfWeek")
    public int getDstStopDayOfWeek() {
        // Polycom Day of Week mapping is the same as Java.
        return getZone().getStopDayOfWeek();
    }

    public int getMonthIndexPolycomFromJava(int indexJava) {
        // Polycom Month mapping starts at 1, but Java starts at 0.
        return indexJava + 1;
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.month")
    public int getDstStartMonth() {
        return getMonthIndexPolycomFromJava(getZone().getStartMonth());
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.month")
    public int getDstStopMonth() {
        return getMonthIndexPolycomFromJava(getZone().getStopMonth());
    }

    @SettingEntry(path = "voIpProt/server/1/address")
    public String getRegistrationServer() {
        return m_defaults.getDomainName();
    }

    @SettingEntry(path = "attendant/uri")
    public String getAttendantUri() {
        if (m_speedDial != null && m_speedDial.isBlf()) {
            return SipUri.format(m_speedDial.getResourceListId(true), m_defaults.getDomainName(), false);
        }
        return null;
    }

    @SettingEntry(path = "attendant/reg")
    public String getAttendantReg() {
        if (m_speedDial != null && m_speedDial.isBlf()) {
            return "1";
        }
        return null;
    }

    @SettingEntry(path = "call/directedCallPickupString")
    public String getDirectedCallPickupString() {
        return m_defaults.getDirectedCallPickupCode();
    }

    @SettingEntry(path = "log/device.syslog/serverName")
    public String getSyslogServer() {
        return m_defaults.getServer(0, UnmanagedService.SYSLOG);
    }

}

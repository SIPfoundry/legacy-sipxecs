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
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

// XCF-668 Removed setting outbound proxy defaults
// So by default, polycom phones will attempt to send sip traffic to server
// it's registered with. Setting an outbound proxy to domain would be redundant
// therefore unnec. and could potentially cause issues.
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
    public long getGmtOffset() {
        return getZone().getOffsetInSeconds();
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.enable")
    public boolean isDstEnabled() {
        return getZone().getDstSavings() != 0;
    }

    @SettingEntry(path = "tcpIpApp.sntp/address")
    public String getNtpServer() {
        return m_defaults.getNtpServer();
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.dayOfWeek.lastInMonth")
    public boolean isStartLastInMonth() {
        if (!isDstEnabled()) {
            return false;
        }
        if (getZone().getStartWeek() == DeviceTimeZone.DST_LASTWEEK) {
            return true;
        }

        return false;
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.dayOfWeek.lastInMonth")
    public boolean isStopDayOfWeekLastInMonth() {
        if (!isDstEnabled()) {
            return false;
        }
        if (getZone().getStopWeek() == DeviceTimeZone.DST_LASTWEEK) {
            return true;
        }

        return false;
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.dayOfWeek")
    public int getStartDayOfWeek() {
        return isDstEnabled() ? dayOfWeek(getZone().getStartDayOfWeek()) : 0;
    }

    static int dayOfWeek(int dayOfWeek) {
        // Polycom uses the same mapping as Java.  (1=Sun, 2=Mon,..., 7=Sat)
        return dayOfWeek;
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.month")
    public int getStartMonth() {
        return isDstEnabled() ? getZone().getStartMonth() : 0;
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.time")
    public int getStartTime() {
        return isDstEnabled() ? getZone().getStartMonth() / 3600 : 0;
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.dayOfWeek")
    public int getStopDayOfWeek() {
        return isDstEnabled() ? dayOfWeek(getZone().getStopDayOfWeek()) : 0;
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.month")
    public int getStopMonth() {
        return isDstEnabled() ? getZone().getStopMonth() : 0;
    }

    @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.time")
    public int getStopTime() {
        return isDstEnabled() ? getZone().getStopTime() / 3600 : 0;
    }

    @SettingEntry(paths = { "voIpProt.SIP/protocol/musicOnHold.uri", "voIpProt/reg/musicOnHold.uri" })
    public String getMohUrl() {
        String mohUri = m_defaults.getMusicOnHoldUri(m_defaults.getDomainName());
        return SipUri.stripSipPrefix(mohUri);

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

}

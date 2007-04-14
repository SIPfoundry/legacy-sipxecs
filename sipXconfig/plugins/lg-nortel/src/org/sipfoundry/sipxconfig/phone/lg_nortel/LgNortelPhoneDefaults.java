/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.lg_nortel;

import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

/**
 * Default parameters for LgNortelPhone
 * 
 * The following parameters: outbound_proxy_server, outbound_proxy_port, SIP_service_domain are
 * intentionally *not* set here. Phones use per-line parameters to find proxy and set SIP domain.
 */
public class LgNortelPhoneDefaults {

    private DeviceDefaults m_defaults;
    private int m_lines;

    public LgNortelPhoneDefaults(DeviceDefaults defaults, int lines) {
        m_defaults = defaults;
        m_lines = lines;
    }

    private DeviceTimeZone getZone() {
        return m_defaults.getTimeZone();
    }

    @SettingEntry(path = "VOIP/max_line_num")
    public int getMaxLineNum() {
        return m_lines;
    }

    @SettingEntry(path = "VOIP/moh_url")
    public String getMohUrl() {
        String mohUri = m_defaults.getSipxServer().getMusicOnHoldUri(m_defaults.getDomainName());
        return SipUri.stripSipPrefix(mohUri);
    }

    @SettingEntry(path = "NETTIME/sntp_server_address")
    public String getNtpServer() {
        return m_defaults.getNtpServer();
    }

    @SettingEntry(path = "NETTIME/timezone")
    public int getTimezone() {
        // FIXME: need to translate into LG/Nortel timezone ID
        return getZone().getOffsetInHours();
    }
    
    @SettingEntry(path = "NETTIME/dst_auto_adjust")
    public boolean getUseDst() {
        return getZone().isUsingDaylightTime();
    }
    

    @SettingEntry(path = "NETTIME/dst_start_month")
    public Integer getStartMonth() {
        return nullUnlessDst(getZone().getStartMonth());
    }
    
    private Integer nullUnlessDst(int i) {
        return (getUseDst() ? i : null);
    }

    @SettingEntry(path = "NETTIME/dst_start_day")
    public Integer getStartDay() {
        return nullUnlessDst(getZone().getStartDay());
    }

    @SettingEntry(path = "NETTIME/dst_start_day_of_week")
    public Integer getStartDayOfWeek() {
        return nullUnlessDst(getZone().getStartDayOfWeek());
    }

    @SettingEntry(path = "NETTIME/dst_start_week_of_month")
    public Integer getStartWeekOfMonth() {
        int week = getZone().getStartWeek();
        return nullUnlessDst(adjustWeekOfMonth(week));
    }

    @SettingEntry(path = "NETTIME/dst_start_time")
    public Integer getStartTime() {
        return nullUnlessDst(getZone().getStartTimeInHours());
    }

    @SettingEntry(path = "NETTIME/dst_stop_month")
    public Integer getStopMonth() {
        return nullUnlessDst(getZone().getStopMonth());
    }

    @SettingEntry(path = "NETTIME/dst_stop_day")
    public Integer getStopDay() {
        return nullUnlessDst(getZone().getStopDay());
    }

    @SettingEntry(path = "NETTIME/dst_stop_day_of_week")
    public Integer getStopDayOfWeek() {
        return nullUnlessDst(getZone().getStopDayOfWeek());
    }

    @SettingEntry(path = "NETTIME/dst_stop_week_of_month")
    public Integer getStopWeekOfMonth() {
        int week = getZone().getStopWeek();
        return nullUnlessDst(adjustWeekOfMonth(week));
    }

    @SettingEntry(path = "NETTIME/dst_stop_time")
    public Integer getStopTime() {
        return nullUnlessDst(getZone().getStopTimeInHours());
    }

    @SettingEntry(path = "VOIP/message_url")
    public String getVoicemailExtension() {        
        return m_defaults.getVoiceMail();
    }
    
    /**
     * Adjusts week of month value for LG phones.
     * 
     * In case of LG phones '7' means last month of the week.
     */
    private int adjustWeekOfMonth(int week) {
        return week != DeviceTimeZone.DST_LASTWEEK ? week : 7;
    }
}

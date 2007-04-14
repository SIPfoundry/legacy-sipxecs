/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.util.Calendar;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

import org.apache.commons.lang.time.DateUtils;

public class DeviceTimeZone {

    public static final String DST_US = "US";

    public static final String DST_EU = "EU";

    public static final int DST_LASTWEEK = 8;

    private static final int SECONDS_PER_HOUR = 3600;

    private static final int DST_STARTDAY = 0;

    private static final int DST_STARTDAYOFWEEK = 1;

    private static final int DST_STARTMONTH = 2;

    private static final int DST_STARTTIME = 3;

    private static final int DST_STARTWEEK = 4;

    private static final int DST_STOPDAY = 5;

    private static final int DST_STOPDAYOFWEEK = 6;

    private static final int DST_STOPMONTH = 7;

    private static final int DST_STOPTIME = 8;

    private static final int DST_STOPWEEK = 9;

    private static final int DST_SUNDAY = 1;

    private static final int DST_MARCH = 3;

    private static final int DST_APRIL = 4;

    private static final int DST_OCTOBER = 10;

    private static final int DST_NOVEMBER = 11;

    private String m_dstrule;

    private TimeZone m_tz;

    private int[] m_dstparam = new int[] {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    public DeviceTimeZone() {
        this(null);
    }

    public DeviceTimeZone(String timezone) {
        if (timezone == null) {
            m_tz = TimeZone.getDefault();
        } else {
            m_tz = TimeZone.getTimeZone(timezone);
        }
        String tzn = m_tz.getID();

        // Until there is a setting for DST rule, it must be guessed here based on the timezone
        // name
        //
        // XCF-977 - Doing string compares on ID is not predicable! On my gentoo system, I get
        // US/Eastern but on a FC4 machine I get America/New_York. Both running java1.5.0.06
        // We'll wait for XCF-874 to address this properly. Until then, do not break whatever
        // seems to work for some systems in Europe, but default everyone else to US. Otherwise
        // the default of zero is not very helpful
        if (tzn.matches("^Europe/.*")) {
            setDstRule(DST_EU);
        } else {
            setDstRule(DST_US);
        }
    }

    public void setDstRule(String dstname) {
        m_dstrule = dstname;
        setDstParameters();
    }
    
    public boolean isUsingDaylightTime() {
        return m_tz.useDaylightTime();
    }

    private void setDstParameters() {
        if (isUsingDaylightTime()) {
            if (m_dstrule.equals(DST_US)) {
                m_dstparam[DST_STARTDAY] = 0;
                m_dstparam[DST_STARTDAYOFWEEK] = DST_SUNDAY;
                m_dstparam[DST_STARTTIME] = 2 * SECONDS_PER_HOUR;
                m_dstparam[DST_STOPDAY] = 0;
                m_dstparam[DST_STOPDAYOFWEEK] = DST_SUNDAY;
                m_dstparam[DST_STOPTIME] = 2 * SECONDS_PER_HOUR;

                // according to http://webexhibits.org/daylightsaving/b.html US DST rule changes
                // in 2007
                // this code will break current phone unit tests in 2007

                if (Calendar.getInstance().get(Calendar.YEAR) > 2006) {
                    m_dstparam[DST_STARTMONTH] = DST_MARCH;
                    m_dstparam[DST_STARTWEEK] = 2;
                    m_dstparam[DST_STOPMONTH] = DST_NOVEMBER;
                    m_dstparam[DST_STOPWEEK] = 1;
                } else {
                    m_dstparam[DST_STARTMONTH] = DST_APRIL;
                    m_dstparam[DST_STARTWEEK] = 1;
                    m_dstparam[DST_STOPMONTH] = DST_OCTOBER;
                    m_dstparam[DST_STOPWEEK] = DST_LASTWEEK;
                }

                return;
            }

            if (m_dstrule.equals(DST_EU)) {
                m_dstparam[DST_STARTDAY] = 0;
                m_dstparam[DST_STARTDAYOFWEEK] = DST_SUNDAY;
                m_dstparam[DST_STARTMONTH] = DST_MARCH;
                m_dstparam[DST_STARTTIME] = SECONDS_PER_HOUR + getOffset();
                m_dstparam[DST_STARTWEEK] = DST_LASTWEEK;
                m_dstparam[DST_STOPDAY] = 0;
                m_dstparam[DST_STOPDAYOFWEEK] = DST_SUNDAY;
                m_dstparam[DST_STOPMONTH] = DST_OCTOBER;
                m_dstparam[DST_STOPTIME] = SECONDS_PER_HOUR + getOffset() + getDstOffset();
                m_dstparam[DST_STOPWEEK] = DST_LASTWEEK;
                return;
            }
        }

        m_dstparam = new int[] {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        };
    }

    public boolean getDstActive() {
        return m_tz.inDaylightTime(new Date());

    }

    public int getOffset() {
        return m_tz.getRawOffset() / (int) DateUtils.MILLIS_PER_SECOND;

    }

    public int getOffsetInHours() {
        return m_tz.getRawOffset() / (int) DateUtils.MILLIS_PER_HOUR;
    }

    public int getDstOffset() {
        return m_tz.getDSTSavings() / (int) DateUtils.MILLIS_PER_SECOND;
    }

    public int getOffsetWithDst() {
        int offset = getOffset();
        if (getDstActive()) {
            offset += getDstOffset();
        }
        return offset;

    }

    public String getShortName() {
        return m_tz.getDisplayName(false, TimeZone.SHORT, Locale.US);

    }

    public int getStartDay() {
        return m_dstparam[DST_STARTDAY];
    }

    public int getStartDayOfWeek() {
        return m_dstparam[DST_STARTDAYOFWEEK];
    }

    public int getStartMonth() {
        return m_dstparam[DST_STARTMONTH];
    }

    public int getStartTime() {
        return m_dstparam[DST_STARTTIME];
    }

    public int getStartTimeInHours() {
        return m_dstparam[DST_STARTTIME] / SECONDS_PER_HOUR;
    }

    public int getStartWeek() {
        return m_dstparam[DST_STARTWEEK];
    }

    public int getStopDay() {
        return m_dstparam[DST_STOPDAY];
    }

    public int getStopDayOfWeek() {
        return m_dstparam[DST_STOPDAYOFWEEK];
    }

    public int getStopMonth() {
        return m_dstparam[DST_STOPMONTH];
    }

    public int getStopTime() {
        return m_dstparam[DST_STOPTIME];
    }

    public int getStopTimeInHours() {
        return m_dstparam[DST_STARTTIME] / SECONDS_PER_HOUR;
    }

    public int getStopWeek() {
        return m_dstparam[DST_STOPWEEK];
    }
}

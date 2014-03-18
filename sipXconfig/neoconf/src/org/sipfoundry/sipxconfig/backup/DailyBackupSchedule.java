/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.backup;

import java.sql.Timestamp;
import java.text.DateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.codehaus.jackson.annotate.JsonIgnore;
import org.codehaus.jackson.annotate.JsonPropertyOrder;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.CronSchedule;
import org.sipfoundry.sipxconfig.common.CronSchedule.Type;
import org.sipfoundry.sipxconfig.common.ScheduledDay;
import org.sipfoundry.sipxconfig.common.TimeOfDay;

@JsonPropertyOrder(alphabetic = true)
public class DailyBackupSchedule extends BeanWithId {

    public static final DateFormat GMT_TIME_OF_DAY_FORMAT = DateFormat
            .getTimeInstance(DateFormat.SHORT);

    public static final DateFormat LOCAL_TIME_OF_DAY_FORMAT = DateFormat
            .getTimeInstance(DateFormat.SHORT);

    public static final TimeZone GMT = TimeZone.getTimeZone("GMT");

    static final long ONCE_A_DAY = 1000 * 60 * 60 * 24;

    static final int DAYS_PER_WEEK = 7;

    static final long ONCE_A_WEEK = ONCE_A_DAY * DAYS_PER_WEEK;

    private static final Log LOG = LogFactory.getLog(BackupPlan.class);

    private boolean m_enabled;

    private Date m_time = new Timestamp(0);

    private ScheduledDay m_day = ScheduledDay.EVERYDAY;

    private BackupPlan m_backupPlan;

    private boolean m_allowStaleDate;     // for testing only

    static {
        // Storing dates in GMT keeps times consistent if timezones change
        // disadvantage, conversions of date outside this converter appear
        // wrong unless you live in grenwich
        GMT_TIME_OF_DAY_FORMAT.setTimeZone(GMT);
    }

    @JsonIgnore
    public BackupPlan getBackupPlan() {
        return m_backupPlan;
    }

    public void setBackupPlan(BackupPlan backupPlan) {
        m_backupPlan = backupPlan;
    }

    public String toCronString() {
        CronSchedule cron = new CronSchedule();
        ScheduledDay scheduledDay = getScheduledDay();
        cron.setScheduledDay(scheduledDay);
        //By default 'cron' type is Type.DAILY.
        //But if you don't want this everyday, then they type should be changed to WEEKLY
        if (scheduledDay != ScheduledDay.EVERYDAY) {
            cron.setType(Type.WEEKLY);
        }
        cron.setTimeOfDay(getTimeOfDay());
        return cron.getUnixCronString();
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public ScheduledDay getScheduledDay() {
        return m_day;
    }

    public void setScheduledDay(ScheduledDay day) {
        m_day = day;
    }

    public TimeOfDay getTimeOfDay() {
        return new TimeOfDay(getTime(), GMT);
    }

    @JsonIgnore
    public Date getTime() {
        return m_time;
    }

    public void setTimeOfDay(TimeOfDay timeOfDay) {
        Calendar cal = Calendar.getInstance(GMT);
        cal.setTimeInMillis(0); // clear date information
        cal.set(Calendar.HOUR_OF_DAY, timeOfDay.getHrs());
        cal.set(Calendar.MINUTE, timeOfDay.getMin());
        setTime(cal.getTime());
    }

    public void setTime(Date timeOfDay) {
        m_time = timeOfDay;
    }

    // for testing only
    void setAllowStaleDate(boolean allowStaleDate) {
        m_allowStaleDate = allowStaleDate;
    }

    long getTimerPeriod() {
        return (m_day == ScheduledDay.EVERYDAY ? ONCE_A_DAY : ONCE_A_WEEK);
    }
}

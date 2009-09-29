/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.sql.Timestamp;
import java.text.DateFormat;
import java.text.ParseException;
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;
import java.util.Timer;
import java.util.TimerTask;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.BeanWithId;

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

    public BackupPlan getBackupPlan() {
        return m_backupPlan;
    }

    public void setBackupPlan(BackupPlan backupPlan) {
        m_backupPlan = backupPlan;
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

    Date getTimerDate() {

        // convert thru string because setting timezone just shift time and
        // we want effectively the same time.
        String gmtTimeOfDay = GMT_TIME_OF_DAY_FORMAT.format(getTime());
        try {
            Date localTimeOfDay = LOCAL_TIME_OF_DAY_FORMAT.parse(gmtTimeOfDay);
            Calendar localCal = Calendar.getInstance();
            localCal.setTime(localTimeOfDay);

            Calendar when = Calendar.getInstance();
            when.set(Calendar.HOUR_OF_DAY, localCal.get(Calendar.HOUR_OF_DAY));
            when.set(Calendar.MINUTE, localCal.get(Calendar.MINUTE));
            when.set(Calendar.SECOND, 0);
            when.set(Calendar.MILLISECOND, 0);

            if (m_day != ScheduledDay.EVERYDAY) {
                when.set(Calendar.DAY_OF_WEEK, getScheduledDay().getDayOfWeek());
            }

            // Ensure that the scheduled time is in the future, not the past.
            // Otherwise the timer will fire immediately, not at the desired time.
            // (The allowStaleDate flag suppresses this behavior, only for testing purposes.)
            Date timerDate = when.getTime();
            if (timerDate.getTime() < System.currentTimeMillis() && !m_allowStaleDate) {
                // The scheduled time is in the past.  Add one timer period to push it into the future.
                timerDate.setTime(timerDate.getTime() + getTimerPeriod());
            }

            return timerDate;
        } catch (ParseException e) {
            throw new RuntimeException(e);
        }
    }

    long getTimerPeriod() {
        return (m_day == ScheduledDay.EVERYDAY ? ONCE_A_DAY : ONCE_A_WEEK);
    }

    public void schedule(Timer timer, TimerTask task) {
        if (!isEnabled()) {
            return;
        }

        Date date = getTimerDate();
        long period = getTimerPeriod();
        LOG.info("Setting timer for " + LOCAL_TIME_OF_DAY_FORMAT.format(date));
        timer.schedule(task, date, period);
    }
}

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

import java.util.Calendar;
import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.time.DateUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * This is a schedule that keeps its format as a cron compatible string.
 *
 * The format of the string is: "seconds(0-59) minutes(0-59) hours(0-23) day-of-month(1-32)
 * month(1-12) day-of-week(1-7)"
 *
 * It can be used with Java timer class,
 */
public class CronSchedule extends BeanWithId {

    public static enum Type {
        WEEKLY {
            long getDelay() {
                return 7 * DateUtils.MILLIS_PER_DAY;
            }
        },
        DAILY {
            long getDelay() {
                return DateUtils.MILLIS_PER_DAY;
            }
        },
        HOURLY {
            long getDelay() {
                return DateUtils.MILLIS_PER_HOUR;
            }
        };

        abstract long getDelay();
    }

    private static final String ANY = "*";
    private static final int FIELD_MIN = 1;
    private static final int FIELD_HRS = 2;
    private static final int FIELD_DOF = 5;

    private Type m_type = Type.DAILY;

    private int m_dayOfWeek = Calendar.MONDAY;

    private int m_hrs;

    private int m_min;

    private boolean m_enabled;

    public void setHrs(int hrs) {
        m_hrs = hrs;
    }

    public int getHrs() {
        return m_hrs;
    }

    public void setMin(int min) {
        m_min = min;
    }

    public int getMin() {
        return m_min;
    }

    public void setType(Type type) {
        m_type = type;
    }

    public Type getType() {
        return m_type;
    }

    public void setDayOfWeek(int dayOfWeek) {
        m_dayOfWeek = dayOfWeek;
    }

    public int getDayOfWeek() {
        return m_dayOfWeek;
    }

    public ScheduledDay getScheduledDay() {
        return ScheduledDay.getScheduledDay(m_dayOfWeek);
    }

    public void setScheduledDay(ScheduledDay scheduledDay) {
        m_dayOfWeek = scheduledDay.getDayOfWeek();
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public String getCronString() {
        switch (m_type) {
        case WEEKLY:
            return String.format("0 %d %d ? * %d", m_min, m_hrs, m_dayOfWeek);
        case DAILY:
            return String.format("0 %d %d ? * *", m_min, m_hrs);
        case HOURLY:
            return String.format("0 %d * ? * *", m_min);
        default:
            throw new IllegalStateException("CronSchedule not initialized correctly.");
        }
    }

    public void setCronString(String cronString) {
        String[] fields = StringUtils.split(cronString, " ");
        if (fields.length < 6) {
            throw new IllegalArgumentException("cronString has to have at least 6 fields");
        }

        String minString = fields[FIELD_MIN];
        String hrsString = fields[FIELD_HRS];
        String dowString = fields[FIELD_DOF];

        if (ANY.equals(dowString)) {
            setType(Type.DAILY);
            setScheduledDay(ScheduledDay.EVERYDAY);
        } else {
            setType(Type.WEEKLY);
            setDayOfWeek(Integer.parseInt(dowString));
        }
        if (ANY.equals(hrsString)) {
            setHrs(0);
            setType(Type.HOURLY);
        } else {
            setHrs(Integer.parseInt(hrsString));
        }
        setMin(Integer.parseInt(minString));
    }

    public TimeOfDay getTimeOfDay() {
        return new TimeOfDay(m_hrs, m_min);
    }

    public void setTimeOfDay(TimeOfDay timeOfDay) {
        m_hrs = timeOfDay.getHrs();
        m_min = timeOfDay.getMin();
    }

    Date getFirstDate() {
        Calendar firstDate = Calendar.getInstance();
        if (isWeekly()) {
            firstDate.set(Calendar.DAY_OF_WEEK, m_dayOfWeek);
        }
        if (isWeekly() || isDaily()) {
            firstDate.set(Calendar.HOUR_OF_DAY, m_hrs);
        }
        firstDate.set(Calendar.MINUTE, m_min);
        // make sure it's in the future
        if (firstDate.before(Calendar.getInstance())) {
            firstDate.add(Calendar.MILLISECOND, (int) getDelay());
        }
        firstDate = DateUtils.round(firstDate, Calendar.MINUTE);
        return firstDate.getTime();
    }

    long getDelay() {
        return m_type.getDelay();
    }

    public Timer schedule(TimerTask task) {
        if (m_enabled) {
            Timer timer = new Timer();
            timer.schedule(task, getFirstDate(), getDelay());
            return timer;
        }
        return null;
    }

    // to simplify OGNL expression
    public boolean isWeekly() {
        return m_type == Type.WEEKLY;
    }

    public boolean isDaily() {
        return m_type == Type.DAILY;
    }

    public boolean isHourly() {
        return m_type == Type.HOURLY;
    }
}

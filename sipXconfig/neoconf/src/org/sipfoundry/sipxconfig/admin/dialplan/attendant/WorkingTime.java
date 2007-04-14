/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

import org.sipfoundry.sipxconfig.admin.ScheduledDay;

public class WorkingTime extends ScheduledAttendant {
    private WorkingHours[] m_workingHours;

    /**
     * Initialization is a bit tricky - days here are numbered from 0 to 6, with - 0 being Monday and 6 being Sunday.
     * Days in getScheduleDay and in Calenar object are number from 1 to 7 with 1 being Sunday and 7 being Saturday.
     *
     */
    public WorkingTime() {
        final int days = ScheduledDay.DAYS_OF_WEEK.length;
        final int lastWorkingDay = Calendar.FRIDAY - Calendar.MONDAY;
        m_workingHours = new WorkingHours[days];
        for (int i = 0; i < days; i++) {
            WorkingHours whs = new WorkingHours();
            int dayOfWeek = (i + Calendar.SUNDAY) % days + 1;
            whs.setDay(ScheduledDay.getScheduledDay(dayOfWeek));
            whs.setEnabled(i <= lastWorkingDay);
            m_workingHours[i] = whs;
        }
    }

    public WorkingHours[] getWorkingHours() {
        return m_workingHours;
    }

    public void setWorkingHours(WorkingHours[] workingHours) {
        m_workingHours = workingHours;
    }

    public Object clone() throws CloneNotSupportedException {
        WorkingTime clone = (WorkingTime) super.clone();
        clone.m_workingHours = m_workingHours.clone();
        return clone;
    }

    public static class WorkingHours {
        public static final int DEFAULT_START = 9;
        public static final int DEFAULT_STOP = 18;

        public static final DateFormat TIME_FORMAT = new SimpleDateFormat("HH:mm", Locale.US);

        static {
            TIME_FORMAT.setTimeZone(getGmtTimeZone());
        }

        private boolean m_enabled;
        private ScheduledDay m_day;
        private Date m_start;
        private Date m_stop;

        public WorkingHours() {
            Calendar calendar = Calendar.getInstance(getGmtTimeZone());
            calendar.set(Calendar.HOUR_OF_DAY, DEFAULT_START);
            calendar.set(Calendar.MINUTE, 0);
            m_start = calendar.getTime();
            calendar.set(Calendar.HOUR_OF_DAY, DEFAULT_STOP);
            m_stop = calendar.getTime();
        }

        public void setEnabled(boolean enabled) {
            m_enabled = enabled;
        }

        public boolean isEnabled() {
            return m_enabled;
        }

        public void setDay(ScheduledDay day) {
            m_day = day;
        }

        public ScheduledDay getDay() {
            return m_day;
        }

        public String getStartTime() {
            return formatTime(m_start);
        }

        public String getStopTime() {
            return formatTime(m_stop);
        }

        /**
         * @return UTC time
         */
        public Date getStart() {
            return m_start;
        }

        /**
         * @param start UTC time
         */
        public void setStart(Date start) {
            m_start = start;
        }

        public Date getStop() {
            return m_stop;
        }

        public void setStop(Date stop) {
            m_stop = stop;
        }

        /**
         * Formats time as 24 hours (00:00-23:59), GMT time.
         * 
         * @param date date to format - needs to be GMT time zon
         * @return formatted time string
         */
        private static String formatTime(Date date) {
            return TIME_FORMAT.format(date);
        }

        private static TimeZone getGmtTimeZone() {
            return TimeZone.getTimeZone("GMT");
        }
    }
}

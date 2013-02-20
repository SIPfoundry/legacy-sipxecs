/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan.attendant;

import java.io.Serializable;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.TimeZone;

import org.joda.time.DateTime;
import org.joda.time.DateTimeZone;
import org.sipfoundry.sipxconfig.common.ScheduledDay;
import org.sipfoundry.sipxconfig.common.TimeOfDay;
import org.sipfoundry.sipxconfig.common.UserException;

public class WorkingTime extends ScheduledAttendant {
    private WorkingHours[] m_workingHours;

    /**
     * Initialization is a bit tricky - days here are numbered from 0 to 6, with - 0 being Monday
     * and 6 being Sunday. Days in getScheduleDay and in Calendar object are number from 1 to 7
     * with 1 being Sunday and 7 being Saturday.
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

    public List<Interval> calculateValidTime(TimeZone timeZone) {
        int timeZoneOffsetInMinutes = DateTimeZone.forTimeZone(timeZone).getOffset(
                new DateTime(DateTimeZone.forTimeZone(timeZone)).getMillis()) / 1000 / 60;
        return calculateValidTimes(timeZoneOffsetInMinutes);
    }

    private List<Interval> calculateValidTimes(int timeZoneOffsetInMinutes) {
        WorkingHours[] workingHours = getWorkingHours();
        List<Interval> validTimeList = new ArrayList<Interval>();
        for (WorkingHours wk : workingHours) {
            wk.addMinutesFromSunday(validTimeList, timeZoneOffsetInMinutes);
        }
        return validTimeList;
    }

    public void checkValid() {
        for (WorkingHours workingHour : m_workingHours) {
            if (workingHour.isInvalidPeriod()) {
                throw new InvalidPeriodException();
            }
            if (workingHour.isTheSameHour()) {
                throw new SameStartAndStopHoursException();
            }
        }
        if (overlappingPeriods()) {
            throw new OverlappingPeriodsException();
        }
    }

    public boolean overlappingPeriods() {
        List<Interval> intervals = calculateValidTimes(0);
        Collections.sort(intervals);
        if (intervals.size() < 2) {
            return false;
        }

        int lastStop = intervals.get(0).getStop();
        for (int i = 1; i < intervals.size(); i++) {
            Interval interval = intervals.get(i);
            if (interval.getStart() < lastStop) {
                return true;
            }
            lastStop = interval.getStop();
        }
        return false;
    }

    /**
     * Represents interval (in minutes from beginning of the week).
     */
    public static class Interval implements Comparable<Interval> {
        private int m_start;
        private int m_stop;

        public Interval(int start, int stop) {
            m_start = start;
            m_stop = stop;
        }

        public int getStart() {
            return m_start;
        }

        public int getStop() {
            return m_stop;
        }

        public int compareTo(Interval interval) {
            return m_start - interval.m_start;
        }
    }

    public static class WorkingHours implements Serializable {
        public static final int MINUTES_PER_HOUR = 60;
        public static final int MINUTES_PER_DAY = MINUTES_PER_HOUR * 24;
        public static final int MINUTES_PER_WEEK = MINUTES_PER_DAY * 7;
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

        public TimeOfDay getStartTimeOfDay() {
            return new TimeOfDay(getStart(), getGmtTimeZone());
        }

        /**
         * @param start UTC time
         */
        public void setStart(Date start) {
            m_start = start;
        }

        public void setStartTimeOfDay(TimeOfDay startTimeOfDay) {
            Calendar cal = Calendar.getInstance(getGmtTimeZone());
            cal.setTimeInMillis(0); // clear date information
            cal.set(Calendar.HOUR_OF_DAY, startTimeOfDay.getHrs());
            cal.set(Calendar.MINUTE, startTimeOfDay.getMin());
            setStart(cal.getTime());
        }

        public Date getStop() {
            return m_stop;
        }

        public TimeOfDay getStopTimeOfDay() {
            return new TimeOfDay(getStop(), getGmtTimeZone());
        }

        public void setStop(Date stop) {
            m_stop = stop;
        }

        public void setStopTimeOfDay(TimeOfDay stopTimeOfDay) {
            Calendar cal = Calendar.getInstance(getGmtTimeZone());
            cal.setTimeInMillis(0); // clear date information
            cal.set(Calendar.HOUR_OF_DAY, stopTimeOfDay.getHrs());
            cal.set(Calendar.MINUTE, stopTimeOfDay.getMin());
            setStop(cal.getTime());
        }

        /**
         * Formats time as 24 hours (00:00-23:59), GMT time.
         *
         * @param date date to format - needs to be GMT time zone
         * @return formatted time string
         */
        private static String formatTime(Date date) {
            return TIME_FORMAT.format(date);
        }

        private static TimeZone getGmtTimeZone() {
            return TimeZone.getTimeZone("GMT");
        }

        public static int getHour(Date date) {
            Calendar cal = Calendar.getInstance(getGmtTimeZone());
            cal.setTime(date);
            return cal.get(Calendar.HOUR_OF_DAY);
        }

        public static int getMinute(Date date) {
            Calendar cal = Calendar.getInstance(getGmtTimeZone());
            cal.setTime(date);
            return cal.get(Calendar.MINUTE);
        }

        public boolean isInvalidPeriod() {
            if (getStart().after(getStop())) {
                return true;
            }
            return false;
        }

        public boolean isTheSameHour() {
            if (getStart().getTime() == getStop().getTime()) {
                return true;
            }
            return false;
        }

        private int getMinutesFromMidnight(Date date) {
            return getHour(date) * MINUTES_PER_HOUR + getMinute(date);
        }

        /**
         * Calculates minute intervals that correspond to this working hours object and adds them
         * to minutes list.
         *
         * @param minutes - list of minutes that all intervals need to be added to
         * @param tzOffsetInMinutes - current time zone offset expressed in minutes
         */
        public void addMinutesFromSunday(List<Interval> minutes, int tzOffsetInMinutes) {
            ScheduledDay day = getDay();
            if (day.equals(ScheduledDay.EVERYDAY)) {
                addMinutesFromSunday(minutes, tzOffsetInMinutes, ScheduledDay.DAYS_OF_WEEK);
            } else if (day.equals(ScheduledDay.WEEKDAYS)) {
                addMinutesFromSunday(minutes, tzOffsetInMinutes, ScheduledDay.WEEK_DAYS);
            } else if (day.equals(ScheduledDay.WEEKEND)) {
                addMinutesFromSunday(minutes, tzOffsetInMinutes, ScheduledDay.WEEKEND_DAYS);
            } else {
                addMinutesFromSunday(minutes, tzOffsetInMinutes, day);
            }
        }

        private void addMinutesFromSunday(List<Interval> minutes, int tzOffsetInMinutes, ScheduledDay... days) {
            for (ScheduledDay dow : days) {
                List<Interval> intervals = calculateMinutesFromSunday(dow, tzOffsetInMinutes);
                minutes.addAll(intervals);
            }
        }

        private List<Interval> calculateMinutesFromSunday(ScheduledDay day, int tzOffsetInMinutes) {
            // days here are numbered from 1 to 7 with 1 being Sunday and 7 being Saturday
            int minutesFromSunday = (day.getDayOfWeek() - 1) * MINUTES_PER_DAY;

            int offset = tzOffsetInMinutes;
            if (offset < 0) {
                offset += MINUTES_PER_WEEK;
            }
            minutesFromSunday -= offset;

            List<Interval> intervals = new ArrayList<Interval>();
            int start = minutesFromSunday + getMinutesFromMidnight(m_start);
            if (start < 0) {
                start += MINUTES_PER_WEEK;
            }

            int stop = minutesFromSunday + getMinutesFromMidnight(m_stop);
            if (stop < 0) {
                stop += MINUTES_PER_WEEK;
            }

            if (start <= stop) {
                intervals.add(new Interval(start, stop));
            } else {
                intervals.add(new Interval(start, MINUTES_PER_WEEK));
                intervals.add(new Interval(0, stop));
            }
            return intervals;
        }
    }

    public static class OverlappingPeriodsException extends UserException {
        private static final String ERROR = "&error.overlappingPeriodsException";

        public OverlappingPeriodsException() {
            super(ERROR);
        }
    }

    public static class InvalidPeriodException extends UserException {
        private static final String ERROR = "&error.invalidPeriodException";

        public InvalidPeriodException() {
            super(ERROR);
        }
    }

    public static class SameStartAndStopHoursException extends UserException {
        private static final String ERROR = "&error.sameStartAndStopHoursException";

        public SameStartAndStopHoursException() {
            super(ERROR);
        }
    }
}
